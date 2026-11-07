// Eclipse Loot System — Drop Rates, Class Drops, Binding System
// Drops are percentage-based but when a class item drops, it drops for all classes
// Binding: Bind on Equip (BoE) vs Soulbound on Pickup (SoP)

#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EclipseLootSystem.generated.h"

#include "EclipseCharacter.h" // For EClass, ERarityTier enums

UENUM(BlueprintType)
enum class EBindingType : uint8
{
    None            UMETA(DisplayName = "No binding — freely tradeable"),
    BindOnEquip     UMETA(DisplayName = "Bind on Equip — tradeable until equipped"),
    Soulbound       UMETA(DisplayName = "Soulbound — binds on pickup, cannot trade"),
    AccountBound    UMETA(DisplayName = "Account Bound — tradeable between your characters"),
};

UENUM(BlueprintType)
enum class EDropType : uint8
{
    Universal       UMETA(DisplayName = "Universal — any class can use"),
    ClassSpecific   UMETA(DisplayName = "Class Specific — drops for all classes simultaneously"),
    AncestrySpecific UMETA(DisplayName = "Ancestry Specific — restricted to one ancestry"),
    SharedToken     UMETA(DisplayName = "Shared Token — redeem for any class version"),
};

USTRUCT(BlueprintType)
struct FLootDrop
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) FString ItemName;
    UPROPERTY(BlueprintReadWrite) ERarityTier Rarity = ERarityTier::Common;
    UPROPERTY(BlueprintReadWrite) EDropType DropType = EDropType::Universal;
    UPROPERTY(BlueprintReadWrite) EBindingType Binding = EBindingType::BindOnEquip;
    UPROPERTY(BlueprintReadWrite) float DropChance = 0.0f; // Percentage 0-100
    UPROPERTY(BlueprintReadWrite) int32 ItemLevel = 1;
    UPROPERTY(BlueprintReadWrite) FString SetFamily = ""; // Set bonus family
    UPROPERTY(BlueprintReadWrite) bool bIsWeapon = false;
    UPROPERTY(BlueprintReadWrite) bool bIsArmor = false;
    UPROPERTY(BlueprintReadWrite) bool bIsConsumable = false;
    UPROPERTY(BlueprintReadWrite) bool bIsCraftingMaterial = false;
};

// Aliases used by Boss/Raid systems (legacy naming convention)
using ERarity = ERarityTier;
using FDropEntry = FLootDrop;

USTRUCT(BlueprintType)
struct FLootTable
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) TArray<FLootDrop> Drops;
    UPROPERTY(BlueprintReadWrite) int32 MaxDrops = 3;
    UPROPERTY(BlueprintReadWrite) float LuckModifier = 1.0f; // Player luck stat affects drop rates
};

UCLASS(Blueprintable)
class ECLIPSEROTF_API UEclipseLootSystem : public UObject
{
    GENERATED_BODY()

public:
    UEclipseLootSystem();

    // Generate loot from a kill
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Loot")
    TArray<FLootDrop> GenerateLoot(int32 EnemyLevel, ERarityTier EnemyTier, float LuckStat);

    // Generate boss loot (guaranteed drops + chance for rare)
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Loot")
    TArray<FLootDrop> GenerateBossLoot(int32 BossLevel, ERarityTier BossTier);

    // Generate dungeon end loot
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Loot")
    TArray<FLootDrop> GenerateDungeonLoot(int32 DungeonLevel, ERarityTier DungeonTier);

    // When a class-specific item drops, it drops for ALL classes
    // Returns the class-specific versions for each class
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Loot")
    TArray<FLootDrop> ExpandClassDrop(const FLootDrop& ClassItem);

    // Roll a single drop
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Loot")
    bool RollDrop(float Chance, float LuckModifier);

    // Determine binding type for an item
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Loot")
    EBindingType DetermineBinding(ERarityTier Rarity, bool bIsBossDrop);

    // Trade system — check if item can be traded
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Loot")
    bool CanTrade(const FLootDrop& Item, bool bIsEquipped);

    // Check if item is soulbound
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Loot")
    bool IsSoulbound(const FLootDrop& Item);

private:
    // Drop rate tables (percentage-based)
    // Common: 50%, Uncommon: 25%, Rare: 15%, Epic: 6%, Legendary: 3%, Mythic: 0.8%, Eclipse: 0.2%
    TMap<ERarityTier, float> BaseDropRates;
    
    // Boss drop rates (higher)
    TMap<ERarityTier, float> BossDropRates;

    void InitializeDropTables();
};
