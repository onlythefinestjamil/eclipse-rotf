#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EclipseCharacter.h"
#include "EclipseBossDirector.h"
#include "EclipseLootSystem.h"
#include "EclipseBossHideCrafting.generated.h"

// ============================================================
// BOSS-HIDE CRAFTING SYSTEM
// Kill a boss -> get boss materials (fangs, scales, hides, bones)
// -> craft weapons OR armor at the Blacksmith forge
// ON TOP OF the affinity gain from the boss kill
// ============================================================

UENUM(BlueprintType)
enum class EBossMaterial : uint8
{
    VolcanicCinderhide    UMETA(DisplayName = "Volcanic Cinderhide"),
    AshenHuskLeather      UMETA(DisplayName = "Ashen Husk Leather"),
    SolarForgeScale       UMETA(DisplayName = "Solar Forge-Scale"),
    ShadowrendHide        UMETA(DisplayName = "Shadowrend Hide"),
    VerdantThornhide      UMETA(DisplayName = "Verdant Thornhide"),
    AetherBoundPelt       UMETA(DisplayName = "Aether-Bound Pelt"),
    StormstrikeHide       UMETA(DisplayName = "Stormstrike Hide"),
    AbyssalMembrane       UMETA(DisplayName = "Abyssal Membrane"),
    FracturedGodhide      UMETA(DisplayName = "Fractured Godhide"),
    None                  UMETA(DisplayName = "None")
};

// What kind of boss part drops (fangs, scales, hides, bones, etc.)
UENUM(BlueprintType)
enum class EBossPart : uint8
{
    Hide        UMETA(DisplayName = "Hide"),
    Scales      UMETA(DisplayName = "Scales"),
    Fangs       UMETA(DisplayName = "Fangs"),
    Bones       UMETA(DisplayName = "Bones"),
    Claws       UMETA(DisplayName = "Claws"),
    Horns       UMETA(DisplayName = "Horns"),
    Heart       UMETA(DisplayName = "Heart (rare)"),
    Soul        UMETA(DisplayName = "Soul Remnant (very rare)"),
    Spine       UMETA(DisplayName = "Spine"),
    Leather     UMETA(DisplayName = "Leather"),
    Essence     UMETA(DisplayName = "Aetheric Essence")
};

// What can you craft from a boss material
UENUM(BlueprintType)
enum class ECraftableType : uint8
{
    Greatsword  UMETA(DisplayName = "Greatsword"),
    Longsword   UMETA(DisplayName = "Longsword"),
    Dagger      UMETA(DisplayName = "Daggers"),
    Spear       UMETA(DisplayName = "Spear/Polearm"),
    Staff       UMETA(DisplayName = "Staff"),
    Bow         UMETA(DisplayName = "Bow"),
    Shield      UMETA(DisplayName = "Shield"),
    Helm        UMETA(DisplayName = "Helm"),
    Chestplate  UMETA(DisplayName = "Chestplate"),
    Gauntlets   UMETA(DisplayName = "Gauntlets"),
    Greaves     UMETA(DisplayName = "Greaves"),
    Pauldrons   UMETA(DisplayName = "Pauldrons"),
    Cloak       UMETA(DisplayName = "Cloak/Cape"),
    Amulet      UMETA(DisplayName = "Amulet"),
    Ring        UMETA(DisplayName = "Ring")
};

USTRUCT(BlueprintType)
struct FBossMaterialDrop
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Boss Craft")
    EBossMaterial MaterialType = EBossMaterial::None;

    UPROPERTY(BlueprintReadWrite, Category = "Boss Craft")
    EBossPart PartType = EBossPart::Hide;

    UPROPERTY(BlueprintReadWrite, Category = "Boss Craft")
    FString MaterialName;

    UPROPERTY(BlueprintReadWrite, Category = "Boss Craft")
    FString BossName;

    UPROPERTY(BlueprintReadWrite, Category = "Boss Craft")
    EAffinityType BossAffinity = EAffinityType::Fire;

    UPROPERTY(BlueprintReadWrite, Category = "Boss Craft")
    int32 Quantity = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Boss Craft")
    FString SourceBossId;

    UPROPERTY(BlueprintReadWrite, Category = "Boss Craft")
    FDateTime KillDate;

    UPROPERTY(BlueprintReadWrite, Category = "Boss Craft")
    FString PartDescription;
};

USTRUCT(BlueprintType)
struct FBossHideCraftedItem
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Boss Craft")
    FString ItemName;

    UPROPERTY(BlueprintReadWrite, Category = "Boss Craft")
    EBossMaterial MaterialSource;

    UPROPERTY(BlueprintReadWrite, Category = "Boss Craft")
    EBossPart PartSource;

    UPROPERTY(BlueprintReadWrite, Category = "Boss Craft")
    ECraftableType CraftedType;

    UPROPERTY(BlueprintReadWrite, Category = "Boss Craft")
    EAffinityType PassiveAffinity;

    UPROPERTY(BlueprintReadWrite, Category = "Boss Craft")
    float PassiveBonusPercent = 10.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Boss Craft")
    float BaseStatBonus = 20.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Boss Craft")
    int32 GoldCost = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Boss Craft")
    ERarityTier Tier = ERarityTier::Epic;

    UPROPERTY(BlueprintReadWrite, Category = "Boss Craft")
    bool bSoulboundOnPickup = true;

    UPROPERTY(BlueprintReadWrite, Category = "Boss Craft")
    FString VisualMaterialAssetPath;

    UPROPERTY(BlueprintReadWrite, Category = "Boss Craft")
    FString NiagaraParticleAssetPath;

    UPROPERTY(BlueprintReadWrite, Category = "Boss Craft")
    float DamageBonus = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Boss Craft")
    float DefenseBonus = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Boss Craft")
    float StaminaBonus = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Boss Craft")
    float ManaBonus = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Boss Craft")
    float HealthBonus = 0.0f;
};

UCLASS(BlueprintType, Blueprintable)
class ECLIPSEROTF_API UEclipseBossHideCrafting : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, Category = "Boss Craft")
    TMap<FString, TPair<EBossMaterial, EBossPart>> BossDropMap;

    UPROPERTY(BlueprintReadWrite, Category = "Boss Craft")
    TArray<FBossMaterialDrop> PlayerMaterials;

    UPROPERTY(BlueprintReadWrite, Category = "Boss Craft")
    TArray<FBossHideCraftedItem> CraftedItems;

    UPROPERTY(BlueprintReadWrite, Category = "Boss Craft")
    TSet<FString> CraftedMaterialKeys;

    UEclipseBossHideCrafting();

    // Called when a boss is killed - drops multiple parts + grants affinity
    // Each boss drops 2-4 parts: hide + fangs/scales + rare heart/soul
    UFUNCTION(BlueprintCallable, Category = "Boss Craft")
    TArray<FBossMaterialDrop> OnBossKilled(const FString& BossName, EAffinityType BossAffinity);

    UFUNCTION(BlueprintCallable, Category = "Boss Craft")
    bool HasMaterial(EBossMaterial Material, EBossPart Part) const;

    UFUNCTION(BlueprintCallable, Category = "Boss Craft")
    TArray<FBossMaterialDrop> GetAvailableMaterials() const;

    // Craft a WEAPON from a boss material part
    // Fangs/Claws -> daggers, spears, swords
    // Scales/Horns -> greatswords, shields
    // Hide/Leather -> cloaks, chest armor
    // Bones/Spine -> staffs, greaves
    // Heart/Soul -> legendary variant of any weapon
    UFUNCTION(BlueprintCallable, Category = "Boss Craft")
    bool CraftWeapon(EBossMaterial Material, EBossPart Part, ECraftableType WeaponType, int32 GoldAmount);

    // Craft ARMOR from a boss material part
    UFUNCTION(BlueprintCallable, Category = "Boss Craft")
    bool CraftArmor(EBossMaterial Material, EBossPart Part, ECraftableType ArmorType, int32 GoldAmount);

    UFUNCTION(BlueprintCallable, Category = "Boss Craft")
    float GetPassiveBonus(EBossMaterial Material) const;

    UFUNCTION(BlueprintCallable, Category = "Boss Craft")
    EAffinityType GetMaterialAffinity(EBossMaterial Material) const;

    UFUNCTION(BlueprintCallable, Category = "Boss Craft")
    int32 GetGoldCostForTier(ERarityTier Tier) const;

    UFUNCTION(BlueprintCallable, Category = "Boss Craft")
    TArray<FBossHideCraftedItem> GetCraftableItems() const;

    // Get craftable weapon types from a specific part
    // Fangs -> Daggers, Longsword, Spear
    // Claws -> Daggers, Gauntlets
    // Scales -> Greatsword, Shield, Chestplate, Pauldrons
    // Horns -> Greatsword, Helm, Shield
    // Hide -> Cloak, Chestplate, Greaves
    // Leather -> Cloak, Chestplate, Greaves, Pauldrons
    // Bones -> Staff, Greaves, Amulet
    // Spine -> Staff, Greaves, Pauldrons
    // Heart -> Any weapon OR full armor set (legendary)
    // Soul -> Any weapon OR any armor (mythic)
    // Essence -> Amulet, Ring
    UFUNCTION(BlueprintCallable, Category = "Boss Craft")
    TArray<ECraftableType> GetWeaponOptionsForPart(EBossPart Part) const;

    UFUNCTION(BlueprintCallable, Category = "Boss Craft")
    TArray<ECraftableType> GetArmorOptionsForPart(EBossPart Part) const;

    UFUNCTION(BlueprintCallable, Category = "Boss Craft")
    FString GetVisualMaterialPath(EBossMaterial Material) const;

    UFUNCTION(BlueprintCallable, Category = "Boss Craft")
    FString GetNiagaraParticlePath(EBossMaterial Material) const;

    UFUNCTION(BlueprintCallable, Category = "Boss Craft")
    bool IsMaterialCrafted(EBossMaterial Material, EBossPart Part) const;

    UFUNCTION(BlueprintCallable, Category = "Boss Craft")
    FString GetMaterialDisplayName(EBossMaterial Material) const;

    UFUNCTION(BlueprintCallable, Category = "Boss Craft")
    FString GetPartDisplayName(EBossPart Part) const;

    UFUNCTION(BlueprintCallable, Category = "Boss Craft")
    TArray<FBossHideCraftedItem> GetCraftedWeapons() const;

    UFUNCTION(BlueprintCallable, Category = "Boss Craft")
    TArray<FBossHideCraftedItem> GetCraftedArmor() const;

private:
    void InitializeBossDropMap();
    EAffinityType MaterialToAffinity(EBossMaterial Material) const;
    float GetAffinityBonusPercent(EBossMaterial Material) const;
    FString MakeMaterialKey(EBossMaterial Material, EBossPart Part) const;
    TArray<EBossPart> GetDropsForBoss(const FString& BossName) const;
};
