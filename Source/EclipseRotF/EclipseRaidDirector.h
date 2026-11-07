// Eclipse Raid Director — 5-player dungeons, 10-player raids
// Manages encounter flow, party composition, wipe detection, loot distribution
// Per V6 directive: AI companions fill slots, human replacement without rebuilding encounters

#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "EclipseBossDirector.h"
#include "EclipsePartySystem.h"
#include "EclipseLootSystem.h"
#include "EclipseRaidDirector.generated.h"

UENUM(BlueprintType)
enum class EEncounterType : uint8
{
    Solo            UMETA(DisplayName = "Solo — 1 player, no companions"),
    Dungeon         UMETA(DisplayName = "Dungeon — 5 players (1 Tank, 1 Healer, 3 DPS)"),
    Raid            UMETA(DisplayName = "Raid — 10 players (2 Tanks, 2 Healers, 6 DPS)"),
    WorldBoss       UMETA(DisplayName = "World Boss — Open world, 1-10 players"),
    Arena           UMETA(DisplayName = "Arena — PvP 1v1, 2v2, 3v3, 5v5"),
};

UENUM(BlueprintType)
enum class EEncounterState : uint8
{
    Inactive        UMETA(DisplayName = "Inactive — No encounter running"),
    Setup           UMETA(DisplayName = "Setup — Party formation, role assignment"),
    Ready           UMETA(DisplayName = "Ready — Party formed, waiting to start"),
    InProgress       UMETA(DisplayName = "In Progress — Encounter active"),
    Wipe             UMETA(DisplayName = "Wipe — Party defeated, respawn at checkpoint"),
    Victory          UMETA(DisplayName = "Victory — Boss defeated, loot distribution"),
    Complete         UMETA(DisplayName = "Complete — Loot distributed, encounter ended"),
};

USTRUCT(BlueprintType)
struct FEncounterConfig
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) int32 EncounterID = 0;
    UPROPERTY(BlueprintReadWrite) FString EncounterName = "";
    UPROPERTY(BlueprintReadWrite) EEncounterType Type = EEncounterType::Dungeon;
    UPROPERTY(BlueprintReadWrite) int32 BossID = 0; // Linked boss from BossDirector
    UPROPERTY(BlueprintReadWrite) FString ZoneName = "";
    UPROPERTY(BlueprintReadWrite) FString DungeonName = "";
    UPROPERTY(BlueprintReadWrite) int32 RecommendedLevel = 1;
    UPROPERTY(BlueprintReadWrite) float TimeLimit = -1.0f; // -1 = no time limit
    UPROPERTY(BlueprintReadWrite) int32 MaxRevives = 3; // Revives before wipe
    UPROPERTY(BlueprintReadWrite) bool bAllowAICompanions = true;
    UPROPERTY(BlueprintReadWrite) bool bAllowHumanReplacement = true; // Mid-encounter slot replacement
    UPROPERTY(BlueprintReadWrite) bool bIsFractureEncounter = false; // Uses fracture shift mechanics
};

USTRUCT(BlueprintType)
struct FEncounterResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) bool bVictory = false;
    UPROPERTY(BlueprintReadWrite) float Duration = 0.0f;
    UPROPERTY(BlueprintReadWrite) int32 PartySize = 0;
    UPROPERTY(BlueprintReadWrite) int32 AICount = 0;
    UPROPERTY(BlueprintReadWrite) int32 HumanCount = 0;
    UPROPERTY(BlueprintReadWrite) int32 Deaths = 0;
    UPROPERTY(BlueprintReadWrite) int32 RevivesUsed = 0;
    UPROPERTY(BlueprintReadWrite) TArray<FLootDrop> LootDistributed;
    UPROPERTY(BlueprintReadWrite) int32 BossID = 0;
    UPROPERTY(BlueprintReadWrite) FString EncounterName = "";
};

UCLASS(Blueprintable)
class ECLIPSEROTF_API UEclipseRaidDirector : public UObject
{
    GENERATED_BODY()

public:
    UEclipseRaidDirector();

    // Initialize all dungeons and raids from V6
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Raid")
    void InitializeEncounters();

    // Get all encounters
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Raid")
    TArray<FEncounterConfig> GetAllEncounters() const;

    // Get encounter by ID
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Raid")
    FEncounterConfig GetEncounter(int32 EncounterID) const;

    // Get encounters by type
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Raid")
    TArray<FEncounterConfig> GetEncountersByType(EEncounterType Type) const;

    // Get encounter count
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Raid")
    int32 GetEncounterCount() const;

    // === ENCOUNTER FLOW ===

    // Start an encounter
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Raid")
    bool StartEncounter(int32 EncounterID, int32 PartySize);

    // Update encounter state (call every frame)
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Raid")
    void UpdateEncounter(float DeltaTime);

    // Get current state
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Raid")
    EEncounterState GetState() const;

    // Get active encounter
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Raid")
    FEncounterConfig GetActiveEncounter() const;

    // Get encounter result
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Raid")
    FEncounterResult GetResult() const;

    // Player died (decrement revives)
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Raid")
    void PlayerDowned();

    // Revive player
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Raid")
    bool RevivePlayer();

    // Party wiped
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Raid")
    bool IsWipe() const;

    // Boss defeated (victory)
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Raid")
    void OnBossDefeated();

    // Get remaining revives
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Raid")
    int32 GetRemainingRevives() const;

    // Get time remaining (-1 = no limit)
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Raid")
    float GetTimeRemaining() const;

    // Get encounter duration so far
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Raid")
    float GetDuration() const;

    // Reset encounter
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Raid")
    void ResetEncounter();

    // Replace AI slot with human (mid-encounter)
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Raid")
    bool ReplaceAIWithHuman(int32 Slot, const FString& PlayerName);

    // Replace human with AI (player disconnected mid-encounter)
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Raid")
    bool ReplaceHumanWithAI(int32 Slot);

    // Get party system reference
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Raid")
    UEclipsePartySystem* GetPartySystem() const;

private:
    TArray<FEncounterConfig> AllEncounters;
    EEncounterState State = EEncounterState::Inactive;
    FEncounterConfig ActiveEncounter;
    FEncounterResult LastResult;

    UEclipsePartySystem* PartySystem = nullptr;
    UEclipseBossDirector* BossDirector = nullptr;

    float EncounterTimer = 0.0f;
    int32 RevivesUsed = 0;
    int32 DeathCount = 0;
    bool bBossDown = false;
};
