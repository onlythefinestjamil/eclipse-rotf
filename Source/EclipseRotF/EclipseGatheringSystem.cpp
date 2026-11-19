// EclipseGatheringSystem.cpp — Implementation
// Mining, Skinning, Herbalism, Fishing, Woodcutting, Farming

#include "EclipseGatheringSystem.h"

// === GATHERING COMPONENT ===
UGatheringComponent::UGatheringComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize all skill levels to 0
    MiningLevel = 0;
    MiningXP = 0;
    SkinningLevel = 0;
    SkinningXP = 0;
    HerbalismLevel = 0;
    HerbalismXP = 0;
    FishingLevel = 0;
    FishingXP = 0;
    WoodcuttingLevel = 0;
    WoodcuttingXP = 0;
    FarmingLevel = 0;
    FarmingXP = 0;
}

void UGatheringComponent::BeginPlay()
{
    Super::BeginPlay();
}

// === MINING ===
bool UGatheringComponent::CanMine(EOreType OreType) const
{
    const int32 RequiredLevel = GetOreRequiredLevel(OreType);
    return MiningLevel >= RequiredLevel;
}

FGatherResult UGatheringComponent::MineOre(EOreType OreType)
{
    FGatherResult Result;
    if (!CanMine(OreType))
    {
        Result.bSuccess = false;
        Result.FailureReason = TEXT("Mining level too low for this ore type.");
        return Result;
    }
    
    Result.bSuccess = true;
    Result.ItemName = GetOreName(OreType);
    Result.Quantity = FMath::RandRange(1, 3);
    Result.XPGained = GetOreXP(OreType);
    
    MiningXP += Result.XPGained;
    CheckLevelUp(EGatheringSkill::Mining);
    
    return Result;
}

int32 UGatheringComponent::GetOreRequiredLevel(EOreType OreType) const
{
    switch (OreType)
    {
    case EOreType::Iron:        return 1;
    case EOreType::Copper:      return 1;
    case EOreType::Tin:         return 5;
    case EOreType::Silver:      return 10;
    case EOreType::Gold:        return 25;
    case EOreType::Voidstone:   return 50;
    case EOreType::Shadow:     return 75;
    case EOreType::Crystal:     return 100;
    case EOreType::Temporal:    return 150;
    case EOreType::Fracture:    return 200;
    case EOreType::Aether:     return 250;
    case EOreType::Eclipse:     return 280;
    case EOreType::Mythic:      return 290;
    case EOreType::Legendary:   return 300;
    default: return 1;
    }
}

FString UGatheringComponent::GetOreName(EOreType OreType) const
{
    switch (OreType)
    {
    case EOreType::Iron:        return TEXT("Iron Ore");
    case EOreType::Copper:      return TEXT("Copper Ore");
    case EOreType::Tin:         return TEXT("Tin Ore");
    case EOreType::Silver:      return TEXT("Silver Ore");
    case EOreType::Gold:        return TEXT("Gold Ore");
    case EOreType::Voidstone:   return TEXT("Voidstone");
    case EOreType::Shadow:      return TEXT("Shadow Ore");
    case EOreType::Crystal:     return TEXT("Crystal Ore");
    case EOreType::Temporal:    return TEXT("Temporal Glass");
    case EOreType::Fracture:    return TEXT("Fracture Ore");
    case EOreType::Aether:     return TEXT("Aether Ore");
    case EOreType::Eclipse:    return TEXT("Eclipse Ore");
    case EOreType::Mythic:     return TEXT("Mythic Ore");
    case EOreType::Legendary:  return TEXT("Legendary Ore");
    default: return TEXT("Unknown Ore");
    }
}

int32 UGatheringComponent::GetOreXP(EOreType OreType) const
{
    switch (OreType)
    {
    case EOreType::Iron:        return 10;
    case EOreType::Copper:      return 10;
    case EOreType::Tin:         return 15;
    case EOreType::Silver:      return 25;
    case EOreType::Gold:        return 40;
    case EOreType::Voidstone:   return 75;
    case EOreType::Shadow:     return 100;
    case EOreType::Crystal:     return 150;
    case EOreType::Temporal:    return 225;
    case EOreType::Fracture:    return 300;
    case EOreType::Aether:     return 400;
    case EOreType::Eclipse:    return 500;
    case EOreType::Mythic:     return 600;
    case EOreType::Legendary:  return 750;
    default: return 10;
    }
}

// === SKINNING ===
bool UGatheringComponent::CanSkin(ECreatureHideType HideType) const
{
    return SkinningLevel >= GetHideRequiredLevel(HideType);
}

FGatherResult UGatheringComponent::SkinCreature(ECreatureHideType HideType)
{
    FGatherResult Result;
    if (!CanSkin(HideType))
    {
        Result.bSuccess = false;
        Result.FailureReason = TEXT("Skinning level too low.");
        return Result;
    }
    
    Result.bSuccess = true;
    Result.ItemName = GetHideName(HideType);
    Result.Quantity = FMath::RandRange(1, 2);
    Result.XPGained = GetHideXP(HideType);
    
    SkinningXP += Result.XPGained;
    CheckLevelUp(EGatheringSkill::Skinning);
    
    return Result;
}

int32 UGatheringComponent::GetHideRequiredLevel(ECreatureHideType HideType) const
{
    switch (HideType)
    {
    case ECreatureHideType::Wolf:    return 1;
    case ECreatureHideType::Boar:    return 5;
    case ECreatureHideType::Bear:    return 10;
    case ECreatureHideType::Wraith:  return 25;
    case ECreatureHideType::Drake:   return 50;
    case ECreatureHideType::Golem:   return 75;
    case ECreatureHideType::WraithLord: return 100;
    case ECreatureHideType::Void:    return 150;
    case ECreatureHideType::Fracture: return 200;
    case ECreatureHideType::Eclipse: return 250;
    default: return 1;
    }
}

FString UGatheringComponent::GetHideName(ECreatureHideType HideType) const
{
    switch (HideType)
    {
    case ECreatureHideType::Wolf:    return TEXT("Wolf Pelt");
    case ECreatureHideType::Boar:    return TEXT("Boar Hide");
    case ECreatureHideType::Bear:    return TEXT("Bear Hide");
    case ECreatureHideType::Wraith:  return TEXT("Wraith Essence");
    case ECreatureHideType::Drake:   return TEXT("Drake Scale");
    case ECreatureHideType::Golem:   return TEXT("Golem Core");
    case ECreatureHideType::WraithLord: return TEXT("Wraith Lord Essence");
    case ECreatureHideType::Void:    return TEXT("Void Leather");
    case ECreatureHideType::Fracture: return TEXT("Fracture Hide");
    case ECreatureHideType::Eclipse: return TEXT("Eclipse Pelt");
    default: return TEXT("Unknown Hide");
    }
}

int32 UGatheringComponent::GetHideXP(ECreatureHideType HideType) const
{
    switch (HideType)
    {
    case ECreatureHideType::Wolf:    return 10;
    case ECreatureHideType::Boar:    return 15;
    case ECreatureHideType::Bear:    return 25;
    case ECreatureHideType::Wraith:  return 40;
    case ECreatureHideType::Drake:   return 75;
    case ECreatureHideType::Golem:   return 100;
    case ECreatureHideType::WraithLord: return 150;
    case ECreatureHideType::Void:    return 225;
    case ECreatureHideType::Fracture: return 300;
    case ECreatureHideType::Eclipse: return 400;
    default: return 10;
    }
}

// === HERBALISM ===
bool UGatheringComponent::CanHarvest(EHerbType HerbType) const
{
    return HerbalismLevel >= GetHerbRequiredLevel(HerbType);
}

FGatherResult UGatheringComponent::HarvestHerb(EHerbType HerbType)
{
    FGatherResult Result;
    if (!CanHarvest(HerbType))
    {
        Result.bSuccess = false;
        Result.FailureReason = TEXT("Herbalism level too low.");
        return Result;
    }
    
    Result.bSuccess = true;
    Result.ItemName = GetHerbName(HerbType);
    Result.Quantity = FMath::RandRange(1, 3);
    Result.XPGained = GetHerbXP(HerbType);
    
    HerbalismXP += Result.XPGained;
    CheckLevelUp(EGatheringSkill::Herbalism);
    
    return Result;
}

int32 UGatheringComponent::GetHerbRequiredLevel(EHerbType HerbType) const
{
    switch (HerbType)
    {
    case EHerbType::Peacebloom:     return 1;
    case EHerbType::Silverleaf:     return 1;
    case EHerbType::Earthroot:      return 5;
    case EHerbType::Mageroyal:     return 10;
    case EHerbType::Briarthorn:    return 25;
    case EHerbType::Swiftthistle:  return 50;
    case EHerbType::Sungrass:      return 75;
    case EHerbType::Dreamfoil:     return 100;
    case EHerbType::Icecap:        return 150;
    case EHerbType::FractureBloom:  return 200;
    default: return 1;
    }
}

FString UGatheringComponent::GetHerbName(EHerbType HerbType) const
{
    switch (HerbType)
    {
    case EHerbType::Peacebloom:    return TEXT("Peacebloom");
    case EHerbType::Silverleaf:    return TEXT("Silverleaf");
    case EHerbType::Earthroot:     return TEXT("Earthroot");
    case EHerbType::Mageroyal:     return TEXT("Mageroyal");
    case EHerbType::Briarthorn:    return TEXT("Briarthorn");
    case EHerbType::Swiftthistle:  return TEXT("Swiftthistle");
    case EHerbType::Sungrass:      return TEXT("Sungrass");
    case EHerbType::Dreamfoil:    return TEXT("Dreamfoil");
    case EHerbType::Icecap:        return TEXT("Icecap");
    case EHerbType::FractureBloom: return TEXT("Fracture Bloom");
    default: return TEXT("Unknown Herb");
    }
}

int32 UGatheringComponent::GetHerbXP(EHerbType HerbType) const
{
    switch (HerbType)
    {
    case EHerbType::Peacebloom:    return 10;
    case EHerbType::Silverleaf:    return 10;
    case EHerbType::Earthroot:     return 15;
    case EHerbType::Mageroyal:     return 25;
    case EHerbType::Briarthorn:    return 40;
    case EHerbType::Swiftthistle:  return 75;
    case EHerbType::Sungrass:      return 100;
    case EHerbType::Dreamfoil:    return 150;
    case EHerbType::Icecap:        return 225;
    case EHerbType::FractureBloom: return 300;
    default: return 10;
    }
}

// === FISHING ===
bool UGatheringComponent::CanFish(EFishType FishType) const
{
    return FishingLevel >= GetFishRequiredLevel(FishType);
}

FGatherResult UGatheringComponent::CatchFish(EFishType FishType)
{
    FGatherResult Result;
    if (!CanFish(FishType))
    {
        Result.bSuccess = false;
        Result.FailureReason = TEXT("Fishing level too low.");
        return Result;
    }
    
    Result.bSuccess = true;
    Result.ItemName = GetFishName(FishType);
    Result.Quantity = 1;
    Result.XPGained = GetFishXP(FishType);
    
    FishingXP += Result.XPGained;
    CheckLevelUp(EGatheringSkill::Fishing);
    
    return Result;
}

int32 UGatheringComponent::GetFishRequiredLevel(EFishType FishType) const
{
    switch (FishType)
    {
    case EFishType::Common:    return 1;
    case EFishType::Uncommon: return 10;
    case EFishType::Rare:      return 25;
    case EFishType::Epic:      return 50;
    case EFishType::Legendary: return 100;
    case EFishType::Mythic:    return 200;
    case EFishType::Eclipse:   return 280;
    default: return 1;
    }
}

FString UGatheringComponent::GetFishName(EFishType FishType) const
{
    switch (FishType)
    {
    case EFishType::Common:    return TEXT("Common Fish");
    case EFishType::Uncommon: return TEXT("Uncommon Fish");
    case EFishType::Rare:      return TEXT("Rare Fish");
    case EFishType::Epic:      return TEXT("Epic Fish");
    case EFishType::Legendary: return TEXT("Legendary Fish");
    case EFishType::Mythic:    return TEXT("Mythic Fish");
    case EFishType::Eclipse:   return TEXT("Eclipse Fish");
    default: return TEXT("Unknown Fish");
    }
}

int32 UGatheringComponent::GetFishXP(EFishType FishType) const
{
    switch (FishType)
    {
    case EFishType::Common:    return 10;
    case EFishType::Uncommon: return 25;
    case EFishType::Rare:      return 50;
    case EFishType::Epic:      return 100;
    case EFishType::Legendary: return 200;
    case EFishType::Mythic:    return 350;
    case EFishType::Eclipse:   return 500;
    default: return 10;
    }
}

// === WOODCUTTING ===
bool UGatheringComponent::CanChop(EWoodType WoodType) const
{
    return WoodcuttingLevel >= GetWoodRequiredLevel(WoodType);
}

FGatherResult UGatheringComponent::ChopWood(EWoodType WoodType)
{
    FGatherResult Result;
    if (!CanChop(WoodType))
    {
        Result.bSuccess = false;
        Result.FailureReason = TEXT("Woodcutting level too low.");
        return Result;
    }
    
    Result.bSuccess = true;
    Result.ItemName = GetWoodName(WoodType);
    Result.Quantity = FMath::RandRange(1, 4);
    Result.XPGained = GetWoodXP(WoodType);
    
    WoodcuttingXP += Result.XPGained;
    CheckLevelUp(EGatheringSkill::Woodcutting);
    
    return Result;
}

int32 UGatheringComponent::GetWoodRequiredLevel(EWoodType WoodType) const
{
    switch (WoodType)
    {
    case EWoodType::Pine:     return 1;
    case EWoodType::Oak:      return 10;
    case EWoodType::Ash:      return 25;
    case EWoodType::Eclipse:  return 100;
    case EWoodType::Fracture: return 200;
    default: return 1;
    }
}

FString UGatheringComponent::GetWoodName(EWoodType WoodType) const
{
    switch (WoodType)
    {
    case EWoodType::Pine:     return TEXT("Pine Wood");
    case EWoodType::Oak:      return TEXT("Oak Wood");
    case EWoodType::Ash:      return TEXT("Ash Wood");
    case EWoodType::Eclipse:  return TEXT("Eclipse Wood");
    case EWoodType::Fracture: return TEXT("Fracture Wood");
    default: return TEXT("Unknown Wood");
    }
}

int32 UGatheringComponent::GetWoodXP(EWoodType WoodType) const
{
    switch (WoodType)
    {
    case EWoodType::Pine:     return 10;
    case EWoodType::Oak:      return 25;
    case EWoodType::Ash:      return 50;
    case EWoodType::Eclipse:  return 200;
    case EWoodType::Fracture: return 300;
    default: return 10;
    }
}

// === FARMING ===
bool UGatheringComponent::CanPlant(ESeedType SeedType) const
{
    return FarmingLevel >= GetSeedRequiredLevel(SeedType);
}

FGatherResult UGatheringComponent::PlantSeed(ESeedType SeedType)
{
    FGatherResult Result;
    if (!CanPlant(SeedType))
    {
        Result.bSuccess = false;
        Result.FailureReason = TEXT("Farming level too low.");
        return Result;
    }
    
    Result.bSuccess = true;
    Result.ItemName = GetSeedName(SeedType);
    Result.Quantity = 1;
    Result.XPGained = GetSeedXP(SeedType);
    
    FarmingXP += Result.XPGained;
    CheckLevelUp(EGatheringSkill::Farming);
    
    return Result;
}

int32 UGatheringComponent::GetSeedRequiredLevel(ESeedType SeedType) const
{
    switch (SeedType)
    {
    case ESeedType::Wheat:      return 1;
    case ESeedType::Herb:      return 5;
    case ESeedType::Flower:    return 10;
    case ESeedType::Fruit:     return 25;
    case ESeedType::Magic:     return 50;
    case ESeedType::Fracture:  return 100;
    default: return 1;
    }
}

FString UGatheringComponent::GetSeedName(ESeedType SeedType) const
{
    switch (SeedType)
    {
    case ESeedType::Wheat:     return TEXT("Wheat Seed");
    case ESeedType::Herb:     return TEXT("Herb Seed");
    case ESeedType::Flower:   return TEXT("Flower Seed");
    case ESeedType::Fruit:    return TEXT("Fruit Seed");
    case ESeedType::Magic:    return TEXT("Magic Seed");
    case ESeedType::Fracture: return TEXT("Fracture Seed");
    default: return TEXT("Unknown Seed");
    }
}

int32 UGatheringComponent::GetSeedXP(ESeedType SeedType) const
{
    switch (SeedType)
    {
    case ESeedType::Wheat:     return 10;
    case ESeedType::Herb:     return 15;
    case ESeedType::Flower:   return 25;
    case ESeedType::Fruit:    return 40;
    case ESeedType::Magic:    return 100;
    case ESeedType::Fracture: return 200;
    default: return 10;
    }
}

// === LEVEL UP ===
void UGatheringComponent::CheckLevelUp(EGatheringSkill Skill)
{
    int32* Level = nullptr;
    int32* XP = nullptr;
    
    switch (Skill)
    {
    case EGatheringSkill::Mining:       Level = &MiningLevel;       XP = &MiningXP;       break;
    case EGatheringSkill::Skinning:     Level = &SkinningLevel;     XP = &SkinningXP;     break;
    case EGatheringSkill::Herbalism:    Level = &HerbalismLevel;    XP = &HerbalismXP;    break;
    case EGatheringSkill::Fishing:      Level = &FishingLevel;      XP = &FishingXP;      break;
    case EGatheringSkill::Woodcutting:  Level = &WoodcuttingLevel;  XP = &WoodcuttingXP;  break;
    case EGatheringSkill::Farming:      Level = &FarmingLevel;      XP = &FarmingXP;      break;
    }
    
    if (!Level || !XP) return;
    
    while (*XP >= GetXPForLevel(*Level + 1))
    {
        (*Level)++;
    }
}

int32 UGatheringComponent::GetXPForLevel(int32 TargetLevel) const
{
    // XP curve: 100 * level^1.5
    return FMath::RoundToInt(100.0f * FMath::Pow(static_cast<float>(TargetLevel), 1.5f));
}

int32 UGatheringComponent::GetSkillLevel(EGatheringSkill Skill) const
{
    switch (Skill)
    {
    case EGatheringSkill::Mining:       return MiningLevel;
    case EGatheringSkill::Skinning:     return SkinningLevel;
    case EGatheringSkill::Herbalism:    return HerbalismLevel;
    case EGatheringSkill::Fishing:      return FishingLevel;
    case EGatheringSkill::Woodcutting:  return WoodcuttingLevel;
    case EGatheringSkill::Farming:       return FarmingLevel;
    default: return 0;
    }
}

int32 UGatheringComponent::GetSkillXP(EGatheringSkill Skill) const
{
    switch (Skill)
    {
    case EGatheringSkill::Mining:       return MiningXP;
    case EGatheringSkill::Skinning:     return SkinningXP;
    case EGatheringSkill::Herbalism:    return HerbalismXP;
    case EGatheringSkill::Fishing:      return FishingXP;
    case EGatheringSkill::Woodcutting:  return WoodcuttingXP;
    case EGatheringSkill::Farming:      return FarmingXP;
    default: return 0;
    }
}
