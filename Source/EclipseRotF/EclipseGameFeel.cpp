// Eclipse Game Feel Director — Implementation
// AAA-quality movement, combat, and responsiveness systems.
// Per Directive #2: hit-stop, camera impulse, input buffering, combo windows.
// "The small details players feel subconsciously."
//
// Systems implemented:
//   - Movement speed / turn-rate scaling (combat vs. sprint)
//   - Dodge i-frames (start/end window within the dodge)
//   - Parry windows (normal + perfect parry timing)
//   - Hit-stop (freeze-frame scaling by hit weight)
//   - Camera shake intensity tiers
//   - Input buffering (0.15s window — hold-and-consume)
//   - Combo chaining (0.20s window, max 5 chain)
//   - Animation commitment (no cancel past commit threshold)
//   - Slow-motion (perfect-dodge 0.3x, critical 0.4x time dilation)
//   - Stamina management (delayed regen, punished regen when depleted)
//   - Poise / stagger (threshold, recovery, regen)
//   - Tuning presets + full debugging report

#include "EclipseGameFeel.h"

DEFINE_LOG_CATEGORY_STATIC(LogEclipseGameFeel, Log, All);

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

UEclipseGameFeel::UEclipseGameFeel()
{
    // Sync runtime state to the default profile caps so a custom
    // SetProfile() later stays consistent if nothing else touches them.
    CurrentStamina    = Profile.MaxStamina;
    CurrentPoise      = Profile.BaseStaggerThreshold;
    StaminaRegenTimer = 0.0f;
    PoiseRegenTimer   = 0.0f;
    bStaggered        = false;
    HitStopRemaining  = 0.0f;
    TimeDilation      = 1.0f;
    SlowMoRemaining   = 0.0f;
    BufferedInput     = TEXT("");
    BufferTimer       = 0.0f;
    ComboStep         = 0;
    ComboTimer        = 0.0f;
}

// ---------------------------------------------------------------------------
// Profile accessors
// ---------------------------------------------------------------------------

FGameFeelProfile UEclipseGameFeel::GetProfile() const
{
    return Profile;
}

void UEclipseGameFeel::SetProfile(const FGameFeelProfile& InProfile)
{
    Profile = InProfile;
    // Re-clamp live state to the new caps so we never carry an
    // over-cap value from a looser previous profile.
    CurrentStamina = FMath::Min(CurrentStamina, Profile.MaxStamina);
    CurrentPoise   = FMath::Min(CurrentPoise,   Profile.BaseStaggerThreshold);
}

// ---------------------------------------------------------------------------
// Movement
// ---------------------------------------------------------------------------

float UEclipseGameFeel::GetMoveSpeed(bool bSprinting, bool bInCombat) const
{
    // Sprint overrides everything — combat state only matters when walking.
    if (bSprinting)
    {
        return Profile.SprintSpeed;
    }
    // Combat walk is the tuned base locomotion speed; out-of-combat uses
    // the same value (the "feel" difference is delivered via turn-rate
    // and accel/decel, not a separate walk speed in this profile).
    return Profile.CombatWalkSpeed;
}

float UEclipseGameFeel::GetTurnRate(bool bInCombat) const
{
    // Locked-on combat feels snappier — faster rotational response.
    return bInCombat ? Profile.CombatTurnRate : Profile.TurnRate;
}

// ---------------------------------------------------------------------------
// Dodge
// ---------------------------------------------------------------------------

bool UEclipseGameFeel::CanDodge(float CurrentStaminaIn) const
{
    // Need enough stamina AND not currently mid-dodge cooldown is the
    // caller's responsibility; here we only gate on resource + a
    // non-trivial stamina floor.
    return CurrentStaminaIn >= Profile.DodgeStaminaCost && Profile.DodgeStaminaCost > 0.0f;
}

float UEclipseGameFeel::GetDodgeIFrameStart() const
{
    return Profile.DodgeIFrameStart;
}

float UEclipseGameFeel::GetDodgeIFrameEnd() const
{
    return Profile.DodgeIFrameEnd;
}

bool UEclipseGameFeel::IsInIFrame(float ElapsedSinceDodge) const
{
    // Invulnerable only inside the [Start, End] slice of the dodge.
    // Anything before the startup or after the recovery is hittable.
    return ElapsedSinceDodge >= Profile.DodgeIFrameStart
        && ElapsedSinceDodge <= Profile.DodgeIFrameEnd;
}

// ---------------------------------------------------------------------------
// Parry
// ---------------------------------------------------------------------------

bool UEclipseGameFeel::CanParry(float CurrentStaminaIn) const
{
    return CurrentStaminaIn >= Profile.ParryStaminaCost && Profile.ParryStaminaCost > 0.0f;
}

bool UEclipseGameFeel::IsPerfectParry(float ElapsedSinceParryStart) const
{
    // A perfect parry is the opening slice of the normal parry window.
    // Must be within the perfect window AND still within the full window.
    return ElapsedSinceParryStart >= 0.0f
        && ElapsedSinceParryStart <= Profile.PerfectParryWindow
        && ElapsedSinceParryStart <= Profile.ParryWindow;
}

// ---------------------------------------------------------------------------
// Hit-stop (freeze frames on impact)
// ---------------------------------------------------------------------------

float UEclipseGameFeel::GetHitStopDuration(EHitWeight Weight) const
{
    // Map the semantic weight to its freeze duration. Parry freezes for
    // a beat (spark/read window); PerfectDodge is a slow-mo event, not a
    // freeze, so it contributes zero hit-stop.
    switch (Weight)
    {
    case EHitWeight::Light:        return Profile.LightHitStop;    // 0.02s
    case EHitWeight::Medium:       return Profile.MediumHitStop;   // 0.05s
    case EHitWeight::Heavy:         return Profile.HeavyHitStop;    // 0.10s
    case EHitWeight::Charged:       return Profile.ChargedHitStop; // 0.15s
    case EHitWeight::Critical:      return Profile.CriticalHitStop; // 0.20s
    case EHitWeight::Parry:         return 0.08f;                  // spark freeze
    case EHitWeight::PerfectDodge: return 0.0f;                    // handled by slow-mo
    default:                        return Profile.MediumHitStop;
    }
}

void UEclipseGameFeel::ApplyHitStop(EHitWeight Weight)
{
    const float Duration = GetHitStopDuration(Weight);
    HitStopRemaining = FMath::Max(HitStopRemaining, Duration);

    if (Duration > 0.0f)
    {
        UE_LOG(LogEclipseGameFeel, Log,
            TEXT("[HitStop] Applied %s -> %.3fs freeze (total pending %.3fs)"),
            *UEnum::GetValueAsString(Weight), Duration, HitStopRemaining);
    }
}

bool UEclipseGameFeel::IsHitStopActive() const
{
    return HitStopRemaining > 0.0f;
}

void UEclipseGameFeel::UpdateHitStop(float DeltaTime)
{
    if (HitStopRemaining > 0.0f)
    {
        HitStopRemaining -= DeltaTime;
        if (HitStopRemaining <= 0.0f)
        {
            HitStopRemaining = 0.0f;
            UE_LOG(LogEclipseGameFeel, Verbose, TEXT("[HitStop] Released — world resumes"));
        }
    }
}

// ---------------------------------------------------------------------------
// Camera shake
// ---------------------------------------------------------------------------

float UEclipseGameFeel::GetCameraShake(ECameraShakeIntensity Intensity) const
{
    switch (Intensity)
    {
    case ECameraShakeIntensity::None:    return 0.0f;
    case ECameraShakeIntensity::Subtle:  return Profile.CameraShakeLight * 0.5f; // micro-impact
    case ECameraShakeIntensity::Light:   return Profile.CameraShakeLight;        // 0.5
    case ECameraShakeIntensity::Medium:  return Profile.CameraShakeMedium;       // 1.5
    case ECameraShakeIntensity::Heavy:   return Profile.CameraShakeHeavy;        // 3.0
    case ECameraShakeIntensity::Extreme: return Profile.CameraShakeExtreme;       // 6.0
    default:                             return 0.0f;
    }
}

void UEclipseGameFeel::ApplyCameraShake(ECameraShakeIntensity Intensity)
{
    const float Magnitude = GetCameraShake(Intensity);
    if (Magnitude > 0.0f)
    {
        UE_LOG(LogEclipseGameFeel, Log,
            TEXT("[CameraShake] %s -> magnitude %.2f"),
            *UEnum::GetValueAsString(Intensity), Magnitude);
    }
}

// ---------------------------------------------------------------------------
// Input buffering
// ---------------------------------------------------------------------------
// Design: the most recent input is held for InputBufferWindow seconds so a
// press that lands a hair early still fires on the next available frame.
// BufferInput stamps the buffer + timer; ConsumeBufferedInput drains it if
// still valid. The owning tick should decrement BufferTimer each frame
// (there is no dedicated UpdateBuffer on this UObject); until then the
// held input remains valid for one consume.
// ---------------------------------------------------------------------------

void UEclipseGameFeel::BufferInput(const FString& ActionName)
{
    if (ActionName.IsEmpty())
    {
        return;
    }
    BufferedInput = ActionName;
    BufferTimer   = Profile.InputBufferWindow; // 0.15s

    UE_LOG(LogEclipseGameFeel, VeryVerbose,
        TEXT("[InputBuffer] Buffered '%s' (window %.2fs)"), *ActionName, Profile.InputBufferWindow);
}

FString UEclipseGameFeel::ConsumeBufferedInput()
{
    if (HasBufferedInput())
    {
        const FString Out = BufferedInput;
        ClearBufferedInput();
        UE_LOG(LogEclipseGameFeel, VeryVerbose, TEXT("[InputBuffer] Consumed '%s'"), *Out);
        return Out;
    }
    return TEXT("");
}

bool UEclipseGameFeel::HasBufferedInput() const
{
    return !BufferedInput.IsEmpty() && BufferTimer > 0.0f;
}

void UEclipseGameFeel::ClearBufferedInput()
{
    BufferedInput = TEXT("");
    BufferTimer   = 0.0f;
}

// ---------------------------------------------------------------------------
// Combo system
// ---------------------------------------------------------------------------
// ComboWindow (0.20s) is the chain grace after each hit; ComboResetTime
// (0.5s) is the idle timeout after which the caller should ResetCombo().
// MaxComboChain caps the ladder at 5 steps before it must reset.
// ---------------------------------------------------------------------------

bool UEclipseGameFeel::CanChainCombo(float ElapsedSinceLastAttack) const
{
    // Must still be inside the chain window and not at the cap.
    const bool bWithinWindow = ElapsedSinceLastAttack <= Profile.ComboWindow;
    const bool bBelowCap    = ComboStep < Profile.MaxComboChain;
    return bWithinWindow && bBelowCap;
}

int32 UEclipseGameFeel::GetComboStep() const
{
    return ComboStep;
}

void UEclipseGameFeel::AdvanceCombo()
{
    // Cap at MaxComboChain; the gate in CanChainCombo prevents overflow,
    // but clamp defensively so a stray AdvanceCombo can't overshoot.
    ComboStep = FMath::Min(ComboStep + 1, Profile.MaxComboChain);
    ComboTimer = Profile.ComboWindow;

    UE_LOG(LogEclipseGameFeel, Log,
        TEXT("[Combo] Step %d / %d"), ComboStep, Profile.MaxComboChain);
}

void UEclipseGameFeel::ResetCombo()
{
    if (ComboStep != 0)
    {
        UE_LOG(LogEclipseGameFeel, Verbose, TEXT("[Combo] Reset from step %d"), ComboStep);
    }
    ComboStep  = 0;
    ComboTimer = 0.0f;
}

// ---------------------------------------------------------------------------
// Animation commitment
// ---------------------------------------------------------------------------
// Once an attack passes its commit threshold it can no longer be cancelled
// — the player is "locked in." CanCancel() returns true only while still
// inside the cancelable startup.
// ---------------------------------------------------------------------------

float UEclipseGameFeel::GetCommitTime(const FString& AttackType) const
{
    if (AttackType.Equals(TEXT("Light"),   ESearchCase::IgnoreCase)) return Profile.LightCommitTime;
    if (AttackType.Equals(TEXT("Heavy"),  ESearchCase::IgnoreCase)) return Profile.HeavyCommitTime;
    if (AttackType.Equals(TEXT("Charged"), ESearchCase::IgnoreCase)) return Profile.ChargedCommitTime;
    if (AttackType.Equals(TEXT("Spell"),  ESearchCase::IgnoreCase)) return Profile.SpellCommitTime;
    // Unknown attack type — fall back to the lightest (most cancelable) commit.
    return Profile.LightCommitTime;
}

bool UEclipseGameFeel::CanCancel(float ElapsedSinceStart, const FString& AttackType) const
{
    const float CommitTime = GetCommitTime(AttackType);
    // Cancelable only before the commit point. At or after it, the action
    // is committed and must play out.
    return ElapsedSinceStart < CommitTime;
}

// ---------------------------------------------------------------------------
// Slow-motion (time dilation)
// ---------------------------------------------------------------------------
// Perfect dodge and critical hits drop world time dilation for a beat.
// GetTimeDilation() is read by the owning tick to scale DeltaTime.
// ---------------------------------------------------------------------------

void UEclipseGameFeel::TriggerPerfectDodgeSlowMo()
{
    TimeDilation    = Profile.PerfectDodgeTimeDilation; // 0.3x
    SlowMoRemaining = Profile.PerfectDodgeSlowMoDuration;

    UE_LOG(LogEclipseGameFeel, Log,
        TEXT("[SlowMo] Perfect dodge — dilation %.2fx for %.2fs"),
        TimeDilation, SlowMoRemaining);
}

void UEclipseGameFeel::TriggerCriticalSlowMo()
{
    TimeDilation    = Profile.CriticalTimeDilation; // 0.4x
    SlowMoRemaining = Profile.CriticalSlowMoDuration;

    UE_LOG(LogEclipseGameFeel, Log,
        TEXT("[SlowMo] Critical — dilation %.2fx for %.2fs"),
        TimeDilation, SlowMoRemaining);
}

float UEclipseGameFeel::GetTimeDilation() const
{
    return TimeDilation;
}

void UEclipseGameFeel::UpdateSlowMo(float DeltaTime)
{
    if (SlowMoRemaining > 0.0f)
    {
        SlowMoRemaining -= DeltaTime;
        if (SlowMoRemaining <= 0.0f)
        {
            SlowMoRemaining = 0.0f;
            TimeDilation    = 1.0f; // restore real-time
            UE_LOG(LogEclipseGameFeel, Verbose, TEXT("[SlowMo] Restored dilation to 1.0x"));
        }
    }
    else if (TimeDilation != 1.0f)
    {
        // Defensive: if a slow-mo was triggered with zero duration, snap back.
        TimeDilation = 1.0f;
    }
}

// ---------------------------------------------------------------------------
// Stamina
// ---------------------------------------------------------------------------
// Regen is paused for StaminaRegenDelay after any use, then resumes.
// When fully depleted (at zero), regen runs at the slower StaminaPunishedRegen
// rate — the "exhausted" recovery tax — until stamina climbs back above zero,
// after which normal regen resumes.
// ---------------------------------------------------------------------------

bool UEclipseGameFeel::CanUseStamina(float Cost) const
{
    return Cost >= 0.0f && CurrentStamina >= Cost;
}

void UEclipseGameFeel::UseStamina(float Cost)
{
    if (Cost <= 0.0f)
    {
        return;
    }
    CurrentStamina = FMath::Max(0.0f, CurrentStamina - Cost);
    // Any use restarts the regen delay timer.
    StaminaRegenTimer = Profile.StaminaRegenDelay;

    UE_LOG(LogEclipseGameFeel, VeryVerbose,
        TEXT("[Stamina] Spent %.1f -> %.1f / %.1f (regen delay %.2fs)"),
        Cost, CurrentStamina, Profile.MaxStamina, Profile.StaminaRegenDelay);
}

void UEclipseGameFeel::RegenStamina(float DeltaTime)
{
    if (DeltaTime <= 0.0f)
    {
        return;
    }

    // Still inside the post-use delay — no regen yet.
    if (StaminaRegenTimer > 0.0f)
    {
        StaminaRegenTimer -= DeltaTime;
        if (StaminaRegenTimer < 0.0f)
        {
            StaminaRegenTimer = 0.0f;
        }
        return;
    }

    // Already full — nothing to do.
    if (CurrentStamina >= Profile.MaxStamina)
    {
        return;
    }

    // Depleted (exactly zero) recovers at the punished rate; any stamina
    // above zero recovers at the normal rate. This taxes the moment of
    // exhaustion without needing a dedicated depleted flag.
    const float Rate = (CurrentStamina <= 0.0f)
        ? Profile.StaminaPunishedRegen
        : Profile.StaminaRegenRate;

    CurrentStamina = FMath::Clamp(CurrentStamina + Rate * DeltaTime, 0.0f, Profile.MaxStamina);
}

float UEclipseGameFeel::GetStamina() const
{
    return CurrentStamina;
}

float UEclipseGameFeel::GetStaminaPct() const
{
    if (Profile.MaxStamina <= 0.0f)
    {
        return 0.0f;
    }
    return CurrentStamina / Profile.MaxStamina;
}

// ---------------------------------------------------------------------------
// Poise / stagger
// ---------------------------------------------------------------------------
// Poise is a damage-absorbing buffer (starts at BaseStaggerThreshold).
// Taking poise damage resets the regen delay; if poise hits zero the
// actor is staggered and must recover over StaggerRecoveryTime before
// poise resets to full. Otherwise poise regenerates at PoiseRegenRate
// once PoiseRegenDelay has elapsed since the last hit.
// ---------------------------------------------------------------------------

void UEclipseGameFeel::ApplyPoiseDamage(float Amount)
{
    if (Amount <= 0.0f)
    {
        return;
    }

    CurrentPoise = FMath::Max(0.0f, CurrentPoise - Amount);
    // Every hit restarts the regen-delay clock.
    PoiseRegenTimer = Profile.PoiseRegenDelay;

    UE_LOG(LogEclipseGameFeel, Log,
        TEXT("[Poise] -%.1f -> %.1f / %.1f"),
        Amount, CurrentPoise, Profile.BaseStaggerThreshold);

    if (CurrentPoise <= 0.0f && !bStaggered)
    {
        // Poise broken — enter staggered state and begin recovery.
        bStaggered      = true;
        PoiseRegenTimer = Profile.StaggerRecoveryTime;

        UE_LOG(LogEclipseGameFeel, Warning,
            TEXT("[Poise] STAGGERED — recovering over %.2fs"), Profile.StaggerRecoveryTime);
    }
}

bool UEclipseGameFeel::IsStaggered() const
{
    return bStaggered;
}

void UEclipseGameFeel::UpdatePoise(float DeltaTime)
{
    if (DeltaTime <= 0.0f)
    {
        return;
    }

    if (bStaggered)
    {
        // Count down the stagger recovery; on completion snap poise to full.
        PoiseRegenTimer -= DeltaTime;
        if (PoiseRegenTimer <= 0.0f)
        {
            PoiseRegenTimer = 0.0f;
            bStaggered     = false;
            CurrentPoise   = Profile.BaseStaggerThreshold;

            UE_LOG(LogEclipseGameFeel, Log,
                TEXT("[Poise] Stagger recovered — poise reset to %.1f"), CurrentPoise);
        }
        return;
    }

    // Not staggered — tick the regen delay, then regen poise.
    if (PoiseRegenTimer > 0.0f)
    {
        PoiseRegenTimer -= DeltaTime;
        if (PoiseRegenTimer < 0.0f)
        {
            PoiseRegenTimer = 0.0f;
        }
        return;
    }

    if (CurrentPoise < Profile.BaseStaggerThreshold)
    {
        CurrentPoise = FMath::Clamp(
            CurrentPoise + Profile.PoiseRegenRate * DeltaTime,
            0.0f,
            Profile.BaseStaggerThreshold);
    }
}

float UEclipseGameFeel::GetPoise() const
{
    return CurrentPoise;
}

// ---------------------------------------------------------------------------
// Tuning presets
// ---------------------------------------------------------------------------
// ApplyPreset re-tunes the live profile around a thematic "feel" keyed by
// EHitWeight. Medium resets to the baseline struct defaults; the others
// bias the profile toward that fantasy (nimble, weighty, brutal, lethal,
// defensive, evasive). Derived state is re-clamped to the new caps.
// ---------------------------------------------------------------------------

void UEclipseGameFeel::ApplyPreset(EHitWeight Preset)
{
    UE_LOG(LogEclipseGameFeel, Log,
        TEXT("[Preset] Applying '%s'"), *UEnum::GetValueAsString(Preset));

    switch (Preset)
    {
    case EHitWeight::Light:
        // --- Nimble / responsive ---
        // Fast recovery, low commitment, generous combo, snappy dodge.
        Profile.StaminaRegenRate     = 20.0f;
        Profile.StaminaRegenDelay    = 0.35f;
        Profile.StaminaPunishedRegen = 9.0f;
        Profile.LightCommitTime      = 0.10f;
        Profile.HeavyCommitTime      = 0.25f;
        Profile.ChargedCommitTime    = 0.40f;
        Profile.ComboWindow          = 0.25f;
        Profile.MaxComboChain        = 6;
        Profile.DodgeCooldown        = 0.40f;
        Profile.DodgeStaminaCost     = 16.0f;
        Profile.PoiseRegenRate       = 15.0f;
        Profile.PoiseRegenDelay      = 1.5f;
        Profile.TurnRate             = 800.0f;
        Profile.CombatTurnRate       = 1200.0f;
        break;

    case EHitWeight::Medium:
        // --- Balanced (reset to defaults) ---
        Profile = FGameFeelProfile();
        break;

    case EHitWeight::Heavy:
        // --- Weighty / committed ---
        // Longer commits, heavier stamina tax, beefier poise, bigger shake.
        Profile.StaminaRegenRate      = 12.0f;
        Profile.StaminaRegenDelay     = 0.65f;
        Profile.StaminaPunishedRegen = 5.0f;
        Profile.HeavyCommitTime       = 0.35f;
        Profile.ChargedCommitTime     = 0.55f;
        Profile.SpellCommitTime       = 0.30f;
        Profile.DodgeStaminaCost      = 26.0f;
        Profile.ParryStaminaCost       = 30.0f;
        Profile.BlockStaminaDrainPerHit = 18.0f;
        Profile.CameraShakeHeavy      = 4.0f;
        Profile.CameraShakeExtreme    = 7.0f;
        Profile.PoiseRegenRate        = 7.0f;
        Profile.PoiseRegenDelay       = 2.5f;
        Profile.BaseStaggerThreshold  = 120.0f;
        Profile.StaggerRecoveryTime   = 1.8f;
        Profile.TurnRate             = 640.0f;
        Profile.CombatTurnRate       = 960.0f;
        break;

    case EHitWeight::Charged:
        // --- Brutal ---
        // Extreme commitment, devastating hit-stop, slow stamina.
        Profile.ChargedHitStop        = 0.20f;
        Profile.CriticalHitStop       = 0.25f;
        Profile.HeavyHitStop          = 0.12f;
        Profile.ChargedCommitTime     = 0.65f;
        Profile.HeavyCommitTime       = 0.40f;
        Profile.CameraShakeExtreme    = 8.0f;
        Profile.CameraShakeHeavy      = 5.0f;
        Profile.StaminaRegenRate      = 10.0f;
        Profile.StaminaRegenDelay     = 0.75f;
        Profile.DodgeStaminaCost      = 30.0f;
        Profile.BaseStaggerThreshold  = 150.0f;
        Profile.StaggerRecoveryTime   = 2.0f;
        Profile.PoiseRegenDelay       = 3.0f;
        break;

    case EHitWeight::Critical:
        // --- Lethal ---
        // Maximum impact, longest slow-mo, slowest recovery.
        Profile.CriticalHitStop        = 0.25f;
        Profile.CriticalTimeDilation   = 0.3f;
        Profile.CriticalSlowMoDuration = 0.45f;
        Profile.CameraShakeExtreme     = 10.0f;
        Profile.CameraZoomOnCritical   = 1.2f;
        Profile.CameraZoomDuration     = 0.4f;
        Profile.StaminaRegenRate       = 8.0f;
        Profile.StaminaPunishedRegen   = 4.0f;
        Profile.StaminaRegenDelay      = 0.8f;
        Profile.BaseStaggerThreshold   = 180.0f;
        Profile.StaggerRecoveryTime    = 2.5f;
        Profile.PoiseRegenDelay        = 3.5f;
        break;

    case EHitWeight::Parry:
        // --- Defensive duelist ---
        // Generous parry windows, stamina-efficient blocking.
        Profile.ParryWindow               = 0.18f;
        Profile.PerfectParryWindow        = 0.10f;
        Profile.ParryCooldown             = 1.5f;
        Profile.ParryStaminaCost          = 20.0f;
        Profile.PerfectParryStaminaRegen  = 25.0f;
        Profile.BlockStaminaDrainPerHit   = 12.0f;
        Profile.BlockDamageReduction      = 0.7f;
        Profile.PerfectBlockReduction     = 0.95f;
        Profile.StaminaRegenRate          = 16.0f;
        break;

    case EHitWeight::PerfectDodge:
        // --- Evasion ---
        // Longer i-frames, cheaper dodge, generous slow-mo.
        Profile.DodgeDuration              = 0.38f;
        Profile.DodgeIFrameEnd            = 0.26f;
        Profile.DodgeCooldown             = 0.40f;
        Profile.DodgeStaminaCost          = 15.0f;
        Profile.PerfectDodgeTimeDilation  = 0.25f;
        Profile.PerfectDodgeSlowMoDuration = 0.6f;
        Profile.StaminaRegenRate          = 17.0f;
        Profile.StaminaRegenDelay         = 0.4f;
        break;

    default:
        // Unknown — reset to balanced defaults as a safe fallback.
        Profile = FGameFeelProfile();
        break;
    }

    // Re-clamp live state to the new caps so we never carry an
    // over-cap value from a looser previous preset.
    CurrentStamina = FMath::Min(CurrentStamina, Profile.MaxStamina);
    CurrentPoise   = FMath::Min(CurrentPoise,   Profile.BaseStaggerThreshold);
}

// ---------------------------------------------------------------------------
// Tuning report (debug)
// ---------------------------------------------------------------------------

FString UEclipseGameFeel::GetTuningReport() const
{
    FString Report;
    Report += TEXT("===== Eclipse Game Feel — Tuning Report =====\n");

    // --- Live state ---
    Report += TEXT("\n-- Live State --\n");
    Report += FString::Printf(TEXT("  Stamina:        %.1f / %.1f  (%.0f%%)\n"),
        CurrentStamina, Profile.MaxStamina, GetStaminaPct() * 100.0f);
    Report += FString::Printf(TEXT("  StaminaRegenTimer: %.3fs\n"), StaminaRegenTimer);
    Report += FString::Printf(TEXT("  Poise:          %.1f / %.1f  (Staggered: %s)\n"),
        CurrentPoise, Profile.BaseStaggerThreshold, bStaggered ? TEXT("YES") : TEXT("no"));
    Report += FString::Printf(TEXT("  PoiseRegenTimer: %.3fs\n"), PoiseRegenTimer);
    Report += FString::Printf(TEXT("  HitStopRemaining: %.3fs (active: %s)\n"),
        HitStopRemaining, IsHitStopActive() ? TEXT("YES") : TEXT("no"));
    Report += FString::Printf(TEXT("  TimeDilation:    %.3fx  (SlowMoRemaining: %.3fs)\n"),
        TimeDilation, SlowMoRemaining);
    Report += FString::Printf(TEXT("  BufferedInput:   '%s' (timer %.3fs)\n"),
        *BufferedInput, BufferTimer);
    Report += FString::Printf(TEXT("  ComboStep:       %d / %d  (timer %.3fs)\n"),
        ComboStep, Profile.MaxComboChain, ComboTimer);

    // --- Movement ---
    Report += TEXT("\n-- Movement --\n");
    Report += FString::Printf(TEXT("  WalkAccel:       %.1f | WalkDecel: %.1f\n"),
        Profile.WalkAccel, Profile.WalkDecel);
    Report += FString::Printf(TEXT("  SprintAccel:     %.1f\n"), Profile.SprintAccel);
    Report += FString::Printf(TEXT("  CombatWalkSpeed: %.1f | SprintSpeed: %.1f\n"),
        Profile.CombatWalkSpeed, Profile.SprintSpeed);
    Report += FString::Printf(TEXT("  TurnRate:        %.1f | CombatTurnRate: %.1f\n"),
        Profile.TurnRate, Profile.CombatTurnRate);
    Report += FString::Printf(TEXT("  JumpHeight:      %.1f | GravityScale: %.2f | AirControl: %.2f\n"),
        Profile.JumpHeight, Profile.GravityScale, Profile.AirControl);

    // --- Dodge ---
    Report += TEXT("\n-- Dodge --\n");
    Report += FString::Printf(TEXT("  DodgeDistance:   %.1f | Duration: %.3fs\n"),
        Profile.DodgeDistance, Profile.DodgeDuration);
    Report += FString::Printf(TEXT("  IFrame window:   [%.3f, %.3f]\n"),
        Profile.DodgeIFrameStart, Profile.DodgeIFrameEnd);
    Report += FString::Printf(TEXT("  Cooldown:        %.3fs | StaminaCost: %.1f\n"),
        Profile.DodgeCooldown, Profile.DodgeStaminaCost);

    // --- Parry ---
    Report += TEXT("\n-- Parry --\n");
    Report += FString::Printf(TEXT("  ParryWindow:     %.3fs | PerfectParryWindow: %.3fs\n"),
        Profile.ParryWindow, Profile.PerfectParryWindow);
    Report += FString::Printf(TEXT("  Cooldown:        %.3fs | StaminaCost: %.1f\n"),
        Profile.ParryCooldown, Profile.ParryStaminaCost);
    Report += FString::Printf(TEXT("  PerfectParryRegen: %.1f\n"), Profile.PerfectParryStaminaRegen);

    // --- Block ---
    Report += TEXT("\n-- Block --\n");
    Report += FString::Printf(TEXT("  StaminaDrain/Hit: %.1f | DmgReduction: %.0f%% | Perfect: %.0f%%\n"),
        Profile.BlockStaminaDrainPerHit, Profile.BlockDamageReduction * 100.0f,
        Profile.PerfectBlockReduction * 100.0f);

    // --- Hit-stop ---
    Report += TEXT("\n-- Hit-stop --\n");
    Report += FString::Printf(TEXT("  Light: %.3f | Medium: %.3f | Heavy: %.3f | Charged: %.3f | Critical: %.3f\n"),
        Profile.LightHitStop, Profile.MediumHitStop, Profile.HeavyHitStop,
        Profile.ChargedHitStop, Profile.CriticalHitStop);

    // --- Camera ---
    Report += TEXT("\n-- Camera --\n");
    Report += FString::Printf(TEXT("  Shake L/M/H/X: %.2f / %.2f / %.2f / %.2f\n"),
        Profile.CameraShakeLight, Profile.CameraShakeMedium,
        Profile.CameraShakeHeavy, Profile.CameraShakeExtreme);
    Report += FString::Printf(TEXT("  ZoomOnCritical: %.2f FOV for %.3fs\n"),
        Profile.CameraZoomOnCritical, Profile.CameraZoomDuration);
    Report += FString::Printf(TEXT("  LockOnDistance: %.1f | LockOnAngle: %.1f deg\n"),
        Profile.CameraLockOnDistance, Profile.CameraLockOnAngle);

    // --- Input / Combo ---
    Report += TEXT("\n-- Input / Combo --\n");
    Report += FString::Printf(TEXT("  InputBufferWindow: %.3fs | ComboWindow: %.3fs | ComboReset: %.3fs\n"),
        Profile.InputBufferWindow, Profile.ComboWindow, Profile.ComboResetTime);
    Report += FString::Printf(TEXT("  MaxComboChain: %d\n"), Profile.MaxComboChain);

    // --- Commitment ---
    Report += TEXT("\n-- Animation Commitment --\n");
    Report += FString::Printf(TEXT("  Light: %.3f | Heavy: %.3f | Charged: %.3f | Spell: %.3f\n"),
        Profile.LightCommitTime, Profile.HeavyCommitTime,
        Profile.ChargedCommitTime, Profile.SpellCommitTime);

    // --- Slow-mo ---
    Report += TEXT("\n-- Slow-motion --\n");
    Report += FString::Printf(TEXT("  PerfectDodge: %.2fx for %.3fs | Critical: %.2fx for %.3fs\n"),
        Profile.PerfectDodgeTimeDilation, Profile.PerfectDodgeSlowMoDuration,
        Profile.CriticalTimeDilation, Profile.CriticalSlowMoDuration);

    // --- Stamina ---
    Report += TEXT("\n-- Stamina --\n");
    Report += FString::Printf(TEXT("  Max: %.1f | RegenRate: %.1f/s | Delay: %.3fs | Punished: %.1f/s\n"),
        Profile.MaxStamina, Profile.StaminaRegenRate,
        Profile.StaminaRegenDelay, Profile.StaminaPunishedRegen);

    // --- Poise / Stagger ---
    Report += TEXT("\n-- Poise / Stagger --\n");
    Report += FString::Printf(TEXT("  Threshold: %.1f | Recovery: %.3fs | RegenRate: %.1f/s | RegenDelay: %.3fs\n"),
        Profile.BaseStaggerThreshold, Profile.StaggerRecoveryTime,
        Profile.PoiseRegenRate, Profile.PoiseRegenDelay);

    Report += TEXT("\n=============================================\n");
    return Report;
}
