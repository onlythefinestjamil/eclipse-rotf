#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EclipseCharacter.h"
#include "EclipseAbilityComponent.generated.h"

class UDataTable;
class UTexture2D;

USTRUCT(BlueprintType)
struct FEclipseAbilityRuntime
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) FName Id = NAME_None;
    UPROPERTY(BlueprintReadOnly) FString DisplayName;
    UPROPERTY(BlueprintReadOnly) float Cooldown = 0.0f;
    UPROPERTY(BlueprintReadOnly) float CooldownRemaining = 0.0f;
    UPROPERTY(BlueprintReadOnly) EResourceType CostType = EResourceType::Mana;
    UPROPERTY(BlueprintReadOnly) float Cost = 0.0f;

    /** Action bar order, left to right. */
    UPROPERTY(BlueprintReadOnly) int32 Slot = 0;
    /** Display-only key cap, mirrored from IMC_Default. */
    UPROPERTY(BlueprintReadOnly) FString KeyLabel;
    /** Refused while the combat state machine is busy when true. False makes an ability an escape. */
    UPROPERTY(BlueprintReadOnly) bool bRequiresFreeAction = true;
    /** Higher wins when the rotation layer picks the next button. */
    UPROPERTY(BlueprintReadOnly) int32 RotationPriority = 0;
    UPROPERTY(BlueprintReadOnly) TObjectPtr<UTexture2D> Icon = nullptr;
};

/** The six Aetherborn abilities plus the 1.0s GCD gate that never blocks basic combat actions. */
UCLASS(ClassGroup = (Eclipse), meta = (BlueprintSpawnableComponent))
class ECLIPSEROTF_API UEclipseAbilityComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEclipseAbilityComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Abilities") float GCDDuration = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Abilities") float BaseDamage = 30.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Abilities") int32 WeaponDamage = 50;

    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Abilities") TArray<FEclipseAbilityRuntime> Abilities;
    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Abilities") float GCDRemaining = 0.0f;
    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Abilities") bool bWingShieldActive = false;
    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Abilities") bool bChannelingAetherStorm = false;

    /** Data-driven ability table. Falls back to an identical built-in table when unset. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Eclipse|Abilities") TObjectPtr<UDataTable> AbilityDefs = nullptr;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Abilities") bool ActivateAbility(FName AbilityId);
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Abilities") bool IsOnGCD() const { return GCDRemaining > 0.0f; }
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Abilities") float GetCooldownRemaining(FName AbilityId) const;
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Abilities") FEclipseAbilityRuntime GetAbility(FName AbilityId) const;
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Abilities") FEclipseAbilityRuntime GetAbilityBySlot(int32 Slot) const;

    /** The single gate: GCD, cooldown, resource, and combat commitment. OutReason names the blocker for the HUD. */
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Abilities") bool CanActivateAbility(FName AbilityId, FString& OutReason) const;
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Abilities") TArray<FName> GetReadyAbilities() const;
    /** Highest RotationPriority among the abilities that could fire right now. */
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Abilities") FName GetRotationSuggestion() const;
    /** Fire the rotation suggestion, so a full damage rotation is playable from one button. */
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Abilities") bool TryActivateRotation();

private:
    void SetupAbilityTable();
    bool SpendCost(const FEclipseAbilityRuntime& Ability);
    void ApplyAreaDamage(const FVector& Origin, float Radius, float Damage, const FString& Label);
    int32 FindAbilityIndex(FName AbilityId) const;
    void FinishAetherStorm();
    void EndWingShield();

    FTimerHandle StormChannelHandle;
    FTimerHandle WingShieldHandle;
};
