// Eclipse equipment attribute set. Health and resources stay on AEclipseCharacter (not duplicated here).

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "EclipseAttributeSet.generated.h"

UCLASS()
class ECLIPSEROTF_API UEclipseAttributeSet : public UAttributeSet
{
    GENERATED_BODY()

public:
    UEclipseAttributeSet();

    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Attributes") FGameplayAttributeData WeaponPower;
    ATTRIBUTE_ACCESSORS_BASIC(UEclipseAttributeSet, WeaponPower)

    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Attributes") FGameplayAttributeData CritChance;
    ATTRIBUTE_ACCESSORS_BASIC(UEclipseAttributeSet, CritChance)

    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Attributes") FGameplayAttributeData CritDamage;
    ATTRIBUTE_ACCESSORS_BASIC(UEclipseAttributeSet, CritDamage)

    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Attributes") FGameplayAttributeData PoiseDamage;
    ATTRIBUTE_ACCESSORS_BASIC(UEclipseAttributeSet, PoiseDamage)

    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Attributes") FGameplayAttributeData Armor;
    ATTRIBUTE_ACCESSORS_BASIC(UEclipseAttributeSet, Armor)

    /** Accumulated status buildup on this actor (single affinity at a time). */
    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Attributes") FGameplayAttributeData StatusBuildup;
    ATTRIBUTE_ACCESSORS_BASIC(UEclipseAttributeSet, StatusBuildup)

    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Attributes") FGameplayAttributeData StatusThreshold;
    ATTRIBUTE_ACCESSORS_BASIC(UEclipseAttributeSet, StatusThreshold)
};

/** Infinite equip effect. Magnitudes come from SetByCaller names in EclipseEquipNames. */
UCLASS()
class ECLIPSEROTF_API UEclipseGE_EquipStats : public UGameplayEffect
{
    GENERATED_BODY()
public:
    UEclipseGE_EquipStats();
};

/** Timed tag carrier: duration set on the spec, granted tag added as a dynamic granted tag. */
UCLASS()
class ECLIPSEROTF_API UEclipseGE_TimedTag : public UGameplayEffect
{
    GENERATED_BODY()
public:
    UEclipseGE_TimedTag();
};

namespace EclipseEquipNames
{
    inline const FName WeaponPower(TEXT("Equip.WeaponPower"));
    inline const FName CritChance(TEXT("Equip.CritChance"));
    inline const FName CritDamage(TEXT("Equip.CritDamage"));
    inline const FName PoiseDamage(TEXT("Equip.PoiseDamage"));
}
