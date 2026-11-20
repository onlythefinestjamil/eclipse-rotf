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

// === COOPERATIVE SPOILS SYSTEM ===
// Every participant gets their own personal loot — no competing, no rolling
// The dungeon is conquered together → spoils are earned together

TArray<FLootDrop> UEclipseLootSystem::GeneratePersonalLoot(int32 PlayerLevel, EClass PlayerClass, int32 EnemyLevel, ERarityTier EnemyTier, float LuckStat)
{
    TArray<FLootDrop> Drops;
    float Luck = FMath::Max(0.0f, LuckStat);
    float LuckMod = 1.0f + (Luck * 0.01f);

    // Each player rolls independently — no shared pool, no competing
    int32 NumDrops = FMath::RandRange(1, 2);

    for (int32 i = 0; i < NumDrops; i++)
    {
        float Roll = FMath::FRandRange(0.0f, 100.0f) / LuckMod;
        float Cumulative = 0.0f;
        ERarityTier DropRarity = ERarityTier::Common;

        for (const auto& Pair : BaseDropRates)
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
        Drop.ItemLevel = FMath::Max(EnemyLevel, PlayerLevel);
        Drop.DropType = EDropType::Universal;
        Drop.Binding = DetermineBinding(DropRarity, false);
        Drop.DropChance = BaseDropRates[DropRarity];
        Drops.Add(Drop);
    }

    // Minimum guarantee — every kill gives at least 1 item
    if (Drops.Num() == 0)
    {
        FLootDrop Drop;
        Drop.Rarity = ERarityTier::Common;
        Drop.ItemLevel = EnemyLevel;
        Drop.DropType = EDropType::Universal;
        Drop.Binding = EBindingType::BindOnEquip;
        Drop.DropChance = 100.0f;
        Drops.Add(Drop);
    }

    return Drops;
}

TArray<FLootDrop> UEclipseLootSystem::GenerateBossPersonalSpoils(int32 PlayerLevel, EClass PlayerClass, int32 BossLevel, ERarityTier BossTier)
{
    TArray<FLootDrop> Drops;

    // Each player gets 1-2 personal drops from the boss material table
    int32 NumDrops = FMath::RandRange(1, 2);

    for (int32 i = 0; i < NumDrops; i++)
    {
        FLootDrop Drop;
        float Roll = FMath::FRandRange(0.0f, 100.0f);

        if (Roll < 2.0f)        Drop.Rarity = ERarityTier::Eclipse;
        else if (Roll < 8.0f)   Drop.Rarity = ERarityTier::Mythic;
        else if (Roll < 25.0f)  Drop.Rarity = ERarityTier::Legendary;
        else if (Roll < 55.0f)  Drop.Rarity = ERarityTier::Epic;
        else if (Roll < 85.0f)  Drop.Rarity = ERarityTier::Rare;
        else                     Drop.Rarity = ERarityTier::Uncommon;

        Drop.ItemLevel = BossLevel;
        Drop.DropType = EDropType::ClassSpecific;
        Drop.Binding = EBindingType::Soulbound;
        Drop.DropChance = 100.0f;
        Drop.bIsCraftingMaterial = true;
        Drops.Add(Drop);
    }

    return Drops;
}

TArray<FLootDrop> UEclipseLootSystem::GenerateCompletionBonus(int32 PlayerLevel, EClass PlayerClass, int32 DungeonLevel, ERarityTier DungeonTier)
{
    TArray<FLootDrop> Drops;

    // Guaranteed drop for every participant who completes the dungeon
    FLootDrop Guaranteed;
    Guaranteed.Rarity = (DungeonTier >= ERarityTier::Epic) ? ERarityTier::Epic : ERarityTier::Rare;
    Guaranteed.ItemLevel = FMath::Max(DungeonLevel, PlayerLevel);
    Guaranteed.DropType = EDropType::ClassSpecific;
    Guaranteed.Binding = EBindingType::BindOnEquip;
    Guaranteed.bIsArmor = true;
    Drops.Add(Guaranteed);

    // 30% chance for bonus weapon
    if (FMath::FRandRange(0.0f, 100.0f) <= 30.0f)
    {
        FLootDrop Bonus;
        Bonus.Rarity = ERarityTier::Epic;
        Bonus.ItemLevel = DungeonLevel;
        Bonus.DropType = EDropType::ClassSpecific;
        Bonus.Binding = EBindingType::BindOnEquip;
        Bonus.bIsWeapon = true;
        Drops.Add(Bonus);
    }

    return Drops;
}

TArray<FLootDrop> UEclipseLootSystem::GenerateRaidCompletionSpoils(int32 PlayerLevel, EClass PlayerClass, int32 RaidLevel, ERarityTier RaidTier)
{
    TArray<FLootDrop> Drops;

    // Guaranteed drop 1 — armor (soulbound)
    FLootDrop Spoil1;
    Spoil1.Rarity = (RaidTier >= ERarityTier::Mythic) ? ERarityTier::Legendary : ERarityTier::Epic;
    Spoil1.ItemLevel = FMath::Max(RaidLevel, PlayerLevel);
    Spoil1.DropType = EDropType::ClassSpecific;
    Spoil1.Binding = EBindingType::Soulbound;
    Spoil1.bIsArmor = true;
    Drops.Add(Spoil1);

    // Guaranteed drop 2 — weapon (soulbound)
    FLootDrop Spoil2;
    Spoil2.Rarity = ERarityTier::Legendary;
    Spoil2.ItemLevel = RaidLevel;
    Spoil2.DropType = EDropType::ClassSpecific;
    Spoil2.Binding = EBindingType::Soulbound;
    Spoil2.bIsWeapon = true;
    Drops.Add(Spoil2);

    // 10% chance for Mythic crafting material
    if (FMath::FRandRange(0.0f, 100.0f) <= 10.0f)
    {
        FLootDrop RareBonus;
        RareBonus.Rarity = ERarityTier::Mythic;
        RareBonus.ItemLevel = RaidLevel;
        RareBonus.DropType = EDropType::ClassSpecific;
        RareBonus.Binding = EBindingType::Soulbound;
        RareBonus.bIsCraftingMaterial = true;
        Drops.Add(RareBonus);
    }

    return Drops;
}

int32 UEclipseLootSystem::GetMinimumGuaranteedDrops(ERarityTier ContentTier) const
{
    switch (ContentTier)
    {
    case ERarityTier::Common:    return 1;
    case ERarityTier::Uncommon:  return 1;
    case ERarityTier::Rare:      return 1;
    case ERarityTier::Epic:      return 2;
    case ERarityTier::Legendary:  return 2;
    case ERarityTier::Mythic:     return 2;
    case ERarityTier::Eclipse:    return 3;
    default: return 1;
    }
}
