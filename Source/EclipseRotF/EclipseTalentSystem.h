// Eclipse Talent Tree System — Interconnected buildcraft with real synergy
// Per V6 Talent Directive: not +5% damage filler, but mechanical combo chains
// 5 layers: Class Foundation, Specialization, Affinity, Dual-Affinity Convergence, Game Mode

#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "EclipseCharacter.h"    // EClass, EAncestry, EResourceType
#include "EclipseClassRoles.h"   // EClassRole
#include "EclipseWeaponSwap.h"    // EWeaponStatusEffect (elements)
#include "EclipseSpellSystem.h"   // FSpell, ESpellTier
#include "EclipseTalentSystem.generated.h"

UENUM(BlueprintType)
enum class ETalentLayer : uint8
{
    ClassFoundation       UMETA(DisplayName = "Class Foundation — identity, survival, utility"),
    Specialization         UMETA(DisplayName = "Specialization — combat fantasy, role, rotation"),
    Affinity              UMETA(DisplayName = "Affinity — elemental mechanics, synergies"),
    DualAffinityConvergence UMETA(DisplayName = "Dual-Affinity Convergence — cross-tree synergy, capstones"),
    GameModeAdaptation    UMETA(DisplayName = "Game Mode — Solo, Dungeon, Raid, Arena, Battleground"),
};

UENUM(BlueprintType)
enum class ETalentNodeType : uint8
{
    ActiveAbility     UMETA(DisplayName = "Active — unlocks a new ability"),
    Passive           UMETA(DisplayName = "Passive — permanent mechanical improvement"),
    AbilityTransform   UMETA(DisplayName = "Transform — modifies an existing ability"),
    ProcReaction       UMETA(DisplayName = "Proc — chance-based reaction effect"),
    ResourceMod       UMETA(DisplayName = "Resource — generates or modifies resource flow"),
    CooldownMod       UMETA(DisplayName = "Cooldown — modifies cooldown or cast time"),
    Conditional        UMETA(DisplayName = "Conditional — triggers under specific conditions"),
    Defensive          UMETA(DisplayName = "Defensive — mitigation, mobility, survival"),
    Utility            UMETA(DisplayName = "Utility — CC, movement, group support"),
    CrossSynergy       UMETA(DisplayName = "Cross-Synergy — interacts with another tree"),
    Convergence        UMETA(DisplayName = "Convergence — dual-affinity hybrid mechanic"),
    ChoiceNode         UMETA(DisplayName = "Choice — mutually exclusive options"),
    Keystone           UMETA(DisplayName = "Keystone — defines build identity"),
    Capstone           UMETA(DisplayName = "Capstone — tree-defining ultimate talent"),
};

UENUM(BlueprintType)
enum class ESynergyType : uint8
{
    RequiredPrerequisite  UMETA(DisplayName = "Required — must have this to unlock next"),
    DirectSynergy         UMETA(DisplayName = "Direct — directly enhances another talent"),
    OptionalComplementary UMETA(DisplayName = "Optional — complements but not required"),
    ConditionalGameMode   UMETA(DisplayName = "Game-Mode — active in specific modes"),
    MutuallyExclusive      UMETA(DisplayName = "Exclusive — cannot take both"),
    AntiSynergy            UMETA(DisplayName = "Anti-Synergy — weakens or conflicts"),
};

UENUM(BlueprintType)
enum class EGameModeContext : uint8
{
    AllModes  UMETA(DisplayName = "All — works everywhere"),
    Solo      UMETA(DisplayName = "Solo — campaign, exploration"),
    Dungeon   UMETA(DisplayName = "Dungeon — 5-player"),
    Raid      UMETA(DisplayName = "Raid — 10-player"),
    Arena     UMETA(DisplayName = "Arena — PvP 1v1-5v5"),
    Battleground UMETA(DisplayName = "Battleground — 10v10"),
};

USTRUCT(BlueprintType)
struct FTalentNode
{
    GENERATED_BODY()

    // Identity
    UPROPERTY(BlueprintReadWrite) int32 TalentID = 0;
    UPROPERTY(BlueprintReadWrite) FString TalentName = "";
    UPROPERTY(BlueprintReadWrite) FString Description = "";
    UPROPERTY(BlueprintReadWrite) FString DetailedTooltip = "";

    // Classification
    UPROPERTY(BlueprintReadWrite) EClass ParentClass = EClass::Bulwark;
    UPROPERTY(BlueprintReadWrite) ETalentLayer Layer = ETalentLayer::ClassFoundation;
    UPROPERTY(BlueprintReadWrite) ETalentNodeType NodeType = ETalentNodeType::Passive;
    UPROPERTY(BlueprintReadWrite) EGameModeContext GameMode = EGameModeContext::AllModes;

    // Affinity (for affinity/convergence layers)
    UPROPERTY(BlueprintReadWrite) EWeaponStatusEffect Affinity1 = EWeaponStatusEffect::None;
    UPROPERTY(BlueprintReadWrite) EWeaponStatusEffect Affinity2 = EWeaponStatusEffect::None;
    UPROPERTY(BlueprintReadWrite) bool bIsDualAffinity = false;

    // Tree position
    UPROPERTY(BlueprintReadWrite) int32 Tier = 1;        // 1-7, tier 7 = capstone
    UPROPERTY(BlueprintReadWrite) int32 BranchIndex = 0;  // Which branch (0-4 typically)
    UPROPERTY(BlueprintReadWrite) int32 PositionInTier = 0;

    // Point costs
    UPROPERTY(BlueprintReadWrite) int32 PointCost = 1;
    UPROPERTY(BlueprintReadWrite) int32 MaxRank = 1;
    UPROPERTY(BlueprintReadWrite) int32 CurrentRank = 0;

    // Unlock requirements
    UPROPERTY(BlueprintReadWrite) int32 RequiredLevel = 1;
    UPROPERTY(BlueprintReadWrite) TArray<int32> PrerequisiteTalentIDs;
    UPROPERTY(BlueprintReadWrite) bool bRequiresKeystone = false;
    UPROPERTY(BlueprintReadWrite) int32 RequiredKeystoneID = -1;

    // Mechanical effect
    UPROPERTY(BlueprintReadWrite) FString TriggerEvent = "";     // e.g. "OnHit", "OnDodge", "OnCast"
    UPROPERTY(BlueprintReadWrite) FString ActivationCondition = ""; // e.g. "Target has Burn stacks >= 3"
    UPROPERTY(BlueprintReadWrite) FString EffectDescription = "";   // Exact mechanical effect
    UPROPERTY(BlueprintReadWrite) float EffectValue = 0.0f;
    UPROPERTY(BlueprintReadWrite) float EffectValuePerRank = 0.0f;

    // Resource interactions
    UPROPERTY(BlueprintReadWrite) EResourceType ResourceType = EResourceType::None;
    UPROPERTY(BlueprintReadWrite) float ResourceCost = 0.0f;
    UPROPERTY(BlueprintReadWrite) float ResourceGen = 0.0f;
    UPROPERTY(BlueprintReadWrite) bool bRespectsGCD = true;
    UPROPERTY(BlueprintReadWrite) float CooldownMod = 0.0f; // Negative = shorter CD

    // Proc rules
    UPROPERTY(BlueprintReadWrite) float ProcChance = 0.0f;
    UPROPERTY(BlueprintReadWrite) float InternalCooldown = 0.0f;
    UPROPERTY(BlueprintReadWrite) int32 MaxStacks = 1;

    // Ability interactions
    UPROPERTY(BlueprintReadWrite) TArray<int32> AffectedAbilityIDs;   // Spell IDs this modifies
    UPROPERTY(BlueprintReadWrite) TArray<int32> SynergyTalentIDs;     // Talents that synergize with this
    UPROPERTY(BlueprintReadWrite) TArray<int32> ConflictTalentIDs;    // Mutually exclusive talents
    UPROPERTY(BlueprintReadWrite) int32 UnlockedAbilityID = -1;       // If this talent unlocks a spell

    // Combat behavior
    UPROPERTY(BlueprintReadWrite) FString CombatBehaviorChange = ""; // e.g. "Replaces Light Attack with Empowered Strike"
    UPROPERTY(BlueprintReadWrite) FString RotationPriority = "";      // e.g. "Cast after Burn application"

    // UI
    UPROPERTY(BlueprintReadWrite) FString IconPath = "";
    UPROPERTY(BlueprintReadWrite) FString VFXHook = "";
    UPROPERTY(BlueprintReadWrite) FString AnimationHook = "";
    UPROPERTY(BlueprintReadWrite) FString AudioHook = "";

    // AI compatibility
    UPROPERTY(BlueprintReadWrite) bool bAIUsable = true;
    UPROPERTY(BlueprintReadWrite) FString AIUsageRule = "";

    // PvP
    UPROPERTY(BlueprintReadWrite) float PvPTuningMod = 1.0f;

    // Save
    UPROPERTY(BlueprintReadWrite) int32 SaveSerializationID = 0;
};

USTRUCT(BlueprintType)
struct FTalentBranch
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) FString BranchName = "";
    UPROPERTY(BlueprintReadWrite) FString CombatFantasy = "";
    UPROPERTY(BlueprintReadWrite) FString PrimaryRotation = "";
    UPROPERTY(BlueprintReadWrite) TArray<int32> NodeIDs;
    UPROPERTY(BlueprintReadWrite) int32 KeystoneID = -1;
    UPROPERTY(BlueprintReadWrite) int32 CapstoneID = -1;
};

USTRUCT(BlueprintType)
struct FTalentTree
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) EClass ParentClass = EClass::Bulwark;
    UPROPERTY(BlueprintReadWrite) ETalentLayer Layer = ETalentLayer::ClassFoundation;
    UPROPERTY(BlueprintReadWrite) TArray<FTalentNode> Nodes;
    UPROPERTY(BlueprintReadWrite) TArray<FTalentBranch> Branches;
    UPROPERTY(BlueprintReadWrite) int32 TotalPoints = 0;
    UPROPERTY(BlueprintReadWrite) int32 MaxPoints = 30;
};

USTRUCT(BlueprintType)
struct FBuildArchetype
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) FString BuildName = "";
    UPROPERTY(BlueprintReadWrite) FString Identity = "";
    UPROPERTY(BlueprintReadWrite) FString CentralMechanic = "";
    UPROPERTY(BlueprintReadWrite) EClass Class = EClass::Bulwark;
    UPROPERTY(BlueprintReadWrite) EClassRole Role = EClassRole::DPS;
    UPROPERTY(BlueprintReadWrite) EWeaponStatusEffect PrimaryAffinity = EWeaponStatusEffect::None;
    UPROPERTY(BlueprintReadWrite) EWeaponStatusEffect SecondaryAffinity = EWeaponStatusEffect::None;
    UPROPERTY(BlueprintReadWrite) EGameModeContext PreferredMode = EGameModeContext::AllModes;

    UPROPERTY(BlueprintReadWrite) TArray<int32> AllocatedTalentIDs;
    UPROPERTY(BlueprintReadWrite) TArray<int32> PrimaryAbilityIDs;
    UPROPERTY(BlueprintReadWrite) FString CombatOpener = "";
    UPROPERTY(BlueprintReadWrite) FString NormalRotation = "";
    UPROPERTY(BlueprintReadWrite) FString BurstWindow = "";
    UPROPERTY(BlueprintReadWrite) FString DefensiveRotation = "";
    UPROPERTY(BlueprintReadWrite) FString ResourceStrategy = "";

    UPROPERTY(BlueprintReadWrite) FString Strengths = "";
    UPROPERTY(BlueprintReadWrite) FString Weaknesses = "";
    UPROPERTY(BlueprintReadWrite) FString GroupSynergies = "";

    // Synergy scores
    UPROPERTY(BlueprintReadWrite) float MechanicalCoherence = 0.0f;
    UPROPERTY(BlueprintReadWrite) float TalentAccessibility = 0.0f;
    UPROPERTY(BlueprintReadWrite) float RotationCompleteness = 0.0f;
    UPROPERTY(BlueprintReadWrite) float ResourceSustainability = 0.0f;
    UPROPERTY(BlueprintReadWrite) float OffensiveSynergy = 0.0f;
    UPROPERTY(BlueprintReadWrite) float DefensiveSynergy = 0.0f;
    UPROPERTY(BlueprintReadWrite) float ControlUtility = 0.0f;
    UPROPERTY(BlueprintReadWrite) float EncounterAdaptability = 0.0f;
    UPROPERTY(BlueprintReadWrite) float Counterplay = 0.0f;
    UPROPERTY(BlueprintReadWrite) float BuildDiversity = 0.0f;
};

UCLASS(Blueprintable)
class ECLIPSEROTF_API UEclipseTalentSystem : public UObject
{
    GENERATED_BODY()

public:
    UEclipseTalentSystem();

    // === INITIALIZATION ===
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Talents")
    void InitializeAllTrees();

    // === TALENT QUERIES ===
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Talents")
    TArray<FTalentNode> GetAllTalents() const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Talents")
    FTalentTree GetTree(EClass Class, ETalentLayer Layer) const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Talents")
    TArray<FTalentTree> GetClassTrees(EClass Class) const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Talents")
    FTalentNode GetTalent(int32 TalentID) const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Talents")
    int32 GetTalentCount() const;

    // === POINT ALLOCATION ===
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Talents")
    bool CanAllocateTalent(int32 TalentID, int32 PlayerLevel, int32 AvailablePoints) const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Talents")
    bool AllocateTalent(int32 TalentID);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Talents")
    bool DeallocateTalent(int32 TalentID);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Talents")
    void ResetAllTalents();

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Talents")
    int32 GetAllocatedPoints() const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Talents")
    int32 GetMaxPoints() const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Talents")
    TArray<FTalentNode> GetAllocatedTalents() const;

    // === PREREQUISITE VALIDATION ===
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Talents")
    bool ArePrerequisitesMet(int32 TalentID) const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Talents")
    bool IsMutuallyExclusive(int32 TalentID, int32 OtherTalentID) const;

    // === SYNERGY GRAPH ===
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Talents|Synergy")
    TArray<int32> GetSynergies(int32 TalentID) const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Talents|Synergy")
    TArray<int32> GetConflicts(int32 TalentID) const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Talents|Synergy")
    TArray<int32> GetActiveSynergies() const;

    // === BUILDS ===
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Talents|Builds")
    TArray<FBuildArchetype> GetBuildArchetypes(EClass Class) const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Talents|Builds")
    FBuildArchetype GetBuild(const FString& BuildName) const;

    // === ABILITY UNLOCKS ===
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Talents")
    TArray<int32> GetUnlockedAbilityIDs() const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Talents")
    bool IsAbilityUnlocked(int32 AbilityID) const;

    // === SAVE/LOAD ===
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Talents")
    TMap<int32, int32> GetSavedAllocation() const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Talents")
    void LoadAllocation(const TMap<int32, int32>& InAllocation);

    // === QUALITY GATE VALIDATION ===
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Talents|QA")
    bool ValidateBuildQuality(FString& OutReport) const;

private:
    TArray<FTalentNode> AllTalents;
    TMap<int32, FTalentNode> TalentByID;
    TMap<int32, int32> AllocatedTalents; // TalentID -> Rank
    TArray<FBuildArchetype> AllBuilds;
    int32 MaxTalentPoints = 30;

    // Tree builders
    void InitBulwarkTalents();
    void InitDuskbladeTalents();
    void InitArcanistTalents();
    void InitRiftwalkerTalents();
    void InitDreadknightTalents();
    void InitTemplarTalents();
    void InitSoulbinderTalents();
    void InitWildwardenTalents();
    void InitStormcallerTalents();
    void InitOracleTalents();
    void InitAlchemistTalents();
    void InitBerserkerTalents();
    void InitShadowbladeTalents();
    void InitSpellbladeTalents();
    void InitWarthaneTalents();
    void InitNecroturgeTalents();
    void InitEarthweaverTalents();

    // Build archetypes
    void InitBuildArchetypes();

    // Helpers
    FTalentNode& AddTalent(EClass Class, ETalentLayer Layer, ETalentNodeType Type,
        const FString& Name, int32 Tier, int32 Branch, const FString& Description);
    void AddSynergy(int32 TalentA, int32 TalentB);
    void AddConflict(int32 TalentA, int32 TalentB);
    void SetPrerequisite(int32 ChildID, int32 ParentID);
};
