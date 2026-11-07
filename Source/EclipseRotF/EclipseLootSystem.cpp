// Eclipse Loot System Implementation
// Percentage-based drops, class drops for all classes, binding system

#include "EclipseLootSystem.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"

UEclipseLootSystem::UEclipseLootSystem()
{
    InitializeDropTables();
}

void UEclipseLootSystem::InitializeDropTables()
{
    // Base enemy drop rates (percentage)
    BaseDropRates.Add(ERarityTier::Common,    50.0f);
    BaseDropRates.Add(ERarityTier::Uncommon,  25.0f);
    BaseDropRates.Add(ERarityTier::Rare,      15.0f);
    BaseDropRates.Add(ERarityTier::Epic,       6.0f);
    BaseDropRates.Add(ERarityTier::Legendary,  3.0f);
    BaseDropRates.Add(ERarityTier::Mythic,     0.8f);
    BaseDropRates.Add(ERarityTier::Eclipse,    0.2f);

    // Boss drop rates (higher — guaranteed rare+, chance for legendary+)
    BossDropRates.Add(ERarityTier::Common,     0.0f); // No common from bosses
    BossDropRates.Add(ERarityTier::Uncommon,   10.0f);
    BossDropRates.Add(ERarityTier::Rare,      30.0f);
    BossDropRates.Add(ERarityTier::Epic,      35.0f);
    BossDropRates.Add(ERarityTier::Legendary, 15.0f);
    BossDropRates.Add(ERarityTier::Mythic,     7.0f);
    BossDropRates.Add(ERarityTier::Eclipse,    3.0f);
}

TArray<FLootDrop> UEclipseLootSystem::GenerateLoot(int32 EnemyLevel, ERarityTier EnemyTier, float LuckStat)
{
    TArray<FLootDrop> Drops;
    float Luck = FMath::Max(0.0f, LuckStat); // Luck can't be negative
    float LuckMod = 1.0f + (Luck * 0.01f); // Each luck point = +1% drop chance

    // Roll for number of drops (1-3 for normal enemies)
    int32 NumDrops = FMath::RandRange(1, 3);

    for (int32 i = 0; i < NumDrops; i++)
    {
        // Roll rarity
        float Roll = FMath::FRandRange(0.0f, 100.0f) / LuckMod;
        float Cumulative = 0.0f;
        ERarityTier DropRarity = ERarityTier::Common;

        for (auto& Pair : BaseDropRates)
        {
            Cumulative += Pair.Value;
            if (Roll <= Cumulative)
            {
                DropRarity = Pair.Key;
                break;
            }
        }

        FLootDrop Drop;
        Drop.Rarity = DropRarity;
        Drop.ItemLevel = EnemyLevel;
        Drop.DropChance = BaseDropRates[DropRarity];
        Drop.Binding = DetermineBinding(DropRarity, false);

        // 30% chance for class-specific drop
        // When it drops, it drops for ALL classes
        if (FMath::FRand() < 0.3f)
        {
            Drop.DropType = EDropType::ClassSpecific;
            // ExpandClassDrop will create versions for all 17 classes
        }
        else
        {
            Drop.DropType = EDropType::Universal;
        }

        Drops.Add(Drop);
    }

    return Drops;
}

TArray<FLootDrop> UEclipseLootSystem::GenerateBossLoot(int32 BossLevel, ERarityTier BossTier)
{
    TArray<FLootDrop> Drops;

    // Bosses always drop at least 2 items, up to 5
    int32 NumDrops = FMath::RandRange(2, 5);

    for (int32 i = 0; i < NumDrops; i++)
    {
        float Roll = FMath::FRandRange(0.0f, 100.0f);
        float Cumulative = 0.0f;
        ERarityTier DropRarity = ERarityTier::Rare;

        for (auto& Pair : BossDropRates)
        {
            Cumulative += Pair.Value;
            if (Roll <= Cumulative)
            {
                DropRarity = Pair.Key;
                break;
            }
        }

        FLootDrop Drop;
        Drop.Rarity = DropRarity;
        Drop.ItemLevel = BossLevel;
        Drop.DropChance = BossDropRates[DropRarity];
        Drop.Binding = DetermineBinding(DropRarity, true); // Boss drops more likely soulbound

        // 50% chance for class-specific boss drop
        if (FMath::FRand() < 0.5f)
        {
            Drop.DropType = EDropType::ClassSpecific;
        }
        else
        {
            Drop.DropType = EDropType::Universal;
        }

        Drops.Add(Drop);
    }

    return Drops;
}

TArray<FLootDrop> UEclipseLootSystem::GenerateDungeonLoot(int32 DungeonLevel, ERarityTier DungeonTier)
{
    TArray<FLootDrop> Drops;
    
    // Dungeon end loot: 3-5 items, guaranteed rare+
    int32 NumDrops = FMath::RandRange(3, 5);

    for (int32 i = 0; i < NumDrops; i++)
    {
        ERarityTier MinRarity = (DungeonTier >= ERarityTier::Epic) ? ERarityTier::Epic : ERarityTier::Rare;
        
        float Roll = FMath::FRandRange(0.0f, 100.0f);
        float Cumulative = 0.0f;
        ERarityTier DropRarity = MinRarity;

        for (auto& Pair : BossDropRates)
        {
            if (Pair.Key < MinRarity) continue; // Skip below minimum
            Cumulative += Pair.Value;
            if (Roll <= Cumulative)
            {
                DropRarity = Pair.Key;
                break;
            }
        }

        FLootDrop Drop;
        Drop.Rarity = DropRarity;
        Drop.ItemLevel = DungeonLevel;
        Drop.Binding = DetermineBinding(DropRarity, true);
        Drop.DropType = EDropType::Universal;

        Drops.Add(Drop);
    }

    return Drops;
}

TArray<FLootDrop> UEclipseLootSystem::ExpandClassDrop(const FLootDrop& ClassItem)
{
    TArray<FLootDrop> AllClassVersions;

    // When a class-specific item drops, it drops for ALL 17 classes
    // Each player gets the version for their class
    TArray<EClass> AllClasses = {
        EClass::Bulwark, EClass::Duskblade, EClass::Arcanist,
        EClass::Riftwalker, EClass::Dreadknight, EClass::Templar,
        EClass::Soulbinder, EClass::Wildwarden, EClass::Stormcaller,
        EClass::Oracle, EClass::Alchemist, EClass::Berserker,
        EClass::Shadowblade, EClass::Spellblade, EClass::Warthane,
        EClass::Necroturge, EClass::Earthweaver
    };

    for (EClass Class : AllClasses)
    {
        FLootDrop ClassVersion = ClassItem;
        // Same rarity, same level, same binding — just different class version
        // Item name would be class-specific (e.g., "Sunbreaker Greatblade" for Bulwark,
        // "Veilfang Daggers" for Duskblade, same drop, different class item)
        AllClassVersions.Add(ClassVersion);
    }

    return AllClassVersions;
}

bool UEclipseLootSystem::RollDrop(float Chance, float LuckModifier)
{
    float AdjustedChance = Chance * LuckModifier;
    float Roll = FMath::FRandRange(0.0f, 100.0f);
    return Roll <= AdjustedChance;
}

EBindingType UEclipseLootSystem::DetermineBinding(ERarityTier Rarity, bool bIsBossDrop)
{
    // Binding rules:
    // Common/Uncommon: Bind on Equip (tradeable)
    // Rare: Bind on Equip (tradeable)
    // Epic: 50% BoE, 50% Soulbound
    // Legendary: Soulbound on Pickup (can't trade)
    // Mythic: Soulbound (can't trade)
    // Eclipse: Soulbound + Account Bound (transfer between your characters only)

    switch (Rarity)
    {
    case ERarityTier::Common:
    case ERarityTier::Uncommon:
    case ERarityTier::Rare:
        return EBindingType::BindOnEquip;

    case ERarityTier::Epic:
        if (bIsBossDrop)
        {
            return FMath::FRand() < 0.7f ? EBindingType::Soulbound : EBindingType::BindOnEquip;
        }
        return FMath::FRand() < 0.5f ? EBindingType::Soulbound : EBindingType::BindOnEquip;

    case ERarityTier::Legendary:
        return FMath::FRand() < 0.8f ? EBindingType::Soulbound : EBindingType::BindOnEquip;

    case ERarityTier::Mythic:
        return EBindingType::Soulbound;

    case ERarityTier::Eclipse:
        return EBindingType::AccountBound; // Eclipse items transfer between your characters

    default:
        return EBindingType::BindOnEquip;
    }
}

bool UEclipseLootSystem::CanTrade(const FLootDrop& Item, bool bIsEquipped)
{
    switch (Item.Binding)
    {
    case EBindingType::None:
        return true; // Freely tradeable

    case EBindingType::BindOnEquip:
        return !bIsEquipped; // Can trade if not equipped yet

    case EBindingType::Soulbound:
        return false; // Can never trade

    case EBindingType::AccountBound:
        return false; // Can't trade to other players, but transfers to your characters

    default:
        return false;
    }
}

bool UEclipseLootSystem::IsSoulbound(const FLootDrop& Item)
{
    return Item.Binding == EBindingType::Soulbound || Item.Binding == EBindingType::AccountBound;
}
