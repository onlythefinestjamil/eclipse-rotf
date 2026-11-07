// Eclipse Game Feel Director — AAA-quality movement, combat, responsiveness
// Per Directive #2: hit-stop, camera impulse, input buffering, combo windows
// The small details players feel subconsciously

#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EclipseGameFeel.generated.h"

UENUM(BlueprintType)
enum class EHitWeight : uint8
{
    Light       UMETA(DisplayName = "Light — fast, low impact, 0.02s hit-stop"),
    Medium      UMETA(DisplayName = "Medium — moderate, 0.05s hit-stop"),
    Heavy       UMETA(DisplayName = "Heavy — committed, 0.10s hit-stop, camera shake"),
    Charged      UMETA(DisplayName = "Charged — very heavy, 0.15s hit-stop, large shake"),
    Critical     UMETA(DisplayName = "Critical — 0.20s hit-stop, zoom, large shake"),
    Parry        UMETA(DisplayName = "Parry — 0.08s hit-stop, spark VFX, stamina drain"),
    PerfectDodge UMETA(DisplayName = "Perfect Dodge — 0.05s slow-mo, afterimage"),
};

UENUM(BlueprintType)
enum class ECameraShakeIntensity : uint8
{
    None    UMETA(DisplayName = "None — no shake"),
    Subtle  UMETA(DisplayName = "Subtle — small weapon impact"),
    Light   UMETA(DisplayName = "Light — light attack hit"),
    Medium  UMETA(DisplayName = "Medium — heavy attack hit"),
    Heavy   UMETA(DisplayName = "Heavy — charged/spell impact"),
    Extreme UMETA(DisplayName = "Extreme — boss slam, explosion"),
};

USTRUCT(BlueprintType)
struct FGameFeelProfile
{
    // Movement
    UPROPERTY(BlueprintReadWrite) float WalkAccel = 2048.0f;
    UPROPERTY(BlueprintReadWrite) float WalkDecel = 4096.0f;
    UPROPERTY(BlueprintReadWrite) float SprintAccel = 3072.0f;
    UPROPERTY(BlueprintReadWrite) float CombatWalkSpeed = 300.0f;
    UPROPERTY(BlueprintReadWrite) float SprintSpeed = 600.0f;
    UPROPERTY(BlueprintReadWrite) float TurnRate = 720.0f; // deg/s
    UPROPERTY(BlueprintReadWrite) float CombatTurnRate = 1080.0f; // faster in combat
    UPROPERTY(BlueprintReadWrite) float JumpHeight = 500.0f;
    UPROPERTY(BlueprintReadWrite) float GravityScale = 1.5f;
    UPROPERTY(BlueprintReadWrite) float AirControl = 0.35f;

    // Dodge
    UPROPERTY(BlueprintReadWrite) float DodgeDistance = 400.0f;
    UPROPERTY(BlueprintReadWrite) float DodgeDuration = 0.32f; // i-frame window
    UPROPERTY(BlueprintReadWrite) float DodgeIFrameStart = 0.05f;
    UPROPERTY(BlueprintReadWrite) float DodgeIFrameEnd = 0.22f;
    UPROPERTY(BlueprintReadWrite) float DodgeCooldown = 0.5f;
    UPROPERTY(BlueprintReadWrite) float DodgeStaminaCost = 20.0f;

    // Parry
    UPROPERTY(BlueprintReadWrite) float ParryWindow = 0.15f; // seconds
    UPROPERTY(BlueprintReadWrite) float ParryCooldown = 2.0f;
    UPROPERTY(BlueprintReadWrite) float ParryStaminaCost = 25.0f;
    UPROPERTY(BlueprintReadWrite) float PerfectParryWindow = 0.08f;
    UPROPERTY(BlueprintReadWrite) float PerfectParryStaminaRegen = 15.0f;

    // Block
    UPROPERTY(BlueprintReadWrite) float BlockStaminaDrainPerHit = 15.0f;
    UPROPERTY(BlueprintReadWrite) float BlockDamageReduction = 0.6f; // 60% reduction
    UPROPERTY(BlueprintReadWrite) float PerfectBlockReduction = 0.9f; // 90%

    // Hit-stop (freeze frames on impact)
    UPROPERTY(BlueprintReadWrite) float LightHitStop = 0.02f;
    UPROPERTY(BlueprintReadWrite) float MediumHitStop = 0.05f;
    UPROPERTY(BlueprintReadWrite) float HeavyHitStop = 0.10f;
    UPROPERTY(BlueprintReadWrite) float ChargedHitStop = 0.15f;
    UPROPERTY(BlueprintReadWrite) float CriticalHitStop = 0.20f;

    // Camera
    UPROPERTY(BlueprintReadWrite) float CameraShakeLight = 0.5f;
    UPROPERTY(BlueprintReadWrite) float CameraShakeMedium = 1.5f;
    UPROPERTY(BlueprintReadWrite) float CameraShakeHeavy = 3.0f;
    UPROPERTY(BlueprintReadWrite) float CameraShakeExtreme = 6.0f;
    UPROPERTY(BlueprintReadWrite) float CameraZoomOnCritical = 0.8f; // FOV decrease
    UPROPERTY(BlueprintReadWrite) float CameraZoomDuration = 0.3f;
    UPROPERTY(BlueprintReadWrite) float CameraLockOnDistance = 1500.0f;
    UPROPERTY(BlueprintReadWrite) float CameraLockOnAngle = 45.0f; // degrees

    // Input buffering
    UPROPERTY(BlueprintReadWrite) float InputBufferWindow = 0.15f; // seconds to buffer next input
    UPROPERTY(BlueprintReadWrite) float ComboWindow = 0.20f; // window to chain attacks
    UPROPERTY(BlueprintReadWrite) float ComboResetTime = 0.5f; // idle before combo resets
    UPROPERTY(BlueprintReadWrite) int32 MaxComboChain = 5;

    // Animation commitment
    UPROPERTY(BlueprintReadWrite) float LightCommitTime = 0.15f; // can't cancel after this
    UPROPERTY(BlueprintReadWrite) float HeavyCommitTime = 0.30f;
    UPROPERTY(BlueprintReadWrite) float ChargedCommitTime = 0.45f;
    UPROPERTY(BlueprintReadWrite) float SpellCommitTime = 0.25f;

    // Slow-motion
    UPROPERTY(BlueprintReadWrite) float PerfectDodgeTimeDilation = 0.3f;
    UPROPERTY(BlueprintReadWrite) float PerfectDodgeSlowMoDuration = 0.5f;
    UPROPERTY(BlueprintReadWrite) float CriticalTimeDilation = 0.4f;
    UPROPERTY(BlueprintReadWrite) float CriticalSlowMoDuration = 0.3f;

    // Stagger
    UPROPERTY(BlueprintReadWrite) float BaseStaggerThreshold = 100.0f;
    UPROPERTY(BlueprintReadWrite) float StaggerRecoveryTime = 1.5f;
    UPROPERTY(BlueprintReadWrite) float PoiseRegenRate = 10.0f; // per second
    UPROPERTY(BlueprintReadWrite) float PoiseRegenDelay = 2.0f; // delay after hit

    // Stamina
    UPROPERTY(BlueprintReadWrite) float MaxStamina = 100.0f;
    UPROPERTY(BlueprintReadWrite) float StaminaRegenRate = 15.0f; // per second
    UPROPERTY(BlueprintReadWrite) float StaminaRegenDelay = 0.5f; // delay after use
    UPROPERTY(BlueprintReadWrite) float StaminaPunishedRegen = 7.0f; // when depleted
};

UCLASS(Blueprintable)
class ECLIPSEROTF_API UEclipseGameFeel : public UObject
{
    GENERATED_BODY()

public:
    UEclipseGameFeel();

    // Get the active feel profile
    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel")
    FGameFeelProfile GetProfile() const;

    // Set a custom profile
    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel")
    void SetProfile(const FGameFeelProfile& InProfile);

    // === MOVEMENT ===
    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|Movement")
    float GetMoveSpeed(bool bSprinting, bool bInCombat) const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|Movement")
    float GetTurnRate(bool bInCombat) const;

    // === DODGE ===
    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|Dodge")
    bool CanDodge(float CurrentStamina) const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|Dodge")
    float GetDodgeIFrameStart() const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|Dodge")
    float GetDodgeIFrameEnd() const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|Dodge")
    bool IsInIFrame(float ElapsedSinceDodge) const;

    // === PARRY ===
    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|Parry")
    bool CanParry(float CurrentStamina) const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|Parry")
    bool IsPerfectParry(float ElapsedSinceParryStart) const;

    // === HIT-STOP ===
    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|HitStop")
    float GetHitStopDuration(EHitWeight Weight) const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|HitStop")
    void ApplyHitStop(EHitWeight Weight);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|HitStop")
    bool IsHitStopActive() const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|HitStop")
    void UpdateHitStop(float DeltaTime);

    // === CAMERA SHAKE ===
    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|Camera")
    float GetCameraShake(ECameraShakeIntensity Intensity) const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|Camera")
    void ApplyCameraShake(ECameraShakeIntensity Intensity);

    // === INPUT BUFFERING ===
    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|Input")
    void BufferInput(const FString& ActionName);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|Input")
    FString ConsumeBufferedInput();

    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|Input")
    bool HasBufferedInput() const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|Input")
    void ClearBufferedInput();

    // === COMBO SYSTEM ===
    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|Combo")
    bool CanChainCombo(float ElapsedSinceLastAttack) const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|Combo")
    int32 GetComboStep() const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|Combo")
    void AdvanceCombo();

    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|Combo")
    void ResetCombo();

    // === ANIMATION COMMITMENT ===
    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|Commitment")
    float GetCommitTime(const FString& AttackType) const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|Commitment")
    bool CanCancel(float ElapsedSinceStart, const FString& AttackType) const;

    // === SLOW MOTION ===
    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|SlowMo")
    void TriggerPerfectDodgeSlowMo();

    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|SlowMo")
    void TriggerCriticalSlowMo();

    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|SlowMo")
    float GetTimeDilation() const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|SlowMo")
    void UpdateSlowMo(float DeltaTime);

    // === STAMINA ===
    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|Stamina")
    bool CanUseStamina(float Cost) const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|Stamina")
    void UseStamina(float Cost);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|Stamina")
    void RegenStamina(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|Stamina")
    float GetStamina() const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|Stamina")
    float GetStaminaPct() const;

    // === POISE/STAGGER ===
    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|Stagger")
    void ApplyPoiseDamage(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|Stagger")
    bool IsStaggered() const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|Stagger")
    void UpdatePoise(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel|Stagger")
    float GetPoise() const;

    // === TUNING PRESETS ===
    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel")
    void ApplyPreset(EHitWeight Preset);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|GameFeel")
    FString GetTuningReport() const;

private:
    FGameFeelProfile Profile;
    float CurrentStamina = 100.0f;
    float StaminaRegenTimer = 0.0f;
    float CurrentPoise = 100.0f;
    float PoiseRegenTimer = 0.0f;
    bool bStaggered = false;
    float HitStopRemaining = 0.0f;
    float TimeDilation = 1.0f;
    float SlowMoRemaining = 0.0f;
    FString BufferedInput = "";
    float BufferTimer = 0.0f;
    int32 ComboStep = 0;
    float ComboTimer = 0.0f;
};
