// Eclipse Targeting System — Manual + Auto Target
// Manual targeting = guaranteed crits on weak points (high skill)
// Auto-target = accessibility, normal damage, no guaranteed crit
// Per V4: "Optional target locking and contextual targeting assistance, without automating precision success"

#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EclipseTargetingSystem.generated.h"

UENUM(BlueprintType)
enum class ETargetingMode : uint8
{
    Manual          UMETA(DisplayName = "Manual — free aim, weak points = guaranteed crit"),
    AutoLock        UMETA(DisplayName = "Auto-Lock — locks nearest enemy, no guaranteed crit"),
    SoftLock        UMETA(DisplayName = "Soft-Lock — assists aim but allows manual override"),
    Off             UMETA(DisplayName = "Off — pure manual, no assistance"),
};

UENUM(BlueprintType)
enum class EWeakPointType : uint8
{
    Head            UMETA(DisplayName = "Head — 2x crit, smallest hitbox"),
    Back            UMETA(DisplayName = "Back — 2x crit, requires positioning"),
    Joints          UMETA(DisplayName = "Joints — 1.5x crit, armor gaps"),
    ExposedCore     UMETA(DisplayName = "Exposed Core — 3x crit, boss phase only"),
    Staggered       UMETA(DisplayName = "Staggered — 2.5x crit, after poise break"),
    None            UMETA(DisplayName = "None — no weak point available"),
};

USTRUCT(BlueprintType)
struct FTargetInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) bool bIsValid = false;
    UPROPERTY(BlueprintReadWrite) float Distance = 0.0f;
    UPROPERTY(BlueprintReadWrite) FVector TargetLocation = FVector::ZeroVector;
    UPROPERTY(BlueprintReadWrite) EWeakPointType AvailableWeakPoint = EWeakPointType::None;
    UPROPERTY(BlueprintReadWrite) float WeakPointHitboxSize = 20.0f; // cm radius
    UPROPERTY(BlueprintReadWrite) bool bWeakPointExposed = false;
    UPROPERTY(BlueprintReadWrite) bool bIsBoss = false;
    UPROPERTY(BlueprintReadWrite) bool bIsStaggered = false;
    UPROPERTY(BlueprintReadWrite) bool bHasShield = false;
    UPROPERTY(BlueprintReadWrite) float ShieldHealth = 0.0f;
};

UCLASS(Blueprintable)
class ECLIPSEROTF_API UEclipseTargetingSystem : public UObject
{
    GENERATED_BODY()

public:
    UEclipseTargetingSystem();

    // Set targeting mode
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Targeting")
    void SetTargetingMode(ETargetingMode Mode);

    // Get current targeting mode
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Targeting")
    ETargetingMode GetTargetingMode() const;

    // Lock onto a target (auto-lock or manual selection)
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Targeting")
    void LockOn(const FTargetInfo& Target);

    // Unlock current target
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Targeting")
    void Unlock();

    // Check if currently locked on
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Targeting")
    bool IsLockedOn() const;

    // Get current locked target
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Targeting")
    FTargetInfo GetCurrentTarget() const;

    // Process a manual aim — returns true if weak point hit (guaranteed crit)
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Targeting")
    bool ProcessManualAim(const FVector& AimDirection, const FTargetInfo& Target, float AimAccuracy);

    // Process auto-lock attack — always normal damage, no guaranteed crit
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Targeting")
    float ProcessAutoLockAttack(float BaseDamage);

    // Process manual attack — can hit weak points for guaranteed crit
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Targeting")
    float ProcessManualAttack(float BaseDamage, const FVector& AimDirection, const FTargetInfo& Target);

    // Switch to next target (for auto-lock cycling)
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Targeting")
    void CycleTarget();

    // Get crit multiplier for a weak point type
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Targeting")
    float GetWeakPointMultiplier(EWeakPointType WeakPoint) const;

    // Check if weak point is exposed (boss phase, staggered, shield broken)
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Targeting")
    bool IsWeakPointExposed(const FTargetInfo& Target) const;

    // Calculate aim accuracy — 0.0 = perfect, 1.0 = miss
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Targeting")
    float CalculateAimAccuracy(const FVector& AimDirection, const FVector& WeakPointLocation, float WeakPointRadius);

private:
    ETargetingMode CurrentMode = ETargetingMode::Manual;
    FTargetInfo LockedTarget;
    bool bIsLockedOn = false;
    int32 TargetIndex = 0;
    TArray<FTargetInfo> NearbyTargets;

    // Crit multipliers per weak point
    TMap<EWeakPointType, float> WeakPointMultipliers;

    void InitializeMultipliers();
};
