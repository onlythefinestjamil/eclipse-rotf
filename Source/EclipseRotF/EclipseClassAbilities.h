// Eclipse Class Ability System - Maps all 17 classes to unique abilities
// Companions use THE SAME abilities as human players
// Per directive: AI decisions and player input drive the same actions

#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "EclipseCharacter.h" // For EClass
#include "EclipseClassAbilities.generated.h"

UENUM(BlueprintType)
enum class EAbilityType : uint8
{
    LightAttack    UMETA(DisplayName = "Light Attack - R1, fast, low damage"),
    HeavyAttack     UMETA(DisplayName = "Heavy Attack - R2, high damage, committed"),
    ChargedAttack   UMETA(DisplayName = "Charged Attack - Hold R2, very high damage"),
    WeaponArt       UMETA(DisplayName = "Weapon Art - L2, unique per weapon"),
    Spell1          UMETA(DisplayName = "Spell 1 - basic spell"),
    Spell2          UMETA(DisplayName = "Spell 2 - medium spell"),
    Spell3          UMETA(DisplayName = "Spell 3 - advanced spell"),
    Spell4          UMETA(DisplayName = "Spell 4 - ultimate spell"),
    Ability1        UMETA(DisplayName = "Ability 1 - class ability"),
    Ability2         UMETA(DisplayName = "Ability 2 - class ability"),
    Ability3        UMETA(DisplayName = "Ability 3 - class ability"),
    Ultimate         UMETA(DisplayName = "Ultimate - both triggers, 180s CD"),
    Dodge            UMETA(DisplayName = "Dodge - i-frames, 1s CD"),
    Parry            UMETA(DisplayName = "Parry - 0.15s window, 2s CD"),
    Block            UMETA(DisplayName = "Block - hold L1, drains stamina"),
    GuardCounter     UMETA(DisplayName = "Guard Counter - block + R1"),
    AerialAttack      UMETA(DisplayName = "Aerial Attack - attack in air"),
};

USTRUCT(BlueprintType)
struct FClassAbility
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) FString AbilityName = "";
    UPROPERTY(BlueprintReadWrite) EAbilityType Type = EAbilityType::LightAttack;
    UPROPERTY(BlueprintReadWrite) float Cooldown = 5.0f;
    UPROPERTY(BlueprintReadWrite) float Damage = 50.0f;
    UPROPERTY(BlueprintReadWrite) float ResourceCost = 20.0f; // Mana/Aether/Rage cost
    UPROPERTY(BlueprintReadWrite) EResourceType ResourceType = EResourceType::Mana;
    UPROPERTY(BlueprintReadWrite) float Range = 200.0f; // 0 = melee
    UPROPERTY(BlueprintReadWrite) float CastTime = 0.0f; // 0 = instant
    UPROPERTY(BlueprintReadWrite) bool bIsInstant = true;
    UPROPERTY(BlueprintReadWrite) FString Description = "";
};

UCLASS(Blueprintable)
class ECLIPSEROTF_API UEclipseClassAbilities : public UObject
{
    GENERATED_BODY()

public:
    UEclipseClassAbilities();

    // Get all abilities for a class
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Abilities")
    TArray<FClassAbility> GetClassAbilities(EClass CharacterClass) const;

    // Get specific ability for a class by type
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Abilities")
    FClassAbility GetAbility(EClass CharacterClass, EAbilityType Type) const;

    // Get the class's ultimate ability
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Abilities")
    FClassAbility GetUltimate(EClass CharacterClass) const;

    // Get the class's signature weapon art
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Abilities")
    FClassAbility GetWeaponArt(EClass CharacterClass) const;

    // Get role-appropriate ability for AI companion
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Abilities")
    FClassAbility GetAIBestAbility(EClass CharacterClass, EClassRole Role, float CurrentFlow, float CurrentResource) const;

private:
    void InitializeAllClassAbilities();
    
    // Per-class ability sets
    TMap<EClass, TArray<FClassAbility>> ClassAbilityMap;
    
    // Helper to add an ability
    void AddAbility(EClass Class, const FString& Name, EAbilityType Type, float Cooldown, 
        float Damage, float ResourceCost, EResourceType ResourceType, float Range, float CastTime, const FString& Description);
};