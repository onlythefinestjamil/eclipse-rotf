// Eclipse Combat System — Hybrid Precision + GCD + Fracture Flow
// Combines Elden Ring action control with WoW ability depth
// Manual precision rewards + guaranteed crits + perfect defense

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EclipseCombatSystem.generated.h"

UENUM(BlueprintType)
enum class EAttackType : uint8
{
    Light        UMETA(DisplayName = "Light Attack"),
    Heavy        UMETA(DisplayName = "Heavy Attack"),
    Charged      UMETA(DisplayName = "Charged Attack"),
    WeaponArt    UMETA(DisplayName = "Weapon Art"),
    Spell        UMETA(DisplayName = "Spell"),
    Defensive    UMETA(DisplayName = "Defensive Action"),
};

UENUM(BlueprintType)
enum class EDefensiveType : uint8
{
    Dodge        UMETA(DisplayName = "Dodge"),
    Parry        UMETA(DisplayName = "Parry"),
    Block        UMETA(DisplayName = "Block"),
    GuardBreak   UMETA(DisplayName = "Guard Break"),
};

UENUM(BlueprintType)
enum class ECritType : uint8
{
    Normal       UMETA(DisplayName = "Normal — stat-based"),
    Precision    UMETA(DisplayName = "Precision — guaranteed crit on weak point"),
};

USTRUCT(BlueprintType)
struct FAttackData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) EAttackType AttackType;
    UPROPERTY(BlueprintReadWrite) float BaseDamage = 100.0f;
    UPROPERTY(BlueprintReadWrite) float StaminaCost = 20.0f;
    UPROPERTY(BlueprintReadWrite) float PoiseDamage = 50.0f;
    UPROPERTY(BlueprintReadWrite) float Range = 200.0f;
    UPROPERTY(BlueprintReadWrite) float WindupTime = 0.18f; // Anticipation
    UPROPERTY(BlueprintReadWrite) float ActiveTime = 0.12f; // Hit window
    UPROPERTY(BlueprintReadWrite) float RecoveryTime = 0.38f; // Recovery
    UPROPERTY(BlueprintReadWrite) bool bIsOffGCD = false; // Off-GCD for defensives
    UPROPERTY(BlueprintReadWrite) bool bIsWeakPointHit = false;
};

USTRUCT(BlueprintType)
struct FDefenseResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) bool bPerfectDodge = false;
    UPROPERTY(BlueprintReadWrite) bool bPerfectParry = false;
    UPROPERTY(BlueprintReadWrite) bool bPerfectBlock = false;
    UPROPERTY(BlueprintReadWrite) float DamageMitigated = 0.0f;
    UPROPERTY(BlueprintReadWrite) float StaminaCost = 15.0f;
};

UCLASS(Blueprintable)
class ECLIPSEROTF_API UEclipseCombatSystem : public UObject
{
    GENERATED_BODY()

public:
    UEclipseCombatSystem();

    // GCD
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Combat")
    bool IsOnGCD() const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Combat")
    void TriggerGCD(float Duration = 1.5f);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Combat")
    void SetGCDForClass(EClass CharacterClass);

    // Fracture Flow
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Combat")
    void AddFlow(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Combat")
    bool SpendFlow(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Combat")
    float GetFlow() const;

    // Precision attacks — guaranteed crits on weak points
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Combat")
    float CalculateDamage(const FAttackData& Attack, bool bHitWeakPoint);

    // Defense — perfect dodge/parry/block
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Combat")
    FDefenseResult ProcessDefense(EDefensiveType DefenseType, float TimingWindow);

    // Combo system
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Combat")
    bool CanCombo() const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Combat")
    void AdvanceCombo();

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Combat")
    void ResetCombo();

private:
    float GCDTime = 0.0f;
    float GCDRemaining = 0.0f;
    float FlowAmount = 0.0f;
    float MaxFlow = 100.0f;
    int32 ComboStep = 0;
    int32 MaxComboSteps = 3;
    float ComboWindow = 0.4f; // 400ms combo buffer
};
