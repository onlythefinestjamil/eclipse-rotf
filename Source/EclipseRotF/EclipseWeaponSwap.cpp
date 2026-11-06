// Eclipse Weapon Swap Implementation
// Swap weapons on the fly for different combos and status effects

#include "EclipseWeaponSwap.h"
#include "Engine/Engine.h"

UEclipseWeaponSwap::UEclipseWeaponSwap()
{
    // Default loadouts
    Weapons[0] = FWeaponLoadout();
    Weapons[0].WeaponName = "Iron Greatsword";
    Weapons[0].WeaponType = EWeaponType::Greatsword;
    Weapons[0].StatusEffect = EWeaponStatusEffect::None;
    Weapons[0].ComboStyle = EComboStyle::Heavy;
    Weapons[0].BaseDamage = 120.0f;
    Weapons[0].MaxComboSteps = 4;
    Weapons[0].bIsTwoHanded = true;

    Weapons[1] = FWeaponLoadout();
    Weapons[1].WeaponName = "Steel Daggers";
    Weapons[1].WeaponType = EWeaponType::Daggers;
    Weapons[1].StatusEffect = EWeaponStatusEffect::None;
    Weapons[1].ComboStyle = EComboStyle::Fast;
    Weapons[1].BaseDamage = 50.0f;
    Weapons[1].MaxComboSteps = 6;
    Weapons[1].bIsTwoHanded = false;

    ActiveSlot = 0;
    bCanSwap = true;
    SwapCooldown = 0.0f;
}

void UEclipseWeaponSwap::EquipWeapon(int32 Slot, const FWeaponLoadout& Weapon)
{
    if (Slot < 0 || Slot > 1) return;
    Weapons[Slot] = Weapon;
    UE_LOG(LogTemp, Log, TEXT("[WeaponSwap] Equipped %s to slot %d (Type: %d, Effect: %d, Combo: %d)"),
        *Weapon.WeaponName, Slot, (int32)Weapon.WeaponType, (int32)Weapon.StatusEffect, (int32)Weapon.ComboStyle);
}

void UEclipseWeaponSwap::SwapWeapon()
{
    if (!CanSwap()) return;

    EWeaponStatusEffect oldEffect = Weapons[ActiveSlot].StatusEffect;
    ActiveSlot = 1 - ActiveSlot;
    EWeaponStatusEffect newEffect = Weapons[ActiveSlot].StatusEffect;

    SwapCooldown = SwapCooldownMax;
    bCanSwap = false;

    UE_LOG(LogTemp, Log, TEXT("[WeaponSwap] Swapped to slot %d: %s (Type: %d, Effect: %d, Combo: %d)"),
        ActiveSlot, *Weapons[ActiveSlot].WeaponName,
        (int32)Weapons[ActiveSlot].WeaponType,
        (int32)Weapons[ActiveSlot].StatusEffect,
        (int32)Weapons[ActiveSlot].ComboStyle);

    // Dual affinity burst: if both weapons have different elements, swapping triggers a burst
    if (bDualAffinityOnSwap && oldEffect != EWeaponStatusEffect::None && newEffect != EWeaponStatusEffect::None && oldEffect != newEffect)
    {
        UE_LOG(LogTemp, Log, TEXT("[WeaponSwap] DUAL AFFINITY BURST! %d + %d", (int32)oldEffect, (int32)newEffect));
        // Apply both status effects in a burst around the player
        ApplyStatusEffect(oldEffect, 50.0f, 3.0f);
        ApplyStatusEffect(newEffect, 50.0f, 3.0f);
    }

    LastWeaponEffect = newEffect;
}

FWeaponLoadout UEclipseWeaponSwap::GetCurrentWeapon() const
{
    return Weapons[ActiveSlot];
}

FWeaponLoadout UEclipseWeaponSwap::GetOtherWeapon() const
{
    return Weapons[1 - ActiveSlot];
}

void UEclipseWeaponSwap::ApplyStatusEffect(EWeaponStatusEffect Effect, float Power, float Duration)
{
    if (Effect == EWeaponStatusEffect::None) return;

    UE_LOG(LogTemp, Log, TEXT("[WeaponSwap] Applying status effect: %d (Power: %.1f, Duration: %.1fs)",
        (int32)Effect, Power, Duration));

    // Status effect behavior:
    // Fire: burn damage over time, can spread to nearby enemies
    // Ice: freeze/slow movement and attack speed
    // Lightning: chain damage to nearby enemies, stun on crit
    // Shadow: void damage, life steal, ignores armor
    // Holy: radiant damage, bonus vs undead/demon enemies
    // Poison: damage over time, stacks, reduces healing received
    // Earth: crushing damage, stagger, armor break
    // Water: knockback, wets target (amplifies lightning), slow
    // Wind: push/pull, extra mobility, ranged slash projectile
    // Soul: soul drain, steals Flow/Fracture Flow, bypasses defense
}

EComboStyle UEclipseWeaponSwap::GetCurrentComboStyle() const
{
    return Weapons[ActiveSlot].ComboStyle;
}

EWeaponStatusEffect UEclipseWeaponSwap::GetCurrentStatusEffect() const
{
    return Weapons[ActiveSlot].StatusEffect;
}

bool UEclipseWeaponSwap::CanSwap() const
{
    return bCanSwap && SwapCooldown <= 0.0f;
}
