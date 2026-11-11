#include "EclipseAttributeSet.h"

UEclipseAttributeSet::UEclipseAttributeSet()
{
    InitWeaponPower(0.0f);
    InitCritChance(0.0f);
    InitCritDamage(0.0f);
    InitPoiseDamage(0.0f);
    InitArmor(0.0f);
    InitStatusBuildup(0.0f);
    InitStatusThreshold(100.0f);
}

static FGameplayModifierInfo MakeSetByCallerMod(const FGameplayAttribute& Attribute, FName DataName)
{
    FSetByCallerFloat SetByCaller;
    SetByCaller.DataName = DataName;

    FGameplayModifierInfo Mod;
    Mod.Attribute = Attribute;
    Mod.ModifierOp = EGameplayModOp::Additive;
    Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);
    return Mod;
}

UEclipseGE_EquipStats::UEclipseGE_EquipStats()
{
    DurationPolicy = EGameplayEffectDurationType::Infinite;
    Modifiers.Add(MakeSetByCallerMod(UEclipseAttributeSet::GetWeaponPowerAttribute(), EclipseEquipNames::WeaponPower));
    Modifiers.Add(MakeSetByCallerMod(UEclipseAttributeSet::GetCritChanceAttribute(), EclipseEquipNames::CritChance));
    Modifiers.Add(MakeSetByCallerMod(UEclipseAttributeSet::GetCritDamageAttribute(), EclipseEquipNames::CritDamage));
    Modifiers.Add(MakeSetByCallerMod(UEclipseAttributeSet::GetPoiseDamageAttribute(), EclipseEquipNames::PoiseDamage));
}

UEclipseGE_TimedTag::UEclipseGE_TimedTag()
{
    DurationPolicy = EGameplayEffectDurationType::HasDuration;
    DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(1.0f));
}
