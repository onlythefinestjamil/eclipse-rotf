// Eclipse Companion AI Implementation
// AI companions that fill party slots, same combat framework as human players
// Replaceable in real-time: AI slot -> human player -> AI (on disconnect)

#include "EclipseCompanionAI.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"

UEclipseCompanionAI::UEclipseCompanionAI()
{
    CurrentCommand = ECompanionCommand::Follow;
    CurrentState = EAIState::Idle;
    bIsHuman = false;
}

void UEclipseCompanionAI::InitializeCompanion(const FCompanionProfile& InProfile, int32 SlotIndex)
{
    Profile = InProfile;
    Profile.SlotIndex = SlotIndex;
    bIsHuman = false;
    
    // Adjust behavior based on personality
    switch (Profile.Personality)
    {
    case ECompanionPersonality::Stoic:
        Profile.AggressionLevel = 0.4f;
        Profile.SurvivalInstinct = 0.6f;
        Profile.MechanicAwareness = 0.9f;
        Profile.CoordinationScore = 0.8f;
        ReactionTime = 0.2f;
        break;
    case ECompanionPersonality::Aggressive:
        Profile.AggressionLevel = 0.9f;
        Profile.SurvivalInstinct = 0.2f;
        Profile.MechanicAwareness = 0.5f;
        Profile.CoordinationScore = 0.6f;
        ReactionTime = 0.1f;
        break;
    case ECompanionPersonality::Protective:
        Profile.AggressionLevel = 0.5f;
        Profile.SurvivalInstinct = 0.7f;
        Profile.MechanicAwareness = 0.8f;
        Profile.CoordinationScore = 0.9f;
        ReactionTime = 0.25f;
        break;
    case ECompanionPersonality::Tactical:
        Profile.AggressionLevel = 0.6f;
        Profile.SurvivalInstinct = 0.7f;
        Profile.MechanicAwareness = 0.95f;
        Profile.CoordinationScore = 0.85f;
        ReactionTime = 0.15f;
        break;
    case ECompanionPersonality::Reckless:
        Profile.AggressionLevel = 1.0f;
        Profile.SurvivalInstinct = 0.1f;
        Profile.MechanicAwareness = 0.3f;
        Profile.CoordinationScore = 0.4f;
        ReactionTime = 0.05f;
        break;
    case ECompanionPersonality::Cautious:
        Profile.AggressionLevel = 0.3f;
        Profile.SurvivalInstinct = 0.9f;
        Profile.MechanicAwareness = 0.85f;
        Profile.CoordinationScore = 0.7f;
        ReactionTime = 0.4f;
        break;
    case ECompanionPersonality::Devoted:
        Profile.AggressionLevel = 0.5f;
        Profile.SurvivalInstinct = 0.5f;
        Profile.MechanicAwareness = 0.8f;
        Profile.CoordinationScore = 1.0f;
        ReactionTime = 0.2f;
        break;
    case ECompanionPersonality::Independent:
        Profile.AggressionLevel = 0.7f;
        Profile.SurvivalInstinct = 0.6f;
        Profile.MechanicAwareness = 0.9f;
        Profile.CoordinationScore = 0.5f;
        ReactionTime = 0.15f;
        break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("[Companion] Initialized: %s (%s, %s) - Aggression: %.1f, Survival: %.1f, Mechanics: %.1f, Coordination: %.1f"),
        *Profile.Name, *UEnum::GetValueAsString(Profile.CharacterClass), *UEnum::GetValueAsString(Profile.Role),
        Profile.AggressionLevel, Profile.SurvivalInstinct, Profile.MechanicAwareness, Profile.CoordinationScore);
}

void UEclipseCompanionAI::UpdateCompanion(float DeltaTime, FVector PlayerLocation, FVector TargetLocation)
{
    if (bIsHuman) return; // Human player controls their own character

    CommandHoldTime += DeltaTime;
    if (AbilityCooldown > 0.0f) AbilityCooldown -= DeltaTime;

    float DistToPlayer = FVector::Dist(CurrentLocation, PlayerLocation);
    float DistToTarget = FVector::Dist(CurrentLocation, TargetLocation);

    // Process current command
    switch (CurrentCommand)
    {
    case ECompanionCommand::Follow:
        if (DistToPlayer > 500.0f)
        {
            FVector Dir = (PlayerLocation - CurrentLocation).GetSafeNormal();
            CurrentLocation += Dir * 400.0f * DeltaTime;
        }
        // Also engage target if within range
        if (DistToTarget < 300.0f && AbilityCooldown <= 0.0f)
        {
            UseAbility(0);
        }
        break;

    case ECompanionCommand::AttackTarget:
        // Move toward target and attack
        if (DistToTarget > 200.0f)
        {
            FVector Dir = (TargetLocation - CurrentLocation).GetSafeNormal();
            CurrentLocation += Dir * 500.0f * DeltaTime;
        }
        else if (AbilityCooldown <= 0.0f)
        {
            UseAbility(0);
        }
        break;

    case ECompanionCommand::HoldPosition:
        // Stay put, but attack if target comes close
        if (DistToTarget < 200.0f && AbilityCooldown <= 0.0f)
        {
            UseAbility(0);
        }
        break;

    case ECompanionCommand::DefendAlly:
        // Move to defend a specific ally
        if (DistToPlayer > 300.0f)
        {
            FVector Dir = (PlayerLocation - CurrentLocation).GetSafeNormal();
            CurrentLocation += Dir * 400.0f * DeltaTime;
        }
        break;

    case ECompanionCommand::Retreat:
        // Fall back from target
        if (DistToTarget < 800.0f)
        {
            FVector Dir = (CurrentLocation - TargetLocation).GetSafeNormal();
            CurrentLocation += Dir * 500.0f * DeltaTime;
        }
        break;

    case ECompanionCommand::SpreadOut:
        // Move away from other party members
        {
            FVector SpreadDir = (CurrentLocation - PlayerLocation).GetSafeNormal();
            CurrentLocation += SpreadDir * 300.0f * DeltaTime;
        }
        break;

    case ECompanionCommand::StackUp:
        // Move close to player
        if (DistToPlayer > 200.0f)
        {
            FVector Dir = (PlayerLocation - CurrentLocation).GetSafeNormal();
            CurrentLocation += Dir * 500.0f * DeltaTime;
        }
        break;

    case ECompanionCommand::FocusBoss:
        // All-in on boss target
        if (DistToTarget > 200.0f)
        {
            FVector Dir = (TargetLocation - CurrentLocation).GetSafeNormal();
            CurrentLocation += Dir * 600.0f * DeltaTime;
        }
        else if (AbilityCooldown <= 0.0f)
        {
            UseAbility(0);
        }
        break;

    case ECompanionCommand::HandleMechanic:
        // Move to assigned mechanic location
        {
            float DistToMechanic = FVector::Dist(CurrentLocation, CommandLocation);
            if (DistToMechanic > 100.0f)
            {
                FVector Dir = (CommandLocation - CurrentLocation).GetSafeNormal();
                CurrentLocation += Dir * 500.0f * DeltaTime;
            }
            else
            {
                UE_LOG(LogTemp, Log, TEXT("[Companion] %s handling raid mechanic at assigned position"), *Profile.Name);
            }
        }
        break;
    }

    // Role-specific behavior
    switch (Profile.Role)
    {
    case EClassRole::Tank:
        UpdateTankBehavior(DeltaTime, PlayerLocation, TargetLocation);
        break;
    case EClassRole::Healer:
        UpdateHealerBehavior(DeltaTime, PlayerLocation, TargetLocation);
        break;
    case EClassRole::DPS:
        UpdateDPSBehavior(DeltaTime, PlayerLocation, TargetLocation);
        break;
    case EClassRole::Support:
        UpdateUtilityBehavior(DeltaTime, PlayerLocation, TargetLocation);
        break;
    default:
        UpdateDPSBehavior(DeltaTime, PlayerLocation, TargetLocation);
        break;
    }

    // Boss mechanic awareness - dodge telegraphed attacks
    if (Profile.MechanicAwareness > 0.5f)
    {
        ProcessBossMechanic(DeltaTime, TargetLocation);
    }
}

void UEclipseCompanionAI::UpdateTankBehavior(float DeltaTime, FVector PlayerLocation, FVector TargetLocation)
{
    // Tank: maintain threat, position between boss and party, use defensive abilities
    ThreatLevel = 100.0f; // Tanks generate high threat
    
    // Position between boss and healer
    FVector MidPoint = (TargetLocation + PlayerLocation) * 0.5f;
    float DistToMid = FVector::Dist(CurrentLocation, MidPoint);
    if (DistToMid > 100.0f)
    {
        FVector Dir = (MidPoint - CurrentLocation).GetSafeNormal();
        CurrentLocation += Dir * 300.0f * DeltaTime;
    }
    
    // Use defensive ability when health drops
    if (Profile.Health < Profile.MaxHealth * 0.5f && AbilityCooldown <= 0.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("[Companion/Tank] %s using defensive ability (HP: %.0f/%.0f)"), 
            *Profile.Name, Profile.Health, Profile.MaxHealth);
        AbilityCooldown = 5.0f;
    }
}

void UEclipseCompanionAI::UpdateHealerBehavior(float DeltaTime, FVector PlayerLocation, FVector TargetLocation)
{
    // Healer: stay behind tank, prioritize healing, avoid aggro
    ThreatLevel = 10.0f; // Healers generate low threat
    
    // Stay at safe distance from boss
    float SafeDist = 600.0f;
    float DistToTarget = FVector::Dist(CurrentLocation, TargetLocation);
    if (DistToTarget < SafeDist)
    {
        FVector Dir = (CurrentLocation - TargetLocation).GetSafeNormal();
        CurrentLocation += Dir * 400.0f * DeltaTime;
    }
    
    // Heal allies when they're low
    if (Profile.CoordinationScore > 0.5f)
    {
        HealAlly(50.0f);
    }
}

void UEclipseCompanionAI::UpdateDPSBehavior(float DeltaTime, FVector PlayerLocation, FVector TargetLocation)
{
    // DPS: maximize damage while avoiding mechanics
    ThreatLevel = 50.0f; // DPS generates medium threat
    
    // Position at optimal range based on class
    float OptimalRange = 250.0f; // Melee
    if (Profile.Role == EClassRole::DPS && (Profile.CharacterClass == EClass::Arcanist || 
        Profile.CharacterClass == EClass::Stormcaller || Profile.CharacterClass == EClass::Necromancer))
    {
        OptimalRange = 600.0f; // Ranged/caster
    }
    
    float DistToTarget = FVector::Dist(CurrentLocation, TargetLocation);
    if (FMath::Abs(DistToTarget - OptimalRange) > 50.0f)
    {
        if (DistToTarget > OptimalRange)
        {
            FVector Dir = (TargetLocation - CurrentLocation).GetSafeNormal();
            CurrentLocation += Dir * 400.0f * DeltaTime;
        }
        else
        {
            FVector Dir = (CurrentLocation - TargetLocation).GetSafeNormal();
            CurrentLocation += Dir * 400.0f * DeltaTime;
        }
    }
    
    // Use high-damage abilities when available
    if (AbilityCooldown <= 0.0f && Profile.AggressionLevel > 0.3f)
    {
        UseAbility(0);
    }
}

void UEclipseCompanionAI::UpdateUtilityBehavior(float DeltaTime, FVector PlayerLocation, FVector TargetLocation)
{
    // Utility: interrupts, CC, mechanic handling
    ThreatLevel = 30.0f;
    
    // Watch for castbar / interruptable abilities
    if (Profile.MechanicAwareness > 0.7f && AbilityCooldown <= 0.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("[Companion/Utility] %s watching for interruptable casts"), *Profile.Name);
        AbilityCooldown = 8.0f;
    }
}

void UEclipseCompanionAI::ProcessBossMechanic(float DeltaTime, FVector TargetLocation)
{
    // Dodge telegraphed attacks based on mechanic awareness
    // Higher awareness = faster reaction = better dodge
    
    if (FMath::FRand() < Profile.MechanicAwareness * 0.1f * DeltaTime)
    {
        UE_LOG(LogTemp, Log, TEXT("[Companion] %s dodging telegraphed attack (awareness: %.2f)"), 
            *Profile.Name, Profile.MechanicAwareness);
    }
}

void UEclipseCompanionAI::ReactToDanger(float DeltaTime, FVector DangerLocation)
{
    // React to AoE/danger zones based on survival instinct
    float DangerDist = FVector::Dist(CurrentLocation, DangerLocation);
    
    if (DangerDist < 300.0f && Profile.SurvivalInstinct > 0.3f)
    {
        // Dodge away from danger
        FVector EscapeDir = (CurrentLocation - DangerLocation).GetSafeNormal();
        CurrentLocation += EscapeDir * 600.0f * DeltaTime;
        UE_LOG(LogTemp, Log, TEXT("[Companion] %s dodging danger (instinct: %.2f)"), 
            *Profile.Name, Profile.SurvivalInstinct);
    }
}

float UEclipseCompanionAI::CalculateOptimalPosition(FVector PlayerLocation, FVector TargetLocation) const
{
    // Calculate optimal position based on role, personality, and current situation
    float Score = 0.0f;
    
    // Tanks want to be between boss and party
    // Healers want to be at safe range
    // DPS want to be at their optimal range
    
    return Score;
}

void UEclipseCompanionAI::ReceiveCommand(ECompanionCommand Command, FVector InCommandLocation)
{
    CurrentCommand = Command;
    CommandLocation = InCommandLocation;
    CommandHoldTime = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("[Companion] %s received command: %d"), *Profile.Name, (int32)Command);
}

ECompanionCommand UEclipseCompanionAI::GetCurrentCommand() const
{
    return CurrentCommand;
}

FCompanionProfile UEclipseCompanionAI::GetProfile() const
{
    return Profile;
}

bool UEclipseCompanionAI::IsHumanControlled() const
{
    return bIsHuman;
}

bool UEclipseCompanionAI::CanBeReplacedByHuman() const
{
    return !bIsHuman;
}

void UEclipseCompanionAI::ReplaceWithHuman(const FString& PlayerName)
{
    bIsHuman = true;
    HumanPlayerName = PlayerName;
    UE_LOG(LogTemp, Log, TEXT("[Companion] %s replaced by human player: %s"), *Profile.Name, *PlayerName);
}

void UEclipseCompanionAI::ReplaceWithAI()
{
    bIsHuman = false;
    HumanPlayerName = "";
    CurrentCommand = ECompanionCommand::Follow;
    UE_LOG(LogTemp, Log, TEXT("[Companion] %s reverted to AI control (player disconnected)"), *Profile.Name);
}

float UEclipseCompanionAI::TakeDamage(float Damage, bool bIsCrit)
{
    float ActualDamage = bIsCrit ? Damage * 2.0f : Damage;
    Profile.Health -= ActualDamage;
    
    if (Profile.Health <= 0.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("[Companion] %s has fallen! (HP: 0/%.0f)"), *Profile.Name, Profile.MaxHealth);
        CurrentState = EAIState::Dead;
    }
    
    return ActualDamage;
}

float UEclipseCompanionAI::HealAlly(float HealPower)
{
    float ActualHeal = HealPower * (0.8f + Profile.CoordinationScore * 0.2f);
    UE_LOG(LogTemp, Log, TEXT("[Companion] %s healing ally for %.1f"), *Profile.Name, ActualHeal);
    return ActualHeal;
}

void UEclipseCompanionAI::UseAbility(int32 AbilitySlot)
{
    AbilityCooldown = 3.0f + FMath::FRand() * 2.0f;
    
    // Different abilities based on class and role
    FString AbilityName = "Attack";
    
    if (Profile.Role == EClassRole::Tank)
    {
        AbilityName = "Shield Bash";
        ThreatLevel += 20.0f;
    }
    else if (Profile.Role == EClassRole::Healer)
    {
        AbilityName = "Healing Light";
    }
    else if (Profile.Role == EClassRole::DPS)
    {
        float Damage = 50.0f + Profile.Level * 5.0f;
        AbilityName = FString::Printf(TEXT("Attack (%.0f dmg)"), Damage);
        ThreatLevel += 10.0f;
    }
    else if (Profile.Role == EClassRole::Support)
    {
        AbilityName = "Interrupt";
    }
    
    UE_LOG(LogTemp, Log, TEXT("[Companion] %s uses %s (threat: %.0f)"), *Profile.Name, *AbilityName, ThreatLevel);
}

float UEclipseCompanionAI::GetThreatLevel() const
{
    return ThreatLevel;
}

EAIState UEclipseCompanionAI::GetState() const
{
    return CurrentState;
}
