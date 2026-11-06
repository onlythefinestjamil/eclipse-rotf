// Eclipse Class Roles Implementation
// Tank/DPS/Healer role assignments per class

#include "EclipseClassRoles.h"
#include "Engine/Engine.h"

UEclipseClassRoles::UEclipseClassRoles()
{
    InitializeRoles();
}

void UEclipseClassRoles::InitializeRoles()
{
    // === FLEXIBLE CLASSES (can choose Tank/DPS/Healer) ===
    
    // Templar - Tank/DPS/Healer hybrid (paladin style)
    ClassRoles.Add(EClass::Templar, {
        ERoleFlexibility::Flexible,
        {EClassRole::Tank, EClassRole::DPS, EClassRole::Healer},
        "Holy warrior - can tank with auras, DPS with smites, or heal with divine light"
    });

    // Dreadknight - Tank/DPS (dark knight style)
    ClassRoles.Add(EClass::Dreadknight, {
        ERoleFlexibility::Flexible,
        {EClassRole::Tank, EClassRole::DPS},
        "Dark knight - can tank with drain/aggro or DPS with soul drain burst"
    });

    // Druid - DPS/Healer/Tank (shapeshifter)
    ClassRoles.Add(EClass::Druid, {
        ERoleFlexibility::Flexible,
        {EClassRole::DPS, EClassRole::Healer, EClassRole::Tank},
        "Nature shapeshifter - bear form tanks, caster form DPS, restoration heals"
    });

    // Soulbinder - DPS/Healer (spirit channeler)
    ClassRoles.Add(EClass::Soulbinder, {
        ERoleFlexibility::Flexible,
        {EClassRole::DPS, EClassRole::Healer},
        "Spirit channeler - DPS with soul attacks or heal by binding life force"
    });

    // Spellblade - Tank/DPS (battlemage)
    ClassRoles.Add(EClass::Spellblade, {
        ERoleFlexibility::Flexible,
        {EClassRole::Tank, EClassRole::DPS},
        "Battlemage - can tank with arcane barrier or DPS with spell-infused strikes"
    });

    // Warlord - Tank/DPS (commander)
    ClassRoles.Add(EClass::Warlord, {
        ERoleFlexibility::Flexible,
        {EClassRole::Tank, EClassRole::DPS},
        "Battle commander - can tank with war banner aggro or DPS with command strikes"
    });

    // === FIXED DPS CLASSES ===

    // Umbral Assassin - DPS only (burst)
    ClassRoles.Add(EClass::UmbralAssassin, {
        ERoleFlexibility::Fixed,
        {EClassRole::DPS},
        "Stealth burst - pure DPS, highest single-target damage, fragile"
    });

    // Arcanist - DPS only (caster)
    ClassRoles.Add(EClass::Arcanist, {
        ERoleFlexibility::Fixed,
        {EClassRole::DPS},
        "Pure caster - highest spell damage, lowest survivability, glass cannon"
    });

    // Berserker - DPS only (melee)
    ClassRoles.Add(EClass::Berserker, {
        ERoleFlexibility::Fixed,
        {EClassRole::DPS},
        "Rage DPS - highest sustained melee damage, no defensive abilities, all-in"
    });

    // Shadowblade - DPS only (rogue)
    ClassRoles.Add(EClass::Shadowblade, {
        ERoleFlexibility::Fixed,
        {EClassRole::DPS},
        "Dark rogue - pure melee DPS with shadow abilities, no tanking or healing"
    });

    // Riftwalker - DPS only (mobility)
    ClassRoles.Add(EClass::Riftwalker, {
        ERoleFlexibility::Fixed,
        {EClassRole::DPS},
        "Dimensional striker - pure DPS with teleport mobility, no support role"
    });

    // Stormcaller - DPS only (elemental)
    ClassRoles.Add(EClass::Stormcaller, {
        ERoleFlexibility::Fixed,
        {EClassRole::DPS},
        "Elemental DPS - lightning and wind damage, pure offensive caster"
    });

    // Necromancer - DPS only (dark caster)
    ClassRoles.Add(EClass::Necromancer, {
        ERoleFlexibility::Fixed,
        {EClassRole::DPS},
        "Death mage - pure DPS with summons, no healing or tanking"
    });

    // Alchemist - DPS/Support (flexible utility)
    ClassRoles.Add(EClass::Alchemist, {
        ERoleFlexibility::Flexible,
        {EClassRole::DPS, EClassRole::Support},
        "Potion master - DPS with reagents or support with buffs/debuffs"
    });

    // Oracle - Healer/Support (seer)
    ClassRoles.Add(EClass::Oracle, {
        ERoleFlexibility::Flexible,
        {EClassRole::Healer, EClassRole::Support},
        "Divine seer - healer with foresight abilities or support with debuffs"
    });

    // Wildwarden - Tank/DPS/Healer (nature guardian)
    ClassRoles.Add(EClass::Wildwarden, {
        ERoleFlexibility::Flexible,
        {EClassRole::Tank, EClassRole::DPS, EClassRole::Healer},
        "Nature guardian - can tank with bark skin, DPS with thorns, or heal with grove"
    });

    // Vanguard - Tank only (pure tank)
    ClassRoles.Add(EClass::Vanguard, {
        ERoleFlexibility::Fixed,
        {EClassRole::Tank},
        "Pure tank - highest defense, shield wall, aggro management, no DPS specialization"
    });
}

FClassRoleInfo UEclipseClassRoles::GetClassRoleInfo(EClass CharacterClass) const
{
    if (const FClassRoleInfo* Info = ClassRoles.Find(CharacterClass))
    {
        return *Info;
    }
    return FClassRoleInfo();
}

bool UEclipseClassRoles::CanBeTank(EClass CharacterClass) const
{
    if (const FClassRoleInfo* Info = ClassRoles.Find(CharacterClass))
    {
        return Info->AvailableRoles.Contains(EClassRole::Tank);
    }
    return false;
}

bool UEclipseClassRoles::CanBeHealer(EClass CharacterClass) const
{
    if (const FClassRoleInfo* Info = ClassRoles.Find(CharacterClass))
    {
        return Info->AvailableRoles.Contains(EClassRole::Healer);
    }
    return false;
}

bool UEclipseClassRoles::CanBeDPS(EClass CharacterClass) const
{
    if (const FClassRoleInfo* Info = ClassRoles.Find(CharacterClass))
    {
        return Info->AvailableRoles.Contains(EClassRole::DPS);
    }
    return false;
}

bool UEclipseClassRoles::IsFixedRole(EClass CharacterClass) const
{
    if (const FClassRoleInfo* Info = ClassRoles.Find(CharacterClass))
    {
        return Info->Flexibility == ERoleFlexibility::Fixed;
    }
    return true;
}
