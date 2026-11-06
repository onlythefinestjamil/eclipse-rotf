// Eclipse Enemy AI Implementation
// Fractured enemies, boss, AI states, combat

#include "EclipseEnemyAI.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"

UEclipseEnemyAI::UEclipseEnemyAI()
{
    InitializeEnemy(EEnemyType::FracturedDrone, 1);
}

void UEclipseEnemyAI::InitializeEnemy(EEnemyType Type, int32 Level)
{
    Stats.Type = Type;
    
    // Base stats scaled by level
    float LevelScale = 1.0f + (Level - 1) * 0.15f;
    
    switch (Type)
    {
    case EEnemyType::FracturedDrone:
        Stats.Health = 80.0f * LevelScale;
        Stats.MaxHealth = Stats.Health;
        Stats.Damage = 15.0f * LevelScale;
        Stats.MoveSpeed = 300.0f;
        Stats.AttackRange = 200.0f;
        Stats.AttackSpeed = 1.2f;
        Stats.Poise = 30.0f;
        Stats.MaxPoise = 30.0f;
        Stats.DetectionRange = 1500.0f;
        Stats.KillRewardFlow = 5;
        Stats.KillRewardGold = 10;
        break;

    case EEnemyType::FracturedStalker:
        Stats.Health = 60.0f * LevelScale;
        Stats.MaxHealth = Stats.Health;
        Stats.Damage = 20.0f * LevelScale;
        Stats.MoveSpeed = 500.0f;
        Stats.AttackRange = 180.0f;
        Stats.AttackSpeed = 1.5f;
        Stats.Poise = 20.0f;
        Stats.MaxPoise = 20.0f;
        Stats.DetectionRange = 2000.0f;
        Stats.KillRewardFlow = 8;
        Stats.KillRewardGold = 15;
        break;

    case EEnemyType::FracturedBrute:
        Stats.Health = 200.0f * LevelScale;
        Stats.MaxHealth = Stats.Health;
        Stats.Damage = 35.0f * LevelScale;
        Stats.MoveSpeed = 200.0f;
        Stats.AttackRange = 250.0f;
        Stats.AttackSpeed = 0.8f;
        Stats.Poise = 80.0f;
        Stats.MaxPoise = 80.0f;
        Stats.DetectionRange = 1200.0f;
        Stats.KillRewardFlow = 15;
        Stats.KillRewardGold = 30;
        break;

    case EEnemyType::FracturedCaster:
        Stats.Health = 70.0f * LevelScale;
        Stats.MaxHealth = Stats.Health;
        Stats.Damage = 25.0f * LevelScale;
        Stats.MoveSpeed = 250.0f;
        Stats.AttackRange = 800.0f;
        Stats.AttackSpeed = 1.0f;
        Stats.Poise = 25.0f;
        Stats.MaxPoise = 25.0f;
        Stats.DetectionRange = 1800.0f;
        Stats.KillRewardFlow = 10;
        Stats.KillRewardGold = 20;
        break;

    case EEnemyType::FracturedElite:
        Stats.Health = 400.0f * LevelScale;
        Stats.MaxHealth = Stats.Health;
        Stats.Damage = 40.0f * LevelScale;
        Stats.MoveSpeed = 350.0f;
        Stats.AttackRange = 220.0f;
        Stats.AttackSpeed = 1.1f;
        Stats.Poise = 120.0f;
        Stats.MaxPoise = 120.0f;
        Stats.DetectionRange = 2500.0f;
        Stats.KillRewardFlow = 30;
        Stats.KillRewardGold = 50;
        break;

    case EEnemyType::Boss:
        Stats.Health = 2000.0f * LevelScale;
        Stats.MaxHealth = Stats.Health;
        Stats.Damage = 60.0f * LevelScale;
        Stats.MoveSpeed = 280.0f;
        Stats.AttackRange = 300.0f;
        Stats.AttackSpeed = 0.9f;
        Stats.Poise = 200.0f;
        Stats.MaxPoise = 200.0f;
        Stats.DetectionRange = 3000.0f;
        Stats.KillRewardFlow = 50;
        Stats.KillRewardGold = 200;
        break;
    }

    CurrentState = EAIState::Patrol;
    bIsDead = false;
    StateTimer = 0.0f;
    AttackCooldown = 0.0f;
    StunDuration = 0.0f;
}

void UEclipseEnemyAI::SetState(EAIState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        StateTimer = 0.0f;
        UE_LOG(LogTemp, Log, TEXT("[EnemyAI] State changed to %d"), (int32)NewState);
    }
}

void UEclipseEnemyAI::UpdateAI(float DeltaTime, FVector PlayerLocation)
{
    if (bIsDead) return;

    StateTimer += DeltaTime;
    if (AttackCooldown > 0.0f) AttackCooldown -= DeltaTime;

    float DistToPlayer = FVector::Dist(CurrentLocation, PlayerLocation);

    // Check if stunned
    if (CurrentState == EAIState::Stunned)
    {
        UpdateStunned(DeltaTime);
        return;
    }

    // Check detection
    if (CurrentState == EAIState::Patrol || CurrentState == EAIState::Idle)
    {
        if (DistToPlayer <= Stats.DetectionRange)
        {
            SetState(EAIState::Aware);
        }
        else
        {
            UpdatePatrol(DeltaTime);
            return;
        }
    }

    // Aware -> Aggressive
    if (CurrentState == EAIState::Aware)
    {
        if (DistToPlayer <= Stats.AttackRange + 50.0f)
        {
            SetState(EAIState::Aggressive);
        }
        else
        {
            // Move toward player
            FVector Dir = (PlayerLocation - CurrentLocation).GetSafeNormal();
            CurrentLocation += Dir * Stats.MoveSpeed * DeltaTime;
        }
    }

    // Aggressive - attack
    if (CurrentState == EAIState::Aggressive)
    {
        UpdateAggressive(DeltaTime, PlayerLocation);
    }
}

void UEclipseEnemyAI::UpdatePatrol(float DeltaTime)
{
    // Simple patrol - wander around patrol center
    if (StateTimer > 3.0f)
    {
        // Pick new patrol direction
        StateTimer = 0.0f;
    }
    
    // Slow wandering
    FVector RandomDir = FVector(FMath::FRand() - 0.5f, FMath::FRand() - 0.5f, 0.0f);
    RandomDir.Normalize();
    CurrentLocation += RandomDir * Stats.MoveSpeed * 0.3f * DeltaTime;
}

void UEclipseEnemyAI::UpdateAggressive(float DeltaTime, FVector PlayerLocation)
{
    float DistToPlayer = FVector::Dist(CurrentLocation, PlayerLocation);

    if (DistToPlayer > Stats.AttackRange)
    {
        // Move toward player
        FVector Dir = (PlayerLocation - CurrentLocation).GetSafeNormal();
        CurrentLocation += Dir * Stats.MoveSpeed * DeltaTime;
    }
    else if (AttackCooldown <= 0.0f)
    {
        // Attack!
        UE_LOG(LogTemp, Log, TEXT("[EnemyAI] Attack! Damage: %.1f"), Stats.Damage);
        AttackCooldown = 1.0f / Stats.AttackSpeed;
    }
}

void UEclipseEnemyAI::UpdateStunned(float DeltaTime)
{
    StunDuration -= DeltaTime;
    if (StunDuration <= 0.0f)
    {
        SetState(EAIState::Aggressive);
        Stats.Poise = Stats.MaxPoise; // Reset poise
    }
}

float UEclipseEnemyAI::TakeDamage(float Damage, bool bHitWeakPoint, bool bIsCrit)
{
    if (bIsDead) return 0.0f;

    float ActualDamage = Damage;
    if (bIsCrit) ActualDamage *= 2.0f;
    if (bHitWeakPoint) ActualDamage *= 1.5f;

    Stats.Health -= ActualDamage;
    Stats.Poise -= ActualDamage * 0.5f;

    UE_LOG(LogTemp, Log, TEXT("[EnemyAI] Took %.1f damage (HP: %.0f/%.0f, Poise: %.0f/%.0f)"),
        ActualDamage, Stats.Health, Stats.MaxHealth, Stats.Poise, Stats.MaxPoise);

    // Check poise break
    if (Stats.Poise <= 0.0f)
    {
        BreakPoise(0.0f);
    }

    // Check death
    if (Stats.Health <= 0.0f)
    {
        bIsDead = true;
        SetState(EAIState::Dead);
        UE_LOG(LogTemp, Log, TEXT("[EnemyAI] Enemy killed! Reward: %d Flow, %d Gold"),
            Stats.KillRewardFlow, Stats.KillRewardGold);
    }

    return ActualDamage;
}

bool UEclipseEnemyAI::BreakPoise(float PoiseDamage)
{
    Stats.Poise -= PoiseDamage;
    if (Stats.Poise <= 0.0f)
    {
        StunDuration = 3.0f;
        SetState(EAIState::Stunned);
        UE_LOG(LogTemp, Log, TEXT("[EnemyAI] POISE BROKEN! Stunned for 3s"));
        return true;
    }
    return false;
}

EAIState UEclipseEnemyAI::GetState() const
{
    return CurrentState;
}

FEnemyStats UEclipseEnemyAI::GetStats() const
{
    return Stats;
}

bool UEclipseEnemyAI::IsDead() const
{
    return bIsDead;
}

void UEclipseEnemyAI::Reset()
{
    InitializeEnemy(Stats.Type, 1);
    CurrentLocation = PatrolCenter;
}
