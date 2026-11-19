// EclipseBossHideCrafting.cpp — Implementation
// Boss drops body parts → craft weapons AND armor

#include "EclipseBossHideCrafting.h"
#include "EclipseCharacter.h"

// === BOSS HIDE CRAFTING COMPONENT ===
UBossHideCraftingComponent::UBossHideCraftingComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UBossHideCraftingComponent::BeginPlay()
{
    Super::BeginPlay();
}

// === GET DROPS ===
TArray<FBossMaterialDrop> UBossHideCraftingComponent::GetBossDrops(EBossType BossType) const
{
    TArray<FBossMaterialDrop> Drops;
    
    switch (BossType)
    {
    case EBossType::PyreWardenVelkor:
        Drops.Add(FBossMaterialDrop(EBossMaterial::Horns, TEXT("Velkor's Horns"), 1, 2));
        Drops.Add(FBossMaterialDrop(EBossMaterial::Hide, TEXT("Velkor's Hide"), 1, 3));
        Drops.Add(FBossMaterialDrop(EBossMaterial::Bones, TEXT("Velkor's Bones"), 2, 4));
        break;
    case EBossType::ShadowMonarch:
        Drops.Add(FBossMaterialDrop(EBossMaterial::Claws, TEXT("Shadow Claws"), 2, 3));
        Drops.Add(FBossMaterialDrop(EBossMaterial::Scales, TEXT("Shadow Scales"), 1, 4));
        Drops.Add(FBossMaterialDrop(EBossMaterial::Soul, TEXT("Shadow Soul"), 1, 1));
        break;
    case EBossType::FractureQueen:
        Drops.Add(FBossMaterialDrop(EBossMaterial::Heart, TEXT("Fracture Heart"), 1, 1));
        Drops.Add(FBossMaterialDrop(EBossMaterial::Scales, TEXT("Fracture Scales"), 2, 4));
        Drops.Add(FBossMaterialDrop(EBossMaterial::Horns, TEXT("Fracture Horns"), 1, 2));
        break;
    default:
        Drops.Add(FBossMaterialDrop(EBossMaterial::Bones, TEXT("Generic Bones"), 1, 4));
        Drops.Add(FBossMaterialDrop(EBossMaterial::Hide, TEXT("Generic Hide"), 1, 3));
        break;
    }
    
    return Drops;
}

// === CAN CRAFT ===
bool UBossHideCraftingComponent::CanCraft(EBossMaterial MaterialType, EBossCraftedItemType ItemType) const
{
    // Check if we have enough of this material
    int32 Available = GetMaterialCount(MaterialType);
    int32 Required = GetMaterialRequired(MaterialType, ItemType);
    return Available >= Required;
}

// === CRAFT ===
FCraftResult UBossHideCraftingComponent::CraftItem(EBossMaterial MaterialType, EBossCraftedItemType ItemType)
{
    FCraftResult Result;
    
    if (!CanCraft(MaterialType, ItemType))
    {
        Result.bSuccess = false;
        Result.FailureReason = TEXT("Not enough boss materials.");
        return Result;
    }
    
    int32 Required = GetMaterialRequired(MaterialType, ItemType);
    
    // Consume materials
    ConsumeMaterial(MaterialType, Required);
    
    // Create the item
    Result.bSuccess = true;
    Result.ItemName = GetCraftedItemName(MaterialType, ItemType);
    Result.ItemType = ItemType;
    Result.Rarity = GetCraftedItemRarity(ItemType);
    Result.PassiveBonus = GetPassiveBonus(MaterialType);
    Result.PassiveBonusAmount = GetPassiveBonusAmount(MaterialType);
    Result.bSoulbound = true; // Boss items are always soulbound on pickup
    
    return Result;
}

// === MATERIAL REQUIREMENTS ===
int32 UBossHideCraftingComponent::GetMaterialRequired(EBossMaterial MaterialType, EBossCraftedItemType ItemType) const
{
    switch (MaterialType)
    {
    case EBossMaterial::Fangs:
        switch (ItemType)
        {
        case EBossCraftedItemType::Dagger:    return 2;
        case EBossCraftedItemType::Sword:     return 4;
        case EBossCraftedItemType::Spear:     return 3;
        default: return 99;
        }
    case EBossMaterial::Scales:
        switch (ItemType)
        {
        case EBossCraftedItemType::Greatsword:    return 5;
        case EBossCraftedItemType::Shield:        return 3;
        case EBossCraftedItemType::Chestplate:     return 6;
        default: return 99;
        }
    case EBossMaterial::Hide:
        switch (ItemType)
        {
        case EBossCraftedItemType::Cloak:         return 3;
        case EBossCraftedItemType::Armor:         return 5;
        case EBossCraftedItemType::Greaves:       return 4;
        default: return 99;
        }
    case EBossMaterial::Horns:
        switch (ItemType)
        {
        case EBossCraftedItemType::Greatsword:    return 4;
        case EBossCraftedItemType::Helm:          return 2;
        case EBossCraftedItemType::Shield:        return 3;
        default: return 99;
        }
    case EBossMaterial::Claws:
        switch (ItemType)
        {
        case EBossCraftedItemType::Dagger:        return 3;
        case EBossCraftedItemType::Gauntlets:     return 4;
        default: return 99;
        }
    case EBossMaterial::Bones:
        switch (ItemType)
        {
        case EBossCraftedItemType::Staff:        return 5;
        case EBossCraftedItemType::Greaves:       return 3;
        case EBossCraftedItemType::Amulet:       return 2;
        default: return 99;
        }
    case EBossMaterial::Heart:
        // Heart can craft ANY weapon OR full armor set
        return 1;
    case EBossMaterial::Soul:
        // Soul can craft ANY weapon OR armor at Mythic tier
        return 1;
    default:
        return 99;
    }
}

// === CRAFTED ITEM NAMES ===
FString UBossHideCraftingComponent::GetCraftedItemName(EBossMaterial MaterialType, EBossCraftedItemType ItemType) const
{
    FString MaterialName = GetMaterialDisplayName(MaterialType);
    FString ItemNameStr = GetItemTypeDisplayName(ItemType);
    return MaterialName + TEXT(" ") + ItemNameStr;
}

FString UBossHideCraftingComponent::GetMaterialDisplayName(EBossMaterial MaterialType) const
{
    switch (MaterialType)
    {
    case EBossMaterial::Fangs:   return TEXT("Fang");
    case EBossMaterial::Scales:  return TEXT("Scale");
    case EBossMaterial::Hide:    return TEXT("Hide");
    case EBossMaterial::Horns:   return TEXT("Horn");
    case EBossMaterial::Claws:   return TEXT("Claw");
    case EBossMaterial::Bones:   return TEXT("Bone");
    case EBossMaterial::Heart:   return TEXT("Heart");
    case EBossMaterial::Soul:    return TEXT("Soul");
    default: return TEXT("Unknown");
    }
}

FString UBossHideCraftingComponent::GetItemTypeDisplayName(EBossCraftedItemType ItemType) const
{
    switch (ItemType)
    {
    case EBossCraftedItemType::Dagger:       return TEXT("Dagger");
    case EBossCraftedItemType::Sword:        return TEXT("Sword");
    case EBossCraftedItemType::Spear:       return TEXT("Spear");
    case EBossCraftedItemType::Greatsword:  return TEXT("Greatsword");
    case EBossCraftedItemType::Shield:      return TEXT("Shield");
    case EBossCraftedItemType::Staff:        return TEXT("Staff");
    case EBossCraftedItemType::Helm:         return TEXT("Helm");
    case EBossCraftedItemType::Chestplate:  return TEXT("Chestplate");
    case EBossCraftedItemType::Greaves:     return TEXT("Greaves");
    case EBossCraftedItemType::Gauntlets:   return TEXT("Gauntlets");
    case EBossCraftedItemType::Cloak:       return TEXT("Cloak");
    case EBossCraftedItemType::Armor:       return TEXT("Armor");
    case EBossCraftedItemType::Amulet:      return TEXT("Amulet");
    case EBossCraftedItemType::FullSet:      return TEXT("Full Set");
    default: return TEXT("Unknown");
    }
}

// === RARITY ===
ERarity UBossHideCraftingComponent::GetCraftedItemRarity(EBossCraftedItemType ItemType) const
{
    switch (ItemType)
    {
    case EBossCraftedItemType::Dagger:
    case EBossCraftedItemType::Sword:
    case EBossCraftedItemType::Spear:
        return ERarity::Epic;
    case EBossCraftedItemType::Greatsword:
    case EBossCraftedItemType::Shield:
    case EBossCraftedItemType::Staff:
        return ERarity::Legendary;
    case EBossCraftedItemType::Helm:
    case EBossCraftedItemType::Chestplate:
    case EBossCraftedItemType::Greaves:
    case EBossCraftedItemType::Gauntlets:
    case EBossCraftedItemType::Cloak:
    case EBossCraftedItemType::Armor:
        return ERarity::Epic;
    case EBossCraftedItemType::Amulet:
        return ERarity::Legendary;
    case EBossCraftedItemType::FullSet:
        return ERarity::Mythic;
    default:
        return ERarity::Epic;
    }
}

// === PASSIVE BONUSES ===
FString UBossHideCraftingComponent::GetPassiveBonus(EBossMaterial MaterialType) const
{
    switch (MaterialType)
    {
    case EBossMaterial::Fangs:   return TEXT("Critical Damage");
    case EBossMaterial::Scales:  return TEXT("Defense");
    case EBossMaterial::Hide:    return TEXT("Health");
    case EBossMaterial::Horns:   return TEXT("Attack Power");
    case EBossMaterial::Claws:   return TEXT("Attack Speed");
    case EBossMaterial::Bones:   return TEXT("Spell Power");
    case EBossMaterial::Heart:   return TEXT("All Stats");
    case EBossMaterial::Soul:    return TEXT("All Stats + Elemental");
    default: return TEXT("None");
    }
}

float UBossHideCraftingComponent::GetPassiveBonusAmount(EBossMaterial MaterialType) const
{
    switch (MaterialType)
    {
    case EBossMaterial::Fangs:   return 0.10f; // +10% crit damage
    case EBossMaterial::Scales:  return 0.15f; // +15% defense
    case EBossMaterial::Hide:    return 0.10f; // +10% health
    case EBossMaterial::Horns:   return 0.12f; // +12% attack
    case EBossMaterial::Claws:   return 0.10f; // +10% attack speed
    case EBossMaterial::Bones:   return 0.15f; // +15% spell power
    case EBossMaterial::Heart:   return 0.15f; // +15% all stats
    case EBossMaterial::Soul:    return 0.20f; // +20% all stats + elemental
    default: return 0.0f;
    }
}

// === INVENTORY MANAGEMENT ===
int32 UBossHideCraftingComponent::GetMaterialCount(EBossMaterial MaterialType) const
{
    const FBossMaterialInventory* Found = BossMaterials.Find(MaterialType);
    return Found ? Found->Count : 0;
}

void UBossHideCraftingComponent::AddMaterial(EBossMaterial MaterialType, int32 Quantity)
{
    FBossMaterialInventory& Inv = BossMaterials.FindOrAdd(MaterialType);
    Inv.MaterialType = MaterialType;
    Inv.Count += Quantity;
}

void UBossHideCraftingComponent::ConsumeMaterial(EBossMaterial MaterialType, int32 Quantity)
{
    FBossMaterialInventory* Found = BossMaterials.Find(MaterialType);
    if (Found)
    {
        Found->Count = FMath::Max(0, Found->Count - Quantity);
    }
}

// === HEART/SOUL CRAFTING ===
FCraftResult UBossHideCraftingComponent::CraftFromHeart(EBossCraftedItemType ItemType)
{
    // Heart = any weapon OR full armor set at Legendary
    FCraftResult Result;
    
    if (GetMaterialCount(EBossMaterial::Heart) < 1)
    {
        Result.bSuccess = false;
        Result.FailureReason = TEXT("No Heart available.");
        return Result;
    }
    
    ConsumeMaterial(EBossMaterial::Heart, 1);
    
    Result.bSuccess = true;
    Result.ItemName = TEXT("Heart ") + GetItemTypeDisplayName(ItemType);
    Result.ItemType = ItemType;
    Result.Rarity = ERarity::Legendary;
    Result.PassiveBonus = TEXT("All Stats");
    Result.PassiveBonusAmount = 0.15f;
    Result.bSoulbound = true;
    
    return Result;
}

FCraftResult UBossHideCraftingComponent::CraftFromSoul(EBossCraftedItemType ItemType)
{
    // Soul = any weapon OR armor at Mythic
    FCraftResult Result;
    
    if (GetMaterialCount(EBossMaterial::Soul) < 1)
    {
        Result.bSuccess = false;
        Result.FailureReason = TEXT("No Soul available.");
        return Result;
    }
    
    ConsumeMaterial(EBossMaterial::Soul, 1);
    
    Result.bSuccess = true;
    Result.ItemName = TEXT("Soul ") + GetItemTypeDisplayName(ItemType);
    Result.ItemType = ItemType;
    Result.Rarity = ERarity::Mythic;
    Result.PassiveBonus = TEXT("All Stats + Elemental");
    Result.PassiveBonusAmount = 0.20f;
    Result.bSoulbound = true;
    
    return Result;
}
