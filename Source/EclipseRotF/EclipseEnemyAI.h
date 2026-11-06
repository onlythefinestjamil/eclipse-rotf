// Eclipse Enemy AI System - Enemy types, spawning, behavior
// Per V7: one Fractured enemy in the first vertical slice

#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EclipseEnemyAI.generated.h"

UENUM(BlueprintType)
enum class EEnemyType : uint8
{
    FracturedDrone   UMETA(DisplayName = "Fractured Drone - basic void mimic, melee"),
    FracturedStalker UMETA(DisplayName = "Fractured Stalker - fast, aggressive, dodge-capable"),
    FracturedBrute   UMETA(DisplayName = "Fractured Brute - heavy, slow, high damage"),
    FracturedCaster  UMETA(DisplayName = "Fractured Caster - ranged void attacks"),
    FracturedElite   UMETA(DisplayName = "Fractured Elite - mini-boss, multiple abilities"),
    Boss             UMETA(DisplayName = "Boss - unique boss with phases"),
};

UENUM(BlueprintType)
enum class EAIState : uint8
{
    Idle            UMETA(DisplayName = "Idle - not in combat"),
    Patrol          UMETA(DisplayName = "Patrol - wandering"),
    Aware            UMETA(DisplayName = "Aware - detected player, approaching"),
    Aggressive      UMETA(DisplayName = "Aggressive - attacking"),
    Defensive       UMETA(DisplayName = "Defensive - blocking/dodging"),
    Stunned          UMETA(DisplayName = "Stunned - from parry or poise break"),
    Dead            UMETA(DisplayName = "Dead"),
};

USTRUCT(BlueprintType)
struct FEnemyStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) float Health = 100.0f;
    UPROPERTY(BlueprintReadWrite) float MaxHealth = 100.0f;
    UPROPERTY(BlueprintReadWrite) float Damage = 20.0f;
    UProperty(BlueprintReadWrite) float MoveSpeed = 300.0f;
    UProperty(BlueprintReadWrite) float AttackRange = 200.0f;
    UProperty(BlueprintReadWrite) float AttackSpeed = 1.0f;
    UProperty(BlueprintReadWrite) float Poise = 50.0f;
    UProperty(BlueprintReadWrite) float MaxPoise = 50.0f;
    UProperty(BlueprintReadWrite) float DetectionRange = 1500.0f;
    UProperty(BlueprintReadWrite) int32 KillRewardFlow = 5;
    UProperty(BlueprintReadWrite) int32 KillRewardGold = 10;
    UProperty(BlueprintReadWrite) EEnemyType Type = EEnemyType::FracturedDrone;
};

UCLASS(Blueprintable)
class ECLIPSEROTF_API UEclipseEnemyAI : public UObject
{
    GENERATED_BODY()

public:
    UEclipseEnemyAI();

    // Initialize enemy with type-specific stats
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Enemy")
    void InitializeEnemy(EEnemyType Type, int32 Level = 1);

    // Update AI state
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Enemy")
    void UpdateAI(float DeltaTime, FVector PlayerLocation);

    // Take damage
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Enemy")
    float TakeDamage(float Damage, bool bHitWeakPoint, bool bIsCrit);

    // Stagger/break poise
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Enemy")
    bool BreakPoise(float PoiseDamage);

    // Get current state
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Enemy")
    EAIState GetState() const;

    // Get enemy stats
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Enemy")
    FEnemyStats GetStats() const;

    // Is dead?
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Enemy")
    bool IsDead() const;

    // Reset
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Enemy")
    void Reset();

private:
    FEnemyStats Stats;
    EAIState CurrentState = EAIState::Idle;
    FVector CurrentLocation = FVector::ZeroVector;
    FVector PatrolCenter = FVector::ZeroVector;
    float StateTimer = 0.0f;
    float AttackCooldown = 0.0f;
    float StunDuration = 0.0f;
    bool bIsDead = false;

    void SetState(EAIState NewState);
    void UpdatePatrol(float DeltaTime);
    void UpdateAggressive(float DeltaTime, FVector PlayerLocation);
    void UpdateStunned(float DeltaTime);
};
