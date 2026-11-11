#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EclipseCharacter.h"
#include "EclipseGatheringSystem.generated.h"

// ============================================================
// GATHERING & FARMING SYSTEM
// Mining, Skinning, Herbalism, Fishing, Woodcutting, Farming
// Skills 0-300, nodes respawn, materials feed crafting
// ============================================================

UENUM(BlueprintType)
enum class EGatheringSkill : uint8
{
    Mining         UMETA(DisplayName = "Mining"),
    Skinning       UMETA(DisplayName = "Skinning"),
    Herbalism      UMETA(DisplayName = "Herbalism"),
    Fishing        UMETA(DisplayName = "Fishing"),
    Woodcutting    UMETA(DisplayName = "Woodcutting"),
    Farming        UMETA(DisplayName = "Farming"),
    Crafting       UMETA(DisplayName = "Crafting")
};

UENUM(BlueprintType)
enum class ENodeType : uint8
{
    IronOre        UMETA(DisplayName = "Iron Ore"),
    SilverOre      UMETA(DisplayName = "Silver Ore"),
    GoldOre        UMETA(DisplayName = "Gold Ore"),
    Obsidian       UMETA(DisplayName = "Obsidian"),
    Moonstone      UMETA(DisplayName = "Moonstone"),
    AetherCrystal  UMETA(DisplayName = "Aether Crystal"),
    FracturedStone UMETA(DisplayName = "Fractured Stone"),
    SolarOre       UMETA(DisplayName = "Solar Ore"),
    Voidstone      UMETA(DisplayName = "Voidstone"),
    StormQuartz    UMETA(DisplayName = "Storm Quartz"),
    Frostiron      UMETA(DisplayName = "Frostiron"),
    Venomstone      UMETA(DisplayName = "Venomstone"),
    Soulrite       UMETA(DisplayName = "Soulrite"),
    TemporalGlass   UMETA(DisplayName = "Temporal Glass"),
    // Skinning
    WolfHide       UMETA(DisplayName = "Wolf Hide"),
    BearHide       UMETA(DisplayName = "Bear Hide"),
    DrakeScales     UMETA(DisplayName = "Drake Scales"),
    SpiderSilk     UMETA(DisplayName = "Spider Silk"),
    BoarHide       UMETA(DisplayName = "Boar Hide"),
    SerpentSkin     UMETA(DisplayName = "Serpent Skin"),
    ShadowCatPelt   UMETA(DisplayName = "Shadow Cat Pelt"),
    AetherwolfPelt UMETA(DisplayName = "Aetherwolf Pelt"),
    FrostBearPelt   UMETA(DisplayName = "Frost Bear Pelt"),
    VenomtoadSkin   UMETA(DisplayName = "Venomtoad Skin"),
    // Herbalism
    Aetherbloom    UMETA(DisplayName = "Aetherbloom"),
    Sunpetal       UMETA(DisplayName = "Sunpetal"),
    Shadowcap      UMETA(DisplayName = "Shadowcap Mushroom"),
    Frostleaf      UMETA(DisplayName = "Frostleaf"),
    Emberflower    UMETA(DisplayName = "Emberflower"),
    Stormroot      UMETA(DisplayName = "Stormroot"),
    SoulMoss       UMETA(DisplayName = "Soul Moss"),
    Voidfern       UMETA(DisplayName = "Voidfern"),
    FracturedLotus UMETA(DisplayName = "Fractured Lotus"),
    Moonwort       UMETA(DisplayName = "Moonwort"),
    // Fishing
    CommonFish     UMETA(DisplayName = "Common Fish"),
    Aetherfish     UMETA(DisplayName = "Aetherfish"),
    VoidEel        UMETA(DisplayName = "Void Eel"),
    StormBass      UMETA(DisplayName = "Storm Bass"),
    FrostSalmon    UMETA(DisplayName = "Frost Salmon"),
    SoulCarp       UMETA(DisplayName = "Soul Carp"),
    FracturedTrout UMETA(DisplayName = "Fractured Trout"),
    // Woodcutting
    Ironwood       UMETA(DisplayName = "Ironwood"),
    Shadowwood     UMETA(DisplayName = "Shadowwood"),
    Aetherwood     UMETA(DisplayName = "Aetherwood"),
    Stormwood      UMETA(DisplayName = "Stormwood"),
    FracturedOak   UMETA(DisplayName = "Fractured Oak")
};

USTRUCT(BlueprintType)
struct FGatheringNode
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Gathering")
    ENodeType NodeType;

    UPROPERTY(BlueprintReadWrite, Category = "Gathering")
    EGatheringSkill RequiredSkill;

    UPROPERTY(BlueprintReadWrite, Category = "Gathering")
    int32 RequiredSkillLevel = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Gathering")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Gathering")
    FString Region;

    UPROPERTY(BlueprintReadWrite, Category = "Gathering")
    int32 CurrentYield = 3;

    UPROPERTY(BlueprintReadWrite, Category = "Gathering")
    int32 MaxYield = 3;

    UPROPERTY(BlueprintReadWrite, Category = "Gathering")
    float RespawnTimeMinutes = 10.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Gathering")
    FDateTime LastHarvestedTime;

    UPROPERTY(BlueprintReadWrite, Category = "Gathering")
    bool bIsAvailable = true;

    UPROPERTY(BlueprintReadWrite, Category = "Gathering")
    float RareDropChance = 0.05f;
};

USTRUCT(BlueprintType)
struct FGatheredMaterial
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Gathering")
    ENodeType MaterialType;

    UPROPERTY(BlueprintReadWrite, Category = "Gathering")
    FString MaterialName;

    UPROPERTY(BlueprintReadWrite, Category = "Gathering")
    EGatheringSkill SourceSkill;

    UPROPERTY(BlueprintReadWrite, Category = "Gathering")
    int32 Quantity = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Gathering")
    EAffinityType MaterialAffinity = EAffinityType::Earth;

    UPROPERTY(BlueprintReadWrite, Category = "Gathering")
    ERarityTier Tier = ERarityTier::Common;

    UPROPERTY(BlueprintReadWrite, Category = "Gathering")
    FString Region;
};

UCLASS(BlueprintType, Blueprintable)
class ECLIPSEROTF_API UEclipseGatheringSystem : public UObject
{
    GENERATED_BODY()

public:
    // Player skill levels (0-300)
    UPROPERTY(BlueprintReadWrite, Category = "Gathering")
    TMap<EGatheringSkill, int32> SkillLevels;

    // Player's gathered materials inventory
    UPROPERTY(BlueprintReadWrite, Category = "Gathering")
    TArray<FGatheredMaterial> PlayerInventory;

    // Active nodes in the world
    UPROPERTY(BlueprintReadWrite, Category = "Gathering")
    TArray<FGatheringNode> ActiveNodes;

    UEclipseGatheringSystem();

    // Initialize skill levels to 0
    UFUNCTION(BlueprintCallable, Category = "Gathering")
    void InitializeSkills();

    // Get skill level
    UFUNCTION(BlueprintCallable, Category = "Gathering")
    int32 GetSkillLevel(EGatheringSkill Skill) const;

    // Gain XP and potentially level up
    UFUNCTION(BlueprintCallable, Category = "Gathering")
    bool GainSkillXP(EGatheringSkill Skill, int32 XPAmount);

    // Attempt to harvest a node
    UFUNCTION(BlueprintCallable, Category = "Gathering")
    bool HarvestNode(FGatheringNode& Node);

    // Check if player can harvest a node
    UFUNCTION(BlueprintCallable, Category = "Gathering")
    bool CanHarvest(const FGatheringNode& Node) const;

    // Get materials from inventory
    UFUNCTION(BlueprintCallable, Category = "Gathering")
    TArray<FGatheredMaterial> GetMaterialsByType(ENodeType Type) const;

    // Get all materials by skill
    UFUNCTION(BlueprintCallable, Category = "Gathering")
    TArray<FGatheredMaterial> GetMaterialsBySkill(EGatheringSkill Skill) const;

    // Add material to inventory
    UFUNCTION(BlueprintCallable, Category = "Gathering")
    void AddMaterial(ENodeType Type, int32 Quantity, const FString& Region);

    // Remove material from inventory
    UFUNCTION(BlueprintCallable, Category = "Gathering")
    bool RemoveMaterial(ENodeType Type, int32 Quantity);

    // Check if player has enough material
    UFUNCTION(BlueprintCallable, Category = "Gathering")
    bool HasMaterial(ENodeType Type, int32 Quantity) const;

    // Spawn nodes for a region
    UFUNCTION(BlueprintCallable, Category = "Gathering")
    void SpawnNodesForRegion(const FString& Region);

    // Check and respawn depleted nodes
    UFUNCTION(BlueprintCallable, Category = "Gathering")
    void CheckRespawns();

    // Get node type display name
    UFUNCTION(BlueprintCallable, Category = "Gathering")
    FString GetNodeDisplayName(ENodeType Type) const;

    // Get skill display name
    UFUNCTION(BlueprintCallable, Category = "Gathering")
    FString GetSkillDisplayName(EGatheringSkill Skill) const;

    // Get required skill for a node type
    UFUNCTION(BlueprintCallable, Category = "Gathering")
    EGatheringSkill GetSkillForNode(ENodeType Type) const;

    // Get required skill level for a node type
    UFUNCTION(BlueprintCallable, Category = "Gathering")
    int32 GetRequiredSkillLevel(ENodeType Type) const;

    // Get rarity tier for a node type
    UFUNCTION(BlueprintCallable, Category = "Gathering")
    ERarityTier GetRarityForNode(ENodeType Type) const;

    // Get affinity for a node type
    UFUNCTION(BlueprintCallable, Category = "Gathering")
    EAffinityType GetAffinityForNode(ENodeType Type) const;

    // Farming: plant a seed
    UFUNCTION(BlueprintCallable, Category = "Gathering")
    bool PlantSeed(ENodeType SeedType, const FString& PlotId);

    // Farming: harvest a mature crop
    UFUNCTION(BlueprintCallable, Category = "Gathering")
    bool HarvestCrop(const FString& PlotId);

    // Farming: check crop maturity
    UFUNCTION(BlueprintCallable, Category = "Gathering")
    bool IsCropMature(const FString& PlotId) const;

private:
    void InitializeNodeDefaults();
    FString GetMaterialNameForNode(ENodeType Type) const;
};
