// Eclipse Damage Rotation System — Manual Attacks + Casts + Abilities
// No auto-attack. Everything is manual. High skill ceiling.
// Light/Heavy chains + Spells + Abilities + Weapon Arts = rotation

#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EclipseDamageRotation.generated.h"

#include "EclipseCharacter.h" // For EClass, EResourceType

UENUM(BlueprintType)
enum class EInputAction : uint8
{
    // Manual attacks (NO auto-attack — everything manual)
    LightAttack      UMETA(DisplayName = "Light Attack — R1"),
    HeavyAttack      UMETA(DisplayName = "Heavy Attack — R2"),
    ChargedAttack    UMETA(DisplayName = "Charged Attack — Hold R2"),
    WeaponArt        UMETA(DisplayName = "Weapon Art — L2"),
    
    // Spells (cast manually, not auto)
    Spell1           UMETA(DisplayName = "Spell 1 — face button"),
    Spell2           UMETA(DisplayName = "Spell 2 — face button"),
    Spell3           UMETA(DisplayName = "Spell 3 — face button"),
    Spell4           UMETA(DisplayName = "Spell 4 — face button"),
    
    // Abilities (cooldown-based, manual trigger)
    Ability1         UMETA(DisplayName = "Ability 1 — shoulder button"),
    Ability2         UMETA(DisplayName = "Ability 2 — shoulder button"),
    Ability3        UMETA(DisplayName = "Ability 3 — shoulder button"),
    Ultimate         UMETA(DisplayName = "Ultimate — both triggers"),
    
    // Defensive (manual, off-GCD)
    Dodge            UMETA(DisplayName = "Dodge — circle/B"),
    Parry            UMETA(DisplayName = "Parry — L1"),
    Block            UMETA(DisplayName = "Block — hold L1"),
    GuardCounter     UMETA(DisplayName = "Guard Counter — block + R1"),
    
    // Movement
    Sprint           UMETA(DisplayName = "Sprint — hold"),
    Jump             UMETA(DisplayName = "Jump"),
    AerialAttack     UMETA(DisplayName = "Aerial Attack — attack in air"),
    
    // Cast targeting
    AimSpell         UMETA(DisplayName = "Aim Spell — hold to aim"),
    ReleaseSpell     UMETA(DisplayName = "Release Spell — release to cast"),
};

UENUM(BlueprintType)
enum class ERotationPriority : uint8
{
    Spam      UMETA(DisplayName = "Spam — repeat highest damage"),
    Priority  UMETA(DisplayName = "Priority — use best available"),
    Builder   UMETA(DisplayName = "Builder — build resources then spend"),
    Weaver    UMETA(DisplayName = "Weaver — alternate spells and melee"),
    Burst     UMETA(DisplayName = "Burst — save cooldowns for burst window"),
    Sustain   UMETA(DisplayName = "Sustain — consistent DPS over time"),
};

USTRUCT(BlueprintType)
struct FDamageRotation
{
    GENERATED_BODY()

    // The rotation: what order to use abilities for max damage
    UPROPERTY(BlueprintReadWrite) TArray<EInputAction> Sequence;
    
    // Priority system (WoW-style: use highest priority available)
    UPROPERTY(BlueprintReadWrite) ERotationPriority PriorityType = ERotationPriority::Priority;
    
    // Resource thresholds for builder/spender rotations
    UPROPERTY(BlueprintReadWrite) float ResourceBuildThreshold = 50.0f;
    UPROPERTY(BlueprintReadWrite) float ResourceSpendThreshold = 80.0f;
    
    // Cooldown tracking for each ability
    UPROPERTY(BlueprintReadWrite) TMap<EInputAction, float> Cooldowns;
    UPROPERTY(BlueprintReadWrite) TMap<EInputAction, float> MaxCooldowns;
    
    // Fracture Flow thresholds for ultimate
    UPROPERTY(BlueprintReadWrite) float FlowThresholdForUltimate = 100.0f;
};

UCLASS(Blueprintable)
class ECLIPSEROTF_API UEclipseDamageRotation : public UObject
{
    GENERATED_BODY()

public:
    UEclipseDamageRotation();

    // Process an input action — returns damage dealt
    // This is the core: NO auto-attack, everything is manual
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Rotation")
    float ProcessAction(EInputAction Action, bool bHitWeakPoint, float TimingAccuracy);

    // Get the recommended next action based on rotation priority
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Rotation")
    EInputAction GetRecommendedAction(float CurrentResource, float CurrentFlow);

    // Check if an action is available (off cooldown + has resources)
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Rotation")
    bool IsActionAvailable(EInputAction Action, float CurrentResource);

    // Get cooldown remaining for an action
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Rotation")
    float GetCooldownRemaining(EInputAction Action);

    // Set rotation for a class
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Rotation")
    void SetClassRotation(EClass CharacterClass);

    // Manual attack chains
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Rotation")
    int32 GetComboStep() const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Rotation")
    bool CanChainLight() const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Rotation")
    bool CanChainHeavy() const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Rotation")
    bool CanTransitionToAerial() const;

    // Spell casting (manual, not auto)
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Rotation")
    bool CanCastSpell(int32 SpellSlot, float CurrentResource);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Rotation")
    float CastSpell(int32 SpellSlot, float CurrentResource);

    // Aim spell (hold to aim, release to cast)
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Rotation")
    void StartAimingSpell(int32 SpellSlot);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Rotation")
    float ReleaseAimedSpell();

private:
    // Combo state
    int32 ComboStep = 0;
    int32 MaxComboSteps = 4; // Light-Light-Heavy-Finisher
    float ComboTimer = 0.0f;
    float ComboWindow = 0.5f; // 500ms to continue combo

    // Aerial state
    bool bIsAirborne = false;
    int32 AerialComboStep = 0;

    // Cooldowns
    TMap<EInputAction, float> CooldownRemaining;
    TMap<EInputAction, float> CooldownDuration;

    // Spell slots
    TMap<int32, float> SpellCooldowns;
    TMap<int32, float> SpellCooldownMax;
    TMap<int32, float> SpellResourceCost;

    // Damage values per action type
    float LightDamage = 80.0f;
    float HeavyDamage = 150.0f;
    float ChargedDamage = 250.0f;
    float WeaponArtDamage = 300.0f;
    float SpellDamage = 120.0f;
    float AbilityDamage = 180.0f;
    float UltimateDamage = 500.0f;

    // Aiming
    int32 AimedSpellSlot = -1;
    bool bIsAiming = false;

    void InitializeClassRotation(EClass Class);
};
