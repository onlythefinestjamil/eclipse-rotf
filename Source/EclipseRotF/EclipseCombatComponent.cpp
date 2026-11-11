#include "EclipseCombatComponent.h"

#include "EclipseCharacter.h"
#include "EclipsePerfectDodgeComponent.h"
#include "EclipseEquipmentComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogEclipseCombat, Log, All);

UEclipseCombatComponent::UEclipseCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UEclipseCombatComponent::BeginPlay()
{
    Super::BeginPlay();
    CombatSystem = NewObject<UEclipseCombatSystem>(this);
    UE_LOG(LogEclipseCombat, Log, TEXT("[COMBAT] Combat component ready (combo=%d, parry=%.2fs)"), MaxComboSteps, ParryWindowSeconds);
}

AEclipseCharacter* UEclipseCombatComponent::GetOwnerChar() const
{
    return Cast<AEclipseCharacter>(GetOwner());
}

void UEclipseCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    const float Dt = DeltaTime;

    // Poll the montage instead of binding OnMontageEnded: that delegate is a dynamic multicast in 5.8,
    // and polling also survives hit-stop time dilation cleanly.
    if (bMontageActive)
    {
        const UAnimInstance* AnimInstance = GetOwnerAnimInstance();
        if (!AnimInstance || !CurrentMontage || !AnimInstance->Montage_IsPlaying(CurrentMontage))
        {
            OnCombatMontageEnded(CurrentMontage, false);
        }
    }

    if (ComboWindowRemaining > 0.0f)
    {
        ComboWindowRemaining = FMath::Max(0.0f, ComboWindowRemaining - Dt);
        if (ComboWindowRemaining <= 0.0f)
        {
            ComboIndex = 0;
        }
    }

    if (HeavyRecoveryRemaining > 0.0f)
    {
        HeavyRecoveryRemaining = FMath::Max(0.0f, HeavyRecoveryRemaining - Dt);
    }

    if (DodgeCooldownRemaining > 0.0f)
    {
        DodgeCooldownRemaining = FMath::Max(0.0f, DodgeCooldownRemaining - Dt);
    }

    if (ParryTimer > 0.0f)
    {
        ParryTimer = FMath::Max(0.0f, ParryTimer - Dt);
    }
    bParryWindowOpen = ParryTimer > 0.0f;

    if (bRMBHeld)
    {
        RMBHoldTimer += Dt;
        if (!bBlocking && RMBHoldTimer >= BlockHoldThreshold && State != ECombatState::Dodging)
        {
            const bool bWasIdle = (State == ECombatState::Idle);
            bBlocking = true;
            State = ECombatState::Blocking;
            UE_LOG(LogEclipseCombat, Log, TEXT("[COMBAT] Block started (held %.2fs)"), RMBHoldTimer);

            // Block loop is cosmetic only - never interrupt a swing montage that is already running.
            if (bWasIdle && bMontageDriven && BlockMontage)
            {
                if (PlayCombatMontage(BlockMontage, 0.0f, TEXT("block")))
                {
                    bSwingSwept = true;
                }
            }
        }
    }

    if (State == ECombatState::Dodging)
    {
        DodgeElapsed += Dt;
        bDodgeIFrameActive = (DodgeElapsed >= Feel.DodgeIFrameStart) && (DodgeElapsed <= Feel.DodgeIFrameEnd);
        if (DodgeElapsed >= FMath::Max(Feel.DodgeDuration, 0.05f))
        {
            State = ECombatState::Idle;
            bDodgeIFrameActive = false;
            UE_LOG(LogEclipseCombat, Log, TEXT("[COMBAT] Dodge finished"));
        }
    }
    else
    {
        bDodgeIFrameActive = false;
    }

    // The cast bar is the timing authority, not the montage: a cast completes on CastDuration even
    // when the animation is shorter, and any damage cancels it through FinishCast(false).
    if (State == ECombatState::Casting)
    {
        CastElapsed += Dt;
        CastProgress = CastDuration > 0.0f ? FMath::Clamp(CastElapsed / CastDuration, 0.0f, 1.0f) : 1.0f;
        if (!bCastInterrupted && CastElapsed >= CastDuration)
        {
            FinishCast(true);
        }
    }

    // Stagger without a reaction asset still locks the action for StaggerFallbackSeconds.
    if (State == ECombatState::Stagger && !bStaggerFromMontage)
    {
        StaggerTimer = FMath::Max(0.0f, StaggerTimer - Dt);
        if (StaggerTimer <= 0.0f)
        {
            State = bBlocking ? ECombatState::Blocking : ECombatState::Idle;
        }
    }

    if (State == ECombatState::LightAttack || State == ECombatState::HeavyAttack)
    {
        // A playing montage owns the transition out of the attack state (see the montage poll above).
        if (!bMontageActive)
        {
            AttackTimer -= Dt;
            if (AttackTimer <= 0.0f)
            {
                State = bBlocking ? ECombatState::Blocking : ECombatState::Idle;
                AttackPhase = EAttackPhase::None;
            }
        }
    }

    if (BuffRemaining > 0.0f)
    {
        BuffRemaining = FMath::Max(0.0f, BuffRemaining - Dt);
        if (BuffRemaining <= 0.0f)
        {
            DamageBuffMultiplier = 1.0f;
            UE_LOG(LogEclipseCombat, Log, TEXT("[COMBAT] Weapon art buff expired"));
        }
    }
}

bool UEclipseCombatComponent::IsActing() const
{
    return State != ECombatState::Idle || HeavyRecoveryRemaining > 0.0f;
}

bool UEclipseCombatComponent::InputLightAttack()
{
    if (HeavyRecoveryRemaining > 0.0f)
    {
        UE_LOG(LogEclipseCombat, Log, TEXT("[COMBAT] Light attack refused - heavy recovery %.2fs remaining"), HeavyRecoveryRemaining);
        return false;
    }
    if (State == ECombatState::Dodging)
    {
        UE_LOG(LogEclipseCombat, Log, TEXT("[COMBAT] Light attack refused - dodging"));
        return false;
    }

    ComboIndex = (ComboWindowRemaining > 0.0f) ? FMath::Min(ComboIndex + 1, MaxComboSteps - 1) : 0;

    const float Damage = LightDamage * (1.0f + 0.10f * ComboIndex) * DamageBuffMultiplier;
    State = ECombatState::LightAttack;
    AttackPhase = EAttackPhase::Windup;

    UE_LOG(LogEclipseCombat, Log, TEXT("[COMBAT] Light attack chain=%d damage=%.1f buff=%.2f"), ComboIndex + 1, Damage, DamageBuffMultiplier);

    UAnimMontage* Montage = bMontageDriven ? PickLightAttackMontage() : nullptr;
    if (PlayCombatMontage(Montage, Damage, TEXT("light")))
    {
        // Montage owns the feel: its HitWindow notify sweeps, its ComboWindow notify opens the chain window.
        ComboWindowRemaining = 0.0f;
    }
    else
    {
        ComboWindowRemaining = Feel.ComboWindow;
        // Light swing total: 0.12 windup + 0.08 strike (hit at 0.20) + 0.32 recovery.
        AttackTimer = 0.52f;
        AttackPhase = EAttackPhase::Active;
        UE_LOG(LogEclipseCombat, Log, TEXT("[ANIM] no montage for light %d, timer fallback"), ComboIndex + 1);
        PerformSweep(Damage, TEXT("light"));
    }
    return true;
}

void UEclipseCombatComponent::InputHeavyPressed()
{
    bRMBHeld = true;
    RMBHoldTimer = 0.0f;
    ParryTimer = ParryWindowSeconds;
    UE_LOG(LogEclipseCombat, Log, TEXT("[COMBAT] RMB pressed - parry window open for %.2fs"), ParryWindowSeconds);
}

void UEclipseCombatComponent::InputHeavyReleased()
{
    const bool bWasBlocking = bBlocking;
    const float HeldFor = RMBHoldTimer;
    bRMBHeld = false;
    RMBHoldTimer = 0.0f;

    if (bWasBlocking)
    {
        bBlocking = false;
        if (State == ECombatState::Blocking)
        {
            State = ECombatState::Idle;
        }
        UE_LOG(LogEclipseCombat, Log, TEXT("[COMBAT] Block released (held %.2fs)"), HeldFor);
        return;
    }

    if (HeldFor < BlockHoldThreshold)
    {
        StartHeavyAttack();
    }
}

void UEclipseCombatComponent::StartHeavyAttack()
{
    if (HeavyRecoveryRemaining > 0.0f)
    {
        UE_LOG(LogEclipseCombat, Log, TEXT("[COMBAT] Heavy attack refused - still recovering"));
        return;
    }

    const float Damage = HeavyDamage * DamageBuffMultiplier;
    State = ECombatState::HeavyAttack;
    AttackPhase = EAttackPhase::Windup;
    HeavyRecoveryRemaining = HeavyRecoverySeconds;

    UE_LOG(LogEclipseCombat, Log, TEXT("[COMBAT] Heavy attack damage=%.1f recovery=%.2fs"), Damage, HeavyRecoverySeconds);

    if (PlayCombatMontage(bMontageDriven ? HeavyAttackMontage : nullptr, Damage, TEXT("heavy")))
    {
        ComboWindowRemaining = 0.0f;
    }
    else
    {
        // Heavy swing: 0.48 windup + 0.12 strike (hit at 0.60) + HeavyRecoverySeconds.
        AttackTimer = 0.60f + HeavyRecoverySeconds;
        AttackPhase = EAttackPhase::Active;
        UE_LOG(LogEclipseCombat, Log, TEXT("[ANIM] no montage for heavy, timer fallback"));
        PerformSweep(Damage, TEXT("heavy"));
        DoHitStop(Feel.HeavyHitStop);
    }
}

bool UEclipseCombatComponent::InputDodge(const FVector2D& MoveAxis)
{
    if (HeavyRecoveryRemaining > 0.0f)
    {
        UE_LOG(LogEclipseCombat, Log, TEXT("[COMBAT] Dodge refused - heavy attack commitment (%.2fs)"), HeavyRecoveryRemaining);
        return false;
    }
    if (DodgeCooldownRemaining > 0.0f)
    {
        UE_LOG(LogEclipseCombat, Log, TEXT("[COMBAT] Dodge refused - cooldown %.2fs"), DodgeCooldownRemaining);
        return false;
    }

    AEclipseCharacter* OwnerChar = GetOwnerChar();
    if (!OwnerChar)
    {
        return false;
    }

    if (!OwnerChar->TrySpendResource(EResourceType::Stamina, Feel.DodgeStaminaCost))
    {
        UE_LOG(LogEclipseCombat, Log, TEXT("[COMBAT] Dodge refused - not enough stamina"));
        return false;
    }

    FVector Direction = OwnerChar->GetActorForwardVector() * MoveAxis.Y + OwnerChar->GetActorRightVector() * MoveAxis.X;
    if (Direction.IsNearlyZero())
    {
        Direction = OwnerChar->GetActorForwardVector();
    }
    Direction.Normalize();

    bBlocking = false;
    State = ECombatState::Dodging;
    DodgeElapsed = 0.0f;
    DodgeCooldownRemaining = Feel.DodgeCooldown;

    const float Speed = Feel.DodgeDistance / FMath::Max(Feel.DodgeDuration, 0.05f);
    OwnerChar->LaunchCharacter(Direction * Speed, true, true);

    if (PlayCombatMontage(bMontageDriven ? DodgeMontage : nullptr, 0.0f, TEXT("dodge")))
    {
        // A dodge montage carries no hit window; disarm the sweep in case one was authored on it.
        bSwingSwept = true;
    }
    else
    {
        UE_LOG(LogEclipseCombat, Log, TEXT("[ANIM] no montage for dodge, timer fallback"));
    }

    UE_LOG(LogEclipseCombat, Log, TEXT("[COMBAT] Dodge dir=%s iframes=%.2f-%.2f"), *Direction.ToCompactString(), Feel.DodgeIFrameStart, Feel.DodgeIFrameEnd);
    return true;
}

float UEclipseCombatComponent::ReceiveDamage(float InDamage, AActor* Source)
{
    const FString SourceName = Source ? Source->GetName() : TEXT("Unknown");

    if (bDodgeIFrameActive)
    {
        UE_LOG(LogEclipseCombat, Log, TEXT("[DAMAGE] attacker=%s target=%s amount=0.0 type=perfectdodge"), *SourceName, *GetOwner()->GetName());

        // The perfect-dodge component owns the whole response: slow motion, afterimage trail, reward.
        // No hit-stop here - it would fight the slow motion the component is applying.
        if (AEclipseCharacter* OwnerChar = GetOwnerChar())
        {
            if (UEclipsePerfectDodgeComponent* Dodge = OwnerChar->GetPerfectDodge())
            {
                Dodge->TriggerPerfectDodge(Source);
            }
        }
        return 0.0f;
    }

    if (bParryWindowOpen)
    {
        bParryWindowOpen = false;
        ParryTimer = 0.0f;
        bGuardCounterOpen = true;
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(GuardCounterHandle, this, &UEclipseCombatComponent::CloseGuardCounter, 1.5f, false);
        }
        UE_LOG(LogEclipseCombat, Log, TEXT("[DAMAGE] attacker=%s target=%s amount=0.0 type=perfectparry"), *SourceName, *GetOwner()->GetName());
        // Guard impact reads as its own animation, not as a swing: no hit window is armed.
        if (PlayCombatMontage(bMontageDriven ? ParryMontage : nullptr, 0.0f, TEXT("parry")))
        {
            bSwingSwept = true;
        }
        DoHitStop(Feel.MediumHitStop);
        return 0.0f;
    }

    if (bBlocking)
    {
        float Mitigation = Feel.BlockDamageReduction;
        if (CombatSystem)
        {
            const FDefenseResult Result = CombatSystem->ProcessDefense(EDefensiveType::Block, 0.3f);
            Mitigation = FMath::Clamp(Result.DamageMitigated, 0.0f, 1.0f);
        }
        const float FinalDamage = InDamage * (1.0f - Mitigation);
        UE_LOG(LogEclipseCombat, Log, TEXT("[DAMAGE] attacker=%s target=%s amount=%.1f type=block (mitigated %.0f%%)"), *SourceName, *GetOwner()->GetName(), FinalDamage, Mitigation * 100.0f);
        DoHitStop(Feel.LightHitStop);
        return FinalDamage;
    }

    UE_LOG(LogEclipseCombat, Log, TEXT("[DAMAGE] attacker=%s target=%s amount=%.1f type=unstoppable"), *SourceName, *GetOwner()->GetName(), InDamage);
    PlayHitReaction(InDamage, Source ? Source->GetActorLocation() : GetOwner()->GetActorLocation());
    return InDamage;
}

void UEclipseCombatComponent::SetDamageBuff(float Multiplier, float DurationSeconds)
{
    DamageBuffMultiplier = FMath::Max(1.0f, Multiplier);
    BuffRemaining = FMath::Max(0.0f, DurationSeconds);
    UE_LOG(LogEclipseCombat, Log, TEXT("[COMBAT] Damage buff x%.2f for %.1fs"), DamageBuffMultiplier, BuffRemaining);
}

void UEclipseCombatComponent::ResetParryWindow()
{
    ParryTimer = 0.0f;
    bParryWindowOpen = false;
}

void UEclipseCombatComponent::PerformSweep(float DamageAmount, const FString& TypeLabel)
{
    AActor* Owner = GetOwner();
    UWorld* World = GetWorld();
    if (!Owner || !World)
    {
        return;
    }

    const FVector Start = Owner->GetActorLocation();
    const FVector End = Start + Owner->GetActorForwardVector() * AttackRange;
    const FCollisionShape Shape = FCollisionShape::MakeSphere(AttackRadius);
    FCollisionQueryParams Params(SCENE_QUERY_STAT(EclipseAttackSweep), false, Owner);

    TArray<FHitResult> Hits;
    World->SweepMultiByChannel(Hits, Start, End, FQuat::Identity, ECC_Pawn, Shape, Params);
    DrawDebugSphere(World, End, AttackRadius, 12, FColor::Magenta, false, 0.35f);

    TSet<AActor*> AlreadyHit;
    for (const FHitResult& Hit : Hits)
    {
        AActor* Target = Hit.GetActor();
        if (!Target || Target == Owner || AlreadyHit.Contains(Target))
        {
            continue;
        }
        if (Target->IsA(AEclipseCharacter::StaticClass()))
        {
            continue;
        }
        AlreadyHit.Add(Target);

        const float Applied = UGameplayStatics::ApplyDamage(Target, DamageAmount, Owner->GetInstigatorController(), Owner, UDamageType::StaticClass());
        LastDamageDealt = Applied;
        UE_LOG(LogEclipseCombat, Log, TEXT("[DAMAGE] attacker=%s target=%s amount=%.1f type=%s"), *Owner->GetName(), *Target->GetName(), Applied, *TypeLabel);
        OnHitLanded.Broadcast(Applied, false);

        // Equipment: single-affinity status buildup from the equipped weapon.
        if (AEclipseCharacter* OwnerChar = GetOwnerChar())
        {
            if (OwnerChar->EquipmentComponent)
            {
                OwnerChar->EquipmentComponent->HandleHit(Target);
            }
        }
    }

    if (AlreadyHit.Num() == 0)
    {
        UE_LOG(LogEclipseCombat, Log, TEXT("[COMBAT] %s attack whiffed (no target in %.0fcm)"), *TypeLabel, AttackRange);
    }

    DoHitStop(TypeLabel == TEXT("heavy") ? Feel.HeavyHitStop : Feel.LightHitStop);
}

void UEclipseCombatComponent::DoHitStop(float Seconds)
{
    UWorld* World = GetWorld();
    if (!World || Seconds <= 0.0f)
    {
        return;
    }

    constexpr float HitStopDilation = 0.05f;
    UGameplayStatics::SetGlobalTimeDilation(this, HitStopDilation);
    FTimerHandle StopHandle;
    World->GetTimerManager().SetTimer(StopHandle, this, &UEclipseCombatComponent::EndHitStop, FMath::Max(Seconds * HitStopDilation, 0.0001f), false);
}

void UEclipseCombatComponent::EndHitStop()
{
    // A perfect-dodge slow motion may still be running: restoring to 1.0 here would cancel it early.
    if (const AEclipseCharacter* OwnerChar = GetOwnerChar())
    {
        if (const UEclipsePerfectDodgeComponent* Dodge = OwnerChar->GetPerfectDodge())
        {
            if (Dodge->IsSlowMoActive())
            {
                UGameplayStatics::SetGlobalTimeDilation(this, Dodge->GetSlowMoDilation());
                return;
            }
        }
    }

    UGameplayStatics::SetGlobalTimeDilation(this, 1.0f);
}

void UEclipseCombatComponent::CloseGuardCounter()
{
    bGuardCounterOpen = false;
    UE_LOG(LogEclipseCombat, Log, TEXT("[COMBAT] Guard counter window closed"));
}

// ---------------------------------------------------------------------------------------------
// Montage layer
// ---------------------------------------------------------------------------------------------

UAnimInstance* UEclipseCombatComponent::GetOwnerAnimInstance() const
{
    const AEclipseCharacter* OwnerChar = Cast<AEclipseCharacter>(GetOwner());
    if (!OwnerChar)
    {
        return nullptr;
    }
    const USkeletalMeshComponent* SkelMesh = OwnerChar->GetMesh();
    return SkelMesh ? SkelMesh->GetAnimInstance() : nullptr;
}

UAnimMontage* UEclipseCombatComponent::PickLightAttackMontage() const
{
    if (LightAttackMontages.Num() == 0)
    {
        return nullptr;
    }
    const int32 Index = FMath::Clamp(ComboIndex, 0, LightAttackMontages.Num() - 1);
    return LightAttackMontages[Index];
}

bool UEclipseCombatComponent::PlayCombatMontage(UAnimMontage* Montage, float Damage, const FString& TypeLabel)
{
    if (!bMontageDriven || !Montage)
    {
        return false;
    }

    UAnimInstance* AnimInstance = GetOwnerAnimInstance();
    if (!AnimInstance)
    {
        return false;
    }

    PendingSweepDamage = Damage;
    PendingSweepLabel = TypeLabel;
    bSwingSwept = false;

    const float Length = AnimInstance->Montage_Play(Montage, 1.0f);
    if (Length <= 0.0f)
    {
        UE_LOG(LogEclipseCombat, Warning, TEXT("[ANIM] Montage_Play failed for %s - timer fallback"), *Montage->GetName());
        return false;
    }

    CurrentMontage = Montage;
    MontageDuration = Length;
    ActiveMontageName = Montage->GetName();
    bMontageActive = true;
    AttackTimer = 0.0f;

    UE_LOG(LogEclipseCombat, Log, TEXT("[ANIM] playing %s (%.2fs) for %s"), *ActiveMontageName, Length, *TypeLabel);
    return true;
}

void UEclipseCombatComponent::OnCombatMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (!Montage || Montage != CurrentMontage)
    {
        return;
    }

    bMontageActive = false;
    MontageDuration = 0.0f;

    if (State == ECombatState::LightAttack || State == ECombatState::HeavyAttack || State == ECombatState::Stagger)
    {
        State = bBlocking ? ECombatState::Blocking : ECombatState::Idle;
        AttackPhase = EAttackPhase::None;
        bStaggerFromMontage = false;
        StaggerTimer = 0.0f;
    }

    UE_LOG(LogEclipseCombat, Log, TEXT("[ANIM] %s finished (interrupted=%s)"), *ActiveMontageName, bInterrupted ? TEXT("YES") : TEXT("NO"));
    ActiveMontageName.Reset();
    CurrentMontage = nullptr;
}

bool UEclipseCombatComponent::InputCast(float InCastSeconds)
{
    if (State != ECombatState::Idle || bMontageActive)
    {
        UE_LOG(LogEclipseCombat, Log, TEXT("[COMBAT] Cast refused - already acting (state %d)"), (int32)State);
        return false;
    }

    AEclipseCharacter* OwnerChar = GetOwnerChar();
    if (!OwnerChar)
    {
        return false;
    }

    State = ECombatState::Casting;
    bCastActive = true;
    bCastInterrupted = false;
    bBlocking = false;
    CastDuration = FMath::Max(0.05f, InCastSeconds);
    CastElapsed = 0.0f;
    CastProgress = 0.0f;

    // A cast carries no weapon sweep - disarm it in case a CastPoint notify was authored on the montage.
    if (PlayCombatMontage(bMontageDriven ? CastMontage : nullptr, 0.0f, TEXT("cast")))
    {
        bSwingSwept = true;
    }
    else
    {
        UE_LOG(LogEclipseCombat, Log, TEXT("[ANIM] no montage for cast, cast bar runs alone"));
    }

    UE_LOG(LogEclipseCombat, Log, TEXT("[COMBAT] Cast started (%.2fs)"), CastDuration);
    return true;
}

void UEclipseCombatComponent::FinishCast(bool bCompleted)
{
    if (!bCastActive)
    {
        return;
    }

    bCastActive = false;
    if (bCompleted)
    {
        CastProgress = 1.0f;
    }

    if (State == ECombatState::Casting)
    {
        State = bBlocking ? ECombatState::Blocking : ECombatState::Idle;
    }

    UE_LOG(LogEclipseCombat, Log, TEXT("[COMBAT] Cast %s after %.2fs"),
        bCompleted ? TEXT("completed") : TEXT("interrupted"), CastElapsed);
    OnCastFinished.Broadcast(bCompleted);
}

bool UEclipseCombatComponent::PlayHitReaction(float InDamage, const FVector& SourceLocation)
{
    if (State == ECombatState::Stagger || bDodgeIFrameActive)
    {
        return false;
    }

    // Deliberate commitment: a swing already in flight is not cancelled by light chip damage.
    const bool bMidSwing = bMontageActive && (State == ECombatState::LightAttack || State == ECombatState::HeavyAttack);
    if (bMidSwing && InDamage < StaggerDamageThreshold)
    {
        return false;
    }

    if (bCastActive)
    {
        bCastInterrupted = true;
        FinishCast(false);
    }

    State = ECombatState::Stagger;
    bBlocking = false;

    UAnimMontage* Reaction = nullptr;
    if (HitReactionMontages.Num() > 0)
    {
        Reaction = HitReactionMontages[FMath::RandRange(0, HitReactionMontages.Num() - 1)];
    }

    if (PlayCombatMontage(bMontageDriven ? Reaction : nullptr, 0.0f, TEXT("hitreact")))
    {
        bSwingSwept = true;
        bStaggerFromMontage = true;
        LastReactionReason = Reaction->GetName();
        UE_LOG(LogEclipseCombat, Log, TEXT("[COMBAT] Stagger from %s (dmg %.1f) at %s"),
            *LastReactionReason, InDamage, *SourceLocation.ToCompactString());
        return true;
    }

    // No reaction asset: keep the poise break readable to gameplay with a timed lock instead.
    bStaggerFromMontage = false;
    StaggerTimer = StaggerFallbackSeconds;
    LastReactionReason = TEXT("timed-lock");
    UE_LOG(LogEclipseCombat, Log, TEXT("[ANIM] no hit reaction montage, %.2fs timed stagger"), StaggerFallbackSeconds);
    return false;
}

void UEclipseCombatComponent::OpenHitWindow()
{
    AttackPhase = EAttackPhase::Active;

    if (bSwingSwept)
    {
        UE_LOG(LogEclipseCombat, Warning, TEXT("[ANIM] hit window on %s ignored - swing %s already swept"),
            *ActiveMontageName, *PendingSweepLabel);
        return;
    }

    bSwingSwept = true;
    UE_LOG(LogEclipseCombat, Log, TEXT("[ANIM] hit window open: %s sweep %.1f"), *PendingSweepLabel, PendingSweepDamage);
    PerformSweep(PendingSweepDamage, PendingSweepLabel);
}

void UEclipseCombatComponent::OpenComboWindow()
{
    AttackPhase = EAttackPhase::Recovery;
    ComboWindowRemaining = Feel.ComboWindow;
    UE_LOG(LogEclipseCombat, Log, TEXT("[ANIM] combo window open (%.2fs) after chain=%d"), ComboWindowRemaining, ComboIndex + 1);
}
