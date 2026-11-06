// Eclipse Targeting System Implementation
// Manual = guaranteed crits on weak points. Auto-lock = accessibility, normal damage.

#include "EclipseTargetingSystem.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"

UEclipseTargetingSystem::UEclipseTargetingSystem()
{
    CurrentMode = ETargetingMode::Manual;
    InitializeMultipliers();
}

void UEclipseTargetingSystem::InitializeMultipliers()
{
    // Crit multipliers per weak point type
    WeakPointMultipliers.Add(EWeakPointType::Head,          2.0f);
    WeakPointMultipliers.Add(EWeakPointType::Back,          2.0f);
    WeakPointMultipliers.Add(EWeakPointType::Joints,        1.5f);
    WeakPointMultipliers.Add(EWeakPointType::ExposedCore,   3.0f);
    WeakPointMultipliers.Add(EWeakPointType::Staggered,     2.5f);
    WeakPointMultipliers.Add(EWeakPointType::None,          1.0f); // No weak point = normal
}

void UEclipseTargetingSystem::SetTargetingMode(ETargetingMode Mode)
{
    CurrentMode = Mode;
    UE_LOG(LogTemp, Log, TEXT("[Targeting] Mode set to: %s"), *UEnum::GetValueAsString(Mode));

    // If switching to manual, unlock auto-target
    if (Mode == ETargetingMode::Manual || Mode == ETargetingMode::Off)
    {
        Unlock();
    }
}

ETargetingMode UEclipseTargetingSystem::GetTargetingMode() const
{
    return CurrentMode;
}

void UEclipseTargetingSystem::LockOn(const FTargetInfo& Target)
{
    LockedTarget = Target;
    bIsLockedOn = Target.bIsValid;
    UE_LOG(LogTemp, Log, TEXT("[Targeting] Locked on target at distance: %.1f"), Target.Distance);
}

void UEclipseTargetingSystem::Unlock()
{
    bIsLockedOn = false;
    LockedTarget.bIsValid = false;
    UE_LOG(LogTemp, Log, TEXT("[Targeting] Unlocked"));
}

bool UEclipseTargetingSystem::IsLockedOn() const
{
    return bIsLockedOn;
}

FTargetInfo UEclipseTargetingSystem::GetCurrentTarget() const
{
    return LockedTarget;
}

bool UEclipseTargetingSystem::ProcessManualAim(const FVector& AimDirection, const FTargetInfo& Target, float AimAccuracy)
{
    // Manual aim — check if the player's aim hits the weak point
    // AimAccuracy: 0.0 = perfect aim, 1.0 = complete miss

    if (!Target.bWeakPointExposed)
    {
        return false; // No weak point exposed — can't crit
    }

    // If aim accuracy is good enough (within threshold), it's a guaranteed crit
    float CritThreshold = 0.3f; // 0.3 or better = crit

    if (AimAccuracy <= CritThreshold)
    {
        UE_LOG(LogTemp, Log, TEXT("[Targeting] MANUAL WEAK POINT HIT! Accuracy: %.2f — GUARANTEED CRIT"), AimAccuracy);
        return true;
    }

    UE_LOG(LogTemp, Log, TEXT("[Targeting] Manual aim missed weak point. Accuracy: %.2f (need < %.2f)"), AimAccuracy, CritThreshold);
    return false;
}

float UEclipseTargetingSystem::ProcessAutoLockAttack(float BaseDamage)
{
    // Auto-lock attack — normal damage, NO guaranteed crit
    // Per V4: "without automating precision success"
    // Auto-lock helps you aim but does NOT guarantee crits

    UE_LOG(LogTemp, Log, TEXT("[Targeting] Auto-lock attack — normal damage: %.1f (no guaranteed crit)"), BaseDamage);
    return BaseDamage; // Normal damage only
}

float UEclipseTargetingSystem::ProcessManualAttack(float BaseDamage, const FVector& AimDirection, const FTargetInfo& Target)
{
    // Manual attack — can hit weak points for guaranteed crit
    // This is the high-skill path

    float Damage = BaseDamage;

    if (Target.bWeakPointExposed && Target.AvailableWeakPoint != EWeakPointType::None)
    {
        // Calculate aim accuracy based on how close the aim is to the weak point
        float AimAccuracy = CalculateAimAccuracy(AimDirection, Target.TargetLocation, Target.WeakPointHitboxSize);

        bool bHitWeakPoint = ProcessManualAim(AimDirection, Target, AimAccuracy);

        if (bHitWeakPoint)
        {
            float CritMultiplier = GetWeakPointMultiplier(Target.AvailableWeakPoint);
            Damage *= CritMultiplier;
            UE_LOG(LogTemp, Log, TEXT("[Targeting] MANUAL CRIT! Weak point: %s, Multiplier: %.1fx, Damage: %.1f -> %.1f"),
                *UEnum::GetValueAsString(Target.AvailableWeakPoint),
                CritMultiplier,
                BaseDamage,
                Damage);
        }
        else
        {
            // Hit the target but missed the weak point — normal damage
            UE_LOG(LogTemp, Log, TEXT("[Targeting] Hit target but missed weak point. Damage: %.1f"), Damage);
        }
    }
    else
    {
        // No weak point exposed — normal damage
        UE_LOG(LogTemp, Log, TEXT("[Targeting] No weak point exposed. Normal damage: %.1f"), Damage);
    }

    return Damage;
}

void UEclipseTargetingSystem::CycleTarget()
{
    if (NearbyTargets.Num() == 0) return;

    TargetIndex = (TargetIndex + 1) % NearbyTargets.Num();
    LockedTarget = NearbyTargets[TargetIndex];
    UE_LOG(LogTemp, Log, TEXT("[Targeting] Cycled to target %d/%d"), TargetIndex + 1, NearbyTargets.Num());
}

float UEclipseTargetingSystem::GetWeakPointMultiplier(EWeakPointType WeakPoint) const
{
    if (WeakPointMultipliers.Contains(WeakPoint))
    {
        return WeakPointMultipliers[WeakPoint];
    }
    return 1.0f;
}

bool UEclipseTargetingSystem::IsWeakPointExposed(const FTargetInfo& Target) const
{
    // Weak points are exposed when:
    // 1. Boss is in a vulnerable phase (ExposedCore)
    // 2. Enemy is staggered after poise break (Staggered)
    // 3. Shield is broken (Head/Back exposed)
    // 4. Enemy is attacking (Joints exposed during windup)

    if (Target.bIsStaggered)
    {
        return true; // Staggered = weak point exposed
    }

    if (Target.bHasShield && Target.ShieldHealth <= 0.0f)
    {
        return true; // Shield broken = weak point exposed
    }

    return Target.bWeakPointExposed;
}

float UEclipseTargetingSystem::CalculateAimAccuracy(const FVector& AimDirection, const FVector& WeakPointLocation, float WeakPointRadius)
{
    // Calculate how accurate the aim is
    // Returns 0.0 = perfect (dead center), 1.0 = complete miss

    FVector DirToWeakPoint = WeakPointLocation.GetSafeNormal();
    float DotProduct = FVector::DotProduct(AimDirection.GetSafeNormal(), DirToWeakPoint);

    // DotProduct: 1.0 = perfect, 0.0 = 90 degrees off, -1.0 = opposite direction
    // Convert to accuracy: 0.0 = perfect, 1.0 = worst
    float Accuracy = 1.0f - FMath::Clamp(DotProduct, 0.0f, 1.0f);

    // Factor in weak point radius — larger radius = more forgiving
    if (WeakPointRadius > 20.0f)
    {
        Accuracy *= 0.8f; // Larger weak point = easier to hit
    }
    else if (WeakPointRadius < 10.0f)
    {
        Accuracy *= 1.2f; // Smaller weak point = harder to hit
    }

    return FMath::Clamp(Accuracy, 0.0f, 1.0f);
}
