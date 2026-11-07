// Eclipse Spell System Implementation
// 400+ spells across all 17 classes, 10 elemental affinities, 45 dual-affinity combinations
// Each class gets ~22 spells: Cantrip(2), Basic(3), Intermediate(3), Advanced(3), Ultimate(1),
//   WeaponArt(2), Defensive(3), Utility(3), Ritual(2)
// Class abilities from EclipseClassAbilities.cpp are reflected as spells here.

#include "EclipseSpellSystem.h"
#include "EclipseClassRoles.h" // For EClassRole (required by header)

// ============================================================================
// Static Helpers
// ============================================================================

static FString GetElementString(EWeaponStatusEffect E)
{
    switch (E)
    {
    case EWeaponStatusEffect::Fire:      return TEXT("Fire");
    case EWeaponStatusEffect::Ice:       return TEXT("Ice");
    case EWeaponStatusEffect::Lightning: return TEXT("Lightning");
    case EWeaponStatusEffect::Shadow:    return TEXT("Shadow");
    case EWeaponStatusEffect::Holy:      return TEXT("Holy");
    case EWeaponStatusEffect::Poison:    return TEXT("Poison");
    case EWeaponStatusEffect::Earth:     return TEXT("Earth");
    case EWeaponStatusEffect::Water:     return TEXT("Water");
    case EWeaponStatusEffect::Wind:      return TEXT("Wind");
    case EWeaponStatusEffect::Soul:      return TEXT("Soul");
    default:                             return TEXT("None");
    }
}

static FString MakeDualAffinityKey(EWeaponStatusEffect E1, EWeaponStatusEffect E2)
{
    uint8 V1 = static_cast<uint8>(E1);
    uint8 V2 = static_cast<uint8>(E2);
    if (V1 <= V2)
        return GetElementString(E1) + TEXT("+") + GetElementString(E2);
    return GetElementString(E2) + TEXT("+") + GetElementString(E1);
}

// ============================================================================
// Constructor
// ============================================================================

UEclipseSpellSystem::UEclipseSpellSystem()
{
    InitializeSpells();
}

// ============================================================================
// Initialization
// ============================================================================

void UEclipseSpellSystem::InitializeSpells()
{
    AllSpells.Reserve(500); // Pre-allocate to prevent reallocation during Init

    InitBulwarkSpells();
    InitDuskbladeSpells();
    InitArcanistSpells();
    InitRiftwalkerSpells();
    InitDreadknightSpells();
    InitTemplarSpells();
    InitSoulbinderSpells();
    InitWildwardenSpells();
    InitStormcallerSpells();
    InitOracleSpells();
    InitAlchemistSpells();
    InitBerserkerSpells();
    InitShadowbladeSpells();
    InitSpellbladeSpells();
    InitWarthaneSpells();
    InitNecroturgeSpells();
    InitEarthweaverSpells();
    InitDualAffinitySpells();

    // Build lookup maps from AllSpells
    for (const FSpell& S : AllSpells)
    {
        ClassSpells.FindOrAdd(S.Class).Add(S);
        if (S.Element1 != EWeaponStatusEffect::None)
            ElementSpells.FindOrAdd(S.Element1).Add(S);
        if (S.bIsDualAffinity && S.Element2 != EWeaponStatusEffect::None)
            ElementSpells.FindOrAdd(S.Element2).Add(S);
    }
}

// ============================================================================
// AddSpell Helper
// ============================================================================

FSpell& UEclipseSpellSystem::AddSpell(EClass Class, const FString& Name, ESpellTier Tier,
    ESpellShape Shape, EWeaponStatusEffect Element1, float Damage, float Cost,
    EResourceType ResourceType, float Cooldown, float CastTime, float Range, float Radius,
    float Duration, const FString& Description)
{
    FSpell Spell;
    Spell.SpellID = NextSpellID++;
    Spell.SpellName = Name;
    Spell.Description = Description;
    Spell.Class = Class;
    Spell.Tier = Tier;
    Spell.Shape = Shape;
    Spell.Element1 = Element1;
    Spell.Damage = Damage;
    Spell.ResourceCost = Cost;
    Spell.ResourceType = ResourceType;
    Spell.Cooldown = Cooldown;
    Spell.CastTime = CastTime;
    Spell.Range = Range;
    Spell.Radius = Radius;
    Spell.Duration = Duration;
    AllSpells.Add(Spell);
    return AllSpells.Last();
}

// ============================================================================
// AddDualAffinitySpell Helper
// ============================================================================

void UEclipseSpellSystem::AddDualAffinitySpell(EClass Class, const FString& Name,
    ESpellShape Shape, EWeaponStatusEffect Element1, EWeaponStatusEffect Element2,
    float Damage, float Cost, EResourceType ResourceType, float Cooldown, float CastTime,
    float Range, float Radius, const FString& Description, bool bStuns, bool bKnockbacks,
    bool bAppliesDoT, float DoTDamage, float DoTDuration)
{
    FSpell& Spell = AddSpell(Class, Name, ESpellTier::DualAffinity, Shape, Element1, Damage,
        Cost, ResourceType, Cooldown, CastTime, Range, Radius, 0.0f, Description);
    Spell.Element2 = Element2;
    Spell.bIsDualAffinity = true;
    Spell.bStuns = bStuns;
    Spell.bKnockbacks = bKnockbacks;
    Spell.bAppliesDoT = bAppliesDoT;
    Spell.DoTDamage = DoTDamage;
    Spell.DoTDuration = DoTDuration;

    FString Key = MakeDualAffinityKey(Element1, Element2);
    DualAffinityMap.Add(Key, Spell);
}

// ============================================================================
// BULWARK — Tank/Initiator — Earth/Stamina — 22 spells
// ============================================================================

void UEclipseSpellSystem::InitBulwarkSpells()
{
    // Cantrips
    AddSpell(EClass::Bulwark, TEXT("Shield Tap"), ESpellTier::Cantrip, ESpellShape::SingleTarget, EWeaponStatusEffect::Earth, 35.0f, 5.0f, EResourceType::Stamina, 2.0f, 0.0f, 200.0f, 0.0f, 0.0f, TEXT("Quick shield tap, brief stagger"));
    AddSpell(EClass::Bulwark, TEXT("Phalanx Poke"), ESpellTier::Cantrip, ESpellShape::SingleTarget, EWeaponStatusEffect::Earth, 40.0f, 5.0f, EResourceType::Stamina, 3.0f, 0.0f, 300.0f, 0.0f, 0.0f, TEXT("Spear thrust through shield wall"));

    // Basic
    { FSpell& S = AddSpell(EClass::Bulwark, TEXT("Warspite Slam"), ESpellTier::Basic, ESpellShape::SingleTarget, EWeaponStatusEffect::Earth, 80.0f, 20.0f, EResourceType::Stamina, 8.0f, 0.0f, 200.0f, 0.0f, 0.0f, TEXT("Bash shield into enemy, stuns 1s, high threat")); S.bStuns = true; S.bKnockbacks = true; }
    { FSpell& S = AddSpell(EClass::Bulwark, TEXT("Aegis Riposte"), ESpellTier::Basic, ESpellShape::SingleTarget, EWeaponStatusEffect::Earth, 150.0f, 25.0f, EResourceType::Stamina, 6.0f, 0.0f, 200.0f, 0.0f, 0.0f, TEXT("Block incoming hit then counter with heavy strike")); S.bKnockbacks = true; }
    { FSpell& S = AddSpell(EClass::Bulwark, TEXT("Bulwark Cleave"), ESpellTier::Basic, ESpellShape::Cone, EWeaponStatusEffect::Earth, 100.0f, 20.0f, EResourceType::Stamina, 6.0f, 0.5f, 300.0f, 4.0f, 0.0f, TEXT("Sweeping shield cleave, hits all in frontal arc")); S.bKnockbacks = true; }

    // Intermediate
    { FSpell& S = AddSpell(EClass::Bulwark, TEXT("Aegis Slam"), ESpellTier::Intermediate, ESpellShape::AoE, EWeaponStatusEffect::Earth, 130.0f, 25.0f, EResourceType::Stamina, 10.0f, 0.5f, 300.0f, 5.0f, 0.0f, TEXT("Slam shield into ground, AoE knockback 5m")); S.bKnockbacks = true; }
    { FSpell& S = AddSpell(EClass::Bulwark, TEXT("Phalanx Charge"), ESpellTier::Intermediate, ESpellShape::Line, EWeaponStatusEffect::Earth, 120.0f, 30.0f, EResourceType::Stamina, 12.0f, 0.0f, 600.0f, 0.0f, 0.0f, TEXT("Charge forward in phalanx formation, knockback all in path")); S.bKnockbacks = true; }
    AddSpell(EClass::Bulwark, TEXT("Fortress Stance"), ESpellTier::Intermediate, ESpellShape::Self, EWeaponStatusEffect::Earth, 0.0f, 30.0f, EResourceType::Stamina, 15.0f, 0.0f, 0.0f, 0.0f, 8.0f, TEXT("Defensive stance, -60% damage taken, immune to knockback 8s"));

    // Advanced
    AddSpell(EClass::Bulwark, TEXT("Bastion Formation"), ESpellTier::Advanced, ESpellShape::Self, EWeaponStatusEffect::Earth, 0.0f, 40.0f, EResourceType::Stamina, 30.0f, 0.0f, 0.0f, 0.0f, 5.0f, TEXT("Full damage reduction 50%, immune to knockback 5s"));
    { FSpell& S = AddSpell(EClass::Bulwark, TEXT("Titan's Wrath"), ESpellTier::Advanced, ESpellShape::AoE, EWeaponStatusEffect::Earth, 200.0f, 40.0f, EResourceType::Stamina, 20.0f, 1.0f, 300.0f, 8.0f, 0.0f, TEXT("Crushing ground slam, 8m AoE knockback")); S.bKnockbacks = true; }
    AddSpell(EClass::Bulwark, TEXT("Unbreakable Will"), ESpellTier::Advanced, ESpellShape::Self, EWeaponStatusEffect::Earth, 0.0f, 35.0f, EResourceType::Stamina, 25.0f, 0.0f, 0.0f, 0.0f, 10.0f, TEXT("Immune to CC, +40% defense for 10s"));

    // Ultimate
    { FSpell& S = AddSpell(EClass::Bulwark, TEXT("Sunbreaker Slam"), ESpellTier::Ultimate, ESpellShape::AoE, EWeaponStatusEffect::Earth, 300.0f, 100.0f, EResourceType::Aether, 180.0f, 1.5f, 400.0f, 8.0f, 0.0f, TEXT("Jump and slam greatsword, 8m shockwave, stuns all enemies")); S.bStuns = true; S.bKnockbacks = true; }

    // Weapon Arts
    { FSpell& S = AddSpell(EClass::Bulwark, TEXT("Bulwark Surge"), ESpellTier::WeaponArt, ESpellShape::Line, EWeaponStatusEffect::Earth, 120.0f, 30.0f, EResourceType::Stamina, 12.0f, 0.0f, 600.0f, 0.0f, 0.0f, TEXT("Charge forward with shield, knockback all enemies in path")); S.bKnockbacks = true; }
    { FSpell& S = AddSpell(EClass::Bulwark, TEXT("Bulwark Strike"), ESpellTier::WeaponArt, ESpellShape::Cone, EWeaponStatusEffect::Earth, 160.0f, 30.0f, EResourceType::Stamina, 10.0f, 0.0f, 300.0f, 5.0f, 0.0f, TEXT("Heavy sweeping shield strike, knockback all in cone")); S.bKnockbacks = true; }

    // Defensive
    AddSpell(EClass::Bulwark, TEXT("Stoneheart Mantle"), ESpellTier::Defensive, ESpellShape::Self, EWeaponStatusEffect::Earth, 0.0f, 20.0f, EResourceType::Stamina, 20.0f, 0.0f, 0.0f, 0.0f, 10.0f, TEXT("Hardened skin, +50% armor for 10s"));
    { FSpell& S = AddSpell(EClass::Bulwark, TEXT("Sentinel's Ward"), ESpellTier::Defensive, ESpellShape::SingleTarget, EWeaponStatusEffect::Holy, 0.0f, 25.0f, EResourceType::Stamina, 15.0f, 0.5f, 500.0f, 0.0f, 0.0f, TEXT("Shield an ally, absorb next 200 damage")); S.Healing = 200.0f; }
    AddSpell(EClass::Bulwark, TEXT("Bulwark of Faith"), ESpellTier::Defensive, ESpellShape::Aura, EWeaponStatusEffect::Holy, 0.0f, 30.0f, EResourceType::Stamina, 18.0f, 0.0f, 0.0f, 5.0f, 10.0f, TEXT("Allies in 5m aura gain +30% defense for 10s"));

    // Utility
    AddSpell(EClass::Bulwark, TEXT("Provoking Howl"), ESpellTier::Utility, ESpellShape::AoE, EWeaponStatusEffect::Earth, 0.0f, 15.0f, EResourceType::Stamina, 10.0f, 0.0f, 0.0f, 10.0f, 0.0f, TEXT("Force all enemies in 10m to attack you"));
    AddSpell(EClass::Bulwark, TEXT("Phalanx March"), ESpellTier::Utility, ESpellShape::Party, EWeaponStatusEffect::Earth, 0.0f, 30.0f, EResourceType::Stamina, 25.0f, 0.0f, 0.0f, 10.0f, 8.0f, TEXT("All allies in 10m gain +20% movement speed for 8s"));
    { FSpell& S = AddSpell(EClass::Bulwark, TEXT("Watcher's Sigil"), ESpellTier::Utility, ESpellShape::SingleTarget, EWeaponStatusEffect::Earth, 50.0f, 10.0f, EResourceType::Stamina, 8.0f, 0.0f, 300.0f, 0.0f, 5.0f, TEXT("Mark target, takes 30% more damage for 5s")); S.bAppliesDebuff = true; S.DebuffType = EWeaponStatusEffect::Earth; S.DebuffDuration = 5.0f; }

    // Ritual
    { FSpell& S = AddSpell(EClass::Bulwark, TEXT("Final Bastion"), ESpellTier::Ritual, ESpellShape::Party, EWeaponStatusEffect::Holy, 0.0f, 100.0f, EResourceType::Aether, 300.0f, 3.0f, 1000.0f, 15.0f, 10.0f, TEXT("Party-wide shield, absorb 500 damage, immune to knockback")); S.Healing = 500.0f; }
    { FSpell& S = AddSpell(EClass::Bulwark, TEXT("Oath of the Bulwark"), ESpellTier::Ritual, ESpellShape::AoE, EWeaponStatusEffect::Earth, 200.0f, 100.0f, EResourceType::Aether, 300.0f, 2.5f, 500.0f, 15.0f, 0.0f, TEXT("15m AoE stun, all allies +50% defense for 15s")); S.bStuns = true; }
}

// ============================================================================
// UMBRAL DUSKBLADE — Burst/Stealth — Shadow/Stamina — 22 spells
// ============================================================================

void UEclipseSpellSystem::InitDuskbladeSpells()
{
    // Cantrips
    AddSpell(EClass::Duskblade, TEXT("Void Tick"), ESpellTier::Cantrip, ESpellShape::SingleTarget, EWeaponStatusEffect::Shadow, 35.0f, 5.0f, EResourceType::Stamina, 1.5f, 0.0f, 200.0f, 0.0f, 0.0f, TEXT("Rapid dagger flick, void-touched"));
    AddSpell(EClass::Duskblade, TEXT("Shadow Slice"), ESpellTier::Cantrip, ESpellShape::SingleTarget, EWeaponStatusEffect::Shadow, 45.0f, 5.0f, EResourceType::Stamina, 2.5f, 0.0f, 250.0f, 0.0f, 0.0f, TEXT("Quick shadow slash, applies brief debuff"));

    // Basic
    { FSpell& S = AddSpell(EClass::Duskblade, TEXT("Gloom Stride"), ESpellTier::Basic, ESpellShape::SingleTarget, EWeaponStatusEffect::Shadow, 60.0f, 15.0f, EResourceType::Stamina, 10.0f, 0.0f, 1500.0f, 0.0f, 0.0f, TEXT("Teleport behind target, leaves afterimages")); S.bStuns = false; }
    { FSpell& S = AddSpell(EClass::Duskblade, TEXT("Vertebra Pierce"), ESpellTier::Basic, ESpellShape::SingleTarget, EWeaponStatusEffect::Shadow, 200.0f, 20.0f, EResourceType::Stamina, 5.0f, 0.0f, 200.0f, 0.0f, 0.0f, TEXT("X-pattern dagger strike, guaranteed crit from behind")); S.bLifesteal = true; S.LifestealPercent = 15.0f; }
    AddSpell(EClass::Duskblade, TEXT("Shade Flurry"), ESpellTier::Basic, ESpellShape::SingleTarget, EWeaponStatusEffect::Shadow, 90.0f, 18.0f, EResourceType::Stamina, 5.0f, 0.0f, 200.0f, 0.0f, 0.0f, TEXT("5 rapid dagger strikes, each applies shadow debuff"));

    // Intermediate
    AddSpell(EClass::Duskblade, TEXT("Umbral Fan"), ESpellTier::Intermediate, ESpellShape::Cone, EWeaponStatusEffect::Shadow, 110.0f, 25.0f, EResourceType::Stamina, 8.0f, 0.0f, 400.0f, 6.0f, 0.0f, TEXT("Throw fan of shadow daggers, pierces all in cone"));
    { FSpell& S = AddSpell(EClass::Duskblade, TEXT("Nightfall Strike"), ESpellTier::Intermediate, ESpellShape::SingleTarget, EWeaponStatusEffect::Shadow, 160.0f, 25.0f, EResourceType::Stamina, 10.0f, 0.5f, 300.0f, 0.0f, 0.0f, TEXT("Empowered strike from shadows, ignores 50% armor")); S.bLifesteal = true; S.LifestealPercent = 20.0f; }
    AddSpell(EClass::Duskblade, TEXT("Crescent Dance"), ESpellTier::Intermediate, ESpellShape::AoE, EWeaponStatusEffect::Shadow, 120.0f, 30.0f, EResourceType::Stamina, 12.0f, 0.5f, 300.0f, 5.0f, 0.0f, TEXT("Spin with daggers, hit all enemies in 5m"));

    // Advanced
    AddSpell(EClass::Duskblade, TEXT("Cinder Shroud"), ESpellTier::Advanced, ESpellShape::Self, EWeaponStatusEffect::Shadow, 0.0f, 25.0f, EResourceType::Stamina, 20.0f, 0.0f, 0.0f, 8.0f, 5.0f, TEXT("Vanish into void smoke, invisible 5s, +50% next hit"));
    { FSpell& S = AddSpell(EClass::Duskblade, TEXT("Deathblow"), ESpellTier::Advanced, ESpellShape::SingleTarget, EWeaponStatusEffect::Shadow, 250.0f, 35.0f, EResourceType::Stamina, 15.0f, 1.0f, 300.0f, 0.0f, 0.0f, TEXT("Teleport strike, guaranteed crit, +100% on low HP targets")); S.bLifesteal = true; S.LifestealPercent = 30.0f; }
    { FSpell& S = AddSpell(EClass::Duskblade, TEXT("Executioner's Mark"), ESpellTier::Advanced, ESpellShape::SingleTarget, EWeaponStatusEffect::Shadow, 80.0f, 30.0f, EResourceType::Stamina, 18.0f, 0.0f, 600.0f, 0.0f, 8.0f, TEXT("Mark target, takes 50% more damage for 8s, reveals position")); S.bAppliesDebuff = true; S.DebuffType = EWeaponStatusEffect::Shadow; S.DebuffDuration = 8.0f; }

    // Ultimate
    { FSpell& S = AddSpell(EClass::Duskblade, TEXT("Veilfang Massacre"), ESpellTier::Ultimate, ESpellShape::AoE, EWeaponStatusEffect::Shadow, 400.0f, 100.0f, EResourceType::Aether, 180.0f, 0.5f, 600.0f, 10.0f, 0.0f, TEXT("5 rapid teleport strikes, each guaranteed crit, 10m AoE")); S.bLifesteal = true; S.LifestealPercent = 25.0f; }

    // Weapon Arts
    { FSpell& S = AddSpell(EClass::Duskblade, TEXT("Voidrender Daggers"), ESpellTier::WeaponArt, ESpellShape::Line, EWeaponStatusEffect::Shadow, 150.0f, 20.0f, EResourceType::Stamina, 8.0f, 0.0f, 300.0f, 0.0f, 0.0f, TEXT("Throw both daggers in void trajectory, pierce all in line")); S.bLifesteal = true; S.LifestealPercent = 20.0f; }
    { FSpell& S = AddSpell(EClass::Duskblade, TEXT("Phantom Edge"), ESpellTier::WeaponArt, ESpellShape::SingleTarget, EWeaponStatusEffect::Shadow, 180.0f, 25.0f, EResourceType::Stamina, 10.0f, 0.0f, 250.0f, 0.0f, 0.0f, TEXT("Phase through enemy, damage applies after passing")); S.bLifesteal = true; S.LifestealPercent = 25.0f; }

    // Defensive
    AddSpell(EClass::Duskblade, TEXT("Shadow Veil"), ESpellTier::Defensive, ESpellShape::Self, EWeaponStatusEffect::Shadow, 0.0f, 20.0f, EResourceType::Stamina, 15.0f, 0.0f, 0.0f, 0.0f, 4.0f, TEXT("Become invisible for 4s, +50% damage on next attack"));
    AddSpell(EClass::Duskblade, TEXT("Evanesce"), ESpellTier::Defensive, ESpellShape::Self, EWeaponStatusEffect::Shadow, 0.0f, 15.0f, EResourceType::Stamina, 12.0f, 0.0f, 0.0f, 0.0f, 3.0f, TEXT("Phantom dash, i-frames for 3s, leaves afterimage"));
    { FSpell& S = AddSpell(EClass::Duskblade, TEXT("Void Ward"), ESpellTier::Defensive, ESpellShape::Self, EWeaponStatusEffect::Shadow, 0.0f, 25.0f, EResourceType::Stamina, 20.0f, 0.0f, 0.0f, 0.0f, 6.0f, TEXT("Absorb next 150 damage, reflect 50% as shadow")); S.Healing = 150.0f; }

    // Utility
    AddSpell(EClass::Duskblade, TEXT("Shadow Teleport"), ESpellTier::Utility, ESpellShape::Self, EWeaponStatusEffect::Shadow, 0.0f, 15.0f, EResourceType::Stamina, 8.0f, 0.0f, 800.0f, 0.0f, 0.0f, TEXT("Instant teleport to targeted location, stealthed on arrival"));
    AddSpell(EClass::Duskblade, TEXT("Fadeout"), ESpellTier::Utility, ESpellShape::Self, EWeaponStatusEffect::Shadow, 0.0f, 20.0f, EResourceType::Stamina, 25.0f, 0.0f, 0.0f, 0.0f, 5.0f, TEXT("Full invisibility 5s, breaks on attack"));
    { FSpell& S = AddSpell(EClass::Duskblade, TEXT("Mark of Silence"), ESpellTier::Utility, ESpellShape::SingleTarget, EWeaponStatusEffect::Shadow, 60.0f, 15.0f, EResourceType::Stamina, 12.0f, 0.0f, 400.0f, 0.0f, 4.0f, TEXT("Mark target, silenced for 4s, cannot cast spells")); S.bSilences = true; }

    // Ritual
    { FSpell& S = AddSpell(EClass::Duskblade, TEXT("Dance of a Thousand Cuts"), ESpellTier::Ritual, ESpellShape::AoE, EWeaponStatusEffect::Shadow, 350.0f, 100.0f, EResourceType::Aether, 300.0f, 2.0f, 500.0f, 12.0f, 5.0f, TEXT("1000 cuts over 5s, hits all enemies in 12m every 0.5s")); S.bLifesteal = true; S.LifestealPercent = 15.0f; S.bAppliesDoT = true; S.DoTDamage = 30.0f; S.DoTDuration = 5.0f; }
    { FSpell& S = AddSpell(EClass::Duskblade, TEXT("Eclipse of Shadows"), ESpellTier::Ritual, ESpellShape::Zone, EWeaponStatusEffect::Shadow, 280.0f, 100.0f, EResourceType::Aether, 300.0f, 2.5f, 800.0f, 15.0f, 8.0f, TEXT("Summon eclipse zone, all enemies silenced and take 50/s")); S.bSilences = true; S.bAppliesDoT = true; S.DoTDamage = 50.0f; S.DoTDuration = 8.0f; }
}

// ============================================================================
// ARCANIST — Ranged DPS/Control — Lightning/Mana — 22 spells
// ============================================================================

void UEclipseSpellSystem::InitArcanistSpells()
{
    // Cantrips
    AddSpell(EClass::Arcanist, TEXT("Arc Spark"), ESpellTier::Cantrip, ESpellShape::Projectile, EWeaponStatusEffect::Lightning, 40.0f, 5.0f, EResourceType::Mana, 1.5f, 0.0f, 600.0f, 0.0f, 0.0f, TEXT("Quick arcane spark, low cost burst"));
    AddSpell(EClass::Arcanist, TEXT("Mana Bolt"), ESpellTier::Cantrip, ESpellShape::Projectile, EWeaponStatusEffect::Lightning, 50.0f, 5.0f, EResourceType::Mana, 2.0f, 0.0f, 800.0f, 0.0f, 0.0f, TEXT("Basic mana projectile, fast cast"));

    // Basic
    AddSpell(EClass::Arcanist, TEXT("Aether Lance"), ESpellTier::Basic, ESpellShape::Line, EWeaponStatusEffect::Lightning, 120.0f, 20.0f, EResourceType::Mana, 3.0f, 0.0f, 1200.0f, 0.0f, 0.0f, TEXT("Concentrated beam of amethyst energy, pierces all in line"));
    { FSpell& S = AddSpell(EClass::Arcanist, TEXT("Rime Burst"), ESpellTier::Basic, ESpellShape::AoE, EWeaponStatusEffect::Ice, 80.0f, 30.0f, EResourceType::Mana, 8.0f, 0.5f, 500.0f, 8.0f, 0.0f, TEXT("Freeze all enemies in 8m for 3s, ice shatters on hit")); S.bFreezes = true; }
    AddSpell(EClass::Arcanist, TEXT("Crystal Volley"), ESpellTier::Basic, ESpellShape::Projectile, EWeaponStatusEffect::Lightning, 90.0f, 18.0f, EResourceType::Mana, 5.0f, 0.5f, 800.0f, 0.0f, 0.0f, TEXT("Fire 3 homing missiles, each deals 30 damage"));

    // Intermediate
    { FSpell& S = AddSpell(EClass::Arcanist, TEXT("Celestial Ruin"), ESpellTier::Intermediate, ESpellShape::Ground, EWeaponStatusEffect::Fire, 250.0f, 40.0f, EResourceType::Mana, 12.0f, 1.0f, 800.0f, 6.0f, 3.0f, TEXT("Rain meteors on target area, 5 impacts over 3s")); S.bAppliesDoT = true; S.DoTDamage = 20.0f; S.DoTDuration = 3.0f; }
    AddSpell(EClass::Arcanist, TEXT("Wardspark Wall"), ESpellTier::Intermediate, ESpellShape::Self, EWeaponStatusEffect::Lightning, 0.0f, 30.0f, EResourceType::Mana, 15.0f, 0.0f, 0.0f, 0.0f, 6.0f, TEXT("Arcane shield, absorb 200 damage, reflect 25%"));
    AddSpell(EClass::Arcanist, TEXT("Gravity Well"), ESpellTier::Intermediate, ESpellShape::Zone, EWeaponStatusEffect::Lightning, 100.0f, 35.0f, EResourceType::Mana, 15.0f, 0.5f, 800.0f, 8.0f, 4.0f, TEXT("Create gravity well, pull enemies to center, 25/s"));

    // Advanced
    AddSpell(EClass::Arcanist, TEXT("Leyline Surge"), ESpellTier::Advanced, ESpellShape::AoE, EWeaponStatusEffect::Lightning, 220.0f, 45.0f, EResourceType::Mana, 18.0f, 1.0f, 600.0f, 8.0f, 0.0f, TEXT("Massive arcane explosion, 8m AoE, knockback all"));
    { FSpell& S = AddSpell(EClass::Arcanist, TEXT("Chronolock"), ESpellTier::Advanced, ESpellShape::AoE, EWeaponStatusEffect::Ice, 60.0f, 50.0f, EResourceType::Mana, 30.0f, 1.0f, 1000.0f, 10.0f, 3.0f, TEXT("Freeze time for all enemies in 10m for 3s")); S.bFreezes = true; S.bStuns = true; }
    AddSpell(EClass::Arcanist, TEXT("Sigil of Power"), ESpellTier::Advanced, ESpellShape::Self, EWeaponStatusEffect::Lightning, 0.0f, 40.0f, EResourceType::Mana, 25.0f, 0.0f, 0.0f, 0.0f, 10.0f, TEXT("Empower self, +50% spell damage for 10s"));

    // Ultimate
    { FSpell& S = AddSpell(EClass::Arcanist, TEXT("Astral Annihilation"), ESpellTier::Ultimate, ESpellShape::AoE, EWeaponStatusEffect::Lightning, 600.0f, 100.0f, EResourceType::Aether, 180.0f, 2.0f, 1500.0f, 15.0f, 0.0f, TEXT("15m amethyst explosion, stun + knockback, ignites residual Aether")); S.bStuns = true; S.bKnockbacks = true; }

    // Weapon Arts
    AddSpell(EClass::Arcanist, TEXT("Sigil of Fracture"), ESpellTier::WeaponArt, ESpellShape::Ground, EWeaponStatusEffect::Lightning, 180.0f, 50.0f, EResourceType::Mana, 15.0f, 1.0f, 1000.0f, 15.0f, 5.0f, TEXT("Rotating energy sigil, beams lance all enemies within"));
    AddSpell(EClass::Arcanist, TEXT("Mana Surge"), ESpellTier::WeaponArt, ESpellShape::AoE, EWeaponStatusEffect::Lightning, 160.0f, 40.0f, EResourceType::Mana, 12.0f, 0.5f, 600.0f, 6.0f, 0.0f, TEXT("Detonate mana, 6m arcane burst, restores 20 mana on hit"));

    // Defensive
    { FSpell& S = AddSpell(EClass::Arcanist, TEXT("Wardspark Barrier"), ESpellTier::Defensive, ESpellShape::Self, EWeaponStatusEffect::Lightning, 0.0f, 25.0f, EResourceType::Mana, 15.0f, 0.0f, 0.0f, 0.0f, 8.0f, TEXT("Arcane barrier, absorb 250 damage for 8s")); S.Healing = 250.0f; }
    AddSpell(EClass::Arcanist, TEXT("Slipstream"), ESpellTier::Defensive, ESpellShape::Self, EWeaponStatusEffect::Lightning, 0.0f, 15.0f, EResourceType::Mana, 10.0f, 0.0f, 800.0f, 0.0f, 0.0f, TEXT("Instant teleport to safety, breaks targeting"));
    AddSpell(EClass::Arcanist, TEXT("Flux Ward"), ESpellTier::Defensive, ESpellShape::Self, EWeaponStatusEffect::Lightning, 0.0f, 30.0f, EResourceType::Mana, 20.0f, 0.0f, 0.0f, 0.0f, 6.0f, TEXT("Convert Mana to barrier, 1 Mana = 5 shield, lasts 6s"));

    // Utility
    { FSpell& S = AddSpell(EClass::Arcanist, TEXT("Timestream Drag"), ESpellTier::Utility, ESpellShape::SingleTarget, EWeaponStatusEffect::Ice, 30.0f, 15.0f, EResourceType::Mana, 8.0f, 0.0f, 800.0f, 0.0f, 5.0f, TEXT("Slow target by 50% for 5s")); S.bAppliesDebuff = true; S.DebuffType = EWeaponStatusEffect::Ice; S.DebuffDuration = 5.0f; }
    AddSpell(EClass::Arcanist, TEXT("Aether Sense"), ESpellTier::Utility, ESpellShape::AoE, EWeaponStatusEffect::Lightning, 0.0f, 10.0f, EResourceType::Mana, 20.0f, 0.0f, 0.0f, 20.0f, 8.0f, TEXT("Reveal all enemies and traps in 20m for 8s"));
    AddSpell(EClass::Arcanist, TEXT("Memory strand"), ESpellTier::Utility, ESpellShape::Self, EWeaponStatusEffect::Lightning, 0.0f, 20.0f, EResourceType::Mana, 30.0f, 1.0f, 0.0f, 0.0f, 0.0f, TEXT("Restore 50 Mana instantly, 30s cooldown"));

    // Ritual
    { FSpell& S = AddSpell(EClass::Arcanist, TEXT("Arcane Cataclysm"), ESpellTier::Ritual, ESpellShape::Zone, EWeaponStatusEffect::Lightning, 350.0f, 100.0f, EResourceType::Aether, 300.0f, 3.0f, 1500.0f, 20.0f, 10.0f, TEXT("Massive arcane cataclysm, 20m zone, 35/s for 10s")); S.bStuns = true; S.bAppliesDoT = true; S.DoTDamage = 35.0f; S.DoTDuration = 10.0f; }
    { FSpell& S = AddSpell(EClass::Arcanist, TEXT("Leylines Unbound"), ESpellTier::Ritual, ESpellShape::Party, EWeaponStatusEffect::Lightning, 0.0f, 100.0f, EResourceType::Aether, 300.0f, 2.5f, 1000.0f, 20.0f, 15.0f, TEXT("All allies restore 100 Mana, +50% spell damage for 15s")); S.Healing = 100.0f; }
}

// ============================================================================
// RIFTWALKER — Mobility/Disruptor — Wind/Mana — 22 spells
// ============================================================================

void UEclipseSpellSystem::InitRiftwalkerSpells()
{
    // Cantrips
    AddSpell(EClass::Riftwalker, TEXT("Dimension Stride"), ESpellTier::Cantrip, ESpellShape::Self, EWeaponStatusEffect::Wind, 0.0f, 5.0f, EResourceType::Mana, 1.0f, 0.0f, 300.0f, 0.0f, 0.0f, TEXT("Quick blink 3m, leaves afterimage"));
    AddSpell(EClass::Riftwalker, TEXT("Tearcutter"), ESpellTier::Cantrip, ESpellShape::SingleTarget, EWeaponStatusEffect::Wind, 50.0f, 5.0f, EResourceType::Mana, 2.0f, 0.0f, 300.0f, 0.0f, 0.0f, TEXT("Slash through dimensional rift, short range"));

    // Basic
    AddSpell(EClass::Riftwalker, TEXT("Chrono Blink"), ESpellTier::Basic, ESpellShape::Self, EWeaponStatusEffect::Wind, 80.0f, 15.0f, EResourceType::Stamina, 6.0f, 0.0f, 800.0f, 0.0f, 0.0f, TEXT("Instant teleport to targeted location, leaves afterimage"));
    AddSpell(EClass::Riftwalker, TEXT("Timestream Hew"), ESpellTier::Basic, ESpellShape::SingleTarget, EWeaponStatusEffect::Wind, 110.0f, 18.0f, EResourceType::Mana, 5.0f, 0.0f, 400.0f, 0.0f, 0.0f, TEXT("Slash through time, damage applies after 0.5s delay"));
    AddSpell(EClass::Riftwalker, TEXT("Fracture Tear"), ESpellTier::Basic, ESpellShape::Line, EWeaponStatusEffect::Wind, 100.0f, 20.0f, EResourceType::Mana, 8.0f, 0.3f, 600.0f, 0.0f, 0.0f, TEXT("Tear a rift in space, pierces all enemies in line"));

    // Intermediate
    AddSpell(EClass::Riftwalker, TEXT("Time Dilation"), ESpellTier::Intermediate, ESpellShape::AoE, EWeaponStatusEffect::Wind, 0.0f, 40.0f, EResourceType::Mana, 25.0f, 0.0f, 0.0f, 10.0f, 4.0f, TEXT("Slow all enemies in 10m by 50% for 4s, you move normally"));
    { FSpell& S = AddSpell(EClass::Riftwalker, TEXT("Astral Harrier"), ESpellTier::Intermediate, ESpellShape::AoE, EWeaponStatusEffect::Wind, 120.0f, 30.0f, EResourceType::Stamina, 15.0f, 0.0f, 500.0f, 5.0f, 0.0f, TEXT("Rapid multidirectional slashes, hits all in melee range")); S.bStuns = true; }
    AddSpell(EClass::Riftwalker, TEXT("Rift Bomb"), ESpellTier::Intermediate, ESpellShape::Projectile, EWeaponStatusEffect::Wind, 140.0f, 28.0f, EResourceType::Mana, 10.0f, 0.5f, 800.0f, 4.0f, 0.0f, TEXT("Throw dimensional bomb, explodes in 4m rift"));

    // Advanced
    AddSpell(EClass::Riftwalker, TEXT("Chronolock Field"), ESpellTier::Advanced, ESpellShape::Zone, EWeaponStatusEffect::Wind, 80.0f, 45.0f, EResourceType::Mana, 25.0f, 1.0f, 800.0f, 10.0f, 5.0f, TEXT("Stasis zone, all enemies frozen for 5s"));
    AddSpell(EClass::Riftwalker, TEXT("Slipstream Salvo"), ESpellTier::Advanced, ESpellShape::Self, EWeaponStatusEffect::Wind, 100.0f, 40.0f, EResourceType::Mana, 20.0f, 0.0f, 1200.0f, 0.0f, 0.0f, TEXT("3 rapid blinks, each deals 33 damage to nearby enemies"));
    AddSpell(EClass::Riftwalker, TEXT("Tachyon Surge"), ESpellTier::Advanced, ESpellShape::Self, EWeaponStatusEffect::Wind, 0.0f, 35.0f, EResourceType::Mana, 25.0f, 0.0f, 0.0f, 0.0f, 8.0f, TEXT("+80% attack and cast speed for 8s, -50% cooldowns"));

    // Ultimate
    { FSpell& S = AddSpell(EClass::Riftwalker, TEXT("Gateweaver Cataclysm"), ESpellTier::Ultimate, ESpellShape::Zone, EWeaponStatusEffect::Wind, 500.0f, 100.0f, EResourceType::Aether, 180.0f, 1.5f, 1200.0f, 12.0f, 3.0f, TEXT("5 dimensional rifts, each fires energy beam for 3s")); S.bStuns = true; }

    // Weapon Arts
    { FSpell& S = AddSpell(EClass::Riftwalker, TEXT("Dimension Hew"), ESpellTier::WeaponArt, ESpellShape::SingleTarget, EWeaponStatusEffect::Wind, 200.0f, 25.0f, EResourceType::Stamina, 10.0f, 0.0f, 400.0f, 0.0f, 0.0f, TEXT("Phase through enemy, damage applies after passing")); S.bStuns = true; }
    AddSpell(EClass::Riftwalker, TEXT("Riftwalker's Edge"), ESpellTier::WeaponArt, ESpellShape::Line, EWeaponStatusEffect::Wind, 170.0f, 30.0f, EResourceType::Mana, 12.0f, 0.0f, 800.0f, 0.0f, 0.0f, TEXT("Slash through a rift, blade exits from another rift behind target"));

    // Defensive
    AddSpell(EClass::Riftwalker, TEXT("Dimension Step"), ESpellTier::Defensive, ESpellShape::Self, EWeaponStatusEffect::Wind, 0.0f, 20.0f, EResourceType::Mana, 12.0f, 0.0f, 0.0f, 0.0f, 3.0f, TEXT("Phase out of reality, invulnerable for 3s, cannot act"));
    AddSpell(EClass::Riftwalker, TEXT("Timestream Revert"), ESpellTier::Defensive, ESpellShape::Self, EWeaponStatusEffect::Wind, 0.0f, 35.0f, EResourceType::Mana, 30.0f, 0.0f, 0.0f, 0.0f, 0.0f, TEXT("Rewind 3s, restore HP and position to 3s ago"));
    AddSpell(EClass::Riftwalker, TEXT("Blink Shield"), ESpellTier::Defensive, ESpellShape::Self, EWeaponStatusEffect::Wind, 0.0f, 25.0f, EResourceType::Mana, 18.0f, 0.0f, 0.0f, 0.0f, 5.0f, TEXT("Absorb 200 damage, auto-blink when broken"));

    // Utility
    AddSpell(EClass::Riftwalker, TEXT("Group Blink"), ESpellTier::Utility, ESpellShape::Party, EWeaponStatusEffect::Wind, 0.0f, 40.0f, EResourceType::Mana, 30.0f, 0.0f, 500.0f, 0.0f, 0.0f, TEXT("Teleport all allies to targeted location"));
    AddSpell(EClass::Riftwalker, TEXT("Teargate"), ESpellTier::Utility, ESpellShape::Ground, EWeaponStatusEffect::Wind, 0.0f, 30.0f, EResourceType::Mana, 20.0f, 0.5f, 1000.0f, 3.0f, 10.0f, TEXT("Create 2-way portal, allies can travel between for 10s"));
    { FSpell& S = AddSpell(EClass::Riftwalker, TEXT("Timestream Fold"), ESpellTier::Utility, ESpellShape::Zone, EWeaponStatusEffect::Wind, 50.0f, 35.0f, EResourceType::Mana, 22.0f, 0.5f, 600.0f, 8.0f, 6.0f, TEXT("8m zone, enemies slowed 70%, allies +30% speed for 6s")); S.bAppliesDebuff = true; S.DebuffType = EWeaponStatusEffect::Wind; S.DebuffDuration = 6.0f; }

    // Ritual
    { FSpell& S = AddSpell(EClass::Riftwalker, TEXT("Timestream Rupture"), ESpellTier::Ritual, ESpellShape::AoE, EWeaponStatusEffect::Wind, 320.0f, 100.0f, EResourceType::Aether, 300.0f, 3.0f, 1500.0f, 20.0f, 0.0f, TEXT("Fracture time itself, 20m AoE, all enemies frozen 5s")); S.bFreezes = true; S.bStuns = true; }
    { FSpell& S = AddSpell(EClass::Riftwalker, TEXT("Tearstorm"), ESpellTier::Ritual, ESpellShape::Zone, EWeaponStatusEffect::Wind, 280.0f, 100.0f, EResourceType::Aether, 300.0f, 2.5f, 1200.0f, 18.0f, 10.0f, TEXT("Summon dimensional storm, 28/s for 10s, random teleports")); S.bAppliesDoT = true; S.DoTDamage = 28.0f; S.DoTDuration = 10.0f; }
}

// ============================================================================
// DREADKNIGHT — Drain Tank/Bruiser — Shadow/Rage — 22 spells
// ============================================================================

void UEclipseSpellSystem::InitDreadknightSpells()
{
    // Cantrips
    { FSpell& S = AddSpell(EClass::Dreadknight, TEXT("Void Tap"), ESpellTier::Cantrip, ESpellShape::SingleTarget, EWeaponStatusEffect::Shadow, 40.0f, 5.0f, EResourceType::Rage, 2.0f, 0.0f, 200.0f, 0.0f, 0.0f, TEXT("Quick void touch, drains 10 HP")); S.bLifesteal = true; S.LifestealPercent = 25.0f; }
    AddSpell(EClass::Dreadknight, TEXT("Tenebrous Cleave"), ESpellTier::Cantrip, ESpellShape::SingleTarget, EWeaponStatusEffect::Shadow, 50.0f, 5.0f, EResourceType::Rage, 2.5f, 0.0f, 250.0f, 0.0f, 0.0f, TEXT("Shadow-infused weapon slash"));

    // Basic
    { FSpell& S = AddSpell(EClass::Dreadknight, TEXT("Eidolon Siphon"), ESpellTier::Basic, ESpellShape::SingleTarget, EWeaponStatusEffect::Soul, 80.0f, 20.0f, EResourceType::Rage, 10.0f, 0.0f, 300.0f, 0.0f, 0.0f, TEXT("Drain 30 HP from target, heal self for 50%")); S.bLifesteal = true; S.LifestealPercent = 50.0f; }
    AddSpell(EClass::Dreadknight, TEXT("Blood Pact"), ESpellTier::Basic, ESpellShape::Self, EWeaponStatusEffect::Shadow, 0.0f, 40.0f, EResourceType::Rage, 20.0f, 0.0f, 0.0f, 0.0f, 10.0f, TEXT("Sacrifice 20% HP, +50% damage and 30% lifesteal for 10s"));
    AddSpell(EClass::Dreadknight, TEXT("Gloom Reave"), ESpellTier::Basic, ESpellShape::Cone, EWeaponStatusEffect::Shadow, 90.0f, 18.0f, EResourceType::Rage, 6.0f, 0.0f, 300.0f, 4.0f, 0.0f, TEXT("Sweeping dark strike, hits all in frontal cone"));

    // Intermediate
    { FSpell& S = AddSpell(EClass::Dreadknight, TEXT("Void Reaver"), ESpellTier::Intermediate, ESpellShape::Cone, EWeaponStatusEffect::Shadow, 150.0f, 35.0f, EResourceType::Rage, 15.0f, 0.5f, 400.0f, 5.0f, 0.0f, TEXT("Sweep weapon in arc, drain 3 enemies simultaneously")); S.bLifesteal = true; S.LifestealPercent = 40.0f; }
    AddSpell(EClass::Dreadknight, TEXT("Tenebrous Surge"), ESpellTier::Intermediate, ESpellShape::Projectile, EWeaponStatusEffect::Shadow, 130.0f, 28.0f, EResourceType::Rage, 10.0f, 0.3f, 600.0f, 0.0f, 0.0f, TEXT("Fire bolt of dark energy, pierces armor"));
    AddSpell(EClass::Dreadknight, TEXT("Ossuary Platemail"), ESpellTier::Intermediate, ESpellShape::Self, EWeaponStatusEffect::Soul, 0.0f, 30.0f, EResourceType::Rage, 18.0f, 0.0f, 0.0f, 0.0f, 8.0f, TEXT("Bone armor, absorb 150 damage, reflects 20% as shadow"));

    // Advanced
    { FSpell& S = AddSpell(EClass::Dreadknight, TEXT("Soul Harvester"), ESpellTier::Advanced, ESpellShape::AoE, EWeaponStatusEffect::Soul, 180.0f, 40.0f, EResourceType::Rage, 20.0f, 0.8f, 400.0f, 8.0f, 0.0f, TEXT("AoE soul harvest, drain all in 8m, heal 40% of damage")); S.bLifesteal = true; S.LifestealPercent = 40.0f; }
    { FSpell& S = AddSpell(EClass::Dreadknight, TEXT("Fraying Hex"), ESpellTier::Advanced, ESpellShape::SingleTarget, EWeaponStatusEffect::Shadow, 60.0f, 35.0f, EResourceType::Rage, 18.0f, 0.0f, 500.0f, 0.0f, 8.0f, TEXT("Curse target, -40% damage dealt, -30% move speed for 8s")); S.bAppliesDebuff = true; S.DebuffType = EWeaponStatusEffect::Shadow; S.DebuffDuration = 8.0f; }
    AddSpell(EClass::Dreadknight, TEXT("Profane Vigor"), ESpellTier::Advanced, ESpellShape::Self, EWeaponStatusEffect::Shadow, 0.0f, 35.0f, EResourceType::Rage, 22.0f, 0.0f, 0.0f, 0.0f, 10.0f, TEXT("+60% damage, +30% lifesteal, immune to fear for 10s"));

    // Ultimate
    { FSpell& S = AddSpell(EClass::Dreadknight, TEXT("Bone Regent Crown"), ESpellTier::Ultimate, ESpellShape::AoE, EWeaponStatusEffect::Soul, 350.0f, 100.0f, EResourceType::Aether, 180.0f, 1.0f, 600.0f, 12.0f, 0.0f, TEXT("Summon crown of bone, AoE drain all in 12m, heal party 30%")); S.bLifesteal = true; S.LifestealPercent = 50.0f; S.bHealsCaster = true; S.Healing = 150.0f; }

    // Weapon Arts
    { FSpell& S = AddSpell(EClass::Dreadknight, TEXT("Blood Oath Strike"), ESpellTier::WeaponArt, ESpellShape::SingleTarget, EWeaponStatusEffect::Shadow, 180.0f, 25.0f, EResourceType::Rage, 8.0f, 0.0f, 250.0f, 0.0f, 0.0f, TEXT("Heavy overhead strike, heals for 100% of damage dealt")); S.bLifesteal = true; S.LifestealPercent = 100.0f; }
    { FSpell& S = AddSpell(EClass::Dreadknight, TEXT("Soul Reaver"), ESpellTier::WeaponArt, ESpellShape::Line, EWeaponStatusEffect::Soul, 160.0f, 30.0f, EResourceType::Rage, 12.0f, 0.0f, 500.0f, 0.0f, 0.0f, TEXT("Soul-stealing slash, pierces enemies, drains Flow")); S.bLifesteal = true; S.LifestealPercent = 35.0f; }

    // Defensive
    { FSpell& S = AddSpell(EClass::Dreadknight, TEXT("Sanguine Mantle"), ESpellTier::Defensive, ESpellShape::Aura, EWeaponStatusEffect::Shadow, 0.0f, 30.0f, EResourceType::Rage, 20.0f, 0.0f, 0.0f, 8.0f, 10.0f, TEXT("Allies in 8m gain 15% lifesteal for 10s")); S.Healing = 50.0f; }
    AddSpell(EClass::Dreadknight, TEXT("Tenebrous Ward"), ESpellTier::Defensive, ESpellShape::Self, EWeaponStatusEffect::Shadow, 0.0f, 25.0f, EResourceType::Rage, 15.0f, 0.0f, 0.0f, 0.0f, 6.0f, TEXT("Shadow barrier, absorb 200 damage, heal 10% on expire"));
    AddSpell(EClass::Dreadknight, TEXT("Reaper's Grasp"), ESpellTier::Defensive, ESpellShape::Self, EWeaponStatusEffect::Soul, 0.0f, 35.0f, EResourceType::Rage, 25.0f, 0.0f, 0.0f, 0.0f, 5.0f, TEXT("Below 30% HP, take 50% less damage and +50% lifesteal for 5s"));

    // Utility
    AddSpell(EClass::Dreadknight, TEXT("Dread"), ESpellTier::Utility, ESpellShape::SingleTarget, EWeaponStatusEffect::Shadow, 50.0f, 15.0f, EResourceType::Rage, 12.0f, 0.0f, 400.0f, 0.0f, 3.0f, TEXT("Fear target for 3s, they flee randomly"));
    AddSpell(EClass::Dreadknight, TEXT("Tenebrous Crossing"), ESpellTier::Utility, ESpellShape::Self, EWeaponStatusEffect::Shadow, 0.0f, 20.0f, EResourceType::Rage, 15.0f, 0.0f, 600.0f, 0.0f, 0.0f, TEXT("Teleport through shadow, arrive stealthed"));
    { FSpell& S = AddSpell(EClass::Dreadknight, TEXT("Eidolon Tether"), ESpellTier::Utility, ESpellShape::SingleTarget, EWeaponStatusEffect::Soul, 0.0f, 25.0f, EResourceType::Rage, 20.0f, 0.0f, 400.0f, 0.0f, 8.0f, TEXT("Link with ally, redirect 30% of their damage to you, heal them 50% of your lifesteal")); S.Healing = 80.0f; }

    // Ritual
    { FSpell& S = AddSpell(EClass::Dreadknight, TEXT("Host of the Accursed"), ESpellTier::Ritual, ESpellShape::Summon, EWeaponStatusEffect::Soul, 200.0f, 100.0f, EResourceType::Aether, 300.0f, 3.0f, 600.0f, 12.0f, 30.0f, TEXT("Summon 5 bone knights, each deals 40/s, last 30s")); S.bLifesteal = true; S.LifestealPercent = 20.0f; }
    { FSpell& S = AddSpell(EClass::Dreadknight, TEXT("Profane Communion"), ESpellTier::Ritual, ESpellShape::AoE, EWeaponStatusEffect::Shadow, 300.0f, 100.0f, EResourceType::Aether, 300.0f, 2.5f, 800.0f, 15.0f, 0.0f, TEXT("15m AoE drain, all enemies lose 150 HP, all allies heal 100")); S.bLifesteal = true; S.LifestealPercent = 50.0f; S.bHealsCaster = true; S.Healing = 100.0f; }
}

// ============================================================================
// TEMPLAR — Tank/Healer — Holy/Mana — 22 spells
// ============================================================================

void UEclipseSpellSystem::InitTemplarSpells()
{
    // Cantrips
    AddSpell(EClass::Templar, TEXT("Hallowed Radiance"), ESpellTier::Cantrip, ESpellShape::SingleTarget, EWeaponStatusEffect::Holy, 40.0f, 5.0f, EResourceType::Mana, 2.0f, 0.0f, 400.0f, 0.0f, 0.0f, TEXT("Quick holy flash, bonus vs undead"));
    AddSpell(EClass::Templar, TEXT("Sigil Strike"), ESpellTier::Cantrip, ESpellShape::SingleTarget, EWeaponStatusEffect::Holy, 45.0f, 5.0f, EResourceType::Mana, 2.5f, 0.0f, 200.0f, 0.0f, 0.0f, TEXT("Weapon strike with holy seal, brief stagger"));

    // Basic
    AddSpell(EClass::Templar, TEXT("Dawnshield"), ESpellTier::Basic, ESpellShape::Self, EWeaponStatusEffect::Holy, 60.0f, 25.0f, EResourceType::Mana, 12.0f, 0.0f, 300.0f, 0.0f, 0.0f, TEXT("Holy barrier, absorb next 200 damage, reflect 50% as holy"));
    { FSpell& S = AddSpell(EClass::Templar, TEXT("Hallowed Dawn"), ESpellTier::Basic, ESpellShape::AoE, EWeaponStatusEffect::Holy, 0.0f, 30.0f, EResourceType::Mana, 8.0f, 0.5f, 500.0f, 8.0f, 0.0f, TEXT("Heal all allies in 8m for 80 HP, cleanse all debuffs")); S.Healing = 80.0f; }
    { FSpell& S = AddSpell(EClass::Templar, TEXT("Condemn"), ESpellTier::Basic, ESpellShape::SingleTarget, EWeaponStatusEffect::Holy, 120.0f, 20.0f, EResourceType::Mana, 6.0f, 0.0f, 300.0f, 0.0f, 0.0f, TEXT("Holy judgment strike, bonus vs undead/demon")); S.bStuns = true; }

    // Intermediate
    AddSpell(EClass::Templar, TEXT("Hallow Ground"), ESpellTier::Intermediate, ESpellShape::Ground, EWeaponStatusEffect::Holy, 80.0f, 30.0f, EResourceType::Mana, 15.0f, 0.5f, 500.0f, 6.0f, 8.0f, TEXT("Consecrated ground, 10/s holy damage to enemies, heal allies 10/s"));
    { FSpell& S = AddSpell(EClass::Templar, TEXT("Hallowed Burst"), ESpellTier::Intermediate, ESpellShape::AoE, EWeaponStatusEffect::Holy, 140.0f, 35.0f, EResourceType::Mana, 12.0f, 0.5f, 400.0f, 8.0f, 0.0f, TEXT("Holy explosion, damage enemies, heal allies in 8m for 60")); S.Healing = 60.0f; }
    AddSpell(EClass::Templar, TEXT("Banner of Light"), ESpellTier::Intermediate, ESpellShape::Aura, EWeaponStatusEffect::Holy, 0.0f, 35.0f, EResourceType::Mana, 18.0f, 0.0f, 0.0f, 10.0f, 10.0f, TEXT("Plant holy banner, allies in 10m +20% damage, +15% defense for 10s"));

    // Advanced
    AddSpell(EClass::Templar, TEXT("Consecrated Bulwark"), ESpellTier::Advanced, ESpellShape::SingleTarget, EWeaponStatusEffect::Holy, 0.0f, 40.0f, EResourceType::Mana, 25.0f, 0.0f, 600.0f, 0.0f, 6.0f, TEXT("Shield ally, absorb 400 damage, immune to CC for 6s"));
    { FSpell& S = AddSpell(EClass::Templar, TEXT("Tribunal Strike"), ESpellTier::Advanced, ESpellShape::SingleTarget, EWeaponStatusEffect::Holy, 200.0f, 35.0f, EResourceType::Mana, 15.0f, 0.5f, 300.0f, 0.0f, 0.0f, TEXT("Charged holy strike, 2x damage vs undead, stuns 2s")); S.bStuns = true; }
    AddSpell(EClass::Templar, TEXT("Sanctuary Mantle"), ESpellTier::Advanced, ESpellShape::Aura, EWeaponStatusEffect::Holy, 0.0f, 40.0f, EResourceType::Mana, 22.0f, 0.0f, 0.0f, 12.0f, 8.0f, TEXT("All allies in 12m take 30% less damage for 8s"));

    // Ultimate
    { FSpell& S = AddSpell(EClass::Templar, TEXT("Star Confessor"), ESpellTier::Ultimate, ESpellShape::AoE, EWeaponStatusEffect::Holy, 400.0f, 100.0f, EResourceType::Aether, 180.0f, 1.5f, 800.0f, 12.0f, 0.0f, TEXT("Holy nova, heal allies 150, damage enemies 300 in 12m")); S.Healing = 150.0f; S.bStuns = true; }

    // Weapon Arts
    { FSpell& S = AddSpell(EClass::Templar, TEXT("Judicator Verdict"), ESpellTier::WeaponArt, ESpellShape::SingleTarget, EWeaponStatusEffect::Holy, 200.0f, 30.0f, EResourceType::Mana, 10.0f, 0.0f, 400.0f, 0.0f, 0.0f, TEXT("Charge weapon with holy light, next strike 2x damage and stuns")); S.bStuns = true; }
    AddSpell(EClass::Templar, TEXT("Aegis of Dawn"), ESpellTier::WeaponArt, ESpellShape::Wall, EWeaponStatusEffect::Holy, 80.0f, 30.0f, EResourceType::Mana, 12.0f, 0.5f, 500.0f, 8.0f, 8.0f, TEXT("Create wall of holy light, blocks enemies, heals allies through it"));

    // Defensive
    { FSpell& S = AddSpell(EClass::Templar, TEXT("Palm of Mercy"), ESpellTier::Defensive, ESpellShape::SingleTarget, EWeaponStatusEffect::Holy, 0.0f, 40.0f, EResourceType::Mana, 20.0f, 0.5f, 400.0f, 0.0f, 0.0f, TEXT("Massive single-target heal, restores 300 HP, cleanses all")); S.Healing = 300.0f; }
    AddSpell(EClass::Templar, TEXT("Consecrated Plate"), ESpellTier::Defensive, ESpellShape::Self, EWeaponStatusEffect::Holy, 0.0f, 25.0f, EResourceType::Mana, 18.0f, 0.0f, 0.0f, 0.0f, 10.0f, TEXT("Holy armor, +40% defense, reflect 25% melee damage for 10s"));
    AddSpell(EClass::Templar, TEXT("Lustral Wash"), ESpellTier::Defensive, ESpellShape::AoE, EWeaponStatusEffect::Holy, 0.0f, 20.0f, EResourceType::Mana, 12.0f, 0.0f, 0.0f, 6.0f, 0.0f, TEXT("Remove all debuffs from allies in 6m, immune to debuffs 3s"));

    // Utility
    AddSpell(EClass::Templar, TEXT("Taunt of Light"), ESpellTier::Utility, ESpellShape::AoE, EWeaponStatusEffect::Holy, 30.0f, 15.0f, EResourceType::Mana, 10.0f, 0.0f, 0.0f, 8.0f, 0.0f, TEXT("Holy taunt, all enemies in 8m forced to attack you, bonus threat"));
    AddSpell(EClass::Templar, TEXT("Dawnfire Beacon"), ESpellTier::Utility, ESpellShape::Ground, EWeaponStatusEffect::Holy, 0.0f, 25.0f, EResourceType::Mana, 15.0f, 0.5f, 600.0f, 6.0f, 10.0f, TEXT("Place beacon, allies in 6m heal 15/s, +20% defense for 10s"));
    AddSpell(EClass::Templar, TEXT("Boon of Fortitude"), ESpellTier::Utility, ESpellShape::Party, EWeaponStatusEffect::Holy, 0.0f, 30.0f, EResourceType::Mana, 20.0f, 0.0f, 0.0f, 10.0f, 8.0f, TEXT("All allies in 10m +25% damage and +15% defense for 8s"));

    // Ritual
    { FSpell& S = AddSpell(EClass::Templar, TEXT("Celestial Judgement"), ESpellTier::Ritual, ESpellShape::Zone, EWeaponStatusEffect::Holy, 350.0f, 100.0f, EResourceType::Aether, 300.0f, 3.0f, 1200.0f, 20.0f, 10.0f, TEXT("20m zone of holy fire, 35/s to enemies, heal allies 30/s")); S.bStuns = true; S.bAppliesDoT = true; S.DoTDamage = 35.0f; S.DoTDuration = 10.0f; }
    { FSpell& S = AddSpell(EClass::Templar, TEXT("Celestial Revival"), ESpellTier::Ritual, ESpellShape::Party, EWeaponStatusEffect::Holy, 0.0f, 100.0f, EResourceType::Aether, 300.0f, 3.0f, 1000.0f, 15.0f, 0.0f, TEXT("Revive all fallen allies at 50% HP, full buff party for 15s")); S.Healing = 300.0f; }
}

// ============================================================================
// SOULBINDER — Healer/Caster — Soul/Mana — 22 spells
// ============================================================================

void UEclipseSpellSystem::InitSoulbinderSpells()
{
    // Cantrips
    { FSpell& S = AddSpell(EClass::Soulbinder, TEXT("Soul Spark"), ESpellTier::Cantrip, ESpellShape::Projectile, EWeaponStatusEffect::Soul, 40.0f, 5.0f, EResourceType::Mana, 1.5f, 0.0f, 600.0f, 0.0f, 0.0f, TEXT("Quick soul projectile, drains 5 Flow from target")); S.bLifesteal = true; S.LifestealPercent = 10.0f; }
    AddSpell(EClass::Soulbinder, TEXT("Spirit Touch"), ESpellTier::Cantrip, ESpellShape::SingleTarget, EWeaponStatusEffect::Soul, 50.0f, 5.0f, EResourceType::Mana, 2.0f, 0.0f, 300.0f, 0.0f, 0.0f, TEXT("Spirit-infused touch, bypasses defense"));

    // Basic
    AddSpell(EClass::Soulbinder, TEXT("Echo Shepherd"), ESpellTier::Basic, ESpellShape::SingleTarget, EWeaponStatusEffect::Soul, 100.0f, 20.0f, EResourceType::Mana, 8.0f, 0.0f, 400.0f, 0.0f, 5.0f, TEXT("Bind enemy soul, 20/s for 5s, gain 20 Flow on kill"));
    AddSpell(EClass::Soulbinder, TEXT("Spirit Bolt"), ESpellTier::Basic, ESpellShape::Projectile, EWeaponStatusEffect::Soul, 90.0f, 18.0f, EResourceType::Mana, 4.0f, 0.0f, 800.0f, 0.0f, 0.0f, TEXT("Spirit projectile, pierces armor, +50% vs spectral"));
    AddSpell(EClass::Soulbinder, TEXT("Warding Spirit"), ESpellTier::Basic, ESpellShape::Self, EWeaponStatusEffect::Soul, 0.0f, 22.0f, EResourceType::Mana, 12.0f, 0.0f, 0.0f, 0.0f, 6.0f, TEXT("Spirit shield, absorb 150 damage, spirit reflects 30%"));

    // Intermediate
    { FSpell& S = AddSpell(EClass::Soulbinder, TEXT("Hex Sovereign"), ESpellTier::Intermediate, ESpellShape::SingleTarget, EWeaponStatusEffect::Soul, 80.0f, 25.0f, EResourceType::Mana, 10.0f, 0.0f, 500.0f, 0.0f, 5.0f, TEXT("Curse target, takes 50% more damage for 5s, spreads to nearby")); S.bAppliesDebuff = true; S.DebuffType = EWeaponStatusEffect::Soul; S.DebuffDuration = 5.0f; }
    { FSpell& S = AddSpell(EClass::Soulbinder, TEXT("Soul Chain"), ESpellTier::Intermediate, ESpellShape::Chain, EWeaponStatusEffect::Soul, 120.0f, 25.0f, EResourceType::Mana, 12.0f, 0.0f, 600.0f, 0.0f, 0.0f, TEXT("Chain 3 enemies, shared damage, cannot move while chained")); S.bRoots = true; }
    { FSpell& S = AddSpell(EClass::Soulbinder, TEXT("Spirit Mend"), ESpellTier::Intermediate, ESpellShape::SingleTarget, EWeaponStatusEffect::Soul, 0.0f, 25.0f, EResourceType::Mana, 8.0f, 0.5f, 600.0f, 0.0f, 0.0f, TEXT("Heal target for 150, restore 30 Mana")); S.Healing = 150.0f; }

    // Advanced
    { FSpell& S = AddSpell(EClass::Soulbinder, TEXT("Grave Covenant"), ESpellTier::Advanced, ESpellShape::SingleTarget, EWeaponStatusEffect::Soul, 0.0f, 40.0f, EResourceType::Mana, 15.0f, 1.0f, 600.0f, 0.0f, 0.0f, TEXT("Revive a fallen ally at 50% HP")); S.Healing = 250.0f; }
    { FSpell& S = AddSpell(EClass::Soulbinder, TEXT("Eidolon Reaping"), ESpellTier::Advanced, ESpellShape::AoE, EWeaponStatusEffect::Soul, 200.0f, 45.0f, EResourceType::Mana, 20.0f, 1.0f, 600.0f, 10.0f, 0.0f, TEXT("Harvest souls in 10m, drain 200 HP from all enemies")); S.bLifesteal = true; S.LifestealPercent = 30.0f; }
    AddSpell(EClass::Soulbinder, TEXT("Spirit Form"), ESpellTier::Advanced, ESpellShape::Self, EWeaponStatusEffect::Soul, 0.0f, 40.0f, EResourceType::Mana, 25.0f, 0.0f, 0.0f, 0.0f, 10.0f, TEXT("Become spirit, +50% spell damage, immune to physical for 10s"));

    // Ultimate
    { FSpell& S = AddSpell(EClass::Soulbinder, TEXT("Lantern of Departed Choir"), ESpellTier::Ultimate, ESpellShape::AoE, EWeaponStatusEffect::Soul, 300.0f, 100.0f, EResourceType::Aether, 180.0f, 1.0f, 1000.0f, 12.0f, 0.0f, TEXT("Summon spectral choir, AoE spirit damage, heal all allies 100")); S.Healing = 100.0f; }

    // Weapon Arts
    { FSpell& S = AddSpell(EClass::Soulbinder, TEXT("Eidolon Rend"), ESpellTier::WeaponArt, ESpellShape::Cone, EWeaponStatusEffect::Soul, 160.0f, 30.0f, EResourceType::Mana, 12.0f, 0.5f, 600.0f, 6.0f, 0.0f, TEXT("Flay souls from all in cone, drain 30 Flow")); S.bLifesteal = true; S.LifestealPercent = 25.0f; }
    AddSpell(EClass::Soulbinder, TEXT("Spirit Vessel"), ESpellTier::WeaponArt, ESpellShape::Summon, EWeaponStatusEffect::Soul, 100.0f, 35.0f, EResourceType::Mana, 15.0f, 1.0f, 500.0f, 5.0f, 20.0f, TEXT("Summon spirit vessel, fights for 20s, reflects damage to heal allies"));

    // Defensive
    { FSpell& S = AddSpell(EClass::Soulbinder, TEXT("Spirit Shield"), ESpellTier::Defensive, ESpellShape::SingleTarget, EWeaponStatusEffect::Soul, 0.0f, 25.0f, EResourceType::Mana, 12.0f, 0.0f, 500.0f, 0.0f, 8.0f, TEXT("Shield ally, absorb 250, reflect 40% as spirit damage")); S.Healing = 250.0f; }
    { FSpell& S = AddSpell(EClass::Soulbinder, TEXT("Eidolon Tether"), ESpellTier::Defensive, ESpellShape::SingleTarget, EWeaponStatusEffect::Soul, 0.0f, 30.0f, EResourceType::Mana, 18.0f, 0.0f, 400.0f, 0.0f, 8.0f, TEXT("Link souls with ally, split damage 50/50, both heal 10/s")); S.Healing = 80.0f; }
    AddSpell(EClass::Soulbinder, TEXT("Ethereal Form"), ESpellTier::Defensive, ESpellShape::Self, EWeaponStatusEffect::Soul, 0.0f, 35.0f, EResourceType::Mana, 20.0f, 0.0f, 0.0f, 0.0f, 5.0f, TEXT("Phase out, invulnerable 5s, cannot attack but can heal"));

    // Utility
    { FSpell& S = AddSpell(EClass::Soulbinder, TEXT("Eidolon Bind"), ESpellTier::Utility, ESpellShape::SingleTarget, EWeaponStatusEffect::Soul, 60.0f, 20.0f, EResourceType::Mana, 15.0f, 0.0f, 500.0f, 0.0f, 4.0f, TEXT("Root target for 4s, drain 10 Flow per second")); S.bRoots = true; S.bLifesteal = true; S.LifestealPercent = 20.0f; }
    AddSpell(EClass::Soulbinder, TEXT("Spirit Sight"), ESpellTier::Utility, ESpellShape::AoE, EWeaponStatusEffect::Soul, 0.0f, 15.0f, EResourceType::Mana, 15.0f, 0.0f, 0.0f, 15.0f, 10.0f, TEXT("Reveal all enemies in 15m for 10s, see through stealth"));
    AddSpell(EClass::Soulbinder, TEXT("Mana Transfer"), ESpellTier::Utility, ESpellShape::SingleTarget, EWeaponStatusEffect::Soul, 0.0f, 20.0f, EResourceType::Mana, 12.0f, 0.0f, 400.0f, 0.0f, 0.0f, TEXT("Transfer 50 Mana to ally, costs 20 Mana"));

    // Ritual
    { FSpell& S = AddSpell(EClass::Soulbinder, TEXT("Choir of Fallen"), ESpellTier::Ritual, ESpellShape::Zone, EWeaponStatusEffect::Soul, 250.0f, 100.0f, EResourceType::Aether, 300.0f, 3.0f, 1000.0f, 15.0f, 12.0f, TEXT("Summon choir of fallen spirits, 20/s AoE damage, heal allies 20/s")); S.bAppliesDoT = true; S.DoTDamage = 20.0f; S.DoTDuration = 12.0f; S.Healing = 200.0f; }
    { FSpell& S = AddSpell(EClass::Soulbinder, TEXT("Requiem of Binding"), ESpellTier::Ritual, ESpellShape::AoE, EWeaponStatusEffect::Soul, 200.0f, 100.0f, EResourceType::Aether, 300.0f, 2.5f, 800.0f, 12.0f, 0.0f, TEXT("All enemies in 12m silenced and rooted for 8s, drain 50/s")); S.bSilences = true; S.bRoots = true; }
}

// ============================================================================
// WILDWARDEN — Ranger/Hybrid — Earth/Mana — 22 spells
// ============================================================================

void UEclipseSpellSystem::InitWildwardenSpells()
{
    // Cantrips
    AddSpell(EClass::Wildwarden, TEXT("Thorn Prick"), ESpellTier::Cantrip, ESpellShape::Projectile, EWeaponStatusEffect::Earth, 35.0f, 5.0f, EResourceType::Mana, 1.5f, 0.0f, 600.0f, 0.0f, 0.0f, TEXT("Thorn dart, fast projectile"));
    AddSpell(EClass::Wildwarden, TEXT("Bark Skin Tap"), ESpellTier::Cantrip, ESpellShape::Self, EWeaponStatusEffect::Earth, 0.0f, 5.0f, EResourceType::Stamina, 3.0f, 0.0f, 0.0f, 0.0f, 3.0f, TEXT("Brief bark skin, +20% armor for 3s"));

    // Basic
    AddSpell(EClass::Wildwarden, TEXT("Ironbark Mantle"), ESpellTier::Basic, ESpellShape::Self, EWeaponStatusEffect::Earth, 0.0f, 30.0f, EResourceType::Stamina, 15.0f, 0.0f, 0.0f, 0.0f, 8.0f, TEXT("Bark armor, -60% physical damage, thorns reflect 20 damage for 8s"));
    { FSpell& S = AddSpell(EClass::Wildwarden, TEXT("Thorn Sentinel"), ESpellTier::Basic, ESpellShape::AoE, EWeaponStatusEffect::Earth, 90.0f, 20.0f, EResourceType::Mana, 8.0f, 0.0f, 500.0f, 6.0f, 3.0f, TEXT("Summon thorn vines, root enemies in 6m for 3s")); S.bRoots = true; S.bAppliesDoT = true; S.DoTDamage = 15.0f; S.DoTDuration = 3.0f; }
    AddSpell(EClass::Wildwarden, TEXT("Vine Whip"), ESpellTier::Basic, ESpellShape::Line, EWeaponStatusEffect::Earth, 80.0f, 15.0f, EResourceType::Mana, 5.0f, 0.0f, 400.0f, 0.0f, 0.0f, TEXT("Whip thorned vine, pierces enemies in a line"));

    // Intermediate
    { FSpell& S = AddSpell(EClass::Wildwarden, TEXT("Beastheart"), ESpellTier::Intermediate, ESpellShape::Self, EWeaponStatusEffect::Earth, 150.0f, 35.0f, EResourceType::Rage, 20.0f, 0.0f, 300.0f, 0.0f, 6.0f, TEXT("Beast form, +100% attack speed and damage for 6s")); S.bLifesteal = true; S.LifestealPercent = 15.0f; }
    AddSpell(EClass::Wildwarden, TEXT("Snarethorn Lash"), ESpellTier::Intermediate, ESpellShape::Ground, EWeaponStatusEffect::Earth, 80.0f, 28.0f, EResourceType::Mana, 12.0f, 0.5f, 600.0f, 8.0f, 5.0f, TEXT("Roots erupt in 8m, root all enemies for 5s, 16/s damage"));
    AddSpell(EClass::Wildwarden, TEXT("Thorned Hold"), ESpellTier::Intermediate, ESpellShape::Cone, EWeaponStatusEffect::Earth, 120.0f, 25.0f, EResourceType::Mana, 10.0f, 0.5f, 400.0f, 6.0f, 0.0f, TEXT("Vines lash out in cone, pull enemies 5m closer"));

    // Advanced
    AddSpell(EClass::Wildwarden, TEXT("Wildgrove Boon"), ESpellTier::Advanced, ESpellShape::Party, EWeaponStatusEffect::Earth, 0.0f, 40.0f, EResourceType::Mana, 20.0f, 0.5f, 500.0f, 10.0f, 10.0f, TEXT("All allies in 10m heal 100, +20% defense for 10s"));
    { FSpell& S = AddSpell(EClass::Wildwarden, TEXT("Savage Maul"), ESpellTier::Advanced, ESpellShape::SingleTarget, EWeaponStatusEffect::Earth, 200.0f, 35.0f, EResourceType::Rage, 15.0f, 0.5f, 300.0f, 0.0f, 0.0f, TEXT("Beast form maul, causes bleed for 5s")); S.bAppliesDoT = true; S.DoTDamage = 25.0f; S.DoTDuration = 5.0f; S.bAppliesDebuff = true; S.DebuffType = EWeaponStatusEffect::Earth; S.DebuffDuration = 5.0f; }
    AddSpell(EClass::Wildwarden, TEXT("Thicket Trap"), ESpellTier::Advanced, ESpellShape::Ground, EWeaponStatusEffect::Earth, 100.0f, 35.0f, EResourceType::Mana, 18.0f, 0.5f, 500.0f, 8.0f, 10.0f, TEXT("Create thicket, enemies rooted and take 10/s, allies gain stealth inside"));

    // Ultimate
    { FSpell& S = AddSpell(EClass::Wildwarden, TEXT("Grove Seer Awakening"), ESpellTier::Ultimate, ESpellShape::AoE, EWeaponStatusEffect::Earth, 350.0f, 100.0f, EResourceType::Aether, 180.0f, 2.0f, 1200.0f, 15.0f, 0.0f, TEXT("Massive roots in 15m, root all enemies, 150 heal to allies")); S.bRoots = true; S.Healing = 150.0f; }

    // Weapon Arts
    { FSpell& S = AddSpell(EClass::Wildwarden, TEXT("Verdant Howl"), ESpellTier::WeaponArt, ESpellShape::AoE, EWeaponStatusEffect::Earth, 100.0f, 25.0f, EResourceType::Stamina, 10.0f, 0.0f, 800.0f, 10.0f, 0.0f, TEXT("Primal roar, fear all in 10m for 3s, knockback 5m")); S.bStuns = true; S.bKnockbacks = true; }
    AddSpell(EClass::Wildwarden, TEXT("Feral Charge"), ESpellTier::WeaponArt, ESpellShape::Line, EWeaponStatusEffect::Earth, 150.0f, 30.0f, EResourceType::Rage, 12.0f, 0.0f, 500.0f, 0.0f, 0.0f, TEXT("Charge as beast, knockback all in path, +50% damage for 3s after"));

    // Defensive
    AddSpell(EClass::Wildwarden, TEXT("Granite Aspect"), ESpellTier::Defensive, ESpellShape::Self, EWeaponStatusEffect::Earth, 0.0f, 25.0f, EResourceType::Stamina, 18.0f, 0.0f, 0.0f, 0.0f, 8.0f, TEXT("Turn to stone, +80% armor, immune to knockback, -50% move speed for 8s"));
    { FSpell& S = AddSpell(EClass::Wildwarden, TEXT("Rejuvenating Bloom"), ESpellTier::Defensive, ESpellShape::Self, EWeaponStatusEffect::Earth, 0.0f, 28.0f, EResourceType::Mana, 15.0f, 0.0f, 0.0f, 5.0f, 8.0f, TEXT("Heal self 40 instantly + 10/s for 8s")); S.Healing = 120.0f; }
    AddSpell(EClass::Wildwarden, TEXT("Briarweave"), ESpellTier::Defensive, ESpellShape::Self, EWeaponStatusEffect::Earth, 0.0f, 20.0f, EResourceType::Stamina, 15.0f, 0.0f, 0.0f, 0.0f, 8.0f, TEXT("Reflect 30 damage to melee attackers, +20% armor for 8s"));

    // Utility
    AddSpell(EClass::Wildwarden, TEXT("Wildgrove Summons"), ESpellTier::Utility, ESpellShape::Summon, EWeaponStatusEffect::Earth, 60.0f, 30.0f, EResourceType::Mana, 20.0f, 1.0f, 400.0f, 5.0f, 20.0f, TEXT("Summon beast companion, fights for 20s, 40 damage per hit"));
    AddSpell(EClass::Wildwarden, TEXT("Pathfinder Mark"), ESpellTier::Utility, ESpellShape::Ground, EWeaponStatusEffect::Earth, 0.0f, 15.0f, EResourceType::Mana, 10.0f, 0.0f, 500.0f, 3.0f, 15.0f, TEXT("Mark path, allies move +30% speed through marked area for 15s"));
    AddSpell(EClass::Wildwarden, TEXT("Wildgather"), ESpellTier::Utility, ESpellShape::Self, EWeaponStatusEffect::Earth, 0.0f, 10.0f, EResourceType::Mana, 15.0f, 0.5f, 0.0f, 0.0f, 0.0f, TEXT("Gather herbs, restore 40 Mana and 30 HP"));

    // Ritual
    { FSpell& S = AddSpell(EClass::Wildwarden, TEXT("Groveheart Ritual"), ESpellTier::Ritual, ESpellShape::Zone, EWeaponStatusEffect::Earth, 250.0f, 100.0f, EResourceType::Aether, 300.0f, 3.0f, 1000.0f, 20.0f, 15.0f, TEXT("Sacred grove, allies heal 20/s, enemies take 25/s, rooted on entry")); S.bRoots = true; S.bAppliesDoT = true; S.DoTDamage = 25.0f; S.DoTDuration = 15.0f; S.Healing = 300.0f; }
    { FSpell& S = AddSpell(EClass::Wildwarden, TEXT("Primordial Awakening"), ESpellTier::Ritual, ESpellShape::Summon, EWeaponStatusEffect::Earth, 300.0f, 100.0f, EResourceType::Aether, 300.0f, 3.0f, 600.0f, 12.0f, 30.0f, TEXT("Summon ancient treant, 60/s AoE, 300 HP, lasts 30s")); S.bKnockbacks = true; }
}

// ============================================================================
// STORMCALLER — Ranged Burst — Lightning/Mana — 22 spells
// ============================================================================

void UEclipseSpellSystem::InitStormcallerSpells()
{
    // Cantrips
    AddSpell(EClass::Stormcaller, TEXT("Galvanic Charge"), ESpellTier::Cantrip, ESpellShape::Projectile, EWeaponStatusEffect::Lightning, 45.0f, 5.0f, EResourceType::Mana, 1.5f, 0.0f, 700.0f, 0.0f, 0.0f, TEXT("Quick lightning bolt, brief stun on crit"));
    AddSpell(EClass::Stormcaller, TEXT("Zephyr"), ESpellTier::Cantrip, ESpellShape::Cone, EWeaponStatusEffect::Wind, 35.0f, 5.0f, EResourceType::Mana, 2.0f, 0.0f, 400.0f, 0.0f, 0.0f, TEXT("Quick wind blast, pushes enemies back 3m"));

    // Basic
    { FSpell& S = AddSpell(EClass::Stormcaller, TEXT("Thundercall Staff"), ESpellTier::Basic, ESpellShape::Chain, EWeaponStatusEffect::Lightning, 100.0f, 15.0f, EResourceType::Mana, 5.0f, 0.0f, 600.0f, 0.0f, 0.0f, TEXT("Lightning bolt, chains to 3 enemies for 50% each")); S.bStuns = true; }
    { FSpell& S = AddSpell(EClass::Stormcaller, TEXT("Wind Slash"), ESpellTier::Basic, ESpellShape::Cone, EWeaponStatusEffect::Wind, 80.0f, 20.0f, EResourceType::Stamina, 8.0f, 0.0f, 400.0f, 0.0f, 0.0f, TEXT("Blade of wind, knockback all in cone, push 8m")); S.bKnockbacks = true; }
    AddSpell(EClass::Stormcaller, TEXT("Volt Lance"), ESpellTier::Basic, ESpellShape::SingleTarget, EWeaponStatusEffect::Lightning, 130.0f, 18.0f, EResourceType::Mana, 5.0f, 0.5f, 800.0f, 0.0f, 0.0f, TEXT("Direct lightning strike, high single-target damage"));

    // Intermediate
    { FSpell& S = AddSpell(EClass::Stormcaller, TEXT("Storm Surge"), ESpellTier::Intermediate, ESpellShape::Ground, EWeaponStatusEffect::Lightning, 200.0f, 40.0f, EResourceType::Mana, 15.0f, 1.0f, 1000.0f, 8.0f, 5.0f, TEXT("Lightning storm, 8 strikes over 5s in target area")); S.bStuns = true; S.bAppliesDoT = true; S.DoTDamage = 25.0f; S.DoTDuration = 5.0f; }
    AddSpell(EClass::Stormcaller, TEXT("Tempest Drive"), ESpellTier::Intermediate, ESpellShape::Cone, EWeaponStatusEffect::Wind, 140.0f, 30.0f, EResourceType::Mana, 12.0f, 0.5f, 600.0f, 8.0f, 0.0f, TEXT("Hurricane-force wind, knockback + slow all in cone"));
    AddSpell(EClass::Stormcaller, TEXT("Crack of Sky"), ESpellTier::Intermediate, ESpellShape::AoE, EWeaponStatusEffect::Lightning, 150.0f, 35.0f, EResourceType::Mana, 12.0f, 0.3f, 500.0f, 8.0f, 0.0f, TEXT("Sonic boom, 8m AoE, stuns all for 2s"));

    // Advanced
    { FSpell& S = AddSpell(EClass::Stormcaller, TEXT("Surge Breach"), ESpellTier::Advanced, ESpellShape::Self, EWeaponStatusEffect::Lightning, 0.0f, 40.0f, EResourceType::Mana, 25.0f, 0.0f, 0.0f, 0.0f, 10.0f, TEXT("Charge with electricity, +80% lightning damage, spells chain +2 for 10s")); S.bStuns = true; }
    AddSpell(EClass::Stormcaller, TEXT("Tempest Wrath"), ESpellTier::Advanced, ESpellShape::Zone, EWeaponStatusEffect::Lightning, 180.0f, 45.0f, EResourceType::Mana, 20.0f, 1.0f, 1000.0f, 12.0f, 8.0f, TEXT("Summon storm zone, random lightning strikes, 22/s for 8s"));
    { FSpell& S = AddSpell(EClass::Stormcaller, TEXT("Zephyr Throat"), ESpellTier::Advanced, ESpellShape::Line, EWeaponStatusEffect::Wind, 160.0f, 40.0f, EResourceType::Mana, 18.0f, 0.5f, 800.0f, 0.0f, 0.0f, TEXT("Create wind tunnel, pulls enemies through, lightning at end")); S.bKnockbacks = true; }

    // Ultimate
    { FSpell& S = AddSpell(EClass::Stormcaller, TEXT("Tempest Cataclysm"), ESpellTier::Ultimate, ESpellShape::Zone, EWeaponStatusEffect::Lightning, 500.0f, 100.0f, EResourceType::Aether, 180.0f, 2.0f, 1500.0f, 15.0f, 5.0f, TEXT("Tornado + lightning, pull all enemies to center, stun 4s")); S.bStuns = true; S.bKnockbacks = true; }

    // Weapon Arts
    { FSpell& S = AddSpell(EClass::Stormcaller, TEXT("Forked Voltage"), ESpellTier::WeaponArt, ESpellShape::Chain, EWeaponStatusEffect::Lightning, 150.0f, 30.0f, EResourceType::Mana, 12.0f, 0.0f, 800.0f, 0.0f, 0.0f, TEXT("Lightning chains through all in 15m, 6 chains max")); S.bStuns = true; }
    { FSpell& S = AddSpell(EClass::Stormcaller, TEXT("Storm Staff"), ESpellTier::WeaponArt, ESpellShape::AoE, EWeaponStatusEffect::Lightning, 170.0f, 35.0f, EResourceType::Mana, 10.0f, 0.5f, 600.0f, 8.0f, 0.0f, TEXT("Staff slam, 8m lightning burst, stuns all")); S.bStuns = true; }

    // Defensive
    AddSpell(EClass::Stormcaller, TEXT("Zephyr Wall"), ESpellTier::Defensive, ESpellShape::Self, EWeaponStatusEffect::Wind, 0.0f, 25.0f, EResourceType::Mana, 15.0f, 0.0f, 0.0f, 0.0f, 6.0f, TEXT("Wind shield, deflects projectiles, +30% dodge for 6s"));
    AddSpell(EClass::Stormcaller, TEXT("Galvanic Field"), ESpellTier::Defensive, ESpellShape::Aura, EWeaponStatusEffect::Lightning, 50.0f, 30.0f, EResourceType::Mana, 18.0f, 0.0f, 0.0f, 6.0f, 8.0f, TEXT("Electric aura, enemies in 6m take 10/s, allies gain 10% dodge"));
    AddSpell(EClass::Stormcaller, TEXT("Galvanic Conduit"), ESpellTier::Defensive, ESpellShape::Self, EWeaponStatusEffect::Lightning, 0.0f, 20.0f, EResourceType::Mana, 12.0f, 0.0f, 0.0f, 0.0f, 5.0f, TEXT("Draw lightning, absorb next spell, reflect as 2x lightning"));

    // Utility
    { FSpell& S = AddSpell(EClass::Stormcaller, TEXT("Zephyr Stride"), ESpellTier::Utility, ESpellShape::Self, EWeaponStatusEffect::Wind, 0.0f, 20.0f, EResourceType::Mana, 15.0f, 0.0f, 800.0f, 0.0f, 0.0f, TEXT("Ride wind, +100% move speed for 5s, ignore terrain")); }
    AddSpell(EClass::Stormcaller, TEXT("Galvanic Pull"), ESpellTier::Utility, ESpellShape::AoE, EWeaponStatusEffect::Lightning, 30.0f, 25.0f, EResourceType::Mana, 18.0f, 0.0f, 600.0f, 10.0f, 0.0f, TEXT("Magnetic pull, all enemies in 10m pulled 8m toward you"));
    AddSpell(EClass::Stormcaller, TEXT("Surging Volt"), ESpellTier::Utility, ESpellShape::Self, EWeaponStatusEffect::Lightning, 0.0f, 15.0f, EResourceType::Mana, 25.0f, 0.0f, 0.0f, 0.0f, 0.0f, TEXT("Restore 60 Mana, next spell +50% damage"));

    // Ritual
    { FSpell& S = AddSpell(EClass::Stormcaller, TEXT("Great Spiral"), ESpellTier::Ritual, ESpellShape::Zone, EWeaponStatusEffect::Wind, 300.0f, 100.0f, EResourceType::Aether, 300.0f, 3.0f, 1500.0f, 20.0f, 12.0f, TEXT("Massive hurricane, 25/s, all enemies knocked around randomly for 12s")); S.bKnockbacks = true; S.bAppliesDoT = true; S.DoTDamage = 25.0f; S.DoTDuration = 12.0f; }
    { FSpell& S = AddSpell(EClass::Stormcaller, TEXT("Storm God's Wrath"), ESpellTier::Ritual, ESpellShape::AoE, EWeaponStatusEffect::Lightning, 400.0f, 100.0f, EResourceType::Aether, 300.0f, 2.5f, 1500.0f, 20.0f, 0.0f, TEXT("20m lightning catastrophe, 400 burst + 40/s for 10s, all stunned")); S.bStuns = true; S.bAppliesDoT = true; S.DoTDamage = 40.0f; S.DoTDuration = 10.0f; }
}

// ============================================================================
// ORACLE — Support/Caster — Holy/Mana — 22 spells
// ============================================================================

void UEclipseSpellSystem::InitOracleSpells()
{
    // Cantrips
    { FSpell& S = AddSpell(EClass::Oracle, TEXT("Divine Spark"), ESpellTier::Cantrip, ESpellShape::Projectile, EWeaponStatusEffect::Holy, 40.0f, 5.0f, EResourceType::Mana, 1.5f, 0.0f, 600.0f, 0.0f, 0.0f, TEXT("Holy spark, bonus vs undead")); }
    { FSpell& S = AddSpell(EClass::Oracle, TEXT("Lesser Boon"), ESpellTier::Cantrip, ESpellShape::SingleTarget, EWeaponStatusEffect::Holy, 0.0f, 5.0f, EResourceType::Mana, 3.0f, 0.0f, 400.0f, 0.0f, 0.0f, TEXT("Quick heal 30 HP to ally")); S.Healing = 30.0f; }

    // Basic
    AddSpell(EClass::Oracle, TEXT("Premonition"), ESpellTier::Basic, ESpellShape::Self, EWeaponStatusEffect::Holy, 0.0f, 20.0f, EResourceType::Mana, 10.0f, 0.0f, 0.0f, 0.0f, 8.0f, TEXT("Reveal all enemy telegraphs for 8s, +20% dodge for all allies"));
    { FSpell& S = AddSpell(EClass::Oracle, TEXT("Divine Prophecy"), ESpellTier::Basic, ESpellShape::SingleTarget, EWeaponStatusEffect::Holy, 0.0f, 25.0f, EResourceType::Mana, 6.0f, 0.5f, 600.0f, 0.0f, 5.0f, TEXT("Heal target 150, auto-heal 200 if below 20% HP in 5s")); S.Healing = 150.0f; }
    { FSpell& S = AddSpell(EClass::Oracle, TEXT("Tribunal"), ESpellTier::Basic, ESpellShape::SingleTarget, EWeaponStatusEffect::Holy, 130.0f, 35.0f, EResourceType::Mana, 15.0f, 0.0f, 500.0f, 0.0f, 8.0f, TEXT("Holy mark, all allies +25% damage to marked target for 8s")); S.bAppliesDebuff = true; S.DebuffType = EWeaponStatusEffect::Holy; S.DebuffDuration = 8.0f; }

    // Intermediate
    { FSpell& S = AddSpell(EClass::Oracle, TEXT("Astral Rain"), ESpellTier::Intermediate, ESpellShape::Ground, EWeaponStatusEffect::Holy, 120.0f, 30.0f, EResourceType::Mana, 14.0f, 0.8f, 800.0f, 6.0f, 2.0f, TEXT("Rain holy stars, 6 impacts over 2s, heal allies 30 per impact")); S.Healing = 180.0f; }
    { FSpell& S = AddSpell(EClass::Oracle, TEXT("Consecrated Brilliance"), ESpellTier::Intermediate, ESpellShape::AoE, EWeaponStatusEffect::Holy, 100.0f, 35.0f, EResourceType::Mana, 12.0f, 0.5f, 500.0f, 8.0f, 0.0f, TEXT("8m holy burst, damage enemies, heal allies 80")); S.Healing = 80.0f; }
    AddSpell(EClass::Oracle, TEXT("Clairvoyance"), ESpellTier::Intermediate, ESpellShape::Self, EWeaponStatusEffect::Holy, 0.0f, 25.0f, EResourceType::Mana, 20.0f, 0.0f, 0.0f, 20.0f, 10.0f, TEXT("Reveal entire map for 10s, see enemy HP and debuffs"));

    // Advanced
    { FSpell& S = AddSpell(EClass::Oracle, TEXT("Heaven's Gate"), ESpellTier::Advanced, ESpellShape::Party, EWeaponStatusEffect::Holy, 0.0f, 45.0f, EResourceType::Mana, 25.0f, 1.0f, 500.0f, 12.0f, 8.0f, TEXT("All allies heal 150, +30% damage, immune to debuffs for 8s")); S.Healing = 150.0f; }
    { FSpell& S = AddSpell(EClass::Oracle, TEXT("Doomsday Prophecy"), ESpellTier::Advanced, ESpellShape::SingleTarget, EWeaponStatusEffect::Holy, 50.0f, 40.0f, EResourceType::Mana, 22.0f, 1.0f, 600.0f, 0.0f, 6.0f, TEXT("Mark target with doom, takes 50% more damage, 200 holy burst after 6s")); S.bAppliesDebuff = true; S.DebuffType = EWeaponStatusEffect::Holy; S.DebuffDuration = 6.0f; }
    AddSpell(EClass::Oracle, TEXT("Boon of Alacrity"), ESpellTier::Advanced, ESpellShape::Party, EWeaponStatusEffect::Holy, 0.0f, 35.0f, EResourceType::Mana, 20.0f, 0.0f, 0.0f, 10.0f, 10.0f, TEXT("All allies +40% attack and cast speed for 10s"));

    // Ultimate
    { FSpell& S = AddSpell(EClass::Oracle, TEXT("All-Knowing Grace"), ESpellTier::Ultimate, ESpellShape::Party, EWeaponStatusEffect::Holy, 0.0f, 100.0f, EResourceType::Aether, 180.0f, 1.0f, 2000.0f, 20.0f, 10.0f, TEXT("Full party heal 300, restore all resources, 10s debuff immunity")); S.Healing = 300.0f; }

    // Weapon Arts
    { FSpell& S = AddSpell(EClass::Oracle, TEXT("Starfall Strike"), ESpellTier::WeaponArt, ESpellShape::Ground, EWeaponStatusEffect::Holy, 120.0f, 30.0f, EResourceType::Mana, 14.0f, 0.8f, 800.0f, 6.0f, 2.0f, TEXT("Rain holy stars on area, 6 impacts, heal allies 30 per impact")); S.Healing = 30.0f; }
    AddSpell(EClass::Oracle, TEXT("Oracle's Eye"), ESpellTier::WeaponArt, ESpellShape::Self, EWeaponStatusEffect::Holy, 0.0f, 35.0f, EResourceType::Mana, 15.0f, 0.0f, 0.0f, 15.0f, 8.0f, TEXT("Third eye, +50% crit chance, reveal all stealth for 8s"));

    // Defensive
    { FSpell& S = AddSpell(EClass::Oracle, TEXT("Hallowed Mending"), ESpellTier::Defensive, ESpellShape::SingleTarget, EWeaponStatusEffect::Holy, 0.0f, 30.0f, EResourceType::Mana, 10.0f, 0.5f, 600.0f, 0.0f, 0.0f, TEXT("Heal target 200, cleanse 2 debuffs")); S.Healing = 200.0f; }
    AddSpell(EClass::Oracle, TEXT("Consecrated Guard"), ESpellTier::Defensive, ESpellShape::SingleTarget, EWeaponStatusEffect::Holy, 0.0f, 35.0f, EResourceType::Mana, 18.0f, 0.0f, 500.0f, 0.0f, 6.0f, TEXT("Target ally immune to damage for 3s, then heal 100"));
    AddSpell(EClass::Oracle, TEXT("Lustral Mantle"), ESpellTier::Defensive, ESpellShape::Aura, EWeaponStatusEffect::Holy, 0.0f, 30.0f, EResourceType::Mana, 20.0f, 0.0f, 0.0f, 8.0f, 8.0f, TEXT("Aura cleanses 1 debuff per second from allies in 8m, heal 10/s"));

    // Utility
    { FSpell& S = AddSpell(EClass::Oracle, TEXT("Fate Weaving"), ESpellTier::Utility, ESpellShape::SingleTarget, EWeaponStatusEffect::Holy, 0.0f, 20.0f, EResourceType::Mana, 15.0f, 0.0f, 400.0f, 0.0f, 8.0f, TEXT("Reroll target's next attack, guarantee crit or guarantee miss")); }
    AddSpell(EClass::Oracle, TEXT("Sight"), ESpellTier::Utility, ESpellShape::AoE, EWeaponStatusEffect::Holy, 0.0f, 15.0f, EResourceType::Mana, 12.0f, 0.0f, 0.0f, 25.0f, 5.0f, TEXT("Reveal 25m area for 5s, all stealth invisible, traps visible"));
    AddSpell(EClass::Oracle, TEXT("Karmic Reckoning"), ESpellTier::Utility, ESpellShape::SingleTarget, EWeaponStatusEffect::Holy, 80.0f, 25.0f, EResourceType::Mana, 18.0f, 0.0f, 500.0f, 0.0f, 8.0f, TEXT("Mark enemy, next attack against them heals attacker for 50% of damage"));

    // Ritual
    { FSpell& S = AddSpell(EClass::Oracle, TEXT("Celestial Mandate"), ESpellTier::Ritual, ESpellShape::Party, EWeaponStatusEffect::Holy, 0.0f, 100.0f, EResourceType::Aether, 300.0f, 3.0f, 2000.0f, 20.0f, 10.0f, TEXT("Prevent all ally deaths for 10s, full heal on expiry, +50% damage")); S.Healing = 500.0f; }
    { FSpell& S = AddSpell(EClass::Oracle, TEXT("Prophecy of Ruin"), ESpellTier::Ritual, ESpellShape::Zone, EWeaponStatusEffect::Holy, 300.0f, 100.0f, EResourceType::Aether, 300.0f, 3.0f, 1500.0f, 20.0f, 12.0f, TEXT("Doom zone, 30/s holy damage, all enemies -50% damage for 12s")); S.bStuns = true; S.bAppliesDoT = true; S.DoTDamage = 30.0f; S.DoTDuration = 12.0f; S.bAppliesDebuff = true; S.DebuffType = EWeaponStatusEffect::Holy; S.DebuffDuration = 12.0f; }
}

// ============================================================================
// ALCHEMIST — Utility/Control — Poison/Reagents — 22 spells
// ============================================================================

void UEclipseSpellSystem::InitAlchemistSpells()
{
    // Cantrips
    { FSpell& S = AddSpell(EClass::Alchemist, TEXT("Acid Splash"), ESpellTier::Cantrip, ESpellShape::Projectile, EWeaponStatusEffect::Poison, 35.0f, 5.0f, EResourceType::Reagents, 1.5f, 0.0f, 500.0f, 0.0f, 0.0f, TEXT("Small acid vial, applies weak poison")); S.bAppliesDoT = true; S.DoTDamage = 8.0f; S.DoTDuration = 3.0f; }
    AddSpell(EClass::Alchemist, TEXT("Quick Mix"), ESpellTier::Cantrip, ESpellShape::Self, EWeaponStatusEffect::Poison, 0.0f, 5.0f, EResourceType::Reagents, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, TEXT("Instant reagent mix, restore 10 Reagents"));

    // Basic
    { FSpell& S = AddSpell(EClass::Alchemist, TEXT("Reagent Bomb"), ESpellTier::Basic, ESpellShape::AoE, EWeaponStatusEffect::Poison, 100.0f, 20.0f, EResourceType::Reagents, 8.0f, 0.0f, 500.0f, 5.0f, 0.0f, TEXT("Alchemical bomb, AoE + random status (fire/ice/poison)")); S.bAppliesDebuff = true; S.DebuffType = EWeaponStatusEffect::Poison; S.DebuffDuration = 5.0f; }
    { FSpell& S = AddSpell(EClass::Alchemist, TEXT("Healing Draught"), ESpellTier::Basic, ESpellShape::Projectile, EWeaponStatusEffect::Poison, 0.0f, 25.0f, EResourceType::Reagents, 12.0f, 0.0f, 400.0f, 0.0f, 0.0f, TEXT("Throw healing flask, heal ally 120, cleanse 2 debuffs")); S.Healing = 120.0f; }
    { FSpell& S = AddSpell(EClass::Alchemist, TEXT("Toxic Flask"), ESpellTier::Basic, ESpellShape::Ground, EWeaponStatusEffect::Poison, 60.0f, 18.0f, EResourceType::Reagents, 8.0f, 0.0f, 500.0f, 4.0f, 6.0f, TEXT("Throw toxic flask, 4m pool, 15/s poison for 6s")); S.bAppliesDoT = true; S.DoTDamage = 15.0f; S.DoTDuration = 6.0f; S.bAppliesDebuff = true; S.DebuffType = EWeaponStatusEffect::Poison; S.DebuffDuration = 6.0f; }

    // Intermediate
    { FSpell& S = AddSpell(EClass::Alchemist, TEXT("Mutation Elixir"), ESpellTier::Intermediate, ESpellShape::Party, EWeaponStatusEffect::Poison, 0.0f, 40.0f, EResourceType::Reagents, 20.0f, 0.0f, 500.0f, 8.0f, 10.0f, TEXT("Buff all allies in 8m, +30% damage, +20% defense for 10s")); S.Healing = 50.0f; }
    AddSpell(EClass::Alchemist, TEXT("Flash Powder"), ESpellTier::Intermediate, ESpellShape::AoE, EWeaponStatusEffect::Fire, 80.0f, 25.0f, EResourceType::Reagents, 10.0f, 0.0f, 500.0f, 6.0f, 0.0f, TEXT("Alchemical flash, blind all in 6m, 3s miss chance"));
    { FSpell& S = AddSpell(EClass::Alchemist, TEXT("Corrosive Phial"), ESpellTier::Intermediate, ESpellShape::Ground, EWeaponStatusEffect::Poison, 80.0f, 20.0f, EResourceType::Reagents, 10.0f, 0.0f, 600.0f, 5.0f, 8.0f, TEXT("5m acid pool, 20/s, armor reduced 50% for 8s")); S.bAppliesDoT = true; S.DoTDamage = 20.0f; S.DoTDuration = 8.0f; S.bAppliesDebuff = true; S.DebuffType = EWeaponStatusEffect::Poison; S.DebuffDuration = 8.0f; }

    // Advanced
    { FSpell& S = AddSpell(EClass::Alchemist, TEXT("Cureall Draught"), ESpellTier::Advanced, ESpellShape::Party, EWeaponStatusEffect::Poison, 0.0f, 40.0f, EResourceType::Reagents, 18.0f, 0.5f, 500.0f, 10.0f, 0.0f, TEXT("Cure all debuffs and DoTs from all allies in 10m, heal 80")); S.Healing = 80.0f; }
    { FSpell& S = AddSpell(EClass::Alchemist, TEXT("Noxious Cloud"), ESpellTier::Advanced, ESpellShape::Zone, EWeaponStatusEffect::Poison, 120.0f, 40.0f, EResourceType::Reagents, 22.0f, 0.5f, 600.0f, 10.0f, 8.0f, TEXT("Toxic cloud zone, 20/s poison, -50% healing to enemies for 8s")); S.bAppliesDoT = true; S.DoTDamage = 20.0f; S.DoTDuration = 8.0f; S.bAppliesDebuff = true; S.DebuffType = EWeaponStatusEffect::Poison; S.DebuffDuration = 8.0f; }
    AddSpell(EClass::Alchemist, TEXT("Alchemical Shift"), ESpellTier::Advanced, ESpellShape::Self, EWeaponStatusEffect::Poison, 0.0f, 35.0f, EResourceType::Reagents, 25.0f, 0.0f, 0.0f, 0.0f, 0.0f, TEXT("Convert 50 Reagents to 100 Mana, or 50 Mana to 80 Reagents"));

    // Ultimate
    { FSpell& S = AddSpell(EClass::Alchemist, TEXT("Primal Catalyst"), ESpellTier::Ultimate, ESpellShape::AoE, EWeaponStatusEffect::Poison, 300.0f, 100.0f, EResourceType::Aether, 180.0f, 1.5f, 1000.0f, 10.0f, 0.0f, TEXT("Transmute area, 300 damage to enemies, convert to healing for allies")); S.Healing = 150.0f; }

    // Weapon Arts
    { FSpell& S = AddSpell(EClass::Alchemist, TEXT("Acid Spray"), ESpellTier::WeaponArt, ESpellShape::Cone, EWeaponStatusEffect::Poison, 120.0f, 25.0f, EResourceType::Reagents, 10.0f, 0.0f, 400.0f, 6.0f, 0.0f, TEXT("Spray acid in cone, -50% armor, 15/s for 5s")); S.bAppliesDoT = true; S.DoTDamage = 15.0f; S.DoTDuration = 5.0f; S.bAppliesDebuff = true; S.DebuffType = EWeaponStatusEffect::Poison; S.DebuffDuration = 5.0f; }
    AddSpell(EClass::Alchemist, TEXT("Frenzy Flask"), ESpellTier::WeaponArt, ESpellShape::SingleTarget, EWeaponStatusEffect::Fire, 0.0f, 20.0f, EResourceType::Reagents, 12.0f, 0.0f, 300.0f, 0.0f, 8.0f, TEXT("Throw berserk potion, ally +50% damage, +50% attack speed, -20% defense for 8s"));

    // Defensive
    { FSpell& S = AddSpell(EClass::Alchemist, TEXT("Petrified Tincture"), ESpellTier::Defensive, ESpellShape::Self, EWeaponStatusEffect::Earth, 0.0f, 25.0f, EResourceType::Reagents, 15.0f, 0.0f, 0.0f, 0.0f, 8.0f, TEXT("Drink potion, +60% armor, immune to knockback for 8s")); S.Healing = 50.0f; }
    { FSpell& S = AddSpell(EClass::Alchemist, TEXT("Toxinclear"), ESpellTier::Defensive, ESpellShape::SingleTarget, EWeaponStatusEffect::Poison, 0.0f, 15.0f, EResourceType::Reagents, 8.0f, 0.0f, 400.0f, 0.0f, 0.0f, TEXT("Cure poison and all DoTs from target, heal 60")); S.Healing = 60.0f; }
    AddSpell(EClass::Alchemist, TEXT("Veilscreen"), ESpellTier::Defensive, ESpellShape::AoE, EWeaponStatusEffect::Poison, 0.0f, 20.0f, EResourceType::Reagents, 15.0f, 0.0f, 0.0f, 8.0f, 5.0f, TEXT("Alchemical smoke, allies in 8m invisible 5s, enemies blinded"));

    // Utility
    AddSpell(EClass::Alchemist, TEXT("Shuriken Volley"), ESpellTier::Utility, ESpellShape::Projectile, EWeaponStatusEffect::Poison, 70.0f, 10.0f, EResourceType::Reagents, 5.0f, 0.0f, 600.0f, 0.0f, 0.0f, TEXT("Throw alchemical star, applies random element on hit"));
    { FSpell& S = AddSpell(EClass::Alchemist, TEXT("Adhesive Blast"), ESpellTier::Utility, ESpellShape::AoE, EWeaponStatusEffect::Earth, 30.0f, 15.0f, EResourceType::Reagents, 12.0f, 0.0f, 500.0f, 6.0f, 5.0f, TEXT("Sticky bomb, root all in 6m for 5s")); S.bRoots = true; }
    AddSpell(EClass::Alchemist, TEXT("Reagent Harvest"), ESpellTier::Utility, ESpellShape::Self, EWeaponStatusEffect::Poison, 0.0f, 10.0f, EResourceType::Reagents, 30.0f, 1.0f, 0.0f, 0.0f, 0.0f, TEXT("Gather materials, restore 40 Reagents"));

    // Ritual
    { FSpell& S = AddSpell(EClass::Alchemist, TEXT("Master Transmutation"), ESpellTier::Ritual, ESpellShape::Zone, EWeaponStatusEffect::Poison, 250.0f, 100.0f, EResourceType::Aether, 300.0f, 3.0f, 1000.0f, 15.0f, 10.0f, TEXT("15m transmutation circle, enemies take 25/s, allies heal 25/s")); S.bAppliesDoT = true; S.DoTDamage = 25.0f; S.DoTDuration = 10.0f; S.Healing = 250.0f; }
    { FSpell& S = AddSpell(EClass::Alchemist, TEXT("Draught of Endless Breath"), ESpellTier::Ritual, ESpellShape::Party, EWeaponStatusEffect::Poison, 0.0f, 100.0f, EResourceType::Aether, 300.0f, 3.0f, 800.0f, 12.0f, 15.0f, TEXT("All allies immortal for 5s, then heal 200, +50% all stats for 15s")); S.Healing = 200.0f; }
}

// ============================================================================
// BERSERKER — Plate DPS — Fire/Rage — 22 spells
// ============================================================================

void UEclipseSpellSystem::InitBerserkerSpells()
{
    // Cantrips
    AddSpell(EClass::Berserker, TEXT("Rage Slash"), ESpellTier::Cantrip, ESpellShape::SingleTarget, EWeaponStatusEffect::Fire, 45.0f, 5.0f, EResourceType::Rage, 1.0f, 0.0f, 250.0f, 0.0f, 0.0f, TEXT("Wild slash, builds 5 Rage"));
    AddSpell(EClass::Berserker, TEXT("Crown Crack"), ESpellTier::Cantrip, ESpellShape::SingleTarget, EWeaponStatusEffect::Fire, 50.0f, 5.0f, EResourceType::Rage, 2.0f, 0.0f, 200.0f, 0.0f, 0.0f, TEXT("Savage headbutt, brief stagger"));

    // Basic
    AddSpell(EClass::Berserker, TEXT("Rage Frenzy"), ESpellTier::Basic, ESpellShape::Self, EWeaponStatusEffect::Fire, 80.0f, 30.0f, EResourceType::Rage, 10.0f, 0.0f, 300.0f, 0.0f, 8.0f, TEXT("Enter frenzy, +50% attack speed, +30% damage, no stamina cost for 8s"));
    { FSpell& S = AddSpell(EClass::Berserker, TEXT("Seismic Sunder"), ESpellTier::Basic, ESpellShape::AoE, EWeaponStatusEffect::Earth, 150.0f, 40.0f, EResourceType::Rage, 15.0f, 0.0f, 600.0f, 10.0f, 0.0f, TEXT("Slam 2h axe into ground, AoE knockback 10m")); S.bKnockbacks = true; S.bStuns = true; }
    { FSpell& S = AddSpell(EClass::Berserker, TEXT("Gore Hunger"), ESpellTier::Basic, ESpellShape::SingleTarget, EWeaponStatusEffect::Fire, 120.0f, 20.0f, EResourceType::Rage, 8.0f, 0.0f, 200.0f, 0.0f, 0.0f, TEXT("Savage strike, 100% lifesteal, build 20 Rage on hit")); S.bLifesteal = true; S.LifestealPercent = 100.0f; }

    // Intermediate
    AddSpell(EClass::Berserker, TEXT("Cyclone Slash"), ESpellTier::Intermediate, ESpellShape::AoE, EWeaponStatusEffect::Fire, 140.0f, 30.0f, EResourceType::Rage, 12.0f, 0.0f, 300.0f, 6.0f, 0.0f, TEXT("Spin with axe, hit all in 6m, 5 hits over 2s"));
    { FSpell& S = AddSpell(EClass::Berserker, TEXT("Reckless Hew"), ESpellTier::Intermediate, ESpellShape::Cone, EWeaponStatusEffect::Fire, 160.0f, 28.0f, EResourceType::Rage, 10.0f, 0.5f, 400.0f, 6.0f, 0.0f, TEXT("Reckless overhead swing, +50% damage, take 10% more damage for 3s")); S.bKnockbacks = true; }
    AddSpell(EClass::Berserker, TEXT("Battlehowl"), ESpellTier::Intermediate, ESpellShape::Self, EWeaponStatusEffect::Fire, 0.0f, 35.0f, EResourceType::Rage, 15.0f, 0.0f, 0.0f, 8.0f, 6.0f, TEXT("Bellow, +40% damage, immune to fear and stun for 6s"));

    // Advanced
    AddSpell(EClass::Berserker, TEXT("Sanguine Fury"), ESpellTier::Advanced, ESpellShape::Self, EWeaponStatusEffect::Fire, 0.0f, 40.0f, EResourceType::Rage, 25.0f, 0.0f, 0.0f, 0.0f, 10.0f, TEXT("Lower HP = higher damage, +100% damage at <50% HP for 10s"));
    { FSpell& S = AddSpell(EClass::Berserker, TEXT("Spinning Hew"), ESpellTier::Advanced, ESpellShape::AoE, EWeaponStatusEffect::Fire, 200.0f, 40.0f, EResourceType::Rage, 18.0f, 0.5f, 500.0f, 10.0f, 0.0f, TEXT("Throw axe in cyclone, hits all in 10m, returns")); S.bKnockbacks = true; }
    { FSpell& S = AddSpell(EClass::Berserker, TEXT("Headsman"), ESpellTier::Advanced, ESpellShape::SingleTarget, EWeaponStatusEffect::Fire, 220.0f, 35.0f, EResourceType::Rage, 15.0f, 0.5f, 250.0f, 0.0f, 0.0f, TEXT("Execute, +200% damage vs targets below 30% HP")); S.bLifesteal = true; S.LifestealPercent = 50.0f; }

    // Ultimate
    { FSpell& S = AddSpell(EClass::Berserker, TEXT("Berserker Rampage"), ESpellTier::Ultimate, ESpellShape::Self, EWeaponStatusEffect::Fire, 500.0f, 100.0f, EResourceType::Aether, 180.0f, 0.0f, 800.0f, 0.0f, 10.0f, TEXT("2x damage, immune to CC, 50% lifesteal, 10s duration")); S.bLifesteal = true; S.LifestealPercent = 50.0f; }

    // Weapon Arts
    { FSpell& S = AddSpell(EClass::Berserker, TEXT("Pulverize"), ESpellTier::WeaponArt, ESpellShape::SingleTarget, EWeaponStatusEffect::Fire, 250.0f, 35.0f, EResourceType::Rage, 8.0f, 0.5f, 300.0f, 0.0f, 0.0f, TEXT("Charged overhead axe slam, guaranteed stagger, 2x vs staggered")); S.bStuns = true; }
    { FSpell& S = AddSpell(EClass::Berserker, TEXT("Furious Maelstrom"), ESpellTier::WeaponArt, ESpellShape::AoE, EWeaponStatusEffect::Fire, 180.0f, 40.0f, EResourceType::Rage, 12.0f, 0.0f, 400.0f, 6.0f, 0.0f, TEXT("8 rapid axe swings, each hit builds 5 Rage")); S.bLifesteal = true; S.LifestealPercent = 20.0f; }

    // Defensive
    AddSpell(EClass::Berserker, TEXT("Tempered Resolve"), ESpellTier::Defensive, ESpellShape::Self, EWeaponStatusEffect::Fire, 0.0f, 30.0f, EResourceType::Rage, 20.0f, 0.0f, 0.0f, 0.0f, 6.0f, TEXT("Immune to CC, -50% damage taken for 6s, but cannot move"));
    { FSpell& S = AddSpell(EClass::Berserker, TEXT("Sanguine Plate"), ESpellTier::Defensive, ESpellShape::Self, EWeaponStatusEffect::Fire, 0.0f, 25.0f, EResourceType::Rage, 15.0f, 0.0f, 0.0f, 0.0f, 8.0f, TEXT("Coat self in blood, +40% armor, melee attackers take 20 damage for 8s")); S.Healing = 60.0f; }
    AddSpell(EClass::Berserker, TEXT("Furyward"), ESpellTier::Defensive, ESpellShape::Self, EWeaponStatusEffect::Fire, 0.0f, 20.0f, EResourceType::Rage, 12.0f, 0.0f, 0.0f, 0.0f, 5.0f, TEXT("Convert 50 Rage to 250 shield, lasts 5s"));

    // Utility
    AddSpell(EClass::Berserker, TEXT("Dread Howl"), ESpellTier::Utility, ESpellShape::Cone, EWeaponStatusEffect::Fire, 50.0f, 20.0f, EResourceType::Rage, 15.0f, 0.0f, 600.0f, 0.0f, 3.0f, TEXT("Fear all enemies in cone for 3s, knockback 5m"));
    AddSpell(EClass::Berserker, TEXT("Wroth Gathering"), ESpellTier::Utility, ESpellShape::Self, EWeaponStatusEffect::Fire, 0.0f, 0.0f, EResourceType::Rage, 8.0f, 0.0f, 0.0f, 0.0f, 0.0f, TEXT("Take 50 self-damage, gain 40 Rage instantly"));
    AddSpell(EClass::Berserker, TEXT("Battering Rush"), ESpellTier::Utility, ESpellShape::Line, EWeaponStatusEffect::Fire, 80.0f, 15.0f, EResourceType::Rage, 10.0f, 0.0f, 600.0f, 0.0f, 0.0f, TEXT("Charge to target, knockback on arrival, build 20 Rage"));

    // Ritual
    { FSpell& S = AddSpell(EClass::Berserker, TEXT("Unbound Fury"), ESpellTier::Ritual, ESpellShape::Self, EWeaponStatusEffect::Fire, 350.0f, 100.0f, EResourceType::Aether, 300.0f, 2.0f, 800.0f, 10.0f, 15.0f, TEXT("3x damage, immune to all CC, 80% lifesteal, unstoppable for 15s")); S.bLifesteal = true; S.LifestealPercent = 80.0f; }
    { FSpell& S = AddSpell(EClass::Berserker, TEXT("Blood God's Blessing"), ESpellTier::Ritual, ESpellShape::Party, EWeaponStatusEffect::Fire, 200.0f, 100.0f, EResourceType::Aether, 300.0f, 2.5f, 1000.0f, 15.0f, 15.0f, TEXT("All allies +100% damage, 50% lifesteal, immune to death for 15s")); S.bLifesteal = true; S.LifestealPercent = 50.0f; S.Healing = 200.0f; }
}

// ============================================================================
// SHADOWBLADE — Stealth/Duskblade — Shadow/Stamina — 22 spells
// ============================================================================

void UEclipseSpellSystem::InitShadowbladeSpells()
{
    // Cantrips
    AddSpell(EClass::Shadowblade, TEXT("Shadow Flick"), ESpellTier::Cantrip, ESpellShape::SingleTarget, EWeaponStatusEffect::Shadow, 40.0f, 5.0f, EResourceType::Stamina, 1.5f, 0.0f, 250.0f, 0.0f, 0.0f, TEXT("Quick shadow slash, builds combo"));
    AddSpell(EClass::Shadowblade, TEXT("Void Cut"), ESpellTier::Cantrip, ESpellShape::SingleTarget, EWeaponStatusEffect::Shadow, 45.0f, 5.0f, EResourceType::Stamina, 2.0f, 0.0f, 250.0f, 0.0f, 0.0f, TEXT("Void-infused cut, ignores 20% armor"));

    // Basic
    { FSpell& S = AddSpell(EClass::Shadowblade, TEXT("Gloom Reave"), ESpellTier::Basic, ESpellShape::Cone, EWeaponStatusEffect::Shadow, 100.0f, 20.0f, EResourceType::Stamina, 6.0f, 0.0f, 300.0f, 4.0f, 0.0f, TEXT("Sweeping dark slash, hits all in arc, 5s bleed")); S.bAppliesDoT = true; S.DoTDamage = 12.0f; S.DoTDuration = 5.0f; }
    { FSpell& S = AddSpell(EClass::Shadowblade, TEXT("Umbral Strike"), ESpellTier::Basic, ESpellShape::SingleTarget, EWeaponStatusEffect::Shadow, 150.0f, 25.0f, EResourceType::Stamina, 10.0f, 0.0f, 200.0f, 0.0f, 0.0f, TEXT("Phase through enemy, void damage, ignores 50% armor")); S.bLifesteal = true; S.LifestealPercent = 15.0f; }
    AddSpell(EClass::Shadowblade, TEXT("Dark Step"), ESpellTier::Basic, ESpellShape::Self, EWeaponStatusEffect::Shadow, 80.0f, 15.0f, EResourceType::Stamina, 8.0f, 0.0f, 600.0f, 0.0f, 0.0f, TEXT("Dash through shadows, briefly invisible, next attack guaranteed crit"));

    // Intermediate
    { FSpell& S = AddSpell(EClass::Shadowblade, TEXT("Gloomfangs"), ESpellTier::Intermediate, ESpellShape::Projectile, EWeaponStatusEffect::Shadow, 130.0f, 22.0f, EResourceType::Stamina, 8.0f, 0.0f, 500.0f, 0.0f, 0.0f, TEXT("Throw 3 shadow daggers, each applies 3s bleed")); S.bAppliesDoT = true; S.DoTDamage = 10.0f; S.DoTDuration = 3.0f; }
    AddSpell(EClass::Shadowblade, TEXT("Nullgrip"), ESpellTier::Intermediate, ESpellShape::SingleTarget, EWeaponStatusEffect::Shadow, 80.0f, 25.0f, EResourceType::Stamina, 12.0f, 0.0f, 400.0f, 0.0f, 3.0f, TEXT("Shadow grip, pull target 8m toward you, root 3s"));
    AddSpell(EClass::Shadowblade, TEXT("Eclipse Step"), ESpellTier::Intermediate, ESpellShape::Self, EWeaponStatusEffect::Shadow, 100.0f, 30.0f, EResourceType::Stamina, 15.0f, 0.0f, 800.0f, 0.0f, 0.0f, TEXT("Multi-blink, 3 teleports, each leaves afterimage that deals 40 damage"));

    // Advanced
    AddSpell(EClass::Shadowblade, TEXT("Gloom Mastery"), ESpellTier::Advanced, ESpellShape::Self, EWeaponStatusEffect::Shadow, 0.0f, 35.0f, EResourceType::Stamina, 25.0f, 0.0f, 0.0f, 0.0f, 10.0f, TEXT("+50% shadow damage, 30% lifesteal, stealth on kill for 10s"));
    { FSpell& S = AddSpell(EClass::Shadowblade, TEXT("Gloom Shade"), ESpellTier::Advanced, ESpellShape::Summon, EWeaponStatusEffect::Shadow, 80.0f, 40.0f, EResourceType::Stamina, 20.0f, 0.0f, 0.0f, 6.0f, 10.0f, TEXT("Create shadow clone, mirrors your attacks for 10s, 80 damage each")); S.bLifesteal = true; S.LifestealPercent = 10.0f; }
    { FSpell& S = AddSpell(EClass::Shadowblade, TEXT("Executioner's Mark"), ESpellTier::Advanced, ESpellShape::SingleTarget, EWeaponStatusEffect::Shadow, 60.0f, 30.0f, EResourceType::Stamina, 18.0f, 0.0f, 600.0f, 0.0f, 8.0f, TEXT("Mark target, all attacks vs target crit, takes 40% more damage for 8s")); S.bAppliesDebuff = true; S.DebuffType = EWeaponStatusEffect::Shadow; S.DebuffDuration = 8.0f; }

    // Ultimate
    { FSpell& S = AddSpell(EClass::Shadowblade, TEXT("Eclipse of Blades"), ESpellTier::Ultimate, ESpellShape::AoE, EWeaponStatusEffect::Shadow, 450.0f, 100.0f, EResourceType::Aether, 180.0f, 1.0f, 800.0f, 10.0f, 5.0f, TEXT("Whirlwind of shadow blades, hit all in 10m every 0.5s for 5s")); S.bLifesteal = true; S.LifestealPercent = 30.0f; }

    // Weapon Arts
    { FSpell& S = AddSpell(EClass::Shadowblade, TEXT("Void Edge"), ESpellTier::WeaponArt, ESpellShape::SingleTarget, EWeaponStatusEffect::Shadow, 200.0f, 30.0f, EResourceType::Stamina, 12.0f, 0.0f, 400.0f, 0.0f, 0.0f, TEXT("Infuse blade with void, next 3 hits deal void damage and heal 20%")); S.bLifesteal = true; S.LifestealPercent = 20.0f; }
    AddSpell(EClass::Shadowblade, TEXT("Gloom Hew"), ESpellTier::WeaponArt, ESpellShape::SingleTarget, EWeaponStatusEffect::Shadow, 180.0f, 25.0f, EResourceType::Stamina, 8.0f, 0.0f, 500.0f, 0.0f, 0.0f, TEXT("Teleport to target and strike, guaranteed crit from stealth"));

    // Defensive
    AddSpell(EClass::Shadowblade, TEXT("Veil of Gloom"), ESpellTier::Defensive, ESpellShape::Self, EWeaponStatusEffect::Shadow, 0.0f, 20.0f, EResourceType::Stamina, 15.0f, 0.0f, 0.0f, 0.0f, 5.0f, TEXT("Invisible 5s, +100% crit on first attack from stealth"));
    { FSpell& S = AddSpell(EClass::Shadowblade, TEXT("Gloom Evasion"), ESpellTier::Defensive, ESpellShape::Self, EWeaponStatusEffect::Shadow, 0.0f, 15.0f, EResourceType::Stamina, 10.0f, 0.0f, 0.0f, 0.0f, 3.0f, TEXT("Triple dodge, +80% evasion for 3s, leaves 3 afterimages")); }
    { FSpell& S = AddSpell(EClass::Shadowblade, TEXT("Nullshell"), ESpellTier::Defensive, ESpellShape::Self, EWeaponStatusEffect::Shadow, 0.0f, 25.0f, EResourceType::Stamina, 18.0f, 0.0f, 0.0f, 0.0f, 4.0f, TEXT("Absorb 200, when broken, become invisible for 2s")); S.Healing = 200.0f; }

    // Utility
    { FSpell& S = AddSpell(EClass::Shadowblade, TEXT("Gloom Snare"), ESpellTier::Utility, ESpellShape::Ground, EWeaponStatusEffect::Shadow, 80.0f, 20.0f, EResourceType::Stamina, 15.0f, 0.0f, 400.0f, 4.0f, 10.0f, TEXT("Place shadow trap, triggers on enemy proximity, roots 3s + 80 damage")); S.bRoots = true; }
    AddSpell(EClass::Shadowblade, TEXT("Gloomstride"), ESpellTier::Utility, ESpellShape::Self, EWeaponStatusEffect::Shadow, 0.0f, 15.0f, EResourceType::Stamina, 8.0f, 0.0f, 1000.0f, 0.0f, 0.0f, TEXT("Teleport to any visible location, stealthed for 1s"));
    AddSpell(EClass::Shadowblade, TEXT("Hush Stride"), ESpellTier::Utility, ESpellShape::Self, EWeaponStatusEffect::Shadow, 0.0f, 10.0f, EResourceType::Stamina, 12.0f, 0.0f, 0.0f, 0.0f, 8.0f, TEXT("Silent movement, no aggro, +50% move speed, 8s or until attack"));

    // Ritual
    { FSpell& S = AddSpell(EClass::Shadowblade, TEXT("Thousand Shadows"), ESpellTier::Ritual, ESpellShape::AoE, EWeaponStatusEffect::Shadow, 300.0f, 100.0f, EResourceType::Aether, 300.0f, 2.0f, 800.0f, 15.0f, 8.0f, TEXT("1000 shadow slashes, all enemies in 15m take 40/s for 8s, you invisible")); S.bAppliesDoT = true; S.DoTDamage = 40.0f; S.DoTDuration = 8.0f; }
    { FSpell& S = AddSpell(EClass::Shadowblade, TEXT("Veil of Eclipse"), ESpellTier::Ritual, ESpellShape::Zone, EWeaponStatusEffect::Shadow, 250.0f, 100.0f, EResourceType::Aether, 300.0f, 2.5f, 1000.0f, 15.0f, 10.0f, TEXT("Eclipse zone, enemies blinded and silenced, 25/s for 10s")); S.bSilences = true; S.bAppliesDoT = true; S.DoTDamage = 25.0f; S.DoTDuration = 10.0f; }
}

// ============================================================================
// SPELLBLADE — Hybrid DPS — Lightning/Mana — 22 spells
// ============================================================================

void UEclipseSpellSystem::InitSpellbladeSpells()
{
    // Cantrips
    AddSpell(EClass::Spellblade, TEXT("Crystal Cleave"), ESpellTier::Cantrip, ESpellShape::SingleTarget, EWeaponStatusEffect::Lightning, 45.0f, 5.0f, EResourceType::Mana, 1.5f, 0.0f, 250.0f, 0.0f, 0.0f, TEXT("Weapon slash with arcane energy"));
    AddSpell(EClass::Spellblade, TEXT("Flux Blade"), ESpellTier::Cantrip, ESpellShape::SingleTarget, EWeaponStatusEffect::Lightning, 40.0f, 5.0f, EResourceType::Mana, 2.0f, 0.0f, 300.0f, 0.0f, 0.0f, TEXT("Strike that restores 5 Mana on hit"));

    // Basic
    AddSpell(EClass::Spellblade, TEXT("Wardspark Wall"), ESpellTier::Basic, ESpellShape::Self, EWeaponStatusEffect::Lightning, 0.0f, 30.0f, EResourceType::Mana, 12.0f, 0.0f, 0.0f, 0.0f, 6.0f, TEXT("Magical shield, absorb 300 damage, reflect 25% as arcane"));
    { FSpell& S = AddSpell(EClass::Spellblade, TEXT("Runic Hew"), ESpellTier::Basic, ESpellShape::SingleTarget, EWeaponStatusEffect::Lightning, 130.0f, 20.0f, EResourceType::Mana, 5.0f, 0.0f, 250.0f, 0.0f, 0.0f, TEXT("Infuse weapon with arcane, +100% damage, applies arcane mark")); S.bAppliesDebuff = true; S.DebuffType = EWeaponStatusEffect::Lightning; S.DebuffDuration = 5.0f; }
    AddSpell(EClass::Spellblade, TEXT("Spell Parry"), ESpellTier::Basic, ESpellShape::Self, EWeaponStatusEffect::Lightning, 80.0f, 15.0f, EResourceType::Stamina, 8.0f, 0.0f, 200.0f, 0.0f, 0.0f, TEXT("Parry spell projectile back at caster with 50% extra damage"));

    // Intermediate
    AddSpell(EClass::Spellblade, TEXT("Leyline Edge"), ESpellTier::Intermediate, ESpellShape::Line, EWeaponStatusEffect::Lightning, 120.0f, 25.0f, EResourceType::Mana, 8.0f, 0.0f, 500.0f, 0.0f, 0.0f, TEXT("Slash sends arcane wave, pierces all in line"));
    AddSpell(EClass::Spellblade, TEXT("Flux Sear"), ESpellTier::Intermediate, ESpellShape::SingleTarget, EWeaponStatusEffect::Lightning, 100.0f, 25.0f, EResourceType::Mana, 10.0f, 0.0f, 400.0f, 0.0f, 0.0f, TEXT("Strike drains 30 Mana from target, deals 100 + drained Mana as damage"));
    AddSpell(EClass::Spellblade, TEXT("Leyline Surge"), ESpellTier::Intermediate, ESpellShape::AoE, EWeaponStatusEffect::Lightning, 140.0f, 30.0f, EResourceType::Mana, 12.0f, 0.5f, 500.0f, 6.0f, 0.0f, TEXT("Weapon slam releases arcane burst, 6m AoE"));

    // Advanced
    AddSpell(EClass::Spellblade, TEXT("Flux Ward"), ESpellTier::Advanced, ESpellShape::Self, EWeaponStatusEffect::Lightning, 0.0f, 40.0f, EResourceType::Mana, 10.0f, 0.0f, 0.0f, 0.0f, 10.0f, TEXT("Convert Mana to barrier, 1 Mana = 5 shield, lasts 10s or depleted"));
    { FSpell& S = AddSpell(EClass::Spellblade, TEXT("Wardpiercer"), ESpellTier::Advanced, ESpellShape::SingleTarget, EWeaponStatusEffect::Lightning, 180.0f, 35.0f, EResourceType::Mana, 15.0f, 0.5f, 300.0f, 0.0f, 0.0f, TEXT("Strike that dispels all buffs on target, 2x damage vs buffed targets")); S.bSilences = true; }
    AddSpell(EClass::Spellblade, TEXT("Leyline Aspect"), ESpellTier::Advanced, ESpellShape::Self, EWeaponStatusEffect::Lightning, 0.0f, 40.0f, EResourceType::Mana, 25.0f, 0.0f, 0.0f, 0.0f, 10.0f, TEXT("Arcane empowerment, +50% spell and weapon damage, -50% mana cost for 10s"));

    // Ultimate
    { FSpell& S = AddSpell(EClass::Spellblade, TEXT("Aether Severance"), ESpellTier::Ultimate, ESpellShape::SingleTarget, EWeaponStatusEffect::Lightning, 400.0f, 100.0f, EResourceType::Aether, 180.0f, 0.5f, 600.0f, 5.0f, 0.0f, TEXT("Cut through reality, 5m guaranteed crit, sever all magical effects")); S.bStuns = true; }

    // Weapon Arts
    AddSpell(EClass::Spellblade, TEXT("Crystal Onslaught"), ESpellTier::WeaponArt, ESpellShape::Cone, EWeaponStatusEffect::Lightning, 170.0f, 30.0f, EResourceType::Mana, 10.0f, 0.0f, 400.0f, 5.0f, 0.0f, TEXT("5 rapid arcane slashes in cone, each restores 10 Mana"));
    AddSpell(EClass::Spellblade, TEXT("Mana Blade"), ESpellTier::WeaponArt, ESpellShape::SingleTarget, EWeaponStatusEffect::Lightning, 160.0f, 35.0f, EResourceType::Mana, 12.0f, 0.0f, 300.0f, 0.0f, 8.0f, TEXT("Pure mana blade, no physical damage, 100% arcane, ignores armor for 8s"));

    // Defensive
    AddSpell(EClass::Spellblade, TEXT("Wardspark Mantle"), ESpellTier::Defensive, ESpellShape::Self, EWeaponStatusEffect::Lightning, 0.0f, 25.0f, EResourceType::Mana, 15.0f, 0.0f, 0.0f, 0.0f, 6.0f, TEXT("Arcane ward, +40% magic resist, reflect 30% spells for 6s"));
    AddSpell(EClass::Spellblade, TEXT("Wardslayer Guard"), ESpellTier::Defensive, ESpellShape::Self, EWeaponStatusEffect::Lightning, 0.0f, 20.0f, EResourceType::Mana, 12.0f, 0.0f, 0.0f, 0.0f, 5.0f, TEXT("Absorb next spell completely, 5s duration"));
    AddSpell(EClass::Spellblade, TEXT("Flux Bulwark"), ESpellTier::Defensive, ESpellShape::Aura, EWeaponStatusEffect::Lightning, 0.0f, 30.0f, EResourceType::Mana, 20.0f, 0.0f, 0.0f, 6.0f, 8.0f, TEXT("Allies in 6m gain 10% magic resist and 5 Mana/s for 8s"));

    // Utility
    AddSpell(EClass::Spellblade, TEXT("Blink Strike"), ESpellTier::Utility, ESpellShape::SingleTarget, EWeaponStatusEffect::Lightning, 80.0f, 20.0f, EResourceType::Mana, 10.0f, 0.0f, 600.0f, 0.0f, 0.0f, TEXT("Teleport to enemy and strike, +50% damage on arrival"));
    AddSpell(EClass::Spellblade, TEXT("Aether Tap"), ESpellTier::Utility, ESpellShape::SingleTarget, EWeaponStatusEffect::Lightning, 50.0f, 15.0f, EResourceType::Mana, 12.0f, 0.0f, 500.0f, 0.0f, 0.0f, TEXT("Drain 50 Mana from enemy, restore to self"));
    AddSpell(EClass::Spellblade, TEXT("Aetheric Imbuement"), ESpellTier::Utility, ESpellShape::Party, EWeaponStatusEffect::Lightning, 0.0f, 30.0f, EResourceType::Mana, 18.0f, 0.0f, 500.0f, 8.0f, 8.0f, TEXT("All allies in 8m gain +20% spell damage and 20 Mana for 8s"));

    // Ritual
    { FSpell& S = AddSpell(EClass::Spellblade, TEXT("Aether Storm"), ESpellTier::Ritual, ESpellShape::Zone, EWeaponStatusEffect::Lightning, 280.0f, 100.0f, EResourceType::Aether, 300.0f, 3.0f, 1200.0f, 15.0f, 10.0f, TEXT("15m arcane storm, 28/s to enemies, allies gain 20 Mana/s")); S.bStuns = true; S.bAppliesDoT = true; S.DoTDamage = 28.0f; S.DoTDuration = 10.0f; }
    { FSpell& S = AddSpell(EClass::Spellblade, TEXT("Reality Cut"), ESpellTier::Ritual, ESpellShape::Line, EWeaponStatusEffect::Lightning, 350.0f, 100.0f, EResourceType::Aether, 300.0f, 2.5f, 1500.0f, 0.0f, 0.0f, TEXT("Cut through reality, 1500 range line, severs all buffs, 350 arcane")); S.bSilences = true; }
}

// ============================================================================
// WARTHANE — Tank/Protector — Earth/Rage — 22 spells
// ============================================================================

void UEclipseSpellSystem::InitWarthaneSpells()
{
    // Cantrips
    AddSpell(EClass::Warthane, TEXT("Battlehowl"), ESpellTier::Cantrip, ESpellShape::Self, EWeaponStatusEffect::Earth, 30.0f, 5.0f, EResourceType::Rage, 2.0f, 0.0f, 300.0f, 0.0f, 0.0f, TEXT("Brief war cry, +10% damage to nearby allies for 3s"));
    AddSpell(EClass::Warthane, TEXT("Pommel Blow"), ESpellTier::Cantrip, ESpellShape::SingleTarget, EWeaponStatusEffect::Earth, 45.0f, 5.0f, EResourceType::Stamina, 2.5f, 0.0f, 200.0f, 0.0f, 0.0f, TEXT("Hilt strike, brief stagger"));

    // Basic
    AddSpell(EClass::Warthane, TEXT("Warhost Pennant"), ESpellTier::Basic, ESpellShape::Ground, EWeaponStatusEffect::Earth, 0.0f, 40.0f, EResourceType::Rage, 20.0f, 0.0f, 0.0f, 15.0f, 10.0f, TEXT("Plant war banner, allies in 15m +20% damage, +15% defense for 10s"));
    { FSpell& S = AddSpell(EClass::Warthane, TEXT("Commanding Shout"), ESpellTier::Basic, ESpellShape::Cone, EWeaponStatusEffect::Earth, 100.0f, 25.0f, EResourceType::Stamina, 15.0f, 0.0f, 800.0f, 0.0f, 0.0f, TEXT("Fear all in 10m for 3s, knockback 5m, high threat")); S.bStuns = true; S.bKnockbacks = true; }
    AddSpell(EClass::Warthane, TEXT("Warspite Slam"), ESpellTier::Basic, ESpellShape::SingleTarget, EWeaponStatusEffect::Earth, 90.0f, 18.0f, EResourceType::Stamina, 6.0f, 0.0f, 200.0f, 0.0f, 0.0f, TEXT("Shield bash, stuns 1s, generates high threat"));

    // Intermediate
    AddSpell(EClass::Warthane, TEXT("Battlehorn Resound"), ESpellTier::Intermediate, ESpellShape::Party, EWeaponStatusEffect::Earth, 0.0f, 50.0f, EResourceType::Rage, 25.0f, 0.0f, 0.0f, 10.0f, 8.0f, TEXT("All allies heal 50, gain 30 Flow, immune to fear for 8s"));
    { FSpell& S = AddSpell(EClass::Warthane, TEXT("Phalanx Wall"), ESpellTier::Intermediate, ESpellShape::Wall, EWeaponStatusEffect::Earth, 80.0f, 35.0f, EResourceType::Stamina, 15.0f, 0.5f, 500.0f, 8.0f, 8.0f, TEXT("Create shield wall, blocks enemies, allies behind take -30% damage")); S.bKnockbacks = true; }
    { FSpell& S = AddSpell(EClass::Warthane, TEXT("Warthane's Presence"), ESpellTier::Intermediate, ESpellShape::Aura, EWeaponStatusEffect::Earth, 50.0f, 30.0f, EResourceType::Rage, 18.0f, 0.0f, 0.0f, 10.0f, 6.0f, TEXT("Commanding aura, enemies in 10m -20% damage, forced to attack you for 6s")); S.bStuns = true; }

    // Advanced
    AddSpell(EClass::Warthane, TEXT("Unyielding Stance"), ESpellTier::Advanced, ESpellShape::Self, EWeaponStatusEffect::Earth, 0.0f, 40.0f, EResourceType::Rage, 25.0f, 0.0f, 0.0f, 0.0f, 10.0f, TEXT("Immune to CC, +60% defense, +100% threat generation for 10s"));
    { FSpell& S = AddSpell(EClass::Warthane, TEXT("Shield Breaker"), ESpellTier::Advanced, ESpellShape::SingleTarget, EWeaponStatusEffect::Earth, 180.0f, 25.0f, EResourceType::Stamina, 8.0f, 0.0f, 300.0f, 0.0f, 0.0f, TEXT("Overhead smash, break guard/poise instantly, guaranteed stagger")); S.bStuns = true; }
    AddSpell(EClass::Warthane, TEXT("Battle Plan"), ESpellTier::Advanced, ESpellShape::Party, EWeaponStatusEffect::Earth, 0.0f, 45.0f, EResourceType::Rage, 22.0f, 0.0f, 500.0f, 12.0f, 10.0f, TEXT("All allies +30% damage, +20% defense, +20% move speed for 10s"));

    // Ultimate
    { FSpell& S = AddSpell(EClass::Warthane, TEXT("Bannerlord Decree"), ESpellTier::Ultimate, ESpellShape::AoE, EWeaponStatusEffect::Earth, 300.0f, 100.0f, EResourceType::Aether, 180.0f, 1.0f, 1500.0f, 15.0f, 15.0f, TEXT("War banner slam, 15m AoE stun, all allies +50% damage for 15s")); S.bStuns = true; S.bKnockbacks = true; }

    // Weapon Arts
    { FSpell& S = AddSpell(EClass::Warthane, TEXT("Shield Breaker Art"), ESpellTier::WeaponArt, ESpellShape::SingleTarget, EWeaponStatusEffect::Earth, 180.0f, 25.0f, EResourceType::Stamina, 8.0f, 0.0f, 300.0f, 0.0f, 0.0f, TEXT("Overhead smash, break guard, guaranteed stagger, 2x vs blocking")); S.bStuns = true; }
    AddSpell(EClass::Warthane, TEXT("War Banner Throw"), ESpellTier::WeaponArt, ESpellShape::Projectile, EWeaponStatusEffect::Earth, 150.0f, 30.0f, EResourceType::Rage, 12.0f, 0.0f, 800.0f, 0.0f, 0.0f, TEXT("Throw war banner like spear, pierces, plants on landing, 8s aura"));

    // Defensive
    AddSpell(EClass::Warthane, TEXT("Bastion Formation"), ESpellTier::Defensive, ESpellShape::Self, EWeaponStatusEffect::Earth, 0.0f, 30.0f, EResourceType::Stamina, 20.0f, 0.0f, 0.0f, 0.0f, 6.0f, TEXT("Raise shield wall, -60% frontal damage, immune to knockback for 6s"));
    AddSpell(EClass::Warthane, TEXT("Bulwark stance"), ESpellTier::Defensive, ESpellShape::Aura, EWeaponStatusEffect::Earth, 0.0f, 35.0f, EResourceType::Rage, 18.0f, 0.0f, 0.0f, 8.0f, 8.0f, TEXT("Allies in 8m +40% defense, immune to fear for 8s"));
    AddSpell(EClass::Warthane, TEXT("Final Breach"), ESpellTier::Defensive, ESpellShape::Self, EWeaponStatusEffect::Earth, 0.0f, 40.0f, EResourceType::Rage, 30.0f, 0.0f, 0.0f, 0.0f, 5.0f, TEXT("Below 30% HP, immune to death for 5s, +100% damage, heal 200 on expire"));

    // Utility
    AddSpell(EClass::Warthane, TEXT("Battering Rush"), ESpellTier::Utility, ESpellShape::Line, EWeaponStatusEffect::Earth, 100.0f, 20.0f, EResourceType::Stamina, 10.0f, 0.0f, 600.0f, 0.0f, 0.0f, TEXT("Charge forward, knockback all in path, taunt on hit"));
    AddSpell(EClass::Warthane, TEXT("Hearten"), ESpellTier::Utility, ESpellShape::SingleTarget, EWeaponStatusEffect::Earth, 0.0f, 20.0f, EResourceType::Rage, 12.0f, 0.0f, 400.0f, 0.0f, 8.0f, TEXT("Inspire ally, +30% damage, restore 20 Rage/Mana/Stamina for 8s"));
    AddSpell(EClass::Warthane, TEXT("Taunt"), ESpellTier::Utility, ESpellShape::AoE, EWeaponStatusEffect::Earth, 30.0f, 15.0f, EResourceType::Rage, 10.0f, 0.0f, 0.0f, 12.0f, 4.0f, TEXT("Force all enemies in 12m to target you for 4s, +200% threat"));

    // Ritual
    { FSpell& S = AddSpell(EClass::Warthane, TEXT("Warthane's Decree"), ESpellTier::Ritual, ESpellShape::AoE, EWeaponStatusEffect::Earth, 300.0f, 100.0f, EResourceType::Aether, 300.0f, 2.5f, 1200.0f, 20.0f, 15.0f, TEXT("20m battlefield, all allies +50% damage, +30% defense, immune to fear, 15s")); S.bStuns = true; S.Healing = 200.0f; }
    { FSpell& S = AddSpell(EClass::Warthane, TEXT("Last Bastion"), ESpellTier::Ritual, ESpellShape::Wall, EWeaponStatusEffect::Earth, 200.0f, 100.0f, EResourceType::Aether, 300.0f, 3.0f, 800.0f, 20.0f, 15.0f, TEXT("Summon massive fortress wall, blocks all enemies, allies behind fully immune for 15s")); S.bKnockbacks = true; }
}

// ============================================================================
// NECROTURGE — Summoner/Caster — Soul/Mana — 22 spells
// ============================================================================

void UEclipseSpellSystem::InitNecroturgeSpells()
{
    // Cantrips
    AddSpell(EClass::Necroturge, TEXT("Splintered Ossein"), ESpellTier::Cantrip, ESpellShape::Projectile, EWeaponStatusEffect::Soul, 40.0f, 5.0f, EResourceType::Mana, 1.5f, 0.0f, 600.0f, 0.0f, 0.0f, TEXT("Fire bone shard, +50% vs living targets"));
    AddSpell(EClass::Necroturge, TEXT("Tenebrous Pulse"), ESpellTier::Cantrip, ESpellShape::Self, EWeaponStatusEffect::Soul, 35.0f, 5.0f, EResourceType::Mana, 2.0f, 0.0f, 300.0f, 4.0f, 0.0f, TEXT("Necrotic pulse, 4m AoE, applies decay"));

    // Basic
    AddSpell(EClass::Necroturge, TEXT("Exhume Warrior"), ESpellTier::Basic, ESpellShape::Summon, EWeaponStatusEffect::Soul, 40.0f, 20.0f, EResourceType::Mana, 8.0f, 0.5f, 500.0f, 0.0f, 30.0f, TEXT("Summon skeleton warrior, lasts 30s, taunts and deals melee damage"));
    AddSpell(EClass::Necroturge, TEXT("Ossuary Lance"), ESpellTier::Basic, ESpellShape::Line, EWeaponStatusEffect::Soul, 120.0f, 15.0f, EResourceType::Mana, 5.0f, 0.0f, 800.0f, 0.0f, 0.0f, TEXT("Fire bone projectile, pierces enemies, +50% vs living"));
    { FSpell& S = AddSpell(EClass::Necroturge, TEXT("Rotting Blight"), ESpellTier::Basic, ESpellShape::SingleTarget, EWeaponStatusEffect::Soul, 60.0f, 18.0f, EResourceType::Mana, 8.0f, 0.0f, 600.0f, 0.0f, 8.0f, TEXT("Inflict decay, 15/s for 8s, -30% healing to target")); S.bAppliesDoT = true; S.DoTDamage = 15.0f; S.DoTDuration = 8.0f; S.bAppliesDebuff = true; S.DebuffType = EWeaponStatusEffect::Soul; S.DebuffDuration = 8.0f; }

    // Intermediate
    { FSpell& S = AddSpell(EClass::Necroturge, TEXT("Necrotic Pulse"), ESpellTier::Intermediate, ESpellShape::AoE, EWeaponStatusEffect::Soul, 180.0f, 35.0f, EResourceType::Mana, 12.0f, 0.8f, 600.0f, 8.0f, 0.0f, TEXT("Explosion of necrotic energy, 8m AoE, applies decay")); S.bAppliesDoT = true; S.DoTDamage = 20.0f; S.DoTDuration = 5.0f; S.bAppliesDebuff = true; S.DebuffType = EWeaponStatusEffect::Soul; S.DebuffDuration = 5.0f; }
    AddSpell(EClass::Necroturge, TEXT("Exhume Archer"), ESpellTier::Intermediate, ESpellShape::Summon, EWeaponStatusEffect::Soul, 30.0f, 25.0f, EResourceType::Mana, 10.0f, 0.5f, 600.0f, 0.0f, 25.0f, TEXT("Summon skeleton archer, ranged attacks, 25 damage per shot, lasts 25s"));
    { FSpell& S = AddSpell(EClass::Necroturge, TEXT("Eidolon Siphon"), ESpellTier::Intermediate, ESpellShape::Chain, EWeaponStatusEffect::Soul, 100.0f, 28.0f, EResourceType::Mana, 12.0f, 0.0f, 600.0f, 0.0f, 0.0f, TEXT("Drain souls, chains to 3 enemies, heal 30% of damage")); S.bLifesteal = true; S.LifestealPercent = 30.0f; }

    // Advanced
    { FSpell& S = AddSpell(EClass::Necroturge, TEXT("Hexed Ossein"), ESpellTier::Advanced, ESpellShape::SingleTarget, EWeaponStatusEffect::Soul, 100.0f, 35.0f, EResourceType::Mana, 15.0f, 0.0f, 600.0f, 0.0f, 5.0f, TEXT("Cursed bone strike, target takes 30% more damage for 5s")); S.bAppliesDebuff = true; S.DebuffType = EWeaponStatusEffect::Soul; S.DebuffDuration = 5.0f; }
    AddSpell(EClass::Necroturge, TEXT("Carrion Burst"), ESpellTier::Advanced, ESpellShape::AoE, EWeaponStatusEffect::Soul, 200.0f, 30.0f, EResourceType::Mana, 15.0f, 0.5f, 500.0f, 8.0f, 0.0f, TEXT("Detonate nearby corpse, 8m AoE 200 damage, applies decay"));
    AddSpell(EClass::Necroturge, TEXT("Ossuary Cell"), ESpellTier::Advanced, ESpellShape::Ground, EWeaponStatusEffect::Soul, 80.0f, 40.0f, EResourceType::Mana, 18.0f, 0.8f, 500.0f, 6.0f, 6.0f, TEXT("Rise bones around target, root 6s, 15/s damage"));

    // Ultimate
    { FSpell& S = AddSpell(EClass::Necroturge, TEXT("Lich Ascension"), ESpellTier::Ultimate, ESpellShape::Self, EWeaponStatusEffect::Soul, 250.0f, 100.0f, EResourceType::Aether, 180.0f, 2.0f, 1200.0f, 0.0f, 10.0f, TEXT("Transform into Lich, +200% spell damage, raise 5 skeletons, immune to death for 10s")); S.bLifesteal = true; S.LifestealPercent = 30.0f; }

    // Weapon Arts
    { FSpell& S = AddSpell(EClass::Necroturge, TEXT("Cursed Bone Staff"), ESpellTier::WeaponArt, ESpellShape::SingleTarget, EWeaponStatusEffect::Soul, 100.0f, 25.0f, EResourceType::Mana, 10.0f, 0.0f, 600.0f, 0.0f, 5.0f, TEXT("Staff strike, target takes 30% more damage from all sources for 5s")); S.bAppliesDebuff = true; S.DebuffType = EWeaponStatusEffect::Soul; S.DebuffDuration = 5.0f; }
    AddSpell(EClass::Necroturge, TEXT("Wraithbone Stave"), ESpellTier::WeaponArt, ESpellShape::AoE, EWeaponStatusEffect::Soul, 150.0f, 30.0f, EResourceType::Mana, 12.0f, 0.5f, 600.0f, 6.0f, 0.0f, TEXT("Staff slam, 6m soul burst, raises 2 skeletons from corpses"));

    // Defensive
    { FSpell& S = AddSpell(EClass::Necroturge, TEXT("Ossuary Platemail"), ESpellTier::Defensive, ESpellShape::Self, EWeaponStatusEffect::Soul, 0.0f, 25.0f, EResourceType::Mana, 15.0f, 0.0f, 0.0f, 0.0f, 8.0f, TEXT("Bone armor, absorb 200, reflect 30% as necrotic")); S.Healing = 200.0f; }
    AddSpell(EClass::Necroturge, TEXT("Warden of Hollows"), ESpellTier::Defensive, ESpellShape::Summon, EWeaponStatusEffect::Soul, 0.0f, 30.0f, EResourceType::Mana, 20.0f, 0.5f, 300.0f, 3.0f, 15.0f, TEXT("Summon bone guardian, 500 HP, taunts all enemies for 15s"));
    AddSpell(EClass::Necroturge, TEXT("Eidolon Ward"), ESpellTier::Defensive, ESpellShape::Self, EWeaponStatusEffect::Soul, 0.0f, 35.0f, EResourceType::Mana, 25.0f, 0.0f, 0.0f, 0.0f, 6.0f, TEXT("Absorb 300, while active spells cost 50% less Mana"));

    // Utility
    AddSpell(EClass::Necroturge, TEXT("Exhume Adept"), ESpellTier::Utility, ESpellShape::Summon, EWeaponStatusEffect::Soul, 20.0f, 35.0f, EResourceType::Mana, 20.0f, 0.5f, 500.0f, 0.0f, 20.0f, TEXT("Summon skeleton mage, casts 50 damage spells, lasts 20s"));
    { FSpell& S = AddSpell(EClass::Necroturge, TEXT("Dread Gaze"), ESpellTier::Utility, ESpellShape::AoE, EWeaponStatusEffect::Soul, 50.0f, 20.0f, EResourceType::Mana, 15.0f, 0.0f, 500.0f, 8.0f, 3.0f, TEXT("Fear all enemies in 8m for 3s, they flee and take +20% damage")); S.bAppliesDebuff = true; S.DebuffType = EWeaponStatusEffect::Soul; S.DebuffDuration = 3.0f; }
    AddSpell(EClass::Necroturge, TEXT("Disinter"), ESpellTier::Utility, ESpellShape::Ground, EWeaponStatusEffect::Soul, 60.0f, 25.0f, EResourceType::Mana, 18.0f, 0.5f, 400.0f, 6.0f, 10.0f, TEXT("Unearth corpses in 6m, 60 damage + slow all enemies by 40% for 10s"));

    // Ritual
    { FSpell& S = AddSpell(EClass::Necroturge, TEXT("Legion of Hollows"), ESpellTier::Ritual, ESpellShape::Summon, EWeaponStatusEffect::Soul, 200.0f, 100.0f, EResourceType::Aether, 300.0f, 3.0f, 800.0f, 15.0f, 30.0f, TEXT("Raise 10 skeletons + 2 bone knights, army fights for 30s, 40 damage each")); }
    { FSpell& S = AddSpell(EClass::Necroturge, TEXT("Plague of Death"), ESpellTier::Ritual, ESpellShape::Zone, EWeaponStatusEffect::Soul, 280.0f, 100.0f, EResourceType::Aether, 300.0f, 3.0f, 1200.0f, 20.0f, 12.0f, TEXT("20m plague zone, 30/s decay, -50% healing, spreads on kill")); S.bAppliesDoT = true; S.DoTDamage = 30.0f; S.DoTDuration = 12.0f; S.bAppliesDebuff = true; S.DebuffType = EWeaponStatusEffect::Soul; S.DebuffDuration = 12.0f; }
}

// ============================================================================
// EARTHWEAVER — Hybrid/Shapeshifter — Water/Mana — 22 spells
// ============================================================================

void UEclipseSpellSystem::InitEarthweaverSpells()
{
    // Cantrips
    { FSpell& S = AddSpell(EClass::Earthweaver, TEXT("Briarlash"), ESpellTier::Cantrip, ESpellShape::SingleTarget, EWeaponStatusEffect::Earth, 50.0f, 5.0f, EResourceType::Mana, 2.0f, 0.0f, 400.0f, 0.0f, 0.0f, TEXT("Thorn whip, pull enemy 5m, apply 3s bleed")); S.bAppliesDoT = true; S.DoTDamage = 8.0f; S.DoTDuration = 3.0f; }
    AddSpell(EClass::Earthweaver, TEXT("Lunar Ember"), ESpellTier::Cantrip, ESpellShape::Projectile, EWeaponStatusEffect::Water, 45.0f, 5.0f, EResourceType::Mana, 2.0f, 0.0f, 600.0f, 0.0f, 0.0f, TEXT("Lunar energy bolt, +50% vs undead"));

    // Basic
    AddSpell(EClass::Earthweaver, TEXT("Ursine Aspect"), ESpellTier::Basic, ESpellShape::Self, EWeaponStatusEffect::Earth, 0.0f, 40.0f, EResourceType::Mana, 15.0f, 0.5f, 0.0f, 0.0f, 15.0f, TEXT("Transform into bear, +100% HP, +50% armor, taunt all for 15s"));
    { FSpell& S = AddSpell(EClass::Earthweaver, TEXT("Bloom Renewal"), ESpellTier::Basic, ESpellShape::SingleTarget, EWeaponStatusEffect::Water, 0.0f, 25.0f, EResourceType::Mana, 8.0f, 0.0f, 600.0f, 0.0f, 10.0f, TEXT("Heal 60 instantly + 10/s for 10s, cleanses poison")); S.Healing = 160.0f; }
    AddSpell(EClass::Earthweaver, TEXT("Rootsnare"), ESpellTier::Basic, ESpellShape::Ground, EWeaponStatusEffect::Earth, 60.0f, 18.0f, EResourceType::Mana, 8.0f, 0.0f, 500.0f, 4.0f, 4.0f, TEXT("Roots in 4m, root all for 4s, 15/s"));

    // Intermediate
    AddSpell(EClass::Earthweaver, TEXT("Galvanic Aspect"), ESpellTier::Intermediate, ESpellShape::Self, EWeaponStatusEffect::Lightning, 150.0f, 35.0f, EResourceType::Mana, 20.0f, 0.0f, 0.0f, 0.0f, 8.0f, TEXT("Transform into lightning wolf, +80% speed, lightning damage on attacks for 8s"));
    { FSpell& S = AddSpell(EClass::Earthweaver, TEXT("Petalwell"), ESpellTier::Intermediate, ESpellShape::Ground, EWeaponStatusEffect::Water, 0.0f, 30.0f, EResourceType::Mana, 12.0f, 0.5f, 500.0f, 6.0f, 8.0f, TEXT("Create healing bloom, allies in 6m heal 15/s for 8s")); S.Healing = 120.0f; }
    { FSpell& S = AddSpell(EClass::Earthweaver, TEXT("Tempest Cry"), ESpellTier::Intermediate, ESpellShape::AoE, EWeaponStatusEffect::Lightning, 140.0f, 35.0f, EResourceType::Mana, 15.0f, 0.8f, 800.0f, 8.0f, 0.0f, TEXT("Call lightning storm, 8m AoE, chains to 3 enemies")); S.bStuns = true; }

    // Advanced
    AddSpell(EClass::Earthweaver, TEXT("Heartwood Aspect"), ESpellTier::Advanced, ESpellShape::Self, EWeaponStatusEffect::Earth, 200.0f, 45.0f, EResourceType::Mana, 25.0f, 0.5f, 0.0f, 0.0f, 12.0f, TEXT("Transform into treant, +200% HP, +100% armor, thorns reflect 50, 12s"));
    { FSpell& S = AddSpell(EClass::Earthweaver, TEXT("Wildgrove Fury"), ESpellTier::Advanced, ESpellShape::AoE, EWeaponStatusEffect::Earth, 220.0f, 40.0f, EResourceType::Mana, 18.0f, 1.0f, 1000.0f, 12.0f, 0.0f, TEXT("Massive nature eruption, 12m AoE, roots + thorns + lightning")); S.bRoots = true; S.bStuns = true; }
    AddSpell(EClass::Earthweaver, TEXT("Stellar Aspect"), ESpellTier::Advanced, ESpellShape::Self, EWeaponStatusEffect::Water, 0.0f, 40.0f, EResourceType::Mana, 22.0f, 0.0f, 0.0f, 0.0f, 10.0f, TEXT("Moonkin form, +80% spell damage, +50% Mana regen, ranged attacks for 10s"));

    // Ultimate
    { FSpell& S = AddSpell(EClass::Earthweaver, TEXT("Nature's Wrath Ultimate"), ESpellTier::Ultimate, ESpellShape::Zone, EWeaponStatusEffect::Earth, 350.0f, 100.0f, EResourceType::Aether, 180.0f, 1.5f, 1500.0f, 20.0f, 0.0f, TEXT("Massive nature eruption, 20m, roots + thorns + lightning, heal allies 200")); S.bRoots = true; S.bStuns = true; S.Healing = 200.0f; }

    // Weapon Arts
    { FSpell& S = AddSpell(EClass::Earthweaver, TEXT("Thorn Whip Art"), ESpellTier::WeaponArt, ESpellShape::SingleTarget, EWeaponStatusEffect::Earth, 80.0f, 15.0f, EResourceType::Mana, 6.0f, 0.0f, 400.0f, 0.0f, 5.0f, TEXT("Enhanced thorn whip, pull 5m, bleed 5s, +50% vs rooted targets")); S.bRoots = true; S.bAppliesDoT = true; S.DoTDamage = 15.0f; S.DoTDuration = 5.0f; }
    AddSpell(EClass::Earthweaver, TEXT("Feral Talons"), ESpellTier::WeaponArt, ESpellShape::Cone, EWeaponStatusEffect::Earth, 160.0f, 30.0f, EResourceType::Mana, 10.0f, 0.0f, 300.0f, 5.0f, 0.0f, TEXT("Beast form claw swipe, 5m cone, 3 hits, each applies bleed"));

    // Defensive
    { FSpell& S = AddSpell(EClass::Earthweaver, TEXT("Ironbark Mantle"), ESpellTier::Defensive, ESpellShape::SingleTarget, EWeaponStatusEffect::Earth, 0.0f, 25.0f, EResourceType::Mana, 15.0f, 0.0f, 400.0f, 0.0f, 8.0f, TEXT("Barkskin on ally, +60% armor, reflect 20 damage for 8s")); S.Healing = 50.0f; }
    { FSpell& S = AddSpell(EClass::Earthweaver, TEXT("Verdant Renewal"), ESpellTier::Defensive, ESpellShape::SingleTarget, EWeaponStatusEffect::Water, 0.0f, 30.0f, EResourceType::Mana, 12.0f, 0.5f, 600.0f, 0.0f, 8.0f, TEXT("Heal target 100 instantly + 15/s for 8s")); S.Healing = 220.0f; }
    AddSpell(EClass::Earthweaver, TEXT("Granite Aspect"), ESpellTier::Defensive, ESpellShape::Self, EWeaponStatusEffect::Earth, 0.0f, 35.0f, EResourceType::Mana, 20.0f, 0.0f, 0.0f, 0.0f, 6.0f, TEXT("Turn to stone, +100% armor, immune to CC, -50% move for 6s"));

    // Utility
    AddSpell(EClass::Earthweaver, TEXT("Wisp Flame"), ESpellTier::Utility, ESpellShape::AoE, EWeaponStatusEffect::Water, 50.0f, 20.0f, EResourceType::Mana, 12.0f, 0.0f, 500.0f, 8.0f, 8.0f, TEXT("Faerie fire in 8m, all enemies revealed, -20% dodge, +30% damage taken for 8s"));
    AddSpell(EClass::Earthweaver, TEXT("Feline Surge"), ESpellTier::Utility, ESpellShape::Self, EWeaponStatusEffect::Earth, 0.0f, 20.0f, EResourceType::Mana, 10.0f, 0.0f, 600.0f, 0.0f, 5.0f, TEXT("Cat form, +100% move speed, stealth, +50% crit for 5s or until attack"));
    AddSpell(EClass::Earthweaver, TEXT("Restorative Bloom"), ESpellTier::Utility, ESpellShape::SingleTarget, EWeaponStatusEffect::Water, 0.0f, 15.0f, EResourceType::Mana, 8.0f, 0.0f, 400.0f, 0.0f, 0.0f, TEXT("Cure poison, disease, and all DoTs from target, heal 30"));

    // Ritual
    { FSpell& S = AddSpell(EClass::Earthweaver, TEXT("Groveheart Rousing"), ESpellTier::Ritual, ESpellShape::Zone, EWeaponStatusEffect::Earth, 300.0f, 100.0f, EResourceType::Aether, 300.0f, 3.0f, 1200.0f, 20.0f, 15.0f, TEXT("Sacred grove, allies heal 25/s, enemies rooted + 25/s for 15s")); S.bRoots = true; S.bAppliesDoT = true; S.DoTDamage = 25.0f; S.DoTDuration = 15.0f; S.Healing = 375.0f; }
    { FSpell& S = AddSpell(EClass::Earthweaver, TEXT("Groveheart Manifest"), ESpellTier::Ritual, ESpellShape::Summon, EWeaponStatusEffect::Earth, 250.0f, 100.0f, EResourceType::Aether, 300.0f, 3.0f, 600.0f, 12.0f, 30.0f, TEXT("Summon 3 ancient treants, 500 HP each, 60 damage per hit, last 30s")); S.bRoots = true; }
}

// ============================================================================
// DUAL AFFINITY SPELLS — 45 combinations across all 10 elements
// ============================================================================

void UEclipseSpellSystem::InitDualAffinitySpells()
{
    // === Fire combinations (9) ===
    AddDualAffinitySpell(EClass::Alchemist, TEXT("Thermal Shock"), ESpellShape::AoE, EWeaponStatusEffect::Fire, EWeaponStatusEffect::Ice, 200.0f, 40.0f, EResourceType::Reagents, 15.0f, 0.5f, 600.0f, 8.0f, TEXT("Steam burst from extreme temperature shift, 200 AoE + burn + freeze"), false, false, true, 25.0f, 4.0f);
    AddDualAffinitySpell(EClass::Stormcaller, TEXT("Plasma Storm"), ESpellShape::AoE, EWeaponStatusEffect::Fire, EWeaponStatusEffect::Lightning, 250.0f, 45.0f, EResourceType::Mana, 20.0f, 1.0f, 800.0f, 10.0f, TEXT("Ionized firestorm, 250 AoE, stun + burn + chain lightning"), true, false, true, 30.0f, 5.0f);
    AddDualAffinitySpell(EClass::Shadowblade, TEXT("Umbral Inferno"), ESpellShape::AoE, EWeaponStatusEffect::Fire, EWeaponStatusEffect::Shadow, 220.0f, 40.0f, EResourceType::Stamina, 18.0f, 0.5f, 600.0f, 8.0f, TEXT("Shadow flames, 220 AoE, burn + life steal + ignores armor"), false, false, true, 25.0f, 5.0f);
    AddDualAffinitySpell(EClass::Templar, TEXT("Sacred Flame"), ESpellShape::AoE, EWeaponStatusEffect::Fire, EWeaponStatusEffect::Holy, 200.0f, 40.0f, EResourceType::Mana, 15.0f, 0.5f, 600.0f, 8.0f, TEXT("Holy fire, 200 AoE, burns enemies, heals allies in radius, +100% vs undead"), false, false, true, 20.0f, 5.0f);
    AddDualAffinitySpell(EClass::Alchemist, TEXT("Caustic Blaze"), ESpellShape::Ground, EWeaponStatusEffect::Fire, EWeaponStatusEffect::Poison, 180.0f, 35.0f, EResourceType::Reagents, 15.0f, 0.5f, 600.0f, 6.0f, TEXT("Burning acid pool, 180 burst + 20/s burn + 15/s poison for 6s"), false, false, true, 20.0f, 6.0f);
    AddDualAffinitySpell(EClass::Warthane, TEXT("Volcanic Eruption"), ESpellShape::AoE, EWeaponStatusEffect::Fire, EWeaponStatusEffect::Earth, 280.0f, 45.0f, EResourceType::Rage, 20.0f, 1.0f, 800.0f, 10.0f, TEXT("Volcanic eruption, 280 AoE, knockback + burn + armor break"), false, true, true, 30.0f, 6.0f);
    AddDualAffinitySpell(EClass::Stormcaller, TEXT("Steam Vent"), ESpellShape::Zone, EWeaponStatusEffect::Fire, EWeaponStatusEffect::Water, 160.0f, 35.0f, EResourceType::Mana, 15.0f, 0.5f, 600.0f, 8.0f, TEXT("Superheated steam vent, 160 burst + 20/s burn + slow for 6s"), false, false, true, 20.0f, 6.0f);
    AddDualAffinitySpell(EClass::Stormcaller, TEXT("Inferno Maelstrom"), ESpellShape::AoE, EWeaponStatusEffect::Fire, EWeaponStatusEffect::Wind, 240.0f, 45.0f, EResourceType::Mana, 18.0f, 1.0f, 1000.0f, 12.0f, TEXT("Wind-fueled firestorm, 240 AoE, knockback + burn + spread"), false, true, true, 30.0f, 5.0f);
    AddDualAffinitySpell(EClass::Dreadknight, TEXT("Soulfire Combustion"), ESpellShape::SingleTarget, EWeaponStatusEffect::Fire, EWeaponStatusEffect::Soul, 260.0f, 40.0f, EResourceType::Rage, 15.0f, 0.5f, 500.0f, 0.0f, TEXT("Ignite target's soul, 260 damage, life steal + burn + drains Flow"), false, false, true, 25.0f, 5.0f);

    // === Ice combinations (8) ===
    AddDualAffinitySpell(EClass::Arcanist, TEXT("Arcane Blizzard"), ESpellShape::Zone, EWeaponStatusEffect::Ice, EWeaponStatusEffect::Lightning, 200.0f, 45.0f, EResourceType::Mana, 20.0f, 1.0f, 800.0f, 10.0f, TEXT("Frozen lightning storm, 200 burst + freeze + stun + 20/s for 6s"), true, false, true, 20.0f, 6.0f);
    AddDualAffinitySpell(EClass::Duskblade, TEXT("Frozen Void"), ESpellShape::SingleTarget, EWeaponStatusEffect::Ice, EWeaponStatusEffect::Shadow, 220.0f, 35.0f, EResourceType::Stamina, 15.0f, 0.5f, 500.0f, 0.0f, TEXT("Void-touched ice strike, 220 damage, freeze + void damage + armor pierce"), false, false, true, 20.0f, 4.0f);
    AddDualAffinitySpell(EClass::Oracle, TEXT("Aurora Lance"), ESpellShape::Line, EWeaponStatusEffect::Ice, EWeaponStatusEffect::Holy, 200.0f, 40.0f, EResourceType::Mana, 15.0f, 0.5f, 1000.0f, 0.0f, TEXT("Piercing aurora beam, 200 damage in line, freeze + heal allies in beam"), false, false, false, 0.0f, 0.0f);
    AddDualAffinitySpell(EClass::Alchemist, TEXT("Toxic Frost"), ESpellShape::Ground, EWeaponStatusEffect::Ice, EWeaponStatusEffect::Poison, 170.0f, 35.0f, EResourceType::Reagents, 15.0f, 0.5f, 500.0f, 6.0f, TEXT("Frozen poison pool, 170 burst + 15/s poison + slow for 6s"), false, false, true, 15.0f, 6.0f);
    AddDualAffinitySpell(EClass::Wildwarden, TEXT("Permafrost"), ESpellShape::Ground, EWeaponStatusEffect::Ice, EWeaponStatusEffect::Earth, 190.0f, 40.0f, EResourceType::Mana, 18.0f, 0.8f, 600.0f, 8.0f, TEXT("Permafrost eruption, 190 burst + root all in 8m + 20/s for 5s"), false, false, true, 20.0f, 5.0f);
    AddDualAffinitySpell(EClass::Wildwarden, TEXT("Glacial Surge"), ESpellShape::Cone, EWeaponStatusEffect::Ice, EWeaponStatusEffect::Water, 180.0f, 35.0f, EResourceType::Mana, 12.0f, 0.5f, 600.0f, 8.0f, TEXT("Glacial wave, 180 cone damage, knockback + freeze + slow"), false, true, false, 0.0f, 0.0f);
    AddDualAffinitySpell(EClass::Stormcaller, TEXT("Blizzard Veil"), ESpellShape::AoE, EWeaponStatusEffect::Ice, EWeaponStatusEffect::Wind, 200.0f, 40.0f, EResourceType::Mana, 15.0f, 0.5f, 800.0f, 10.0f, TEXT("Wind-driven blizzard, 200 AoE, freeze + knockback + 15/s for 5s"), false, true, true, 15.0f, 5.0f);
    AddDualAffinitySpell(EClass::Necroturge, TEXT("Frost Wraith"), ESpellShape::Summon, EWeaponStatusEffect::Ice, EWeaponStatusEffect::Soul, 150.0f, 40.0f, EResourceType::Mana, 20.0f, 1.0f, 500.0f, 5.0f, TEXT("Summon frost wraith, 30/s frost + soul drain, lasts 20s"), false, false, true, 30.0f, 8.0f);

    // === Lightning combinations (7) ===
    AddDualAffinitySpell(EClass::Riftwalker, TEXT("Void Lightning"), ESpellShape::Chain, EWeaponStatusEffect::Lightning, EWeaponStatusEffect::Shadow, 220.0f, 40.0f, EResourceType::Mana, 15.0f, 0.5f, 800.0f, 0.0f, TEXT("Void lightning, chains 5 enemies, stun + void damage + 20/s for 4s"), true, false, true, 20.0f, 4.0f);
    AddDualAffinitySpell(EClass::Templar, TEXT("Divine Storm"), ESpellShape::AoE, EWeaponStatusEffect::Lightning, EWeaponStatusEffect::Holy, 230.0f, 45.0f, EResourceType::Mana, 18.0f, 1.0f, 800.0f, 10.0f, TEXT("Holy lightning storm, 230 AoE, stun + bonus vs undead + heal allies 50"), true, false, false, 0.0f, 0.0f);
    AddDualAffinitySpell(EClass::Alchemist, TEXT("Volatile Discharge"), ESpellShape::AoE, EWeaponStatusEffect::Lightning, EWeaponStatusEffect::Poison, 190.0f, 35.0f, EResourceType::Reagents, 15.0f, 0.5f, 600.0f, 8.0f, TEXT("Electrified toxin cloud, 190 AoE, stun + 20/s poison for 5s"), true, false, true, 20.0f, 5.0f);
    AddDualAffinitySpell(EClass::Warthane, TEXT("Seismic Storm"), ESpellShape::Ground, EWeaponStatusEffect::Lightning, EWeaponStatusEffect::Earth, 250.0f, 45.0f, EResourceType::Stamina, 20.0f, 1.0f, 800.0f, 10.0f, TEXT("Earthquake + lightning, 250 burst, stun + knockback + 25/s for 5s"), true, true, true, 25.0f, 5.0f);
    AddDualAffinitySpell(EClass::Stormcaller, TEXT("Electrified Torrent"), ESpellShape::Line, EWeaponStatusEffect::Lightning, EWeaponStatusEffect::Water, 200.0f, 40.0f, EResourceType::Mana, 15.0f, 0.5f, 800.0f, 0.0f, TEXT("Electrified water stream, 200 line damage, stun + knockback + wet"), true, true, false, 0.0f, 0.0f);
    AddDualAffinitySpell(EClass::Stormcaller, TEXT("Tempest Edge"), ESpellShape::Cone, EWeaponStatusEffect::Lightning, EWeaponStatusEffect::Wind, 240.0f, 40.0f, EResourceType::Mana, 15.0f, 0.5f, 600.0f, 8.0f, TEXT("Wind-charged lightning slash, 240 cone, stun + knockback"), true, true, false, 0.0f, 0.0f);
    AddDualAffinitySpell(EClass::Soulbinder, TEXT("Spirit Storm"), ESpellShape::AoE, EWeaponStatusEffect::Lightning, EWeaponStatusEffect::Soul, 220.0f, 40.0f, EResourceType::Mana, 18.0f, 0.8f, 600.0f, 10.0f, TEXT("Spirit-charged lightning, 220 AoE, stun + life steal + 20/s soul drain"), true, false, true, 20.0f, 5.0f);

    // === Shadow combinations (6) ===
    AddDualAffinitySpell(EClass::Templar, TEXT("Twilight Judgment"), ESpellShape::AoE, EWeaponStatusEffect::Shadow, EWeaponStatusEffect::Holy, 240.0f, 45.0f, EResourceType::Mana, 20.0f, 1.0f, 800.0f, 10.0f, TEXT("Twilight blast, 240 AoE, stun + life steal + bonus vs undead, heals allies"), true, false, true, 20.0f, 5.0f);
    AddDualAffinitySpell(EClass::Shadowblade, TEXT("Venomous Shadows"), ESpellShape::AoE, EWeaponStatusEffect::Shadow, EWeaponStatusEffect::Poison, 200.0f, 35.0f, EResourceType::Stamina, 15.0f, 0.5f, 600.0f, 8.0f, TEXT("Toxic shadow burst, 200 AoE, 20/s poison + life steal for 5s"), false, false, true, 20.0f, 5.0f);
    AddDualAffinitySpell(EClass::Dreadknight, TEXT("Grave Maw"), ESpellShape::Ground, EWeaponStatusEffect::Shadow, EWeaponStatusEffect::Earth, 220.0f, 40.0f, EResourceType::Rage, 18.0f, 0.8f, 600.0f, 8.0f, TEXT("Shadow earth maw, 220 burst + root + 20/s shadow for 5s"), false, false, true, 20.0f, 5.0f);
    AddDualAffinitySpell(EClass::Riftwalker, TEXT("Abyssal Tide"), ESpellShape::Line, EWeaponStatusEffect::Shadow, EWeaponStatusEffect::Water, 200.0f, 40.0f, EResourceType::Mana, 15.0f, 0.5f, 800.0f, 0.0f, TEXT("Abyssal water stream, 200 line, knockback + 20/s shadow for 4s"), false, true, true, 20.0f, 4.0f);
    AddDualAffinitySpell(EClass::Shadowblade, TEXT("Cutting Gale"), ESpellShape::Line, EWeaponStatusEffect::Shadow, EWeaponStatusEffect::Wind, 230.0f, 35.0f, EResourceType::Stamina, 12.0f, 0.0f, 600.0f, 0.0f, TEXT("Shadow wind blade, 230 line, knockback + void pierce + 15/s for 3s"), false, true, true, 15.0f, 3.0f);
    AddDualAffinitySpell(EClass::Necroturge, TEXT("Death Coil"), ESpellShape::Projectile, EWeaponStatusEffect::Shadow, EWeaponStatusEffect::Soul, 250.0f, 40.0f, EResourceType::Mana, 15.0f, 0.5f, 600.0f, 0.0f, TEXT("Soul-draining shadow projectile, 250 damage, life steal + 25/s for 4s"), false, false, true, 25.0f, 4.0f);

    // === Holy combinations (5) ===
    AddDualAffinitySpell(EClass::Oracle, TEXT("Elixir of Judgement"), ESpellShape::SingleTarget, EWeaponStatusEffect::Holy, EWeaponStatusEffect::Poison, 200.0f, 35.0f, EResourceType::Mana, 12.0f, 0.5f, 500.0f, 0.0f, TEXT("Divine venom, 200 damage, 15/s poison + heal caster 50% of damage"), false, false, true, 15.0f, 5.0f);
    AddDualAffinitySpell(EClass::Warthane, TEXT("Consecrated Ground"), ESpellShape::Ground, EWeaponStatusEffect::Holy, EWeaponStatusEffect::Earth, 180.0f, 40.0f, EResourceType::Stamina, 18.0f, 0.8f, 600.0f, 8.0f, TEXT("Consecrated earth, 180 burst, allies heal 10/s, enemies 15/s holy for 8s"), false, false, true, 15.0f, 8.0f);
    AddDualAffinitySpell(EClass::Oracle, TEXT("Holy Wellspring"), ESpellShape::Party, EWeaponStatusEffect::Holy, EWeaponStatusEffect::Water, 120.0f, 40.0f, EResourceType::Mana, 15.0f, 0.5f, 600.0f, 10.0f, TEXT("Holy wellspring, heal all allies in 10m for 120 + cleanse all debuffs"), false, false, false, 0.0f, 0.0f);
    AddDualAffinitySpell(EClass::Oracle, TEXT("Divine Zephyr"), ESpellShape::Party, EWeaponStatusEffect::Holy, EWeaponStatusEffect::Wind, 80.0f, 35.0f, EResourceType::Mana, 15.0f, 0.5f, 600.0f, 10.0f, TEXT("Divine wind, heal allies 80, +20% move speed, enemies take 10/s holy for 8s"), false, false, true, 10.0f, 8.0f);
    AddDualAffinitySpell(EClass::Soulbinder, TEXT("Spirit Light"), ESpellShape::Aura, EWeaponStatusEffect::Holy, EWeaponStatusEffect::Soul, 150.0f, 40.0f, EResourceType::Mana, 18.0f, 0.0f, 0.0f, 10.0f, TEXT("Radiant spirit aura, allies heal 15/s, enemies take 15/s spirit damage for 10s"), false, false, true, 15.0f, 10.0f);

    // === Poison combinations (4) ===
    AddDualAffinitySpell(EClass::Wildwarden, TEXT("Corrupted Earth"), ESpellShape::Ground, EWeaponStatusEffect::Poison, EWeaponStatusEffect::Earth, 200.0f, 40.0f, EResourceType::Mana, 18.0f, 0.8f, 600.0f, 8.0f, TEXT("Corrupted earth, 200 burst + root + 25/s poison for 6s"), false, false, true, 25.0f, 6.0f);
    AddDualAffinitySpell(EClass::Alchemist, TEXT("Toxic Brew"), ESpellShape::AoE, EWeaponStatusEffect::Poison, EWeaponStatusEffect::Water, 180.0f, 35.0f, EResourceType::Reagents, 15.0f, 0.5f, 600.0f, 8.0f, TEXT("Toxic water burst, 180 AoE, 20/s poison + slow 40% for 6s"), false, false, true, 20.0f, 6.0f);
    AddDualAffinitySpell(EClass::Wildwarden, TEXT("Miasma Cloud"), ESpellShape::Zone, EWeaponStatusEffect::Poison, EWeaponStatusEffect::Wind, 170.0f, 35.0f, EResourceType::Mana, 18.0f, 0.8f, 800.0f, 10.0f, TEXT("Wind-blown miasma, 170 burst + 18/s poison + slow for 8s"), false, false, true, 18.0f, 8.0f);
    AddDualAffinitySpell(EClass::Necroturge, TEXT("Plague Wraith"), ESpellShape::Summon, EWeaponStatusEffect::Poison, EWeaponStatusEffect::Soul, 120.0f, 40.0f, EResourceType::Mana, 20.0f, 1.0f, 500.0f, 5.0f, TEXT("Summon plague wraith, 25/s poison + soul drain, spreads on kill, 20s"), false, false, true, 25.0f, 8.0f);

    // === Earth combinations (3) ===
    AddDualAffinitySpell(EClass::Earthweaver, TEXT("Tidal Quake"), ESpellShape::AoE, EWeaponStatusEffect::Earth, EWeaponStatusEffect::Water, 220.0f, 40.0f, EResourceType::Mana, 18.0f, 0.8f, 800.0f, 10.0f, TEXT("Tidal earthquake, 220 AoE, knockback + root + 15/s for 5s"), false, true, true, 15.0f, 5.0f);
    AddDualAffinitySpell(EClass::Earthweaver, TEXT("Dust Devil"), ESpellShape::AoE, EWeaponStatusEffect::Earth, EWeaponStatusEffect::Wind, 190.0f, 35.0f, EResourceType::Mana, 15.0f, 0.5f, 600.0f, 8.0f, TEXT("Dust storm, 190 AoE, knockback + slow + blind for 5s"), false, true, false, 0.0f, 0.0f);
    AddDualAffinitySpell(EClass::Dreadknight, TEXT("Bone Citadel"), ESpellShape::Wall, EWeaponStatusEffect::Earth, EWeaponStatusEffect::Soul, 180.0f, 40.0f, EResourceType::Rage, 20.0f, 1.0f, 500.0f, 10.0f, TEXT("Bone earth wall, blocks enemies, allies behind heal 10/s, reflects 30%"), false, false, true, 15.0f, 8.0f);

    // === Water combinations (2) ===
    AddDualAffinitySpell(EClass::Earthweaver, TEXT("Tidal Gale"), ESpellShape::Cone, EWeaponStatusEffect::Water, EWeaponStatusEffect::Wind, 200.0f, 35.0f, EResourceType::Mana, 15.0f, 0.5f, 600.0f, 8.0f, TEXT("Tidal wind blast, 200 cone, knockback + slow + wet for 6s"), false, true, false, 0.0f, 0.0f);
    AddDualAffinitySpell(EClass::Earthweaver, TEXT("Spirit Tide"), ESpellShape::Line, EWeaponStatusEffect::Water, EWeaponStatusEffect::Soul, 210.0f, 40.0f, EResourceType::Mana, 18.0f, 0.5f, 800.0f, 0.0f, TEXT("Soul-infused tidal wave, 210 line, life steal + knockback + drains Flow"), false, true, false, 0.0f, 0.0f);

    // === Wind + Soul (1) ===
    AddDualAffinitySpell(EClass::Earthweaver, TEXT("Ancestral Wind"), ESpellShape::Aura, EWeaponStatusEffect::Wind, EWeaponStatusEffect::Soul, 150.0f, 40.0f, EResourceType::Mana, 18.0f, 0.0f, 0.0f, 10.0f, TEXT("Ancestral spirit wind, allies +30% speed + 15/s heal, enemies 15/s soul for 10s"), false, false, true, 15.0f, 10.0f);
}

// ============================================================================
// Getter Methods
// ============================================================================

TArray<FSpell> UEclipseSpellSystem::GetClassSpells(EClass CharacterClass) const
{
    if (const TArray<FSpell>* Spells = ClassSpells.Find(CharacterClass))
    {
        return *Spells;
    }
    return TArray<FSpell>();
}

FSpell UEclipseSpellSystem::GetSpell(int32 SpellID) const
{
    for (const FSpell& S : AllSpells)
    {
        if (S.SpellID == SpellID)
        {
            return S;
        }
    }
    return FSpell();
}

TArray<FSpell> UEclipseSpellSystem::GetDualAffinitySpells() const
{
    TArray<FSpell> Result;
    for (const FSpell& S : AllSpells)
    {
        if (S.bIsDualAffinity)
        {
            Result.Add(S);
        }
    }
    return Result;
}

TArray<FSpell> UEclipseSpellSystem::GetSpellsByElement(EWeaponStatusEffect Element) const
{
    if (const TArray<FSpell>* Spells = ElementSpells.Find(Element))
    {
        return *Spells;
    }
    return TArray<FSpell>();
}

FSpell UEclipseSpellSystem::GetDualAffinitySpell(EWeaponStatusEffect Element1, EWeaponStatusEffect Element2) const
{
    FString Key = MakeDualAffinityKey(Element1, Element2);
    if (const FSpell* Spell = DualAffinityMap.Find(Key))
    {
        return *Spell;
    }
    return FSpell();
}

int32 UEclipseSpellSystem::GetTotalSpellCount() const
{
    return AllSpells.Num();
}

int32 UEclipseSpellSystem::GetClassSpellCount(EClass CharacterClass) const
{
    if (const TArray<FSpell>* Spells = ClassSpells.Find(CharacterClass))
    {
        return Spells->Num();
    }
    return 0;
}

TArray<FSpell> UEclipseSpellSystem::GetAllSpells() const
{
    return AllSpells;
}

// ============================================================================
// AI Best Spell Selection
// ============================================================================

FSpell UEclipseSpellSystem::GetAIBestSpell(EClass CharacterClass, EClassRole Role,
    float Flow, float Resource, float TargetHP, float SelfHP) const
{
    TArray<FSpell> Spells = GetClassSpells(CharacterClass);
    if (Spells.Num() == 0)
    {
        return FSpell();
    }

    // If critically low HP, prefer defensive/healing spells
    if (SelfHP < 25.0f)
    {
        FSpell BestDefensive;
        float BestHealing = 0.0f;
        for (const FSpell& S : Spells)
        {
            if (S.Tier == ESpellTier::Defensive && Resource >= S.ResourceCost)
            {
                if (S.Healing > BestHealing || BestDefensive.SpellID == 0)
                {
                    BestHealing = S.Healing;
                    BestDefensive = S;
                }
            }
        }
        if (BestDefensive.SpellID > 0)
        {
            return BestDefensive;
        }
    }

    // If Flow is full, use Ultimate
    if (Flow >= 100.0f)
    {
        for (const FSpell& S : Spells)
        {
            if (S.Tier == ESpellTier::Ultimate)
            {
                return S;
            }
        }
    }

    // If target is low HP, prefer highest-damage finishing spell
    if (TargetHP < 20.0f)
    {
        FSpell BestFinisher;
        float BestDamage = 0.0f;
        for (const FSpell& S : Spells)
        {
            if (S.Damage > BestDamage && Resource >= S.ResourceCost && S.Tier != ESpellTier::Ultimate)
            {
                BestDamage = S.Damage;
                BestFinisher = S;
            }
        }
        if (BestFinisher.SpellID > 0)
        {
            return BestFinisher;
        }
    }

    // Role-based tier preference
    ESpellTier PreferredTier = ESpellTier::Basic;
    switch (Role)
    {
    case EClassRole::Tank:
        PreferredTier = ESpellTier::Defensive;
        break;
    case EClassRole::DPS:
        PreferredTier = ESpellTier::Advanced;
        break;
    case EClassRole::Healer:
        PreferredTier = ESpellTier::Defensive;
        break;
    case EClassRole::Hybrid:
        PreferredTier = ESpellTier::Intermediate;
        break;
    case EClassRole::Support:
        PreferredTier = ESpellTier::Utility;
        break;
    }

    // Find preferred tier spell within resource budget
    for (const FSpell& S : Spells)
    {
        if (S.Tier == PreferredTier && Resource >= S.ResourceCost)
        {
            return S;
        }
    }

    // Fallback: first affordable spell of any tier
    for (const FSpell& S : Spells)
    {
        if (Resource >= S.ResourceCost)
        {
            return S;
        }
    }

    // Last resort: cheapest spell available
    FSpell Cheapest;
    float MinCost = 999999.0f;
    for (const FSpell& S : Spells)
    {
        if (S.ResourceCost < MinCost)
        {
            MinCost = S.ResourceCost;
            Cheapest = S;
        }
    }

    return Cheapest.SpellID > 0 ? Cheapest : Spells[0];
}
