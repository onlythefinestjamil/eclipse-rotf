// Eclipse Quest System — Main campaign, side quests, branching choices, consequences
// V6: 128+ quests, persistent consequences, faction reputation, world-state changes

#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EclipseQuestSystem.generated.h"

UENUM(BlueprintType)
enum class EQuestType : uint8
{
    MainCampaign   UMETA(DisplayName = "Main Campaign — Critical path, locked canon"),
    SideQuest      UMMETA(DisplayName = "Side Quest — Optional, world-building"),
    ClassQuest     UMMETA(DisplayName = "Class Quest — Class-specific progression"),
    FactionQuest   UMMETA(DisplayName = "Faction Quest — Reputation-based"),
    CompanionQuest  UMMETA(DisplayName = "Companion Quest — Companion loyalty arc"),
    BossQuest      UMMETA(DisplayName = "Boss Quest — Unlocks/progresses boss encounters"),
    WorldQuest      UMMETA(DisplayName = "World Quest — Repeatable, dynamic"),
    HiddenQuest    UMMETA(DisplayName = "Hidden Quest — Secret discovery"),
    FractureQuest   UMMETA(DisplayName = "Fracture Quest — Alters world state"),
};

UENUM(BlueprintType)
enum class EQuestState : uint8
{
    Locked         UMMETA(DisplayName = "Locked — Prerequisites not met"),
    Available      UMMETA(DisplayName = "Available — Can be accepted"),
    Active         UMMETA(DisplayName = "Active — In progress"),
    Completed      UMMETA(DisplayName = "Completed — Turned in, rewards given"),
    Failed         UMMETA(DisplayName = "Failed — Failed, may have consequences"),
    Abandoned      UMMETA(DisplayName = "Abandoned — Player gave up"),
};

UENUM(BlueprintType)
enum class EObjectiveType : uint8
{
    KillTarget      UMMETA(DisplayName = "Kill — Kill specific enemy/boss"),
    CollectItems    UMMETA(DisplayName = "Collect — Gather N items"),
    ReachLocation    UMMETA(DisplayName = "Reach — Travel to location"),
    TalkToNPC        UMMETA(DisplayName = "Talk — Speak with NPC"),
    EscortNPC        UMMETA(DisplayName = "Escort — Protect NPC from A to B"),
    DefendLocation   UMMETA(DisplayName = "Defend — Survive waves at location"),
    ActivateObject   UMMETA(DisplayName = "Activate — Interact with object/mechanism"),
    ChooseOption     UMMETA(DisplayName = "Choose — Branching dialogue choice"),
    SurviveTime      UMMETA(DisplayName = "Survive — Survive for N seconds"),
    DefeatBoss       UMMETA(DisplayName = "Boss — Defeat specific boss"),
    CraftItem        UMMETA(DisplayName = "Craft — Craft specific item"),
    DiscoverSecret   UMMETA(DisplayName = "Discover — Find hidden area/secret"),
};

USTRUCT(BlueprintType)
struct FQuestObjective
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) int32 ObjectiveID = 0;
    UPROPERTY(BlueprintReadWrite) FString Description = "";
    UPROPERTY(BlueprintReadWrite) EObjectiveType Type = EObjectiveType::KillTarget;
    UPROPERTY(BlueprintReadWrite) int32 RequiredAmount = 1;
    UPROPERTY(BlueprintReadWrite) int32 CurrentAmount = 0;
    UPROPERTY(BlueprintReadWrite) FString TargetName = ""; // NPC, enemy, item, location
    UPROPERTY(BlueprintReadWrite) FString TargetZone = "";
    UPROPERTY(BlueprintReadWrite) bool bIsOptional = false;
    UPROPERTY(BlueprintReadWrite) bool bIsComplete = false;
};

USTRUCT(BlueprintType)
struct FQuestChoice
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) int32 ChoiceID = 0;
    UPROPERTY(BlueprintReadWrite) FString PromptText = "";
    UPROPERTY(BlueprintReadWrite) FString ChoiceText = "";
    UPROPERTY(BlueprintReadWrite) FString ConsequenceText = "";
    UPROPERTY(BlueprintReadWrite) int32 NextQuestID = -1; // -1 = no branch
    UPROPERTY(BlueprintReadWrite) FString SetFlag = ""; // World flag to set
    UPROPERTY(BlueprintReadWrite) float FactionRepChange = 0.0f;
    UPROPERTY(BlueprintReadWrite) FString FactionName = "";
    UPROPERTY(BlueprintReadWrite) bool bLocksOtherQuests = false;
    UPROPERTY(BlueprintReadWrite) TArray<int32> LockedQuestIDs;
};

USTRUCT(BlueprintType)
struct FQuest
{
    GENERATED_BODY()

    // Identity
    UPROPERTY(BlueprintReadWrite) int32 QuestID = 0;
    UPROPERTY(BlueprintReadWrite) FString QuestName = "";
    UPROPERTY(BlueprintReadWrite) FString Description = "";
    UPROPERTY(BlueprintReadWrite) EQuestType Type = EQuestType::SideQuest;
    UPROPERTY(BlueprintReadWrite) EQuestState State = EQuestState::Locked;

    // Location
    UPROPERTY(BlueprintReadWrite) FString Zone = "";
    UPROPERTY(BlueprintReadWrite) FString GiverNPC = "";
    UPROPERTY(BlueprintReadWrite) FString TurnInNPC = "";

    // Prerequisites
    UPROPERTY(BlueprintReadWrite) TArray<int32> PrerequisiteQuestIDs;
    UPROPERTY(BlueprintReadWrite) int32 RequiredLevel = 1;
    UPROPERTY(BlueprintReadWrite) FString RequiredFlag = "";

    // Objectives
    UPROPERTY(BlueprintReadWrite) TArray<FQuestObjective> Objectives;
    UPROPERTY(BlueprintReadWrite) int32 CurrentObjectiveIndex = 0;

    // Choices (branching)
    UPROPERTY(BlueprintReadWrite) bool bHasChoice = false;
    UPROPERTY(BlueprintReadWrite) TArray<FQuestChoice> Choices;
    UPROPERTY(BlueprintReadWrite) int32 SelectedChoiceID = -1;

    // Rewards
    UPROPERTY(BlueprintReadWrite) int32 XPReward = 100;
    UPROPERTY(BlueprintReadWrite) int32 GoldReward = 50;
    UPROPERTY(BlueprintReadWrite) TArray<int32> ItemRewards;
    UPROPERTY(BlueprintReadWrite) float FactionRepReward = 0.0f;
    UPROPERTY(BlueprintReadWrite) FString FactionName = "";

    // Consequences
    UPROPERTY(BlueprintReadWrite) bool bAffectsWorldState = false;
    UPROPERTY(BlueprintReadWrite) FString WorldStateChange = "";
    UPROPERTY(BlueprintReadWrite) bool bAffectsNPCRelationship = false;
    UPROPERTY(BlueprintReadWrite) FString NPCRelationshipChange = "";

    // Fracture-specific
    UPROPERTY(BlueprintReadWrite) bool bIsFractureQuest = false;
    UPROPERTY(BlueprintReadWrite) FString FractureZone = ""; // Zone that changes
};

UCLASS(Blueprintable)
class ECLIPSEROTF_API UEclipseQuestSystem : public UObject
{
    GENERATED_BODY()

public:
    UEclipseQuestSystem();

    // Initialize all 128+ quests from V6
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Quest")
    void InitializeAllQuests();

    // Get all quests
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Quest")
    TArray<FQuest> GetAllQuests() const;

    // Get quest by ID
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Quest")
    FQuest GetQuest(int32 QuestID) const;

    // Get active quests
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Quest")
    TArray<FQuest> GetActiveQuests() const;

    // Get available quests (not yet accepted, prerequisites met)
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Quest")
    TArray<FQuest> GetAvailableQuests(int32 PlayerLevel, const TArray<int32>& CompletedQuestIDs) const;

    // Get completed quests
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Quest")
    TArray<int32> GetCompletedQuestIDs() const;

    // Get quest count
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Quest")
    int32 GetQuestCount() const;

    // Get quests by type
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Quest")
    TArray<FQuest> GetQuestsByType(EQuestType Type) const;

    // Get quests by zone
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Quest")
    TArray<FQuest> GetQuestsByZone(const FString& ZoneName) const;

    // === QUEST FLOW ===

    // Accept a quest
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Quest")
    bool AcceptQuest(int32 QuestID, int32 PlayerLevel, const TArray<int32>& CompletedQuestIDs);

    // Update objective progress
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Quest")
    void UpdateObjective(int32 QuestID, int32 ObjectiveID, int32 Amount);

    // Complete an objective
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Quest")
    void CompleteObjective(int32 QuestID, int32 ObjectiveID);

    // Make a choice (branching)
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Quest")
    bool MakeChoice(int32 QuestID, int32 ChoiceID);

    // Turn in quest (complete)
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Quest")
    bool TurnInQuest(int32 QuestID);

    // Abandon quest
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Quest")
    void AbandonQuest(int32 QuestID);

    // Fail quest
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Quest")
    void FailQuest(int32 QuestID);

    // Check if quest prerequisites are met
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Quest")
    bool ArePrerequisitesMet(int32 QuestID, int32 PlayerLevel, const TArray<int32>& CompletedQuestIDs) const;

    // Get quest state
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Quest")
    EQuestState GetQuestState(int32 QuestID) const;

    // Get current objective for a quest
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Quest")
    FQuestObjective GetCurrentObjective(int32 QuestID) const;

    // === FACTION REPUTATION ===

    // Get faction reputation
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Quest")
    float GetFactionRep(const FString& FactionName) const;

    // Add faction reputation
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Quest")
    void AddFactionRep(const FString& FactionName, float Amount);

    // Get all faction reputations
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Quest")
    TMap<FString, float> GetAllFactionReps() const;

    // === WORLD STATE ===

    // Get world flags
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Quest")
    TMap<FString, bool> GetWorldFlags() const;

    // Set world flag
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Quest")
    void SetWorldFlag(const FString& FlagName, bool Value);

    // Get world flag
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Quest")
    bool GetWorldFlag(const FString& FlagName) const;

    // === SAVE/LOAD ===

    // Get all quest states for saving
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Quest")
    TMap<int32, EQuestState> GetAllQuestStates() const;

    // Load quest states from save
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Quest")
    void LoadQuestStates(const TMap<int32, EQuestState>& InStates);

private:
    TArray<FQuest> AllQuests;
    TMap<int32, EQuestState> QuestStates;
    TMap<FString, float> FactionReputation;
    TMap<FString, bool> WorldFlags;

    void InitMainCampaign();
    void InitSideQuests();
    void InitClassQuests();
    void InitFactionQuests();
    void InitCompanionQuests();
    void InitBossQuests();
    void InitFractureQuests();
    void InitHiddenQuests();
};
