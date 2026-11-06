// Eclipse Spell System - 400+ spells across all 17 classes
// 10 elemental affinities, 45 dual-affinity combinations
// Each class gets ~24 spells: basic, intermediate, advanced, ultimate, dual-affinity, weapon arts, defensive, utility

#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EclipseSpellSystem.generated.h"

#include "EclipseCharacter.h" // For EClass, EAncestry
#include "EclipseWeaponSwap.h" // For EWeaponStatusEffect (elements)

UENUM(BlueprintType)
enum class ESpellTier : uint8
{
    Cantrip      UMETA(DisplayName = "Cantrip - instant cast, low cost, low damage"),
    Basic         UMETA(DisplayName = "Basic - short cast, low cost, basic effect"),
    Intermediate  UMETA(DisplayName = "Intermediate - medium cast, medium cost, enhanced effect"),
    Advanced      UMETA(DisplayName = "Advanced - long cast, high cost, powerful effect"),
    Ultimate       UMETA(DisplayName = "Ultimate - Flow cost, devastating, long CD"),
    DualAffinity   UMETA(DisplayName = "Dual Affinity - combines 2 elements, unique hybrid effect"),
    WeaponArt      UMETA(DisplayName = "Weapon Art - L2, weapon-specific, medium CD"),
    Defensive      UMMETA(DisplayName = "Defensive - block, shield, heal, cleanse, buff"),
    Utility        UMETA(DisplayName = "Utility - CC, teleport, buff, debuff, summon, terrain"),
    Ritual          UMETA(DisplayName = "Ritual - very long cast, party-wide, raid-level"),
};

UENUM(BlueprintType)
enum class ESpellShape : uint8
{
    SingleTarget   UMETA(DisplayName = "Single Target - hits one enemy"),
    AoE            UMETA(DisplayName = "AoE - hits all enemies in radius"),
    Cone           UMETA(DisplayName = "Cone - hits enemies in frontal cone"),
    Line           UMETA(DisplayName = "Line - pierces enemies in a line"),
    Chain          UMETA(DisplayName = "Chain - jumps between enemies"),
    Ground         UMETA(DisplayName = "Ground - persistent area on floor"),
    Self           UMETA(DisplayName = "Self - affects caster only"),
    Party          UMETA(DisplayName = "Party - affects all allies in range"),
    Aura           UMETA(DisplayName = "Aura - persistent around caster"),
    Summon         UMETA(DisplayName = "Summon - creates a minion/construct"),
    Projectile     UMMETA(DisplayName = "Projectile - travels to target"),
    Wall           UMETA(DisplayName = "Wall - creates a barrier"),
    Zone           UMETA(DisplayName = "Zone - large persistent area"),
};

USTRUCT(BlueprintType)
struct FSpell
{
    GENERATED_BODY()

    // Identity
    UPROPERTY(BlueprintReadWrite) int32 SpellID = 0;
    UPROPERTY(BlueprintReadWrite) FString SpellName = "";
    UProperty(BlueprintReadWrite) FString Description = "";
    
    // Classification
    UProperty(BlueprintReadWrite) EClass Class = EClass::Vanguard;
    UProperty(BlueprintReadWrite) ESpellTier Tier = ESpellTier::Basic;
    UProperty(BlueprintReadWrite) ESpellShape Shape = ESpellShape::SingleTarget;
    UProperty(BlueprintReadWrite) EWeaponStatusEffect Element1 = EWeaponStatusEffect::None;
    UProperty(BlueprintReadWrite) EWeaponStatusEffect Element2 = EWeaponStatusEffect::None; // For dual affinity
    
    // Mechanics
    UProperty(BlueprintReadWrite) float Damage = 0.0f;
    UProperty(BlueprintReadWrite) float Healing = 0.0f;
    UProperty(BlueprintReadWrite) float ResourceCost = 20.0f;
    UProperty(BlueprintReadWrite) EResourceType ResourceType = EResourceType::Mana;
    UProperty(BlueprintReadWrite) float Cooldown = 5.0f;
    UProperty(BlueprintReadWrite) float CastTime = 0.0f;
    UProperty(BlueprintReadWrite) float Range = 500.0f;
    UProperty(BlueprintReadWrite) float Radius = 0.0f; // For AoE/Zone
    UProperty(BlueprintReadWrite) float Duration = 0.0f; // For persistent effects
    
    // Effects
    UProperty(BlueprintReadWrite) bool bStuns = false;
    UProperty(BlueprintReadWrite) bool bFreezes = false;
    UProperty(BlueprintReadWrite) bool bKnockbacks = false;
    UProperty(BlueprintReadWrite) bool bRoots = false;
    UProperty(BlueprintReadWrite) bool bSilences = false;
    UProperty(BlueprintReadWrite) bool bHealsCaster = false;
    UProperty(BlueprintReadWrite) bool bLifesteal = false;
    UProperty(BlueprintReadWrite) float LifestealPercent = 0.0f;
    UProperty(BlueprintReadWrite) bool bAppliesDoT = false;
    UProperty(BlueprintReadWrite) float DoTDamage = 0.0f;
    UProperty(BlueprintReadWrite) float DoTDuration = 0.0f;
    UProperty(BlueprintReadWrite) bool bAppliesDebuff = false;
    UProperty(BlueprintReadWrite) EWeaponStatusEffect DebuffType = EWeaponStatusEffect::None;
    UProperty(BlueprintReadWrite) float DebuffDuration = 0.0f;
    UProperty(BlueprintReadWrite) bool bIsDualAffinity = false;
};

UCLASS(Blueprintable)
class ECLIPSEROTF_API UEclipseSpellSystem : public UObject
{
    GENERATED_BODY()

public:
    UEclipseSpellSystem();

    // Get all spells for a class
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Spells")
    TArray<FSpell> GetClassSpells(EClass CharacterClass) const;

    // Get spell by ID
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Spells")
    FSpell GetSpell(int32 SpellID) const;

    // Get all dual-affinity spells
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Spells")
    TArray<FSpell> GetDualAffinitySpells() const;

    // Get spells by element
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Spells")
    TArray<FSpell> GetSpellsByElement(EWeaponStatusEffect Element) const;

    // Get dual-affinity spell for specific combo
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Spells")
    FSpell GetDualAffinitySpell(EWeaponStatusEffect Element1, EWeaponStatusEffect Element2) const;

    // Get total spell count
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Spells")
    int32 GetTotalSpellCount() const;

    // Get spell count per class
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Spells")
    int32 GetClassSpellCount(EClass CharacterClass) const;

    // Get all spells
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Spells")
    TArray<FSpell> GetAllSpells() const;

    // Get best spell for AI based on class, role, resources
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Spells")
    FSpell GetAIBestSpell(EClass CharacterClass, EClassRole Role, float Flow, float Resource, float TargetHP, float SelfHP) const;

private:
    TArray<FSpell> AllSpells;
    TMap<EClass, TArray<FSpell>> ClassSpells;
    TMap<EWeaponStatusEffect, TArray<FSpell>> ElementSpells;
    TMap<FString, FSpell> DualAffinityMap;

    void InitializeSpells();
    int32 NextSpellID = 1;

    // Helper to add a spell
    FSpell& AddSpell(EClass Class, const FString& Name, ESpellTier Tier, ESpellShape Shape,
        EWeaponStatusEffect Element1, float Damage, float Cost, EResourceType ResourceType,
        float Cooldown, float CastTime, float Range, float Radius, float Duration,
        const FString& Description);

    // Helper for dual affinity
    void AddDualAffinitySpell(EClass Class, const FString& Name, ESpellShape Shape,
        EWeaponStatusEffect Element1, EWeaponStatusEffect Element2, float Damage,
        float Cost, EResourceType ResourceType, float Cooldown, float CastTime, float Range,
        float Radius, const FString& Description, bool bStuns = false, bool bKnockbacks = false,
        bool bAppliesDoT = false, float DoTDamage = 0.0f, float DoTDuration = 0.0f);

    // Generate spells for each class
    void InitVanguardSpells();
    void InitUmbralAssassinSpells();
    void InitArcanistSpells();
    void InitRiftwalkerSpells();
    void InitDreadknightSpells();
    void InitTemplarSpells();
    void InitSoulbinderSpells();
    void InitWildwardenSpells();
    void InitStormcallerSpells();
    void InitOracleSpells();
    void InitAlchemistSpells();
    void InitBerserkerSpells();
    void InitShadowbladeSpells();
    void InitSpellbladeSpells();
    void InitWarlordSpells();
    void InitNecromancerSpells();
    void InitDruidSpells();

    // Generate dual affinity spells (45 combinations)
    void InitDualAffinitySpells();
};
