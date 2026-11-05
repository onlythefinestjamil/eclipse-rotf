#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EclipseCharacter.generated.h"

UENUM(BlueprintType)
enum class EAncestry : uint8
{
    Aetherborn UMETA(DisplayName = "Aetherborn - Lunarian"),
    Solari UMETA(DisplayName = "Solari - Sunforged Human"),
    Vyrael UMETA(DisplayName = "Vyrael - Elven Kindred"),
    Tharuk UMETA(DisplayName = "Tharuk - Titanborn"),
    Grimstarr UMETA(DisplayName = "Grimstarr - Forge Dwarf"),
    Kazari UMETA(DisplayName = "Kazari - Beastblood"),
    Umbralis UMETA(DisplayName = "Umbralis - Void-Touched"),
    Draven UMETA(DisplayName = "Draven - Draconic"),
    Nereyth UMETA(DisplayName = "Nereyth - Aquatic Kin"),
    Myceliar UMETA(DisplayName = "Myceliar - Verdant Kin"),
    Forgeborn UMETA(DisplayName = "Forgeborn - Arcane Construct"),
    Hellsworn UMETA(DisplayName = "Hellsworn - Demon-Blooded"),
    Necrevan UMETA(DisplayName = "Necrevan - Oath-Bound Dead"),
};

UENUM(BlueprintType)
enum class EClass : uint8
{
    Vanguard UMETA(DisplayName = "Vanguard - Tank/Initiator"),
    UmbralAssassin UMETA(DisplayName = "Umbral Assassin - Burst/Stealth"),
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
    Shadowblade UMETA(DisplayName = "Shadowblade - Stealth/Assassin"),
    Spellblade UMETA(DisplayName = "Spellblade - Hybrid DPS"),
    Warlord UMETA(DisplayName = "Warlord - Tank/Protector"),
    Necromancer UMETA(DisplayName = "Necromancer - Summoner/Caster"),
    Druid UMETA(DisplayName = "Druid - Hybrid/Shapeshifter"),
};

UENUM(BlueprintType)
enum class EElementalAffinity : uint8
{
    Fire, Ice, Lightning, Shadow, Holy,
    Water, Nature, Soul, Earth, Wind, Poison, Temporal,
};

UENUM(BlueprintType)
enum class EResourceType : uint8
{
    Mana, Stamina, Rage, Reagents, Aether, Chi,
};

UENUM(BlueprintType)
enum class ERarityTier : uint8
{
    Common, Uncommon, Rare, Epic, Legendary, Mythic, Eclipse,
};

UCLASS(Blueprintable)
class ECLIPSEROTF_API AEclipseCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AEclipseCharacter();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Identity")
    EAncestry Ancestry;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Identity")
    EClass CharacterClass;

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

private:
    float LastGCDTriggerTime = -10.0f;
};
