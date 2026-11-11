// Eclipse Combat System Implementation
// Hybrid precision combat — Elden Ring + WoW

#include "EclipseCombatSystem.h"
#include "EclipseCharacter.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

UEclipseCombatSystem::UEclipseCombatSystem()
{
    GCDTime = 1.5f; // Standard GCD
    GCDRemaining = 0.0f;
    FlowAmount = 0.0f;
    MaxFlow = 100.0f;
    ComboStep = 0;
    MaxComboSteps = 3;
    ComboWindow = 0.4f;
}

bool UEclipseCombatSystem::IsOnGCD() const
{
    return GCDRemaining > 0.0f;
}

void UEclipseCombatSystem::TriggerGCD(float Duration)
{
    GCDRemaining = (Duration > 0.0f) ? Duration : GCDTime;
}

void UEclipseCombatSystem::SetGCDForClass(EClass CharacterClass)
{
    // Fast classes get shorter GCD
    switch (CharacterClass)
    {
    case EClass::Duskblade:
    case EClass::Riftwalker:
    case EClass::Shadowblade:
        GCDTime = 1.0f; // Fast
        break;
    case EClass::Bulwark:
    case EClass::Warthane:
    case EClass::Berserker:
        GCDTime = 1.75f; // Slow, heavy weapons
        break;
    default:
        GCDTime = 1.5f; // Standard
        break;
    }
}

void UEclipseCombatSystem::AddFlow(float Amount)
{
    FlowAmount = FMath::Clamp(FlowAmount + Amount, 0.0f, MaxFlow);
}

bool UEclipseCombatSystem::SpendFlow(float Amount)
{
    if (FlowAmount >= Amount)
    {
        FlowAmount -= Amount;
        return true;
    }
    return false;
}

float UEclipseCombatSystem::GetFlow() const
{
    return FlowAmount;
}

float UEclipseCombatSystem::CalculateDamage(const FAttackData& Attack, bool bHitWeakPoint)
{
    float Damage = Attack.BaseDamage;

    if (bHitWeakPoint && Attack.bIsWeakPointHit)
    {
        // GUARANTEED critical hit — precision weak point strike
        // Per V4: "Successfully landing a manually controlled attack on a valid,
        // exposed weak point guarantees a critical strike"
        Damage *= 2.0f; // 100% crit multiplier
        UE_LOG(LogTemp, Log, TEXT("PRECISION CRIT! Damage: %.1f -> %.1f"), Attack.BaseDamage, Damage);
    }
    else
    {
        // Normal stat-based crit chance
        // Would roll against character's crit chance stat
        // For now, no auto-crit
    }

    return Damage;
}

FDefenseResult UEclipseCombatSystem::ProcessDefense(EDefensiveType DefenseType, float TimingWindow)
{
    FDefenseResult Result;

    // Perfect timing window: 0.0 - 0.2 seconds = perfect
    // 0.2 - 0.4 = good (partial)
    // > 0.4 = late (no benefit)

    bool bPerfectTiming = (TimingWindow >= 0.0f && TimingWindow <= 0.2f);

    switch (DefenseType)
    {
    case EDefensiveType::Dodge:
        if (bPerfectTiming)
        {
            Result.bPerfectDodge = true;
            // Perfect dodge = guaranteed avoidance (i-frames)
            AddFlow(10.0f); // Fracture Flow builds from perfect dodges
        }
        Result.StaminaCost = 15.0f;
        break;

    case EDefensiveType::Parry:
        if (bPerfectTiming)
        {
            Result.bPerfectParry = true;
            // Perfect parry = guaranteed deflection + riposte opening
            AddFlow(15.0f); // More Flow for parry (higher skill)
            Result.DamageMitigated = 1.0f; // 100% mitigated
        }
        else
        {
            Result.DamageMitigated = 0.5f; // Partial block if late
        }
        Result.StaminaCost = 25.0f;
        break;

    case EDefensiveType::Block:
        if (bPerfectTiming)
        {
            Result.bPerfectBlock = true;
            Result.DamageMitigated = 0.9f; // 90% mitigated
        }
        else
        {
            Result.DamageMitigated = 0.6f; // 60% normal block
        }
        Result.StaminaCost = 10.0f;
        break;

    case EDefensiveType::GuardBreak:
        Result.DamageMitigated = 0.0f;
        Result.StaminaCost = 40.0f;
        break;
    }

    return Result;
}

bool UEclipseCombatSystem::CanCombo() const
{
    return ComboStep < MaxComboSteps;
}

void UEclipseCombatSystem::AdvanceCombo()
{
    if (CanCombo())
    {
        ComboStep++;
        AddFlow(5.0f); // Flow builds from combos
    }
}

void UEclipseCombatSystem::ResetCombo()
{
    ComboStep = 0;
}
