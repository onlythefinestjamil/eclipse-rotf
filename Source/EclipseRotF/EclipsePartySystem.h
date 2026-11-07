// Eclipse Party/Raid System - 5-player dungeons, 10-player raids, AI + human slots
// Per directive: AI companions fill slots, replaceable by human players without rebuilding encounters

#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "EclipseCompanionAI.h" // For FCompanionProfile, EPartySlot
#include "EclipsePartySystem.generated.h"

UENUM(BlueprintType)
enum class EGroupType : uint8
{
    Solo           UMETA(DisplayName = "Solo - 1 player"),
    Party          UMETA(DisplayName = "Party - 5 players (dungeon)"),
    Raid           UMETA(DisplayName = "Raid - 10 players (raid)"),
    Arena1v1       UMETA(DisplayName = "Arena 1v1"),
    Arena2v2       UMETA(DisplayName = "Arena 2v2"),
    Arena3v3       UMETA(DisplayName = "Arena 3v3"),
    Arena5v5       UMETA(DisplayName = "Arena 5v5"),
    Battleground   UMETA(DisplayName = "Battleground 10v10"),
};

USTRUCT(BlueprintType)
struct FPartyMember
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) int32 SlotIndex = 0;
    UPROPERTY(BlueprintReadWrite) EPartySlot SlotType = EPartySlot::Empty;
    UPROPERTY(BlueprintReadWrite) FString Name = "";
    UPROPERTY(BlueprintReadWrite) EClass CharacterClass = EClass::Vanguard;
    UPROPERTY(BlueprintReadWrite) EClassRole Role = EClassRole::DPS;
    UPROPERTY(BlueprintReadWrite) ERaidRole RaidRole = ERaidRole::MeleeDPS;
    UPROPERTY(BlueprintReadWrite) float Health = 100.0f;
    UPROPERTY(BlueprintReadWrite) float MaxHealth = 100.0f;
    UPROPERTY(BlueprintReadWrite) bool bIsAlive = true;
    UPROPERTY(BlueprintReadWrite) bool bIsAI = true;
    UPROPERTY(BlueprintReadWrite) bool bReady = false;
    UPROPERTY(BlueprintReadWrite) FCompanionProfile CompanionProfile;
};

UCLASS(Blueprintable)
class ECLIPSEROTF_API UEclipsePartySystem : public UObject
{
    GENERATED_BODY()

public:
    UEclipsePartySystem();

    // Initialize group
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Party")
    void InitializeGroup(EGroupType Type);

    // Add player to party
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Party")
    bool AddHumanPlayer(int32 Slot, const FString& PlayerName, EClass Class, EClassRole Role);

    // Add AI companion to party
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Party")
    bool AddAICompanion(int32 Slot, const FCompanionProfile& Profile);

    // Remove player/companion from slot
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Party")
    bool RemoveFromSlot(int32 Slot);

    // Replace AI with human player
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Party")
    bool ReplaceAIWithHuman(int32 Slot, const FString& PlayerName);

    // Replace human with AI (player disconnected)
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Party")
    bool ReplaceHumanWithAI(int32 Slot);

    // Get all party members
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Party")
    TArray<FPartyMember> GetPartyMembers() const;

    // Get party size
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Party")
    int32 GetPartySize() const;

    // Get max party size
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Party")
    int32 GetMaxPartySize() const;

    // Get group type
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Party")
    EGroupType GetGroupType() const;

    // Get role distribution
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Party")
    FString GetRoleDistribution() const;

    // Fill empty slots with AI companions
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Party")
    void FillEmptySlotsWithAI();

    // Set player command to all AI companions
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Party")
    void BroadcastCommand(ECompanionCommand Command, FVector Location = FVector::ZeroVector);

    // Get living members count
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Party")
    int32 GetLivingMembersCount() const;

    // Is the party wiped?
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Party")
    bool IsPartyWiped() const;

    // Get number of AI vs human players
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Party")
    int32 GetAIcount() const;
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Party")
    int32 GetHumanCount() const;

    // Ready check
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Party")
    bool IsEveryoneReady() const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Party")
    void SetMemberReady(int32 Slot, bool bReady);

private:
    EGroupType GroupType = EGroupType::Solo;
    int32 MaxSlots = 1;
    TArray<FPartyMember> Members;

    void InitializeDefaultAICompanions();
    FCompanionProfile GenerateAICompanion(int32 SlotIndex, ERaidRole Role);
};
