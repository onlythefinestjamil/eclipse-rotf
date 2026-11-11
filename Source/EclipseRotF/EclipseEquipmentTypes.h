// Eclipse Equipment Types - data-driven item definitions (balance data separate from art).
// See Docs/ECLIPSE_EQUIPMENT_BIBLE.md. Every authored item starts as EDesignStatus::Proposed.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayEffect.h"
#include "EclipseCharacter.h"   // EClass, EAncestry, EElementalAffinity, ERarityTier
#include "EclipseWeaponSwap.h"   // EWeaponType
#include "EclipseEquipmentTypes.generated.h"

class UStaticMesh;
class UNiagaraSystem;

UENUM(BlueprintType)
enum class EEquipmentSlot : uint8
{
    MainHand,
    OffHand,
    Head,
    Chest,
    Hands,
    Legs,
    Feet,
    Neck,
    Ring1,
    Ring2,
    Trinket,
};

/** Production status label required by the equipment directive (section 15). */
UENUM(BlueprintType)
enum class EDesignStatus : uint8
{
    Proposed,
    Approved,
    InProduction,
    Blocked,
    Implemented,
    Tested,
    Complete,
};

/** One equippable item. Balance values are Provisional until approved in the equipment bible. */
UCLASS(BlueprintType)
class ECLIPSEROTF_API UEclipseItemDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // --- Identity ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eclipse|Item") FName ItemId;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eclipse|Item") FText DisplayName;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eclipse|Item", meta = (MultiLine = true)) FText Lore;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eclipse|Item") EDesignStatus DesignStatus = EDesignStatus::Proposed;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eclipse|Item") EEquipmentSlot Slot = EEquipmentSlot::MainHand;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eclipse|Item") EWeaponType WeaponFamily = EWeaponType::Greatsword;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eclipse|Item") bool bTwoHanded = true;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eclipse|Item") ERarityTier Rarity = ERarityTier::Common;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eclipse|Item") int32 ItemLevel = 1;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eclipse|Item") int32 RequiredLevel = 1;
    /** Empty = usable by every class (shared families). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eclipse|Item") TArray<EClass> AllowedClasses;
    /** Empty = usable by every ancestry. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eclipse|Item") TArray<EAncestry> AllowedAncestries;
    /** Single canon affinity. No secondary affinity until canon open question 1 is resolved. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eclipse|Item") EElementalAffinity PrimaryAffinity = EElementalAffinity::Fire;

    // --- Combat (Provisional, expressed against the current arena baseline 40 / 90 / 300cm) ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eclipse|Combat") float LightDamage = 40.0f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eclipse|Combat") float HeavyDamage = 90.0f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eclipse|Combat") float Reach = 300.0f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eclipse|Combat") float AttackSpeedScale = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eclipse|Combat") float PoiseDamageMultiplier = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eclipse|Combat") float CritChance = 0.05f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eclipse|Combat") float CritDamage = 1.5f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eclipse|Combat") float ArmorPenetration = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eclipse|Combat") float PvPCoefficient = 0.7f;

    // --- Status (single affinity) ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eclipse|Status") float StatusBuildupPerHit = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eclipse|Status") float StatusDuration = 4.0f;
    /** Seconds after the status ends during which it cannot be re-applied (prevents lock loops). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eclipse|Status") float StatusImmunity = 10.0f;

    // --- Progression ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eclipse|Progression") int32 MaxUpgradeLevel = 10;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eclipse|Progression") int32 SocketCount = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eclipse|Progression") FString AcquisitionNote;

    // --- Effects (GAS) ---
    /** Extra GameplayEffects applied to the wearer while equipped (removed on unequip). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eclipse|GAS") TArray<TSubclassOf<UGameplayEffect>> EquipEffects;

    // --- Presentation (art kept as soft references) ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eclipse|Art") TSoftObjectPtr<UStaticMesh> Mesh;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eclipse|Art") FVector MeshScale = FVector(0.08f, 0.08f, 1.2f);
    /** Per-item grip fit, in weapon-socket space, added on top of the character's WeaponAttachOffset. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eclipse|Art") FVector MeshOffset = FVector::ZeroVector;
    /** Per-item grip rotation, composed after the character's WeaponAttachRotation. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eclipse|Art") FRotator MeshRotation = FRotator::ZeroRotator;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eclipse|Art") TSoftObjectPtr<UNiagaraSystem> TrailVFX;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eclipse|Art", meta = (MultiLine = true)) FString VisualBrief;

    /** Provisional: LightDamage x (1 + 0.04 x (ItemLevel - 1)) x RarityMult. */
    UFUNCTION(BlueprintPure, Category = "Eclipse|Item")
    float ComputeWeaponPower() const;

    UFUNCTION(BlueprintPure, Category = "Eclipse|Item")
    static float GetRarityMultiplier(ERarityTier InRarity);

    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        return FPrimaryAssetId(TEXT("EclipseItem"), GetFName());
    }
};
