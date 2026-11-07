// Eclipse Companion AI System - AI companions that fill party slots
// Per directive: Same combat framework as human players, replaceable in real-time
// AI tanks manage threat, healers prioritize survival, DPS coordinate attacks
// Companions have personalities, builds, equipment, progression - NOT generic bots

#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "EclipseCharacter.h" // For EClass, EAncestry, EClassRole
#include "EclipseEnemyAI.h" // For EAIState
#include "EclipseCompanionAI.generated.h"

UENUM(BlueprintType)
enum class ECompanionPersonality : uint8
{
    Stoic         UMETA(DisplayName = "Stoic - calm, methodical, follows orders precisely"),
    Aggressive    UMETA(DisplayName = "Aggressive - pushes forward, takes risks, high DPS focus"),
    Protective     UMETA(DisplayName = "Protective - prioritizes healer safety, guards weak allies"),
    Tactical       UMETA(DisplayName = "Tactical - adapts to enemy patterns, calls out mechanics"),
    Reckless       UMETA(DisplayName = "Reckless - all-in, ignores danger, maximum damage"),
    Cautious        UMETA(DisplayName = "Cautious - defensive, conserves resources, waits for openings"),
    Devoted        UMETA(DisplayName = "Devoted - follows player closely, supports their decisions"),
    Independent    UMETA(DisplayName = "Independent - acts on own judgment, flanks, exploits weak points"),
};

UENUM(BlueprintType)
enum class ECompanionCommand : uint8
{
    Follow         UMETA(DisplayName = "Follow - trail player at 5m distance"),
    HoldPosition   UMETA(DisplayName = "Hold Position - stay at current location"),
    AttackTarget    UMETA(DisplayName = "Attack Target - focus player's current target"),
    DefendAlly      UMETA(DisplayName = "Defend Ally - protect a specific party member"),
    Retreat         UMETA(DisplayName = "Retreat - fall back, regroup, heal up"),
    UseAbility      UMETA(DisplayName = "Use Ability - execute specific class ability"),
    SpreadOut       UMETA(DisplayName = "Spread Out - avoid AoE positioning"),
    StackUp         UMETA(DisplayName = "Stack Up - group for healing/buffs"),
    FocusBoss       UMETA(DisplayName = "Focus Boss - all DPS on boss, ignore adds"),
    HandleMechanic  UMETA(DisplayName = "Handle Mechanic - assigned raid mechanic (interrupt, soak, etc.)"),
};

UENUM(BlueprintType)
enum class EPartySlot : uint8
{
    Player         UMETA(DisplayName = "Player - human controlled"),
    AICompanion     UMETA(DisplayName = "AI Companion - bot controlled, replaceable by human"),
    Empty           UMETA(DisplayName = "Empty - available for joining human or AI"),
};

UENUM(BlueprintType)
enum class ERaidRole : uint8
{
    MainTank       UMETA(DisplayName = "Main Tank - primary aggro holder"),
    OffTank        UMETA(DisplayName = "Off Tank - secondary aggro, mechanic handler"),
    MainHealer     UMETA(DisplayName = "Main Healer - primary healing"),
    SupportHealer   UMETA(DisplayName = "Support Healer - buffs, cleanses, spot healing"),
    MeleeDPS        UMETA(DisplayName = "Melee DPS - close range damage"),
    RangedDPS       UMETA(DisplayName = "Ranged DPS - long range damage"),
    CasterDPS       UMETA(DisplayName = "Caster DPS - spell damage"),
    Utility         UMETA(DisplayName = "Utility - interrupts, CC, mechanic handler"),
};

USTRUCT(BlueprintType)
struct FCompanionProfile
{
    GENERATED_BODY()

    // Identity
    UPROPERTY(BlueprintReadWrite) FString Name = "";
    UPROPERTY(BlueprintReadWrite) int32 SlotIndex = 0;
    UPROPERTY(BlueprintReadWrite) ECompanionPersonality Personality = ECompanionPersonality::Stoic;
    UPROPERTY(BlueprintReadWrite) EAncestry Ancestry = EAncestry::Solari;
    UPROPERTY(BlueprintReadWrite) EClass CharacterClass = EClass::Vanguard;
    UPROPERTY(BlueprintReadWrite) EClassRole Role = EClassRole::Tank;
    UPROPERTY(BlueprintReadWrite) ERaidRole RaidRole = ERaidRole::MainTank;
    
    // Progression (companions have their own builds)
    UPROPERTY(BlueprintReadWrite) int32 Level = 1;
    UPROPERTY(BlueprintReadWrite) FString WeaponName = "Iron Greatsword";
    UPROPERTY(BlueprintReadWrite) int32 WeaponLevel = 1;
    UPROPERTY(BlueprintReadWrite) TArray<FString> Equipment;
    UPROPERTY(BlueprintReadWrite) int32 TalentPoints = 0;
    UPROPERTY(BlueprintReadWrite) TArray<FString> UnlockedAbilities;
    
    // Stats (derived from class + level + equipment)
    UPROPERTY(BlueprintReadWrite) float Health = 100.0f;
    UPROPERTY(BlueprintReadWrite) float MaxHealth = 100.0f;
    UPROPERTY(BlueprintReadWrite) float Stamina = 100.0f;
    UPROPERTY(BlueprintReadWrite) float Mana = 50.0f;
    UPROPERTY(BlueprintReadWrite) float Flow = 0.0f;
    
    // Combat behavior
    UPROPERTY(BlueprintReadWrite) float AggressionLevel = 0.5f; // 0 = passive, 1 = aggressive
    UPROPERTY(BlueprintReadWrite) float SurvivalInstinct = 0.5f; // 0 = reckless, 1 = cautious
    UPROPERTY(BlueprintReadWrite) float MechanicAwareness = 0.8f; // 0 = ignores mechanics, 1 = perfect
    UPROPERTY(BlueprintReadWrite) float CoordinationScore = 0.7f; // 0 = loner, 1 = team player
};

USTRUCT(BlueprintType)
struct FPartySlot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) EPartySlot SlotType = EPartySlot::Empty;
    UPROPERTY(BlueprintReadWrite) FCompanionProfile Companion;
    UPROPERTY(BlueprintReadWrite) int32 SlotIndex = 0;
    UPROPERTY(BlueprintReadWrite) bool bIsHuman = false;
    UPROPERTY(BlueprintReadWrite) FString PlayerName = "";
    UPROPERTY(BlueprintReadWrite) bool bReady = false;
};

UCLASS(Blueprintable)
class ECLIPSEROTF_API UEclipseCompanionAI : public UObject
{
    GENERATED_BODY()

public:
    UEclipseCompanionAI();

    // Initialize companion with profile
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Companion")
    void InitializeCompanion(const FCompanionProfile& Profile, int32 SlotIndex);

    // Update companion behavior (called every frame)
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Companion")
    void UpdateCompanion(float DeltaTime, FVector PlayerLocation, FVector TargetLocation);

    // Receive tactical command from player
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Companion")
    void ReceiveCommand(ECompanionCommand Command, FVector CommandLocation = FVector::ZeroVector);

    // Get current action
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Companion")
    ECompanionCommand GetCurrentCommand() const;

    // Get companion profile
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Companion")
    FCompanionProfile GetProfile() const;

    // Is this slot AI or human?
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Companion")
    bool IsHumanControlled() const;

    // Can this slot be replaced by a human player?
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Companion")
    bool CanBeReplacedByHuman() const;

    // Replace AI with human player
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Companion")
    void ReplaceWithHuman(const FString& PlayerName);

    // Replace human with AI (player disconnects)
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Companion")
    void ReplaceWithAI();

    // Companion takes damage
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Companion")
    float TakeDamage(float Damage, bool bIsCrit);

    // Companion heals ally
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Companion")
    float HealAlly(float HealPower);

    // Companion uses ability
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Companion")
    void UseAbility(int32 AbilitySlot);

    // Get threat level (for tank role)
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Companion")
    float GetThreatLevel() const;

    // Get current state
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Companion")
    EAIState GetState() const;

private:
    FCompanionProfile Profile;
    ECompanionCommand CurrentCommand = ECompanionCommand::Follow;
    EAIState CurrentState = EAIState::Idle;
    FVector CurrentLocation = FVector::ZeroVector;
    FVector CommandLocation = FVector::ZeroVector;
    float AbilityCooldown = 0.0f;
    float ThreatLevel = 0.0f;
    float ReactionTime = 0.3f;
    float CommandHoldTime = 0.0f;
    bool bIsHuman = false;
    FString HumanPlayerName = "";

    void UpdateTankBehavior(float DeltaTime, FVector PlayerLocation, FVector TargetLocation);
    void UpdateHealerBehavior(float DeltaTime, FVector PlayerLocation, FVector TargetLocation);
    void UpdateDPSBehavior(float DeltaTime, FVector PlayerLocation, FVector TargetLocation);
    void UpdateUtilityBehavior(float DeltaTime, FVector PlayerLocation, FVector TargetLocation);
    void ProcessBossMechanic(float DeltaTime, FVector TargetLocation);
    void ReactToDanger(float DeltaTime, FVector DangerLocation);
    float CalculateOptimalPosition(FVector PlayerLocation, FVector TargetLocation) const;
};
