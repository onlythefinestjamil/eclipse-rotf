// Eclipse Equipment Component - equips UEclipseItemDefinition assets on the owning AEclipseCharacter.
// Owns: combat stat push (LightDamage/HeavyDamage/AttackRange), equip GameplayEffects, weapon mesh swap,
// and on-hit single-affinity status buildup. Does NOT own abilities, GCD or costs (UEclipseAbilityComponent).

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ActiveGameplayEffectHandle.h"
#include "EclipseEquipmentComponent.generated.h"

class UEclipseItemDefinition;
class UAbilitySystemComponent;
class AEclipseCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEclipseEquipChanged, UEclipseItemDefinition*, Item);

UCLASS(ClassGroup = (Eclipse), meta = (BlueprintSpawnableComponent))
class ECLIPSEROTF_API UEclipseEquipmentComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEclipseEquipmentComponent();

    virtual void BeginPlay() override;

    UPROPERTY(BlueprintAssignable, Category = "Eclipse|Equipment")
    FEclipseEquipChanged OnWeaponEquipped;

    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Equipment")
    TObjectPtr<UEclipseItemDefinition> EquippedWeapon = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Equipment")
    int32 LoadoutIndex = INDEX_NONE;

    /** Equip into the active weapon slot. Unequips the current weapon first. */
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Equipment")
    bool EquipItem(UEclipseItemDefinition* Item);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Equipment")
    void UnequipWeapon();

    /** Next entry of the owner's DefaultLoadout, wrapping around. */
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Equipment")
    void CycleWeapon();

    /** Called by the combat sweep for every actor a swing hits. Applies status buildup. */
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Equipment")
    void HandleHit(AActor* Target);

private:
    AEclipseCharacter* GetOwnerChar() const;
    UAbilitySystemComponent* GetOwnerASC() const;
    void CacheBaseline();

    FActiveGameplayEffectHandle StatsHandle;
    TArray<FActiveGameplayEffectHandle> ExtraHandles;

    bool bBaselineCached = false;
    float BaseLightDamage = 40.0f;
    float BaseHeavyDamage = 90.0f;
    float BaseAttackRange = 300.0f;
    FVector BaseWeaponScale = FVector(1.0f);
};
