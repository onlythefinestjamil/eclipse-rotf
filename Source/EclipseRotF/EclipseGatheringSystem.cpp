// EclipseGatheringSystem.cpp
// Implementation of the Gathering & Farming System.
// Mining, Skinning, Herbalism, Fishing, Woodcutting, Farming skills (0-300).

#include "EclipseGatheringSystem.h"

// ---------------------------------------------------------------------------
// Helper: clamp an enum-class index into a TMap key range
// ---------------------------------------------------------------------------

UEclipseGatheringSystem::UEclipseGatheringSystem()
{
    InitializeSkills();
    InitializeNodeDefaults();
}

// ---------------------------------------------------------------------------
// Skill initialization
// ---------------------------------------------------------------------------

void UEclipseGatheringSystem::InitializeSkills()
{
    SkillLevels.Add(EGatheringSkill::Mining,       0);
    SkillLevels.Add(EGatheringSkill::Skinning,     0);
    SkillLevels.Add(EGatheringSkill::Herbalism,    0);
    SkillLevels.Add(EGatheringSkill::Fishing,      0);
    SkillLevels.Add(EGatheringSkill::Woodcutting,  0);
    SkillLevels.Add(EGatheringSkill::Farming,      0);
    SkillLevels.Add(EGatheringSkill::Crafting,     0);
}

int32 UEclipseGatheringSystem::GetSkillLevel(EGatheringSkill Skill) const
{
    if (const int32* Found = SkillLevels.Find(Skill))
    {
        return *Found;
    }
    return 0;
}

bool UEclipseGatheringSystem::GainSkillXP(EGatheringSkill Skill, int32 XPAmount)
{
    if (XPAmount <= 0)
    {
        return false;
    }

    int32& Level = SkillLevels.FindOrAdd(Skill);
    Level += XPAmount;
    if (Level > 300)
    {
        Level = 300;
    }
    return true;
}

// ---------------------------------------------------------------------------
// Harvesting
// ---------------------------------------------------------------------------

bool UEclipseGatheringSystem::HarvestNode(FGatheringNode& Node)
{
    if (!CanHarvest(Node))
    {
        return false;
    }

    if (Node.CurrentYield <= 0)
    {
        return false;
    }

    // Decrement the node yield and award skill XP.
    Node.CurrentYield -= 1;
    Node.LastHarvestedTime = FDateTime::Now();

    if (Node.CurrentYield <= 0)
    {
        Node.bIsAvailable = false;
    }

    // Grant XP scaled by required skill level.
    const int32 GrantedXP = FMath::Max(5, Node.RequiredSkillLevel * 2);
    GainSkillXP(Node.RequiredSkill, GrantedXP);

    // Hand the materials to the player.
    AddMaterial(Node.NodeType, 1, Node.Region);

    // Rare drop chance: grant an extra material.
    if (FMath::FRand() < Node.RareDropChance)
    {
        AddMaterial(Node.NodeType, 1, Node.Region);
    }

    return true;
}

bool UEclipseGatheringSystem::CanHarvest(const FGatheringNode& Node) const
{
    if (!Node.bIsAvailable)
    {
        return false;
    }
    if (Node.CurrentYield <= 0)
    {
        return false;
    }
    return GetSkillLevel(Node.RequiredSkill) >= Node.RequiredSkillLevel;
}

// ---------------------------------------------------------------------------
// Inventory queries
// ---------------------------------------------------------------------------

TArray<FGatheredMaterial> UEclipseGatheringSystem::GetMaterialsByType(ENodeType Type) const
{
    TArray<FGatheredMaterial> Result;
    for (const FGatheredMaterial& Mat : PlayerInventory)
    {
        if (Mat.MaterialType == Type)
        {
            Result.Add(Mat);
        }
    }
    return Result;
}

TArray<FGatheredMaterial> UEclipseGatheringSystem::GetMaterialsBySkill(EGatheringSkill Skill) const
{
    TArray<FGatheredMaterial> Result;
    for (const FGatheredMaterial& Mat : PlayerInventory)
    {
        if (Mat.SourceSkill == Skill)
        {
            Result.Add(Mat);
        }
    }
    return Result;
}

void UEclipseGatheringSystem::AddMaterial(ENodeType Type, int32 Quantity, const FString& Region)
{
    if (Quantity <= 0)
    {
        return;
    }

    // Stack into an existing entry when possible.
    for (FGatheredMaterial& Mat : PlayerInventory)
    {
        if (Mat.MaterialType == Type && Mat.Region == Region)
        {
            Mat.Quantity += Quantity;
            return;
        }
    }

    FGatheredMaterial NewMat;
    NewMat.MaterialType     = Type;
    NewMat.MaterialName      = GetMaterialNameForNode(Type);
    NewMat.SourceSkill       = GetSkillForNode(Type);
    NewMat.Quantity          = Quantity;
    NewMat.MaterialAffinity  = GetAffinityForNode(Type);
    NewMat.Tier              = GetRarityForNode(Type);
    NewMat.Region            = Region;
    PlayerInventory.Add(NewMat);
}

bool UEclipseGatheringSystem::RemoveMaterial(ENodeType Type, int32 Quantity)
{
    if (Quantity <= 0)
    {
        return true;
    }

    if (!HasMaterial(Type, Quantity))
    {
        return false;
    }

    int32 Remaining = Quantity;
    for (int32 i = PlayerInventory.Num() - 1; i >= 0 && Remaining > 0; --i)
    {
        FGatheredMaterial& Mat = PlayerInventory[i];
        if (Mat.MaterialType != Type)
        {
            continue;
        }

        if (Mat.Quantity <= Remaining)
        {
            Remaining -= Mat.Quantity;
            PlayerInventory.RemoveAt(i);
        }
        else
        {
            Mat.Quantity -= Remaining;
            Remaining = 0;
        }
    }
    return true;
}

bool UEclipseGatheringSystem::HasMaterial(ENodeType Type, int32 Quantity) const
{
    if (Quantity <= 0)
    {
        return true;
    }

    int32 Total = 0;
    for (const FGatheredMaterial& Mat : PlayerInventory)
    {
        if (Mat.MaterialType == Type)
        {
            Total += Mat.Quantity;
            if (Total >= Quantity)
            {
                return true;
            }
        }
    }
    return Total >= Quantity;
}

// ---------------------------------------------------------------------------
// Node spawning / respawn
// ---------------------------------------------------------------------------

void UEclipseGatheringSystem::SpawnNodesForRegion(const FString& Region)
{
    if (Region.IsEmpty())
    {
        return;
    }

    // Spawn a handful of nodes per region call. The exact mix depends on the
    // region string so different regions produce different material flavors.
    const int32 NodesToSpawn = 6;

    for (int32 i = 0; i < NodesToSpawn; ++i)
    {
        FGatheringNode Node;
        Node.Region              = Region;
        Node.Location            = FVector(FMath::FRand() * 10000.0f,
                                            FMath::FRand() * 10000.0f,
                                            0.0f);
        Node.CurrentYield         = Node.MaxYield;
        Node.bIsAvailable         = true;
        Node.LastHarvestedTime    = FDateTime::MinValue();

        // Deterministically pick a node type for the slot so the region feels
        // stable across calls.
        switch (i % 6)
        {
        case 0: Node.NodeType = ENodeType::IronOre;       break;
        case 1: Node.NodeType = ENodeType::WolfHide;      break;
        case 2: Node.NodeType = ENodeType::Aetherbloom;   break;
        case 3: Node.NodeType = ENodeType::CommonFish;    break;
        case 4: Node.NodeType = ENodeType::Ironwood;     break;
        default:Node.NodeType = ENodeType::SilverOre;    break;
        }

        Node.RequiredSkill      = GetSkillForNode(Node.NodeType);
        Node.RequiredSkillLevel = GetRequiredSkillLevel(Node.NodeType);
        Node.RareDropChance     = 0.05f;
        ActiveNodes.Add(Node);
    }
}

void UEclipseGatheringSystem::CheckRespawns()
{
    const FDateTime Now = FDateTime::Now();
    for (FGatheringNode& Node : ActiveNodes)
    {
        if (Node.bIsAvailable)
        {
            continue;
        }
        if (Node.CurrentYield > 0)
        {
            Node.bIsAvailable = true;
            continue;
        }

        // Respawn after the configured number of minutes.
        const FTimespan RespawnSpan = FTimespan::FromMinutes(Node.RespawnTimeMinutes);
        if (Now - Node.LastHarvestedTime >= RespawnSpan)
        {
            Node.CurrentYield  = Node.MaxYield;
            Node.bIsAvailable   = true;
        }
    }
}

// ---------------------------------------------------------------------------
// Display names
// ---------------------------------------------------------------------------

FString UEclipseGatheringSystem::GetNodeDisplayName(ENodeType Type) const
{
    switch (Type)
    {
    case ENodeType::IronOre:         return TEXT("Iron Ore");
    case ENodeType::SilverOre:       return TEXT("Silver Ore");
    case ENodeType::GoldOre:         return TEXT("Gold Ore");
    case ENodeType::Obsidian:        return TEXT("Obsidian");
    case ENodeType::Moonstone:      return TEXT("Moonstone");
    case ENodeType::AetherCrystal:   return TEXT("Aether Crystal");
    case ENodeType::FracturedStone:  return TEXT("Fractured Stone");
    case ENodeType::SolarOre:        return TEXT("Solar Ore");
    case ENodeType::Voidstone:       return TEXT("Voidstone");
    case ENodeType::StormQuartz:     return TEXT("Storm Quartz");
    case ENodeType::Frostiron:      return TEXT("Frostiron");
    case ENodeType::Venomstone:     return TEXT("Venomstone");
    case ENodeType::Soulrite:        return TEXT("Soulrite");
    case ENodeType::TemporalGlass:   return TEXT("Temporal Glass");
    case ENodeType::WolfHide:         return TEXT("Wolf Hide");
    case ENodeType::BearHide:         return TEXT("Bear Hide");
    case ENodeType::DrakeScales:      return TEXT("Drake Scales");
    case ENodeType::SpiderSilk:      return TEXT("Spider Silk");
    case ENodeType::BoarHide:        return TEXT("Boar Hide");
    case ENodeType::SerpentSkin:      return TEXT("Serpent Skin");
    case ENodeType::ShadowCatPelt:    return TEXT("Shadow Cat Pelt");
    case ENodeType::AetherwolfPelt:  return TEXT("Aetherwolf Pelt");
    case ENodeType::FrostBearPelt:    return TEXT("Frost Bear Pelt");
    case ENodeType::VenomtoadSkin:    return TEXT("Venomtoad Skin");
    case ENodeType::Aetherbloom:     return TEXT("Aetherbloom");
    case ENodeType::Sunpetal:       return TEXT("Sunpetal");
    case ENodeType::Shadowcap:       return TEXT("Shadowcap Mushroom");
    case ENodeType::Frostleaf:      return TEXT("Frostleaf");
    case ENodeType::Emberflower:     return TEXT("Emberflower");
    case ENodeType::Stormroot:       return TEXT("Stormroot");
    case ENodeType::SoulMoss:        return TEXT("Soul Moss");
    case ENodeType::Voidfern:       return TEXT("Voidfern");
    case ENodeType::FracturedLotus: return TEXT("Fractured Lotus");
    case ENodeType::Moonwort:       return TEXT("Moonwort");
    case ENodeType::CommonFish:     return TEXT("Common Fish");
    case ENodeType::Aetherfish:     return TEXT("Aetherfish");
    case ENodeType::VoidEel:        return TEXT("Void Eel");
    case ENodeType::StormBass:      return TEXT("Storm Bass");
    case ENodeType::FrostSalmon:    return TEXT("Frost Salmon");
    case ENodeType::SoulCarp:       return TEXT("Soul Carp");
    case ENodeType::FracturedTrout: return TEXT("Fractured Trout");
    case ENodeType::Ironwood:       return TEXT("Ironwood");
    case ENodeType::Shadowwood:     return TEXT("Shadowwood");
    case ENodeType::Aetherwood:     return TEXT("Aetherwood");
    case ENodeType::Stormwood:      return TEXT("Stormwood");
    case ENodeType::FracturedOak:   return TEXT("Fractured Oak");
    }
    return TEXT("Unknown");
}

FString UEclipseGatheringSystem::GetSkillDisplayName(EGatheringSkill Skill) const
{
    switch (Skill)
    {
    case EGatheringSkill::Mining:       return TEXT("Mining");
    case EGatheringSkill::Skinning:     return TEXT("Skinning");
    case EGatheringSkill::Herbalism:    return TEXT("Herbalism");
    case EGatheringSkill::Fishing:      return TEXT("Fishing");
    case EGatheringSkill::Woodcutting:  return TEXT("Woodcutting");
    case EGatheringSkill::Farming:      return TEXT("Farming");
    case EGatheringSkill::Crafting:     return TEXT("Crafting");
    }
    return TEXT("Unknown");
}

// ---------------------------------------------------------------------------
// Node → skill / level / rarity / affinity mapping
// ---------------------------------------------------------------------------

EGatheringSkill UEclipseGatheringSystem::GetSkillForNode(ENodeType Type) const
{
    switch (Type)
    {
    // Mining
    case ENodeType::IronOre:
    case ENodeType::SilverOre:
    case ENodeType::GoldOre:
    case ENodeType::Obsidian:
    case ENodeType::Moonstone:
    case ENodeType::AetherCrystal:
    case ENodeType::FracturedStone:
    case ENodeType::SolarOre:
    case ENodeType::Voidstone:
    case ENodeType::StormQuartz:
    case ENodeType::Frostiron:
    case ENodeType::Venomstone:
    case ENodeType::Soulrite:
    case ENodeType::TemporalGlass:
        return EGatheringSkill::Mining;

    // Skinning
    case ENodeType::WolfHide:
    case ENodeType::BearHide:
    case ENodeType::DrakeScales:
    case ENodeType::SpiderSilk:
    case ENodeType::BoarHide:
    case ENodeType::SerpentSkin:
    case ENodeType::ShadowCatPelt:
    case ENodeType::AetherwolfPelt:
    case ENodeType::FrostBearPelt:
    case ENodeType::VenomtoadSkin:
        return EGatheringSkill::Skinning;

    // Herbalism
    case ENodeType::Aetherbloom:
    case ENodeType::Sunpetal:
    case ENodeType::Shadowcap:
    case ENodeType::Frostleaf:
    case ENodeType::Emberflower:
    case ENodeType::Stormroot:
    case ENodeType::SoulMoss:
    case ENodeType::Voidfern:
    case ENodeType::FracturedLotus:
    case ENodeType::Moonwort:
        return EGatheringSkill::Herbalism;

    // Fishing
    case ENodeType::CommonFish:
    case ENodeType::Aetherfish:
    case ENodeType::VoidEel:
    case ENodeType::StormBass:
    case ENodeType::FrostSalmon:
    case ENodeType::SoulCarp:
    case ENodeType::FracturedTrout:
        return EGatheringSkill::Fishing;

    // Woodcutting
    case ENodeType::Ironwood:
    case ENodeType::Shadowwood:
    case ENodeType::Aetherwood:
    case ENodeType::Stormwood:
    case ENodeType::FracturedOak:
        return EGatheringSkill::Woodcutting;
    }

    return EGatheringSkill::Mining;
}

int32 UEclipseGatheringSystem::GetRequiredSkillLevel(ENodeType Type) const
{
    switch (Type)
    {
    // Tier 1 — beginner
    case ENodeType::IronOre:
    case ENodeType::WolfHide:
    case ENodeType::BoarHide:
    case ENodeType::Aetherbloom:
    case ENodeType::Sunpetal:
    case ENodeType::CommonFish:
    case ENodeType::Ironwood:
        return 0;

    // Tier 2 — apprentice
    case ENodeType::SilverOre:
    case ENodeType::BearHide:
    case ENodeType::Shadowcap:
    case ENodeType::Frostleaf:
    case ENodeType::Aetherfish:
    case ENodeType::Shadowwood:
        return 30;

    // Tier 3 — journeyman
    case ENodeType::GoldOre:
    case ENodeType::DrakeScales:
    case ENodeType::SpiderSilk:
    case ENodeType::SerpentSkin:
    case ENodeType::Emberflower:
    case ENodeType::Stormroot:
    case ENodeType::VoidEel:
    case ENodeType::StormBass:
    case ENodeType::Aetherwood:
        return 60;

    // Tier 4 — expert
    case ENodeType::Obsidian:
    case ENodeType::Moonstone:
    case ENodeType::ShadowCatPelt:
    case ENodeType::SoulMoss:
    case ENodeType::FrostSalmon:
    case ENodeType::Stormwood:
        return 90;

    // Tier 5 — master
    case ENodeType::AetherCrystal:
    case ENodeType::AetherwolfPelt:
    case ENodeType::Voidfern:
    case ENodeType::SoulCarp:
        return 120;

    // Tier 6 — grandmaster
    case ENodeType::StormQuartz:
    case ENodeType::Frostiron:
    case ENodeType::FrostBearPelt:
    case ENodeType::FracturedLotus:
    case ENodeType::FracturedTrout:
    case ENodeType::FracturedOak:
        return 150;

    // Tier 7 — legendary
    case ENodeType::Venomstone:
    case ENodeType::VenomtoadSkin:
    case ENodeType::Moonwort:
        return 180;

    // Tier 8 — mythic
    case ENodeType::Soulrite:
    case ENodeType::SolarOre:
    case ENodeType::Voidstone:
        return 210;

    // Tier 9 — eclipse
    case ENodeType::FracturedStone:
    case ENodeType::TemporalGlass:
        return 240;
    }

    return 0;
}

ERarityTier UEclipseGatheringSystem::GetRarityForNode(ENodeType Type) const
{
    switch (Type)
    {
    case ENodeType::IronOre:
    case ENodeType::WolfHide:
    case ENodeType::BoarHide:
    case ENodeType::Aetherbloom:
    case ENodeType::Sunpetal:
    case ENodeType::CommonFish:
    case ENodeType::Ironwood:
        return ERarityTier::Common;

    case ENodeType::SilverOre:
    case ENodeType::BearHide:
    case ENodeType::Shadowcap:
    case ENodeType::Frostleaf:
    case ENodeType::Aetherfish:
    case ENodeType::Shadowwood:
        return ERarityTier::Uncommon;

    case ENodeType::GoldOre:
    case ENodeType::DrakeScales:
    case ENodeType::SpiderSilk:
    case ENodeType::SerpentSkin:
    case ENodeType::Emberflower:
    case ENodeType::Stormroot:
    case ENodeType::VoidEel:
    case ENodeType::StormBass:
    case ENodeType::Aetherwood:
        return ERarityTier::Rare;

    case ENodeType::Obsidian:
    case ENodeType::Moonstone:
    case ENodeType::ShadowCatPelt:
    case ENodeType::SoulMoss:
    case ENodeType::FrostSalmon:
    case ENodeType::Stormwood:
        return ERarityTier::Epic;

    case ENodeType::AetherCrystal:
    case ENodeType::AetherwolfPelt:
    case ENodeType::Voidfern:
    case ENodeType::SoulCarp:
        return ERarityTier::Legendary;

    case ENodeType::StormQuartz:
    case ENodeType::Frostiron:
    case ENodeType::FrostBearPelt:
    case ENodeType::FracturedLotus:
    case ENodeType::FracturedTrout:
    case ENodeType::FracturedOak:
    case ENodeType::Venomstone:
    case ENodeType::VenomtoadSkin:
    case ENodeType::Moonwort:
        return ERarityTier::Mythic;

    case ENodeType::Soulrite:
    case ENodeType::SolarOre:
    case ENodeType::Voidstone:
    case ENodeType::FracturedStone:
    case ENodeType::TemporalGlass:
        return ERarityTier::Eclipse;
    }

    return ERarityTier::Common;
}

EElementalAffinity UEclipseGatheringSystem::GetAffinityForNode(ENodeType Type) const
{
    switch (Type)
    {
    // Fire
    case ENodeType::SolarOre:
    case ENodeType::Emberflower:
        return EElementalAffinity::Fire;

    // Ice
    case ENodeType::Frostiron:
    case ENodeType::FrostSalmon:
    case ENodeType::Frostleaf:
    case ENodeType::FrostBearPelt:
        return EElementalAffinity::Ice;

    // Lightning
    case ENodeType::StormQuartz:
    case ENodeType::StormBass:
    case ENodeType::Stormroot:
    case ENodeType::Stormwood:
        return EElementalAffinity::Lightning;

    // Shadow
    case ENodeType::Voidstone:
    case ENodeType::Shadowwood:
    case ENodeType::Shadowcap:
    case ENodeType::ShadowCatPelt:
    case ENodeType::VoidEel:
    case ENodeType::Voidfern:
        return EElementalAffinity::Shadow;

    // Holy
    case ENodeType::Moonstone:
    case ENodeType::Sunpetal:
    case ENodeType::Moonwort:
        return EElementalAffinity::Holy;

    // Water
    case ENodeType::CommonFish:
    case ENodeType::Aetherfish:
        return EElementalAffinity::Water;

    // Soul
    case ENodeType::SoulMoss:
    case ENodeType::Soulrite:
    case ENodeType::SoulCarp:
        return EElementalAffinity::Soul;

    // Earth
    case ENodeType::IronOre:
    case ENodeType::SilverOre:
    case ENodeType::GoldOre:
    case ENodeType::Obsidian:
    case ENodeType::Ironwood:
    case ENodeType::FracturedStone:
    case ENodeType::BoarHide:
    case ENodeType::BearHide:
    case ENodeType::WolfHide:
        return EElementalAffinity::Earth;

    // Wind
    case ENodeType::Aetherbloom:
    case ENodeType::Aetherwood:
    case ENodeType::AetherwolfPelt:
    case ENodeType::AetherCrystal:
        return EElementalAffinity::Wind;

    // Poison
    case ENodeType::Venomstone:
    case ENodeType::VenomtoadSkin:
    case ENodeType::SerpentSkin:
    case ENodeType::SpiderSilk:
        return EElementalAffinity::Poison;

    // Temporal
    case ENodeType::TemporalGlass:
    case ENodeType::FracturedLotus:
    case ENodeType::FracturedTrout:
    case ENodeType::FracturedOak:
    case ENodeType::DrakeScales:
        return EElementalAffinity::Temporal;
    }

    return EElementalAffinity::Earth;
}

// ---------------------------------------------------------------------------
// Farming — planting and harvesting crops
// ---------------------------------------------------------------------------

bool UEclipseGatheringSystem::PlantSeed(ENodeType SeedType, const FString& PlotId)
{
    if (PlotId.IsEmpty())
    {
        return false;
    }

    if (GetSkillLevel(EGatheringSkill::Farming) <= 0)
    {
        return false;
    }

    // Replanting an already-occupied plot is not allowed.
    for (const FGatheringNode& Node : ActiveNodes)
    {
        if (Node.Region == PlotId && Node.bIsAvailable && Node.CurrentYield > 0)
        {
            // Plot already has a growing crop — refuse.
            return false;
        }
    }

    FGatheringNode Crop;
    Crop.NodeType            = SeedType;
    Crop.RequiredSkill       = EGatheringSkill::Farming;
    Crop.RequiredSkillLevel  = GetRequiredSkillLevel(SeedType);
    Crop.Region              = PlotId;
    Crop.Location            = FVector::ZeroVector;
    Crop.CurrentYield         = 1;
    Crop.MaxYield            = 1;
    Crop.RespawnTimeMinutes   = 5.0f;
    Crop.bIsAvailable         = true;
    Crop.RareDropChance       = 0.0f;
    Crop.LastHarvestedTime    = FDateTime::Now();
    ActiveNodes.Add(Crop);

    GainSkillXP(EGatheringSkill::Farming, 5);
    return true;
}

bool UEclipseGatheringSystem::HarvestCrop(const FString& PlotId)
{
    if (PlotId.IsEmpty())
    {
        return false;
    }

    for (FGatheringNode& Node : ActiveNodes)
    {
        if (Node.Region != PlotId || Node.RequiredSkill != EGatheringSkill::Farming)
        {
            continue;
        }
        if (!IsCropMature(PlotId))
        {
            return false;
        }

        AddMaterial(Node.NodeType, 1, PlotId);
        GainSkillXP(EGatheringSkill::Farming, 10);
        Node.CurrentYield = 0;
        Node.bIsAvailable  = false;
        Node.LastHarvestedTime = FDateTime::Now();
        return true;
    }
    return false;
}

bool UEclipseGatheringSystem::IsCropMature(const FString& PlotId) const
{
    if (PlotId.IsEmpty())
    {
        return false;
    }

    const FDateTime Now = FDateTime::Now();
    for (const FGatheringNode& Node : ActiveNodes)
    {
        if (Node.Region != PlotId || Node.RequiredSkill != EGatheringSkill::Farming)
        {
            continue;
        }
        if (!Node.bIsAvailable || Node.CurrentYield <= 0)
        {
            return false;
        }

        // Crop is mature once the respawn timer has elapsed since planting.
        const FTimespan MaturitySpan = FTimespan::FromMinutes(Node.RespawnTimeMinutes);
        return (Now - Node.LastHarvestedTime) >= MaturitySpan;
    }
    return false;
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void UEclipseGatheringSystem::InitializeNodeDefaults()
{
    // Seed the world with a small starter set of nodes so the system is useful
    // the moment it is constructed.
    if (ActiveNodes.Num() > 0)
    {
        return;
    }
    SpawnNodesForRegion(TEXT("Solara"));
}

FString UEclipseGatheringSystem::GetMaterialNameForNode(ENodeType Type) const
{
    // The material name mirrors the node display name for gathering drops.
    return GetNodeDisplayName(Type);
}
