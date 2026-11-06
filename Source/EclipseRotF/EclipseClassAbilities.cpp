// Eclipse Class Abilities Implementation
// All 17 classes with unique abilities — same abilities for AI companions and human players

#include "EclipseClassAbilities.h"
#include "Engine/Engine.h"

UEclipseClassAbilities::UEclipseClassAbilities()
{
    InitializeAllClassAbilities();
}

void UEclipseClassAbilities::AddAbility(EClass Class, const FString& Name, EAbilityType Type, 
    float Cooldown, float Damage, float ResourceCost, EResourceType ResourceType, 
    float Range, float CastTime, const FString& Description)
{
    FClassAbility Ability;
    Ability.AbilityName = Name;
    Ability.Type = Type;
    Ability.Cooldown = Cooldown;
    Ability.Damage = Damage;
    Ability.ResourceCost = ResourceCost;
    Ability.ResourceType = ResourceType;
    Ability.Range = Range;
    Ability.CastTime = CastTime;
    Ability.bIsInstant = (CastTime <= 0.0f);
    Ability.Description = Description;
    ClassAbilityMap.FindOrAdd(Class).Add(Ability);
}

void UEclipseClassAbilities::InitializeAllClassAbilities()
{
    // === VANGUARD === Tank, shield + greatsword
    AddAbility(EClass::Vanguard, "Warspite Slam", EAbilityType::Ability1, 8.0f, 80.0f, 20.0f, EResourceType::Stamina, 200.0f, 0.0f, "Bash shield into enemy, stuns for 1s, generates high threat");
    AddAbility(EClass::Vanguard, "Aegis Riposte", EAbilityType::GuardCounter, 6.0f, 150.0f, 25.0f, EResourceType::Stamina, 200.0f, 0.0f, "Block incoming hit then counter with heavy strike");
    AddAbility(EClass::Vanguard, "Bastion Formation", EAbilityType::Ability2, 30.0f, 0.0f, 40.0f, EResourceType::Stamina, 0.0f, 0.0f, "Reduce all damage by 50% for 5s, immune to knockback");
    AddAbility(EClass::Vanguard, "Sunbreaker Slam", EAbilityType::Ultimate, 180.0f, 300.0f, 100.0f, EResourceType::Flow, 400.0f, 1.5f, "Jump and slam greatsword creating shockwave, stuns all enemies in 8m");
    AddAbility(EClass::Vanguard, "Bulwark Surge", EAbilityType::WeaponArt, 12.0f, 120.0f, 30.0f, EResourceType::Stamina, 600.0f, 0.0f, "Charge forward with shield, knockback all enemies in path");

    // === UMBRAL ASSASSIN === DPS, daggers, stealth
    AddAbility(EClass::UmbralAssassin, "Gloom Stride", EAbilityType::Ability1, 10.0f, 60.0f, 15.0f, EResourceType::Stamina, 1500.0f, 0.0f, "Teleport behind target, 3 rapid blinks, leaves afterimages");
    AddAbility(EClass::UmbralAssassin, "Vertebra Pierce", EAbilityType::Ability2, 5.0f, 200.0f, 20.0f, EResourceType::Stamina, 200.0f, 0.0f, "X-pattern dagger strike from behind, guaranteed crit on unaware target");
    AddAbility(EClass::UmbralAssassin, "Cinder Shroud", EAbilityType::Ability3, 20.0f, 0.0f, 25.0f, EResourceType::Stamina, 0.0f, 0.0f, "Vanish into void smoke, become invisible for 5s, +50% damage on next hit");
    AddAbility(EClass::UmbralAssassin, "Veilfang Massacre", EAbilityType::Ultimate, 180.0f, 400.0f, 100.0f, EResourceType::Flow, 600.0f, 0.5f, "Strike all enemies in 10m with 5 rapid teleport strikes, each guaranteed crit");
    AddAbility(EClass::UmbralAssassin, "Voidrender Daggers", EAbilityType::WeaponArt, 8.0f, 150.0f, 20.0f, EResourceType::Stamina, 300.0f, 0.0f, "Throw both daggers in void trajectory, pierce all enemies in line");

    // === ARCANIST === DPS, spells, glass cannon
    AddAbility(EClass::Arcanist, "Aether Lance", EAbilityType::Spell1, 3.0f, 120.0f, 20.0f, EResourceType::Mana, 1200.0f, 0.0f, "Concentrated beam of amethyst energy, pierces all enemies in line");
    AddAbility(EClass::Arcanist, "Rime Burst", EAbilityType::Spell2, 8.0f, 80.0f, 30.0f, EResourceType::Mana, 500.0f, 0.5f, "Freeze all enemies in 8m radius for 3s, ice shatters on hit");
    AddAbility(EClass::Arcanist, "Celestial Ruin", EAbilityType::Spell3, 12.0f, 250.0f, 40.0f, EResourceType::Mana, 800.0f, 1.0f, "Rain meteors on target area, 5 impacts over 3s");
    AddAbility(EClass::Arcanist, "Astral Annihilation", EAbilityType::Ultimate, 180.0f, 600.0f, 100.0f, EResourceType::Flow, 1500.0f, 2.0f, "Massive 15m amethyst explosion, stun + knockback, ignites residual Aether");
    AddAbility(EClass::Arcanist, "Sigil of Fracture", EAbilityType::WeaponArt, 15.0f, 180.0f, 50.0f, EResourceType::Mana, 1000.0f, 1.0f, "Create a 15-foot rotating energy sigil, beams lance all enemies within");

    // === RIFTWALKER === DPS, teleport mobility
    AddAbility(EClass::Riftwalker, "Chrono Blink", EAbilityType::Ability1, 6.0f, 80.0f, 15.0f, EResourceType::Stamina, 800.0f, 0.0f, "Instant teleport to targeted location, leaves afterimage");
    AddAbility(EClass::Riftwalker, "Time Dilation", EAbilityType::Ability2, 25.0f, 0.0f, 40.0f, EResourceType::Mana, 0.0f, 0.0f, "Slow all enemies in 10m by 50% for 4s while you move at normal speed");
    AddAbility(EClass::Riftwalker, "Astral Harrier", EAbilityType::Ability3, 15.0f, 120.0f, 30.0f, EResourceType::Stamina, 500.0f, 0.0f, "Rapid multidirectional slashes, hits all enemies in melee range");
    AddAbility(EClass::Riftwalker, "Gateweaver Cataclysm", EAbilityType::Ultimate, 180.0f, 500.0f, 100.0f, EResourceType::Flow, 1200.0f, 1.5f, "Open 5 dimensional rifts around target, each fires energy beam for 3s");
    AddAbility(EClass::Riftwalker, "Phase Strike", EAbilityType::WeaponArt, 10.0f, 200.0f, 25.0f, EResourceType::Stamina, 400.0f, 0.0f, "Phase through enemy with weapon, damage applies after passing through");

    // === DREADKNIGHT === Tank/DPS, drain, dark knight
    AddAbility(EClass::Dreadknight, "Soul Drain", EAbilityType::Ability1, 10.0f, 80.0f, 20.0f, EResourceType::Rage, 300.0f, 0.0f, "Red-black tendrils drain 30 health from target, heal self for 50%");
    AddAbility(EClass::Dreadknight, "Blood Pact", EAbilityType::Ability2, 20.0f, 0.0f, 40.0f, EResourceType::Rage, 0.0f, 0.0f, "Sacrifice 20% HP, gain 50% damage buff and 30% lifesteal for 10s");
    AddAbility(EClass::Dreadknight, "Void Reaver", EAbilityType::Ability3, 15.0f, 150.0f, 35.0f, EResourceType::Rage, 400.0f, 0.5f, "Sweep weapon in arc, drain 3 enemies simultaneously");
    AddAbility(EClass::Dreadknight, "Bone Regent Crown", EAbilityType::Ultimate, 180.0f, 350.0f, 100.0f, EResourceType::Flow, 600.0f, 1.0f, "Summon crown of bone, AoE drain all enemies in 12m, heal full party for 30%");
    AddAbility(EClass::Dreadknight, "Blood Oath Strike", EAbilityType::WeaponArt, 8.0f, 180.0f, 25.0f, EResourceType::Rage, 250.0f, 0.0f, "Heavy overhead strike, heals for 100% of damage dealt");

    // === TEMPLAR === Tank/DPS/Healer, holy warrior
    AddAbility(EClass::Templar, "Dawnshield", EAbilityType::Ability1, 12.0f, 60.0f, 25.0f, EResourceType::Mana, 300.0f, 0.0f, "Holy barrier, absorb next 200 damage, reflect 50% as holy");
    AddAbility(EClass::Templar, "Hallowed Dawn", EAbilityType::Ability2, 8.0f, 0.0f, 30.0f, EResourceType::Mana, 500.0f, 0.5f, "Heal all allies in 8m for 80 HP, cleanse all debuffs");
    AddAbility(EClass::Templar, "Condemn", EAbilityType::Ability3, 6.0f, 120.0f, 20.0f, EResourceType::Mana, 300.0f, 0.0f, "Holy judgment strike, bonus damage vs undead/demon enemies");
    AddAbility(EClass::Templar, "Star Confessor", EAbilityType::Ultimate, 180.0f, 400.0f, 100.0f, EResourceType::Flow, 800.0f, 1.5f, "Massive holy nova, heal all allies for 150 and damage all enemies for 300 in 12m");
    AddAbility(EClass::Templar, "Judicator Verdict", EAbilityType::WeaponArt, 10.0f, 200.0f, 30.0f, EResourceType::Mana, 400.0f, 0.0f, "Charge weapon with holy light, next strike deals 2x damage and stuns");

    // === SOULBINDER === DPS/Healer, spirit channeler
    AddAbility(EClass::Soulbinder, "Echo Shepherd", EAbilityType::Ability1, 8.0f, 100.0f, 20.0f, EResourceType::Mana, 400.0f, 0.0f, "Bind enemy soul, deal damage over 5s, if enemy dies during bind, gain 20 Flow");
    AddAbility(EClass::Soulbinder, "Grave Covenant", EAbilityType::Ability2, 15.0f, 0.0f, 40.0f, EResourceType::Mana, 600.0f, 0.0f, "Revive a fallen ally at 50% HP, costs 40 Mana");
    AddAbility(EClass::Soulbinder, "Hex Sovereign", EAbilityType::Ability3, 10.0f, 80.0f, 25.0f, EResourceType::Mana, 500.0f, 0.0f, "Curse target: take 50% more damage for 5s, spread to nearby enemies");
    AddAbility(EClass::Soulbinder, "Lantern of Departed Choir", EAbilityType::Ultimate, 180.0f, 300.0f, 100.0f, EResourceType::Flow, 1000.0f, 1.0f, "Summon spectral choir, AoE spirit damage to all enemies, heal all allies for 100");
    AddAbility(EClass::Soulbinder, "Soul Chain", EAbilityType::WeaponArt, 12.0f, 120.0f, 25.0f, EResourceType::Mana, 600.0f, 0.0f, "Chain up to 3 enemies, shared damage, cannot move while chained");

    // === WILDWARDEN === Tank/DPS/Healer, nature guardian
    AddAbility(EClass::Wildwarden, "Barkskin", EAbilityType::Ability1, 15.0f, 0.0f, 30.0f, EResourceType::Stamina, 0.0f, 0.0f, "Bark skin armor, reduce physical damage 60% for 8s, thorns reflect 20 damage");
    AddAbility(EClass::Wildwarden, "Thorn Sentinel", EAbilityType::Ability2, 8.0f, 90.0f, 20.0f, EResourceType::Mana, 500.0f, 0.0f, "Summon thorn vines, root enemies in 6m for 3s, damage over time");
    AddAbility(EClass::Wildwarden, "Beastheart", EAbilityType::Ability3, 20.0f, 150.0f, 35.0f, EResourceType::Rage, 300.0f, 0.0f, "Transform into beast form, +100% attack speed and damage for 6s");
    AddAbility(EClass::Wildwarden, "Grove Seer Awakening", EAbilityType::Ultimate, 180.0f, 350.0f, 100.0f, EResourceType::Flow, 1200.0f, 2.0f, "Massive roots erupt in 15m, root + damage all enemies, heal all allies for 150");
    AddAbility(EClass::Wildwarden, "Verdant Howl", EAbilityType::WeaponArt, 10.0f, 100.0f, 25.0f, EResourceType::Stamina, 800.0f, 0.0f, "Primal roar, fear all enemies in 10m for 3s, knockback 5m");

    // === STORMCALLER === DPS, lightning/wind
    AddAbility(EClass::Stormcaller, "Thundercall Staff", EAbilityType::Spell1, 5.0f, 100.0f, 15.0f, EResourceType::Mana, 600.0f, 0.0f, "Lightning bolt from staff, chains to 3 nearby enemies for 50% damage each");
    AddAbility(EClass::Stormcaller, "Wind Slash", EAbilityType::Spell2, 8.0f, 80.0f, 20.0f, EResourceType::Stamina, 400.0f, 0.0f, "Blade of wind, knockback all enemies in cone, push 8m");
    AddAbility(EClass::Stormcaller, "Storm Surge", EAbilityType::Spell3, 15.0f, 200.0f, 40.0f, EResourceType::Mana, 1000.0f, 1.0f, "Summon lightning storm, 8 strikes over 5s in target area");
    AddAbility(EClass::Stormcaller, "Tempest Cataclysm", EAbilityType::Ultimate, 180.0f, 500.0f, 100.0f, EResourceType::Flow, 1500.0f, 2.0f, "Massive tornado + lightning, pull all enemies to center, stun 4s");
    AddAbility(EClass::Stormcaller, "Forked Voltage", EAbilityType::WeaponArt, 12.0f, 150.0f, 30.0f, EResourceType::Mana, 800.0f, 0.0f, "Lightning chains through all enemies in 15m, 6 chains max");

    // === ORACLE === Healer/Support, divine seer
    AddAbility(EClass::Oracle, "Premonition", EAbilityType::Ability1, 10.0f, 0.0f, 20.0f, EResourceType::Mana, 0.0f, 0.0f, "Reveal all enemy telegraphs for 8s, +20% dodge chance for all allies");
    AddAbility(EClass::Oracle, "Divine Prophecy", EAbilityType::Ability2, 6.0f, 0.0f, 25.0f, EResourceType::Mana, 600.0f, 0.5f, "Heal target for 150, if target drops below 20% HP within 5s, auto-heal for 200");
    AddAbility(EClass::Oracle, "Judgment", EAbilityType::Ability3, 15.0f, 130.0f, 35.0f, EResourceType::Mana, 500.0f, 0.0f, "Holy mark on target, all allies deal 25% more damage to marked target for 8s");
    AddAbility(EClass::Oracle, "All-Knowing Grace", EAbilityType::Ultimate, 180.0f, 0.0f, 100.0f, EResourceType::Flow, 2000.0f, 1.0f, "Full party heal 300 HP, full resource restore, 10s immunity to debuffs");
    AddAbility(EClass::Oracle, "Astral Rain", EAbilityType::WeaponArt, 14.0f, 120.0f, 30.0f, EResourceType::Mana, 800.0f, 0.8f, "Rain holy stars on target area, 6 impacts over 2s, heal allies in area for 30 per impact");

    // === ALCHEMIST === DPS/Support, reagents
    AddAbility(EClass::Alchemist, "Reagent Bomb", EAbilityType::Ability1, 8.0f, 100.0f, 20.0f, EResourceType::Reagents, 500.0f, 0.0f, "Throw alchemical bomb, AoE damage + random status effect (fire/ice/poison)");
    AddAbility(EClass::Alchemist, "Healing Draught", EAbilityType::Ability2, 12.0f, 0.0f, 25.0f, EResourceType::Reagents, 400.0f, 0.0f, "Throw healing flask at ally, heal 120 + cleanse 2 debuffs");
    AddAbility(EClass::Alchemist, "Mutation Elixir", EAbilityType::Ability3, 20.0f, 0.0f, 40.0f, EResourceType::Reagents, 0.0f, 0.0f, "Buff all allies in 8m: +30% damage, +20% defense for 10s");
    AddAbility(EClass::Alchemist, "Primal Catalyst", EAbilityType::Ultimate, 180.0f, 300.0f, 100.0f, EResourceType::Flow, 1000.0f, 1.5f, "Transmute area: damage enemies 300, convert damage to healing for allies");
    AddAbility(EClass::Alchemist, "Corrosive Phial", EAbilityType::WeaponArt, 10.0f, 80.0f, 20.0f, EResourceType::Reagents, 600.0f, 0.0f, "Throw acid, create 5m pool, enemies take 20/s and armor reduced 50% for 8s");

    // === BERSERKER === DPS, rage, 2h axe
    AddAbility(EClass::Berserker, "Rage Frenzy", EAbilityType::Ability1, 10.0f, 80.0f, 30.0f, EResourceType::Rage, 300.0f, 0.0f, "Enter frenzy, +50% attack speed, +30% damage, no stamina cost for 8s");
    AddAbility(EClass::Berserker, "Seismic Sunder", EAbilityType::Ability2, 15.0f, 150.0f, 40.0f, EResourceType::Rage, 600.0f, 0.0f, "Slam 2h axe into ground, AoE knockback 10m, stagger all enemies");
    AddAbility(EClass::Berserker, "Gore Hunger", EAbilityType::Ability3, 8.0f, 120.0f, 20.0f, EResourceType::Rage, 200.0f, 0.0f, "Savage strike, lifesteal 100% of damage dealt, build 20 Rage on hit");
    AddAbility(EClass::Berserker, "Berserker Rampage", EAbilityType::Ultimate, 180.0f, 500.0f, 100.0f, EResourceType::Flow, 800.0f, 0.0f, "Unstoppable rampage: 2x damage, immune to CC, lifesteal 50%, 10s duration");
    AddAbility(EClass::Berserker, "Pulverize", EAbilityType::WeaponArt, 8.0f, 250.0f, 35.0f, EResourceType::Rage, 300.0f, 0.5f, "Charged overhead axe slam, guaranteed stagger, 2x damage on staggered target");

    // === SHADOWBLADE === DPS, dark rogue
    AddAbility(EClass::Shadowblade, "Gloom Reave", EAbilityType::Ability1, 6.0f, 100.0f, 20.0f, EResourceType::Stamina, 300.0f, 0.0f, "Sweeping dark slash, hits all enemies in arc, applies 5s bleed");
    AddAbility(EClass::Shadowblade, "Umbral Strike", EAbilityType::Ability2, 10.0f, 150.0f, 25.0f, EResourceType::Stamina, 200.0f, 0.0f, "Phase through enemy, void damage, ignores 50% of armor");
    AddAbility(EClass::Shadowblade, "Dark Step", EAbilityType::Ability3, 8.0f, 80.0f, 15.0f, EResourceType::Stamina, 600.0f, 0.0f, "Dash through shadows, become briefly invisible, next attack guaranteed crit");
    AddAbility(EClass::Shadowblade, "Eclipse of Blades", EAbilityType::Ultimate, 180.0f, 450.0f, 100.0f, EResourceType::Flow, 800.0f, 1.0f, "Become a whirlwind of shadow blades, hit all enemies in 10m every 0.5s for 5s");
    AddAbility(EClass::Shadowblade, "Void Edge", EAbilityType::WeaponArt, 12.0f, 200.0f, 30.0f, EResourceType::Stamina, 400.0f, 0.0f, "Infuse blade with void energy, next 3 hits deal void damage and heal for 20%");

    // === SPELLBLADE === Tank/DPS, battlemage
    AddAbility(EClass::Spellblade, "Arcane Barrier", EAbilityType::Ability1, 12.0f, 0.0f, 30.0f, EResourceType::Mana, 0.0f, 0.0f, "Magical shield, absorb 300 damage, reflect 25% as arcane damage");
    AddAbility(EClass::Spellblade, "Rune-Etched Blow", EAbilityType::Ability2, 5.0f, 130.0f, 20.0f, EResourceType::Mana, 250.0f, 0.0f, "Infuse weapon with arcane energy, +100% damage, applies arcane mark");
    AddAbility(EClass::Spellblade, "Spell Parry", EAbilityType::Ability3, 8.0f, 80.0f, 15.0f, EResourceType::Stamina, 200.0f, 0.0f, "Parry spell projectile back at caster with 50% extra damage");
    AddAbility(EClass::Spellblade, "Aether Severance", EAbilityType::Ultimate, 180.0f, 400.0f, 100.0f, EResourceType::Flow, 600.0f, 0.5f, "Cut through reality itself, 5m range guaranteed crit, sever all magical effects on target");
    AddAbility(EClass::Spellblade, "Flux Ward", EAbilityType::WeaponArt, 10.0f, 0.0f, 40.0f, EResourceType::Mana, 0.0f, 0.0f, "Convert Mana to barrier, 1 Mana = 5 shield, lasts 10s or until depleted");

    // === WARLORD === Tank/DPS, commander
    AddAbility(EClass::Warlord, "Legion Standard", EAbilityType::Ability1, 20.0f, 0.0f, 40.0f, EResourceType::Rage, 0.0f, 0.0f, "Plant war banner, all allies in 15m +20% damage and +15% defense for 10s");
    AddAbility(EClass::Warlord, "Commanding Shout", EAbilityType::Ability2, 15.0f, 100.0f, 25.0f, EResourceType::Stamina, 800.0f, 0.0f, "Fear all enemies in 10m for 3s, knockback 5m, generate high threat");
    AddAbility(EClass::Warlord, "Warhorn Echo", EAbilityType::Ability3, 25.0f, 0.0f, 50.0f, EResourceType::Rage, 0.0f, 0.0f, "All allies heal 50, gain 30 Flow, immune to fear for 8s");
    AddAbility(EClass::Warlord, "Bannerlord Decree", EAbilityType::Ultimate, 180.0f, 300.0f, 100.0f, EResourceType::Flow, 1500.0f, 1.0f, "Massive war banner slam, 15m AoE stun, all allies +50% damage for 15s");
    AddAbility(EClass::Warlord, "Shield Breaker", EAbilityType::WeaponArt, 8.0f, 180.0f, 25.0f, EResourceType::Stamina, 300.0f, 0.0f, "Overhead smash, break enemy guard/poise instantly, guaranteed stagger");

    // === NECROMANCER === DPS, summons, death magic
    AddAbility(EClass::Necromancer, "Exhume Warrior", EAbilityType::Spell1, 8.0f, 40.0f, 20.0f, EResourceType::Mana, 500.0f, 0.5f, "Summon skeleton warrior, lasts 30s, taunts enemies and deals melee damage");
    AddAbility(EClass::Necromancer, "Ossuary Lance", EAbilityType::Spell2, 5.0f, 120.0f, 15.0f, EResourceType::Mana, 800.0f, 0.0f, "Fire bone projectile, pierces enemies, +50% damage vs living targets");
    AddAbility(EClass::Necromancer, "Necrotic Pulse", EAbilityType::Spell3, 12.0f, 180.0f, 35.0f, EResourceType::Mana, 600.0f, 0.8f, "Explosion of necrotic energy, 8m AoE, applies decay (damage over time)");
    AddAbility(EClass::Necromancer, "Lich Ascension", EAbilityType::Ultimate, 180.0f, 250.0f, 100.0f, EResourceType::Flow, 1200.0f, 2.0f, "Transform into Lich form: +200% spell damage, raise 5 skeletons, immune to death for 10s");
    AddAbility(EClass::Necromancer, "Cursed Bone Staff", EAbilityType::WeaponArt, 10.0f, 100.0f, 25.0f, EResourceType::Mana, 600.0f, 0.0f, "Staff strike applies curse: target takes 30% more damage from all sources for 5s");

    // === DRUID === DPS/Healer/Tank, shapeshifter
    AddAbility(EClass::Druid, "Ursine Aspect", EAbilityType::Ability1, 15.0f, 0.0f, 40.0f, EResourceType::Mana, 0.0f, 0.5f, "Transform into bear: +100% HP, +50% armor, taunt all enemies, 15s duration");
    AddAbility(EClass::Druid, "Bloom Renewal", EAbilityType::Ability2, 8.0f, 0.0f, 25.0f, EResourceType::Mana, 600.0f, 0.0f, "Heal target for 60 instantly + 10/s for 10s, cleanses poison");
    AddAbility(EClass::Druid, "Galvanic Aspect", EAbilityType::Ability3, 20.0f, 150.0f, 35.0f, EResourceType::Mana, 0.0f, 0.0f, "Transform into lightning wolf: +80% speed, lightning damage on attacks, 8s");
    AddAbility(EClass::Druid, "Nature Wrath", EAbilityType::Ultimate, 180.0f, 350.0f, 100.0f, EResourceType::Flow, 1500.0f, 1.5f, "Massive nature eruption: roots + thorns + lightning in 20m, heal allies for 200");
    AddAbility(EClass::Druid, "Thorn Whip", EAbilityType::WeaponArt, 6.0f, 80.0f, 15.0f, EResourceType::Mana, 400.0f, 0.0f, "Whip of thorns, pull enemy 5m toward you, apply bleed for 5s");
}

TArray<FClassAbility> UEclipseClassAbilities::GetClassAbilities(EClass CharacterClass) const
{
    if (const TArray<FClassAbility>* Abilities = ClassAbilityMap.Find(CharacterClass))
    {
        return *Abilities;
    }
    return TArray<FClassAbility>();
}

FClassAbility UEclipseClassAbilities::GetAbility(EClass CharacterClass, EAbilityType Type) const
{
    if (const TArray<FClassAbility>* Abilities = ClassAbilityMap.Find(CharacterClass))
    {
        for (const auto& Ability : *Abilities)
        {
            if (Ability.Type == Type)
            {
                return Ability;
            }
        }
    }
    return FClassAbility();
}

FClassAbility UEclipseClassAbilities::GetUltimate(EClass CharacterClass) const
{
    return GetAbility(CharacterClass, EAbilityType::Ultimate);
}

FClassAbility UEclipseClassAbilities::GetWeaponArt(EClass CharacterClass) const
{
    return GetAbility(CharacterClass, EAbilityType::WeaponArt);
}

FClassAbility UEclipseClassAbilities::GetAIBestAbility(EClass CharacterClass, EClassRole Role, float CurrentFlow, float CurrentResource) const
{
    if (const TArray<FClassAbility>* Abilities = ClassAbilityMap.Find(CharacterClass))
    {
        // If Flow is full, use Ultimate
        if (CurrentFlow >= 100.0f)
        {
            for (const auto& Ability : *Abilities)
            {
                if (Ability.Type == EAbilityType::Ultimate)
                {
                    UE_LOG(LogTemp, Log, TEXT("[Abilities] AI using Ultimate: %s"), *Ability.AbilityName);
                    return Ability;
                }
            }
        }
        
        // Otherwise pick role-appropriate ability
        EAbilityType PreferredType = EAbilityType::Ability1;
        
        switch (Role)
        {
        case EClassRole::Tank:
            PreferredType = EAbilityType::Ability1; // Defensive/threat ability
            break;
        case EClassRole::Healer:
            PreferredType = EAbilityType::Ability2; // Healing ability
            break;
        case EClassRole::DPS:
            // Check if enough resource for high-damage ability
            for (const auto& Ability : *Abilities)
            {
                if (Ability.Type == EAbilityType::Ability3 && CurrentResource >= Ability.ResourceCost)
                {
                    return Ability;
                }
            }
            PreferredType = EAbilityType::Ability1;
            break;
        case EClassRole::Support:
            PreferredType = EAbilityType::Ability3;
            break;
        }
        
        for (const auto& Ability : *Abilities)
        {
            if (Ability.Type == PreferredType && CurrentResource >= Ability.ResourceCost)
            {
                return Ability;
            }
        }
        
        // Fallback to first available ability
        for (const auto& Ability : *Abilities)
        {
            if (CurrentResource >= Ability.ResourceCost)
            {
                return Ability;
            }
        }
    }
    
    return FClassAbility();
}
