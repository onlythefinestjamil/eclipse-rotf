// Eclipse Weapon Swap System - Swap weapons mid-combat for different combos and status effects
// Per user: "Should be able to swap weapons on the fly to do different combos and apply different weapon status effects"

#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EclipseWeaponSwap.generated.h"

UENUM(BlueprintType)
enum class EWeaponStatusEffect : uint8
{
    None            UMETA(DisplayName = "None - no status effect"),
    Fire            UMETA(DisplayName = "Fire - burn over time, spreads to nearby enemies"),
    Ice             UMETA(DisplayName = "Ice - freeze, slow movement and attack speed"),
    Lightning       UMETA(DisplayName = "Lightning - chain damage, stun on crit"),
    Shadow          UMETA(DisplayName = "Shadow - void damage, life steal, ignores armor"),
    Holy            UMETA(DisplayName = "Holy - radiant damage, bonus vs undead/demon"),
    Poison          UMETA(DisplayName = "Poison - damage over time, stacks, reduces healing"),
    Earth           UMETA(DisplayName = "Earth - crushing damage, stagger, armor break"),
    Water           UMETA(DisplayName = "Water - knockback, wet (amplifies lightning), slow"),
    Wind            UMETA(DisplayName = "Wind - push/pull, extra mobility, ranged slash"),
    Soul            UMETA(DisplayName = "Soul - soul drain, steals Flow, bypasses defense"),
};

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    Greatsword      UMETA(DisplayName = "Greatsword - heavy, slow, high damage, guard break"),
    Daggers         UMETA(DisplayName = "Dual Daggers - fast, low damage, high combo, bleed"),
    Spear           UMETA(DisplayName = "Spear - reach, thrust, poke, counter"),
    GreatAxe        UMETA(DisplayName = "Great Axe - sweeping, knockback, rage build"),
    Hammer          UMETA(DisplayName = "War Hammer - stagger, armor break, guard crush"),
    Staff           UMETA(DisplayName = "Staff - spell amplifier, cast speed, aether channel"),
    Bow             UMETA(DisplayName = "Bow - ranged, mobile shots, precision"),
    Shield          UMETA(DisplayName = "Shield + Sword - block, guard counter, shield bash"),
    Scythe          UMETA(DisplayName = "Scythe - sweeping, soul harvest, life steal"),
    FistWeapons     UMETA(DisplayName = "Fist Weapons - rapid, combo-heavy, parry-focused"),
};

UENUM(BlueprintType)
enum class EComboStyle : uint8
{
    Heavy           UMETA(DisplayName = "Heavy - slow, deliberate, high per-hit damage"),
    Fast            UMETA(DisplayName = "Fast - rapid, fluid, low per-hit, high combo count"),
    Mixed           UMETA(DisplayName = "Mixed - alternating light/heavy for variety"),
    Precision       UMETA(DisplayName = "Precision - weak point focused, crit multiplier"),
    Sweeping        UMETA(DisplayName = "Sweeping - AoE, hits multiple enemies, crowd clear"),
    Thrusting       UMETA(DisplayName = "Thrusting - single target, piercing, range advantage"),
};

USTRUCT(BlueprintType)
struct FWeaponLoadout
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) FString WeaponName = "Iron Greatsword";
    UPROPERTY(BlueprintReadWrite) EWeaponType WeaponType = EWeaponType::Greatsword;
    UPROPERTY(BlueprintReadWrite) EWeaponStatusEffect StatusEffect = EWeaponStatusEffect::None;
    UPROPERTY(BlueprintReadWrite) EComboStyle ComboStyle = EComboStyle::Heavy;
    UPROPERTY(BlueprintReadWrite) int32 WeaponLevel = 1;
    UPROPERTY(BlueprintReadWrite) float BaseDamage = 100.0f;
    UPROPERTY(BlueprintReadWrite) float StatusEffectPower = 0.0f;
    UPROPERTY(BlueprintReadWrite) float StatusEffectDuration = 5.0f;
    UPROPERTY(BlueprintReadWrite) int32 MaxComboSteps = 4;
    UPROPERTY(BlueprintReadWrite) float ComboWindow = 0.5f;
    UPROPERTY(BlueprintReadWrite) bool bIsTwoHanded = true;
};

UCLASS(Blueprintable)
class ECLIPSEROTF_API UEclipseWeaponSwap : public UObject
{
    GENERATED_BODY()

public:
    UEclipseWeaponSwap();

    // Equip a weapon to a slot (0 or 1)
    UFUNCTION(BlueprintCallable, Category = "Eclipse|WeaponSwap")
    void EquipWeapon(int32 Slot, const FWeaponLoadout& Weapon);

    // Swap between weapon slots (on the fly, mid-combat)
    UFUNCTION(BlueprintCallable, Category = "Eclipse|WeaponSwap")
    void SwapWeapon();

    // Get currently active weapon
    UFUNCTION(BlueprintCallable, Category = "Eclipse|WeaponSwap")
    FWeaponLoadout GetCurrentWeapon() const;

    // Get weapon in other slot
    UFUNCTION(BlueprintCallable, Category = "Eclipse|WeaponSwap")
    FWeaponLoadout GetOtherWeapon() const;

    // Apply status effect from current weapon to target
    UFUNCTION(BlueprintCallable, Category = "Eclipse|WeaponSwap")
    void ApplyStatusEffect(EWeaponStatusEffect Effect, float Power, float Duration);

    // Get combo for current weapon
    UFUNCTION(BlueprintCallable, Category = "Eclipse|WeaponSwap")
    EComboStyle GetCurrentComboStyle() const;

    // Get status effect for current weapon
    UFUNCTION(BlueprintCallable, Category = "Eclipse|WeaponSwap")
    EWeaponStatusEffect GetCurrentStatusEffect() const;

    // Can swap right now? (not during active attack frames)
    UFUNCTION(BlueprintCallable, Category = "Eclipse|WeaponSwap")
    bool CanSwap() const;

    // Swap speed (how fast the weapon swap animation plays)
    UPROPERTY(BlueprintReadWrite, Category = "Eclipse|WeaponSwap")
    float SwapSpeed = 0.3f;

    // Does swapping cost Flow/Fracture Flow?
    UPROPERTY(BlueprintReadWrite, Category = "Eclipse|WeaponSwap")
    bool bSwapCostsFlow = false;

    UPROPERTY(BlueprintReadWrite, Category = "Eclipse|WeaponSwap")
    float FlowCost = 10.0f;

    // Dual affinity on swap - swapping to a weapon with a different element
    // triggers a dual affinity burst if both weapons have different elements
    UPROPERTY(BlueprintReadWrite, Category = "Eclipse|WeaponSwap")
    bool bDualAffinityOnSwap = true;

private:
    FWeaponLoadout Weapons[2];
    int32 ActiveSlot = 0;
    bool bCanSwap = true;
    float SwapCooldown = 0.0f;
    float SwapCooldownMax = 0.5f;

    // Last weapon element for dual affinity trigger
    EWeaponStatusEffect LastWeaponEffect = EWeaponStatusEffect::None;
};
