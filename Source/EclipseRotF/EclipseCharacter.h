#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "EclipseCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
class UInputMappingContext;
class UInputAction;
class UEclipseCombatComponent;
class UEclipseAbilityComponent;
class UEclipsePerfectDodgeComponent;
class UAnimInstance;
class UUserWidget;
class UAbilitySystemComponent;
class UEclipseAttributeSet;
class UEclipseEquipmentComponent;
class UEclipseItemDefinition;
struct FInputActionValue;

UENUM(BlueprintType)
enum class EAncestry : uint8
{
    // Core 8 (V4 canonical)
    Aetherborn UMETA(DisplayName = "Aetherborn"),
    Solari UMETA(DisplayName = "Solari - Sunforged Human"),
    Vyrael UMETA(DisplayName = "Vyrael - Elven Kindred"),
    Tharuk UMETA(DisplayName = "Tharuk - Titanborn"),
    Kazari UMETA(DisplayName = "Kazari - Beastblood"),
    Umbralis UMETA(DisplayName = "Umbralis - Void-Touched"),
    Draven UMETA(DisplayName = "Draven - Draconic"),
    // Expansion 5 (13 total — V9.1 Final Canon)
    Grimstarr UMETA(DisplayName = "Grimstarr - Forge Dwarf"),
    Nereyth UMETA(DisplayName = "Nereyth - Aquatic Kin"),
    Myceliar UMETA(DisplayName = "Myceliar - Verdant Kin"),
    Forgeborn UMETA(DisplayName = "Forgeborn - Arcane Construct"),
    Hellsworn UMETA(DisplayName = "Hellsworn - Demon-Blooded"),
    Necrevan UMETA(DisplayName = "Necrevan - Oath-Bound Dead"),
};

UENUM(BlueprintType)
enum class EClassUnlockType : uint8
{
    Starting UMETA(DisplayName = "Starting Class"),
    Unlockable UMETA(DisplayName = "Unlockable Class"),
};

UENUM(BlueprintType)
enum class EClass : uint8
{
    Bulwark UMETA(DisplayName = "Bulwark - Tank/Initiator"),
    Duskblade UMETA(DisplayName = "Duskblade - Burst/Stealth"),
    Arcanist UMETA(DisplayName = "Arcanist - Ranged DPS/Control"),
    Riftwalker UMETA(DisplayName = "Riftwalker - Mobility/Disruptor"),
    Dreadknight UMETA(DisplayName = "Dreadknight - Drain Tank/Bruiser"),
    Templar UMETA(DisplayName = "Templar - Tank/Healer"),
    Soulbinder UMETA(DisplayName = "Soulbinder - Healer/Caster"),
    Wildwarden UMETA(DisplayName = "Wildwarden - Ranger/Hybrid"),
    Stormcaller UMETA(DisplayName = "Stormcaller - Ranged Burst"),
    Oracle UMETA(DisplayName = "Oracle - Support/Caster"),
    Alchemist UMETA(DisplayName = "Alchemist - Utility/Control"),
    Berserker UMETA(DisplayName = "Berserker - Plate DPS"),
    Shadowblade UMETA(DisplayName = "Shadowblade - Stealth/Burst"),
    Spellblade UMETA(DisplayName = "Spellblade - Hybrid DPS"),
    Warthane UMETA(DisplayName = "Warthane - Tank/Protector"),
    Necroturge UMETA(DisplayName = "Necroturge - Summoner/Caster"),
    Earthweaver UMETA(DisplayName = "Earthweaver - Hybrid/Shapeshifter"),
};

UENUM(BlueprintType)
enum class EElementalAffinity : uint8
{
    Fire, Ice, Lightning, Shadow, Holy,
    Water, Soul, Earth, Wind, Poison, Temporal,
};

UENUM(BlueprintType)
enum class EResourceType : uint8
{
    None,
    Mana, Stamina, Rage, Reagents, Aether,
};

UENUM(BlueprintType)
enum class ERarityTier : uint8
{
    Common, Uncommon, Rare, Epic, Legendary, Mythic, Eclipse,
};

/** Canon-locked region list (Game Bible V9 Final, Sections 3 and 13). */
UENUM(BlueprintType)
enum class ERegion : uint8
{
    Solara   UMETA(DisplayName = "Solara - Sun-Scorched Cathedral Lands"),
    Vaelwood UMETA(DisplayName = "Vaelwood - Ancient Forest"),
    Morvath  UMETA(DisplayName = "Morvath - Shadow-Drenched Wastes"),
    Aetheris UMETA(DisplayName = "Aetheris - Floating Islands of Pure Energy"),
    Nhalor   UMETA(DisplayName = "Nhalor - Frozen Peaks"),
    Umbryss  UMETA(DisplayName = "Umbryss - Abyssal Deep"),
};

UCLASS(Blueprintable)
class ECLIPSEROTF_API AEclipseCharacter : public ACharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    AEclipseCharacter();

    // --- Stage 1A playable pawn ---

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Eclipse|Camera")
    TObjectPtr<USpringArmComponent> CameraBoom = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Eclipse|Camera")
    TObjectPtr<UCameraComponent> TPPCamera = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Eclipse|Visual")
    TObjectPtr<UStaticMeshComponent> HeroMesh = nullptr;

    /** The held weapon. Socketed to the right hand so weapon families stay data-driven. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Eclipse|Weapon")
    TObjectPtr<UStaticMeshComponent> WeaponMesh = nullptr;

    /** Bone or socket the weapon rides. A bone name works without an authored socket. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Weapon")
    FName WeaponAttachSocket = TEXT("hand_r");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Weapon")
    FVector WeaponAttachOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Weapon")
    FRotator WeaponAttachRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Weapon")
    FVector WeaponAttachScale = FVector(1.0f);

    /** Swap the held weapon. Pass nullptr to sheathe. */
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Weapon")
    void SetWeaponMesh(UStaticMesh* InWeaponMesh);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Eclipse|Combat")
    TObjectPtr<UEclipseCombatComponent> CombatComponent = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Eclipse|Abilities")
    TObjectPtr<UEclipseAbilityComponent> AbilityComponent = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Eclipse|PerfectDodge")
    TObjectPtr<UEclipsePerfectDodgeComponent> PerfectDodgeComponent = nullptr;

    /** GAS is used for equipment only (attributes, equip effects, status). Abilities stay in AbilityComponent. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Eclipse|GAS")
    TObjectPtr<UAbilitySystemComponent> AbilitySystem = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Eclipse|GAS")
    TObjectPtr<UEclipseAttributeSet> EquipmentAttributes = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Eclipse|Equipment")
    TObjectPtr<UEclipseEquipmentComponent> EquipmentComponent = nullptr;

    /** Weapons cycled by IA_CycleWeapon. Entry 0 is equipped on BeginPlay. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Equipment")
    TArray<TObjectPtr<UEclipseItemDefinition>> DefaultLoadout;

    /** Optional override; falls back to /Game/Input/IA_CycleWeapon. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Equipment")
    TObjectPtr<UInputAction> CycleWeaponAction = nullptr;

    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystem; }
    virtual void PostInitializeComponents() override;

    /** Optional override for the action bar widget. Falls back to /Game/UI/WBP_AbilityBar. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Eclipse|HUD")
    TSubclassOf<UUserWidget> AbilityBarWidgetClass;

    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|HUD")
    TObjectPtr<UUserWidget> AbilityBarWidget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Resources") float Mana = 100.0f;
    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Resources") float MaxMana = 100.0f;
    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Resources") float Stamina = 100.0f;
    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Resources") float MaxStamina = 100.0f;
    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Resources") float Rage = 0.0f;
    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Resources") float MaxRage = 100.0f;
    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Resources") float Aether = 100.0f;
    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Resources") float MaxAether = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Movement") float WalkSpeed = 500.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Movement") float SprintSpeed = 950.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Movement") float SprintStaminaPerSecond = 15.0f;
    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Movement") bool bSprinting = false;
    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Input") FVector2D MoveInputAxis = FVector2D::ZeroVector;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Resources") float GetResourceValue(EResourceType Type) const;
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Resources") bool HasResource(EResourceType Type, float Amount) const;
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Resources") bool TrySpendResource(EResourceType Type, float Amount);
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Resources") void AddResourceValue(EResourceType Type, float Amount);
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Combat") void ApplyIncomingDamage(float Amount);

    UEclipseCombatComponent* GetCombat() const { return CombatComponent; }
    UEclipseAbilityComponent* GetAbilities() const { return AbilityComponent; }
    UEclipsePerfectDodgeComponent* GetPerfectDodge() const { return PerfectDodgeComponent; }
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Input") FVector2D GetMoveAxis() const { return MoveInputAxis; }

    /** Anim instance driving Mesh. Null until a rigged skeletal mesh + AnimBP are assigned. */
    UFUNCTION(BlueprintPure, Category = "Eclipse|Anim")
    UAnimInstance* GetHeroAnimInstance() const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Identity")
    EAncestry Ancestry;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Identity")
    EClass CharacterClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Identity")
    EClassUnlockType ClassUnlockType = EClassUnlockType::Starting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Affinity")
    EElementalAffinity PrimaryAffinity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Affinity")
    EElementalAffinity SecondaryAffinity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Progression")
    int32 CharacterLevel = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Progression")
    int32 PrestigeLevel = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Weapon")
    int32 WeaponLevel = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Weapon")
    int32 TotalKills = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Weapon")
    int32 HonorKills = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Weapon")
    int32 BossKills = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Resources")
    float CurrentHealth = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Resources")
    float MaxHealth = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Resources")
    float CurrentResource = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Resources")
    float MaxResource = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Resources")
    EResourceType PrimaryResource = EResourceType::Mana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Combat")
    float FractureFlow = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Combat")
    float MaxFractureFlow = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Combat")
    float GlobalCooldownDuration = 1.5f;

    /** Cast time for the Ability 1 spell. The bar completes before the spell payload is allowed to fire. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Combat")
    float AetherBoltCastSeconds = 0.9f;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Combat")
    bool IsOnGCD() const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Combat")
    void TriggerGCD();

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Combat")
    void AddFractureFlow(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Combat")
    void SpendFractureFlow(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Weapon")
    void AddKill(bool bIsHonorKill, bool bIsBossKill);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    void EnsureInputAssets();
    void AddDefaultMappingContext();

    void OnMoveInput(const FInputActionValue& Value);
    void OnLookInput(const FInputActionValue& Value);
    void OnSprintStart();
    void OnSprintStop();
    void OnDodgeInput();
    void OnLightAttackInput();
    void OnHeavyPressedInput();
    void OnHeavyReleasedInput();
    void OnAbility1Input();
    /** Bound to the combat component: the spell payload only lands on a completed cast. */
    UFUNCTION()
    void OnCombatCastFinished(bool bCompleted);

    void OnAbility2Input();
    void OnAbility3Input();
    void OnAbility4Input();
    void OnUltimateInput();
    void OnWeaponArtInput();
    /** Fires the highest-priority ability the rotation layer reports as ready. */
    void OnRotationInput();
    void OnToggleKeybindsInput();
    void OnCycleWeaponInput();

    /** Creates the action bar on the local player only; a no-op on the server and on AI pawns. */
    void CreateAbilityBarWidget();

    UPROPERTY() TObjectPtr<UInputMappingContext> DefaultMappingContext = nullptr;
    UPROPERTY() TObjectPtr<UInputAction> MoveAction = nullptr;
    UPROPERTY() TObjectPtr<UInputAction> LookAction = nullptr;
    UPROPERTY() TObjectPtr<UInputAction> SprintAction = nullptr;
    UPROPERTY() TObjectPtr<UInputAction> DodgeAction = nullptr;
    UPROPERTY() TObjectPtr<UInputAction> LightAttackAction = nullptr;
    UPROPERTY() TObjectPtr<UInputAction> HeavyAttackAction = nullptr;
    UPROPERTY() TObjectPtr<UInputAction> BlockAction = nullptr;
    UPROPERTY() TObjectPtr<UInputAction> LockOnAction = nullptr;
    UPROPERTY() TObjectPtr<UInputAction> Ability1Action = nullptr;
    UPROPERTY() TObjectPtr<UInputAction> Ability2Action = nullptr;
    UPROPERTY() TObjectPtr<UInputAction> Ability3Action = nullptr;
    UPROPERTY() TObjectPtr<UInputAction> Ability4Action = nullptr;
    UPROPERTY() TObjectPtr<UInputAction> UltimateAction = nullptr;
    UPROPERTY() TObjectPtr<UInputAction> WeaponArtAction = nullptr;
    UPROPERTY() TObjectPtr<UInputAction> RotationAction = nullptr;
    UPROPERTY() TObjectPtr<UInputAction> ToggleKeybindsAction = nullptr;

    bool bInputAssetsLoaded = false;

private:
    float LastGCDTriggerTime = -10.0f;
};
