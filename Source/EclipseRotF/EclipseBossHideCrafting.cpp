// EclipseBossHideCrafting.cpp
// Implementation of the boss-hide crafting system.
// Kill a boss -> get boss parts (hide, fangs, scales, bones, heart, soul)
// -> craft weapons or armor at the Blacksmith forge.

#include "EclipseBossHideCrafting.h"

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

UEclipseBossHideCrafting::UEclipseBossHideCrafting()
{
    InitializeBossDropMap();
}

void UEclipseBossHideCrafting::InitializeBossDropMap()
{
    // Mapping of boss names to their primary boss-material drop.
    // Each boss has a signature material that drives its crafted gear.
    BossDropMap.Add(TEXT("Pyrothragma"),         EBossMaterial::VolcanicCinderhide);
    BossDropMap.Add(TEXT("Embergheist"),          EBossMaterial::AshenHuskLeather);
    BossDropMap.Add(TEXT("Solairus"),             EBossMaterial::SolarForgeScale);
    BossDropMap.Add(TEXT("Umbraxis"),            EBossMaterial::ShadowrendHide);
    BossDropMap.Add(TEXT("Verdantyr"),            EBossMaterial::VerdantThornhide);
    BossDropMap.Add(TEXT("Aetherion"),           EBossMaterial::AetherBoundPelt);
    BossDropMap.Add(TEXT("StormcallerVael"),      EBossMaterial::StormstrikeHide);
    BossDropMap.Add(TEXT("AbyssalThrone"),        EBossMaterial::AbyssalMembrane);
    BossDropMap.Add(TEXT("FracturedGod"),         EBossMaterial::FracturedGodhide);
}

// ---------------------------------------------------------------------------
// Boss-kill drops
// ---------------------------------------------------------------------------

TArray<FBossMaterialDrop> UEclipseBossHideCrafting::OnBossKilled(const FString& BossName, EElementalAffinity BossAffinity)
{
    TArray<FBossMaterialDrop> Drops;

    if (BossName.IsEmpty())
    {
        return Drops;
    }

    // Resolve the boss's signature material.
    EBossMaterial Signature = EBossMaterial::None;
    if (const EBossMaterial* Found = BossDropMap.Find(BossName))
    {
        Signature = *Found;
    }
    else
    {
        // Unknown boss name — fall back based on affinity.
        switch (BossAffinity)
        {
        case EElementalAffinity::Fire:      Signature = EBossMaterial::VolcanicCinderhide;  break;
        case EElementalAffinity::Ice:      Signature = EBossMaterial::StormstrikeHide;      break;
        case EElementalAffinity::Lightning:Signature = EBossMaterial::StormstrikeHide;      break;
        case EElementalAffinity::Shadow:   Signature = EBossMaterial::ShadowrendHide;       break;
        case EElementalAffinity::Holy:     Signature = EBossMaterial::SolarForgeScale;      break;
        case EElementalAffinity::Earth:    Signature = EBossMaterial::VerdantThornhide;      break;
        case EElementalAffinity::Water:    Signature = EBossMaterial::AbyssalMembrane;       break;
        case EElementalAffinity::Soul:     Signature = EBossMaterial::FracturedGodhide;      break;
        case EElementalAffinity::Wind:     Signature = EBossMaterial::AetherBoundPelt;      break;
        case EElementalAffinity::Poison:   Signature = EBossMaterial::VerdantThornhide;      break;
        case EElementalAffinity::Temporal: Signature = EBossMaterial::FracturedGodhide;     break;
        }
    }

    const TArray<EBossPart> Parts = GetDropsForBoss(BossName);

    for (const EBossPart Part : Parts)
    {
        FBossMaterialDrop Drop;
        Drop.MaterialType  = Signature;
        Drop.PartType       = Part;
        Drop.MaterialName   = GetMaterialDisplayName(Signature);
        Drop.BossName        = BossName;
        Drop.BossAffinity    = BossAffinity;
        Drop.Quantity        = (Part == EBossPart::Heart || Part == EBossPart::Soul) ? 1 : 2;
        Drop.SourceBossId    = BossName;
        Drop.KillDate        = FDateTime::Now();
        Drop.PartDescription = GetPartDisplayName(Part);
        PlayerMaterials.Add(Drop);
        Drops.Add(Drop);
    }

    return Drops;
}

TArray<EBossPart> UEclipseBossHideCrafting::GetDropsForBoss(const FString& BossName) const
{
    TArray<EBossPart> Parts;
    // Every boss drops a Hide (or Leather) plus a secondary part.
    // Harder bosses additionally drop a Heart or Soul.
    Parts.Add(EBossPart::Hide);

    if (BossName == TEXT("Pyrothragma") || BossName == TEXT("Solairus"))
    {
        Parts.Add(EBossPart::Scales);
    }
    else if (BossName == TEXT("Umbraxis") || BossName == TEXT("VerdantThornhide"))
    {
        Parts.Add(EBossPart::Fangs);
    }
    else if (BossName == TEXT("StormcallerVael"))
    {
        Parts.Add(EBossPart::Claws);
    }
    else if (BossName == TEXT("Aetherion"))
    {
        Parts.Add(EBossPart::Horns);
    }
    else if (BossName == TEXT("AbyssalThrone"))
    {
        Parts.Add(EBossPart::Bones);
    }
    else if (BossName == TEXT("FracturedGod"))
    {
        Parts.Add(EBossPart::Soul);
    }
    else
    {
        Parts.Add(EBossPart::Fangs);
    }

    // Endgame bosses add rare drops.
    if (BossName == TEXT("FracturedGod") || BossName == TEXT("AbyssalThrone"))
    {
        Parts.Add(EBossPart::Heart);
        Parts.Add(EBossPart::Spine);
    }

    return Parts;
}

// ---------------------------------------------------------------------------
// Inventory queries
// ---------------------------------------------------------------------------

bool UEclipseBossHideCrafting::HasMaterial(EBossMaterial Material, EBossPart Part) const
{
    for (const FBossMaterialDrop& Drop : PlayerMaterials)
    {
        if (Drop.MaterialType == Material && Drop.PartType == Part && Drop.Quantity > 0)
        {
            return true;
        }
    }
    return false;
}

TArray<FBossMaterialDrop> UEclipseBossHideCrafting::GetAvailableMaterials() const
{
    TArray<FBossMaterialDrop> Result;
    for (const FBossMaterialDrop& Drop : PlayerMaterials)
    {
        if (Drop.Quantity > 0)
        {
            Result.Add(Drop);
        }
    }
    return Result;
}

// ---------------------------------------------------------------------------
// Crafting
// ---------------------------------------------------------------------------

bool UEclipseBossHideCrafting::CraftWeapon(EBossMaterial Material, EBossPart Part, ECraftableType WeaponType, int32 GoldAmount)
{
    if (Material == EBossMaterial::None)
    {
        return false;
    }
    if (!HasMaterial(Material, Part))
    {
        return false;
    }

    const TArray<ECraftableType> Options = GetWeaponOptionsForPart(Part);
    if (!Options.Contains(WeaponType))
    {
        return false;
    }

    const ERarityTier Tier = (Part == EBossPart::Soul)   ? ERarityTier::Mythic :
                              (Part == EBossPart::Heart) ? ERarityTier::Legendary :
                              ERarityTier::Epic;
    if (GoldAmount < GetGoldCostForTier(Tier))
    {
        return false;
    }

    // Consume one of the material.
    for (FBossMaterialDrop& Drop : PlayerMaterials)
    {
        if (Drop.MaterialType == Material && Drop.PartType == Part && Drop.Quantity > 0)
        {
            Drop.Quantity -= 1;
            break;
        }
    }

    FBossHideCraftedItem Item;
    Item.ItemName                  = GetMaterialDisplayName(Material) + TEXT(" ") + GetPartDisplayName(Part) + TEXT(" Weapon");
    Item.MaterialSource             = Material;
    Item.PartSource                 = Part;
    Item.CraftedType                = WeaponType;
    Item.PassiveAffinity            = GetMaterialAffinity(Material);
    Item.PassiveBonusPercent        = GetPassiveBonus(Material);
    Item.BaseStatBonus              = (Part == EBossPart::Soul)   ? 80.0f :
                                        (Part == EBossPart::Heart) ? 50.0f :
                                        (Part == EBossPart::Scales || Part == EBossPart::Horns) ? 35.0f :
                                        (Part == EBossPart::Fangs || Part == EBossPart::Claws) ? 30.0f :
                                        20.0f;
    Item.GoldCost                  = GetGoldCostForTier(Tier);
    Item.Tier                      = Tier;
    Item.bSoulboundOnPickup         = true;
    Item.VisualMaterialAssetPath     = GetVisualMaterialPath(Material);
    Item.NiagaraParticleAssetPath    = GetNiagaraParticlePath(Material);
    Item.DamageBonus                 = Item.BaseStatBonus;
    Item.DefenseBonus                = 0.0f;
    Item.StaminaBonus                = 0.0f;
    Item.ManaBonus                   = 0.0f;
    Item.HealthBonus                  = 0.0f;

    CraftedItems.Add(Item);
    CraftedMaterialKeys.Add(MakeMaterialKey(Material, Part));
    return true;
}

bool UEclipseBossHideCrafting::CraftArmor(EBossMaterial Material, EBossPart Part, ECraftableType ArmorType, int32 GoldAmount)
{
    if (Material == EBossMaterial::None)
    {
        return false;
    }
    if (!HasMaterial(Material, Part))
    {
        return false;
    }

    const TArray<ECraftableType> Options = GetArmorOptionsForPart(Part);
    if (!Options.Contains(ArmorType))
    {
        return false;
    }

    const ERarityTier Tier = (Part == EBossPart::Soul)   ? ERarityTier::Mythic :
                              (Part == EBossPart::Heart) ? ERarityTier::Legendary :
                              ERarityTier::Epic;
    if (GoldAmount < GetGoldCostForTier(Tier))
    {
        return false;
    }

    for (FBossMaterialDrop& Drop : PlayerMaterials)
    {
        if (Drop.MaterialType == Material && Drop.PartType == Part && Drop.Quantity > 0)
        {
            Drop.Quantity -= 1;
            break;
        }
    }

    FBossHideCraftedItem Item;
    Item.ItemName                  = GetMaterialDisplayName(Material) + TEXT(" ") + GetPartDisplayName(Part) + TEXT(" Armor");
    Item.MaterialSource             = Material;
    Item.PartSource                 = Part;
    Item.CraftedType                = ArmorType;
    Item.PassiveAffinity            = GetMaterialAffinity(Material);
    Item.PassiveBonusPercent        = GetPassiveBonus(Material);
    Item.BaseStatBonus              = (Part == EBossPart::Soul)   ? 70.0f :
                                        (Part == EBossPart::Heart) ? 45.0f :
                                        (Part == EBossPart::Spine || Part == EBossPart::Bones) ? 30.0f :
                                        (Part == EBossPart::Hide || Part == EBossPart::Leather) ? 25.0f :
                                        20.0f;
    Item.GoldCost                  = GetGoldCostForTier(Tier);
    Item.Tier                      = Tier;
    Item.bSoulboundOnPickup         = true;
    Item.VisualMaterialAssetPath     = GetVisualMaterialPath(Material);
    Item.NiagaraParticleAssetPath    = GetNiagaraParticlePath(Material);
    Item.DamageBonus                 = 0.0f;
    Item.DefenseBonus                = Item.BaseStatBonus;
    Item.StaminaBonus                = (ArmorType == ECraftableType::Greaves || ArmorType == ECraftableType::Chestplate) ? Item.BaseStatBonus * 0.5f : 0.0f;
    Item.ManaBonus                   = (ArmorType == ECraftableType::Amulet || ArmorType == ECraftableType::Ring) ? Item.BaseStatBonus * 0.5f : 0.0f;
    Item.HealthBonus                  = (ArmorType == ECraftableType::Chestplate || ArmorType == ECraftableType::Helm) ? Item.BaseStatBonus : 0.0f;

    CraftedItems.Add(Item);
    CraftedMaterialKeys.Add(MakeMaterialKey(Material, Part));
    return true;
}

// ---------------------------------------------------------------------------
// Passive / affinity / cost helpers
// ---------------------------------------------------------------------------

float UEclipseBossHideCrafting::GetPassiveBonus(EBossMaterial Material) const
{
    return GetAffinityBonusPercent(Material);
}

EElementalAffinity UEclipseBossHideCrafting::GetMaterialAffinity(EBossMaterial Material) const
{
    return MaterialToAffinity(Material);
}

EElementalAffinity UEclipseBossHideCrafting::MaterialToAffinity(EBossMaterial Material) const
{
    switch (Material)
    {
    case EBossMaterial::VolcanicCinderhide:  return EElementalAffinity::Fire;
    case EBossMaterial::AshenHuskLeather:    return EElementalAffinity::Fire;
    case EBossMaterial::SolarForgeScale:    return EElementalAffinity::Holy;
    case EBossMaterial::ShadowrendHide:      return EElementalAffinity::Shadow;
    case EBossMaterial::VerdantThornhide:    return EElementalAffinity::Earth;
    case EBossMaterial::AetherBoundPelt:    return EElementalAffinity::Wind;
    case EBossMaterial::StormstrikeHide:     return EElementalAffinity::Lightning;
    case EBossMaterial::AbyssalMembrane:     return EElementalAffinity::Water;
    case EBossMaterial::FracturedGodhide:    return EElementalAffinity::Temporal;
    case EBossMaterial::None:               return EElementalAffinity::Fire;
    }
    return EElementalAffinity::Fire;
}

float UEclipseBossHideCrafting::GetAffinityBonusPercent(EBossMaterial Material) const
{
    switch (Material)
    {
    case EBossMaterial::VolcanicCinderhide:  return 15.0f;
    case EBossMaterial::AshenHuskLeather:    return 12.0f;
    case EBossMaterial::SolarForgeScale:    return 18.0f;
    case EBossMaterial::ShadowrendHide:      return 16.0f;
    case EBossMaterial::VerdantThornhide:    return 14.0f;
    case EBossMaterial::AetherBoundPelt:    return 20.0f;
    case EBossMaterial::StormstrikeHide:     return 17.0f;
    case EBossMaterial::AbyssalMembrane:     return 18.0f;
    case EBossMaterial::FracturedGodhide:    return 25.0f;
    case EBossMaterial::None:               return 0.0f;
    }
    return 0.0f;
}

int32 UEclipseBossHideCrafting::GetGoldCostForTier(ERarityTier Tier) const
{
    switch (Tier)
    {
    case ERarityTier::Common:     return 100;
    case ERarityTier::Uncommon:   return 500;
    case ERarityTier::Rare:      return 1500;
    case ERarityTier::Epic:       return 5000;
    case ERarityTier::Legendary:  return 15000;
    case ERarityTier::Mythic:     return 50000;
    case ERarityTier::Eclipse:    return 100000;
    }
    return 100;
}

// ---------------------------------------------------------------------------
// Craftable-item queries
// ---------------------------------------------------------------------------

TArray<FBossHideCraftedItem> UEclipseBossHideCrafting::GetCraftableItems() const
{
    return CraftedItems;
}

TArray<ECraftableType> UEclipseBossHideCrafting::GetWeaponOptionsForPart(EBossPart Part) const
{
    TArray<ECraftableType> Options;
    switch (Part)
    {
    case EBossPart::Hide:
        // Hide is not typically used for weapons on its own; offer the basic
        // longsword so players always have something to craft.
        Options.Add(ECraftableType::Longsword);
        break;
    case EBossPart::Scales:
        Options.Add(ECraftableType::Greatsword);
        Options.Add(ECraftableType::Shield);
        Options.Add(ECraftableType::Chestplate);
        Options.Add(ECraftableType::Pauldrons);
        break;
    case EBossPart::Fangs:
        Options.Add(ECraftableType::Dagger);
        Options.Add(ECraftableType::Longsword);
        Options.Add(ECraftableType::Spear);
        break;
    case EBossPart::Bones:
        Options.Add(ECraftableType::Staff);
        Options.Add(ECraftableType::Greaves);
        Options.Add(ECraftableType::Amulet);
        break;
    case EBossPart::Claws:
        Options.Add(ECraftableType::Dagger);
        Options.Add(ECraftableType::Gauntlets);
        break;
    case EBossPart::Horns:
        Options.Add(ECraftableType::Greatsword);
        Options.Add(ECraftableType::Helm);
        Options.Add(ECraftableType::Shield);
        break;
    case EBossPart::Heart:
        Options.Add(ECraftableType::Greatsword);
        Options.Add(ECraftableType::Longsword);
        Options.Add(ECraftableType::Dagger);
        Options.Add(ECraftableType::Spear);
        Options.Add(ECraftableType::Staff);
        Options.Add(ECraftableType::Bow);
        Options.Add(ECraftableType::Shield);
        Options.Add(ECraftableType::Helm);
        Options.Add(ECraftableType::Chestplate);
        Options.Add(ECraftableType::Gauntlets);
        Options.Add(ECraftableType::Greaves);
        Options.Add(ECraftableType::Pauldrons);
        Options.Add(ECraftableType::Cloak);
        Options.Add(ECraftableType::Amulet);
        Options.Add(ECraftableType::Ring);
        break;
    case EBossPart::Soul:
        Options.Add(ECraftableType::Greatsword);
        Options.Add(ECraftableType::Longsword);
        Options.Add(ECraftableType::Dagger);
        Options.Add(ECraftableType::Spear);
        Options.Add(ECraftableType::Staff);
        Options.Add(ECraftableType::Bow);
        Options.Add(ECraftableType::Shield);
        Options.Add(ECraftableType::Helm);
        Options.Add(ECraftableType::Chestplate);
        Options.Add(ECraftableType::Gauntlets);
        Options.Add(ECraftableType::Greaves);
        Options.Add(ECraftableType::Pauldrons);
        Options.Add(ECraftableType::Cloak);
        Options.Add(ECraftableType::Amulet);
        Options.Add(ECraftableType::Ring);
        break;
    case EBossPart::Spine:
        Options.Add(ECraftableType::Staff);
        Options.Add(ECraftableType::Greaves);
        Options.Add(ECraftableType::Pauldrons);
        break;
    case EBossPart::Leather:
        Options.Add(ECraftableType::Longsword);
        break;
    case EBossPart::Essence:
        Options.Add(ECraftableType::Amulet);
        Options.Add(ECraftableType::Ring);
        break;
    }
    return Options;
}

TArray<ECraftableType> UEclipseBossHideCrafting::GetArmorOptionsForPart(EBossPart Part) const
{
    TArray<ECraftableType> Options;
    switch (Part)
    {
    case EBossPart::Hide:
        Options.Add(ECraftableType::Cloak);
        Options.Add(ECraftableType::Chestplate);
        Options.Add(ECraftableType::Greaves);
        break;
    case EBossPart::Scales:
        Options.Add(ECraftableType::Chestplate);
        Options.Add(ECraftableType::Pauldrons);
        Options.Add(ECraftableType::Shield);
        break;
    case EBossPart::Fangs:
        // Fangs lean toward weapons; offer only a token armor piece.
        Options.Add(ECraftableType::Gauntlets);
        break;
    case EBossPart::Bones:
        Options.Add(ECraftableType::Greaves);
        Options.Add(ECraftableType::Amulet);
        Options.Add(ECraftableType::Helm);
        break;
    case EBossPart::Claws:
        Options.Add(ECraftableType::Gauntlets);
        break;
    case EBossPart::Horns:
        Options.Add(ECraftableType::Helm);
        Options.Add(ECraftableType::Pauldrons);
        break;
    case EBossPart::Heart:
        Options.Add(ECraftableType::Helm);
        Options.Add(ECraftableType::Chestplate);
        Options.Add(ECraftableType::Gauntlets);
        Options.Add(ECraftableType::Greaves);
        Options.Add(ECraftableType::Pauldrons);
        Options.Add(ECraftableType::Cloak);
        Options.Add(ECraftableType::Amulet);
        Options.Add(ECraftableType::Ring);
        Options.Add(ECraftableType::Shield);
        break;
    case EBossPart::Soul:
        Options.Add(ECraftableType::Helm);
        Options.Add(ECraftableType::Chestplate);
        Options.Add(ECraftableType::Gauntlets);
        Options.Add(ECraftableType::Greaves);
        Options.Add(ECraftableType::Pauldrons);
        Options.Add(ECraftableType::Cloak);
        Options.Add(ECraftableType::Amulet);
        Options.Add(ECraftableType::Ring);
        Options.Add(ECraftableType::Shield);
        break;
    case EBossPart::Spine:
        Options.Add(ECraftableType::Greaves);
        Options.Add(ECraftableType::Pauldrons);
        break;
    case EBossPart::Leather:
        Options.Add(ECraftableType::Cloak);
        Options.Add(ECraftableType::Chestplate);
        Options.Add(ECraftableType::Greaves);
        Options.Add(ECraftableType::Pauldrons);
        break;
    case EBossPart::Essence:
        Options.Add(ECraftableType::Amulet);
        Options.Add(ECraftableType::Ring);
        break;
    }
    return Options;
}

FString UEclipseBossHideCrafting::GetVisualMaterialPath(EBossMaterial Material) const
{
    switch (Material)
    {
    case EBossMaterial::VolcanicCinderhide:
        return TEXT("/Game/Materials/BossCraft/VolcanicCinderhide.M_VolcanicCinderhide");
    case EBossMaterial::AshenHuskLeather:
        return TEXT("/Game/Materials/BossCraft/AshenHuskLeather.M_AshenHuskLeather");
    case EBossMaterial::SolarForgeScale:
        return TEXT("/Game/Materials/BossCraft/SolarForgeScale.M_SolarForgeScale");
    case EBossMaterial::ShadowrendHide:
        return TEXT("/Game/Materials/BossCraft/ShadowrendHide.M_ShadowrendHide");
    case EBossMaterial::VerdantThornhide:
        return TEXT("/Game/Materials/BossCraft/VerdantThornhide.M_VerdantThornhide");
    case EBossMaterial::AetherBoundPelt:
        return TEXT("/Game/Materials/BossCraft/AetherBoundPelt.M_AetherBoundPelt");
    case EBossMaterial::StormstrikeHide:
        return TEXT("/Game/Materials/BossCraft/StormstrikeHide.M_StormstrikeHide");
    case EBossMaterial::AbyssalMembrane:
        return TEXT("/Game/Materials/BossCraft/AbyssalMembrane.M_AbyssalMembrane");
    case EBossMaterial::FracturedGodhide:
        return TEXT("/Game/Materials/BossCraft/FracturedGodhide.M_FracturedGodhide");
    case EBossMaterial::None:
        return TEXT("");
    }
    return TEXT("");
}

FString UEclipseBossHideCrafting::GetNiagaraParticlePath(EBossMaterial Material) const
{
    switch (Material)
    {
    case EBossMaterial::VolcanicCinderhide:
        return TEXT("/Game/FX/BossCraft/NS_VolcanicCinderhide.NS_VolcanicCinderhide");
    case EBossMaterial::AshenHuskLeather:
        return TEXT("/Game/FX/BossCraft/NS_AshenHuskLeather.NS_AshenHuskLeather");
    case EBossMaterial::SolarForgeScale:
        return TEXT("/Game/FX/BossCraft/NS_SolarForgeScale.NS_SolarForgeScale");
    case EBossMaterial::ShadowrendHide:
        return TEXT("/Game/FX/BossCraft/NS_ShadowrendHide.NS_ShadowrendHide");
    case EBossMaterial::VerdantThornhide:
        return TEXT("/Game/FX/BossCraft/NS_VerdantThornhide.NS_VerdantThornhide");
    case EBossMaterial::AetherBoundPelt:
        return TEXT("/Game/FX/BossCraft/NS_AetherBoundPelt.NS_AetherBoundPelt");
    case EBossMaterial::StormstrikeHide:
        return TEXT("/Game/FX/BossCraft/NS_StormstrikeHide.NS_StormstrikeHide");
    case EBossMaterial::AbyssalMembrane:
        return TEXT("/Game/FX/BossCraft/NS_AbyssalMembrane.NS_AbyssalMembrane");
    case EBossMaterial::FracturedGodhide:
        return TEXT("/Game/FX/BossCraft/NS_FracturedGodhide.NS_FracturedGodhide");
    case EBossMaterial::None:
        return TEXT("");
    }
    return TEXT("");
}

bool UEclipseBossHideCrafting::IsMaterialCrafted(EBossMaterial Material, EBossPart Part) const
{
    return CraftedMaterialKeys.Contains(MakeMaterialKey(Material, Part));
}

FString UEclipseBossHideCrafting::GetMaterialDisplayName(EBossMaterial Material) const
{
    switch (Material)
    {
    case EBossMaterial::VolcanicCinderhide:  return TEXT("Volcanic Cinderhide");
    case EBossMaterial::AshenHuskLeather:    return TEXT("Ashen Husk Leather");
    case EBossMaterial::SolarForgeScale:    return TEXT("Solar Forge-Scale");
    case EBossMaterial::ShadowrendHide:      return TEXT("Shadowrend Hide");
    case EBossMaterial::VerdantThornhide:    return TEXT("Verdant Thornhide");
    case EBossMaterial::AetherBoundPelt:    return TEXT("Aether-Bound Pelt");
    case EBossMaterial::StormstrikeHide:     return TEXT("Stormstrike Hide");
    case EBossMaterial::AbyssalMembrane:     return TEXT("Abyssal Membrane");
    case EBossMaterial::FracturedGodhide:    return TEXT("Fractured Godhide");
    case EBossMaterial::None:               return TEXT("None");
    }
    return TEXT("Unknown");
}

FString UEclipseBossHideCrafting::GetPartDisplayName(EBossPart Part) const
{
    switch (Part)
    {
    case EBossPart::Hide:      return TEXT("Hide");
    case EBossPart::Scales:    return TEXT("Scales");
    case EBossPart::Fangs:     return TEXT("Fangs");
    case EBossPart::Bones:     return TEXT("Bones");
    case EBossPart::Claws:     return TEXT("Claws");
    case EBossPart::Horns:     return TEXT("Horns");
    case EBossPart::Heart:     return TEXT("Heart (rare)");
    case EBossPart::Soul:      return TEXT("Soul Remnant (very rare)");
    case EBossPart::Spine:     return TEXT("Spine");
    case EBossPart::Leather:   return TEXT("Leather");
    case EBossPart::Essence:   return TEXT("Aetheric Essence");
    }
    return TEXT("Unknown");
}

TArray<FBossHideCraftedItem> UEclipseBossHideCrafting::GetCraftedWeapons() const
{
    TArray<FBossHideCraftedItem> Result;
    for (const FBossHideCraftedItem& Item : CraftedItems)
    {
        switch (Item.CraftedType)
        {
        case ECraftableType::Greatsword:
        case ECraftableType::Longsword:
        case ECraftableType::Dagger:
        case ECraftableType::Spear:
        case ECraftableType::Staff:
        case ECraftableType::Bow:
            Result.Add(Item);
            break;
        case ECraftableType::Shield:
        case ECraftableType::Helm:
        case ECraftableType::Chestplate:
        case ECraftableType::Gauntlets:
        case ECraftableType::Greaves:
        case ECraftableType::Pauldrons:
        case ECraftableType::Cloak:
        case ECraftableType::Amulet:
        case ECraftableType::Ring:
            break;
        }
    }
    return Result;
}

TArray<FBossHideCraftedItem> UEclipseBossHideCrafting::GetCraftedArmor() const
{
    TArray<FBossHideCraftedItem> Result;
    for (const FBossHideCraftedItem& Item : CraftedItems)
    {
        switch (Item.CraftedType)
        {
        case ECraftableType::Shield:
        case ECraftableType::Helm:
        case ECraftableType::Chestplate:
        case ECraftableType::Gauntlets:
        case ECraftableType::Greaves:
        case ECraftableType::Pauldrons:
        case ECraftableType::Cloak:
        case ECraftableType::Amulet:
        case ECraftableType::Ring:
            Result.Add(Item);
            break;
        case ECraftableType::Greatsword:
        case ECraftableType::Longsword:
        case ECraftableType::Dagger:
        case ECraftableType::Spear:
        case ECraftableType::Staff:
        case ECraftableType::Bow:
            break;
        }
    }
    return Result;
}

FString UEclipseBossHideCrafting::MakeMaterialKey(EBossMaterial Material, EBossPart Part) const
{
    return FString::Printf(TEXT("%d_%d"), static_cast<int32>(Material), static_cast<int32>(Part));
}
