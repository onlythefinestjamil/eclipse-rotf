// Eclipse Boss Director — Each boss is a miniature game
// V6: 14 bosses, each with phases, telegraphs, mechanics, enrage, rewards
// Per directive: boss art, animations, attacks, AI, environment, music, difficulty, testing

#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "EclipseCharacter.h"
#include "EclipseLootSystem.h" // For ERarity, FLootDrop
#include "EclipseSpellSystem.h" // For FSpell
#include "EclipseBossDirector.generated.h"

UENUM(BlueprintType)
enum class EBossPhase : uint8
{
    Phase1     UMETA(DisplayName = "Phase 1 — Opening (100-75% HP)"),
    Phase2     UMETA(DisplayName = "Phase 2 — Escalation (75-50% HP)"),
    Phase3     UMETA(DisplayName = "Phase 3 — Desperation (50-25% HP)"),
    Phase4     UMETA(DisplayName = "Phase 4 — Final Stand (25-0% HP)"),
    Transition  UMETA(DisplayName = "Phase Transition — Cinematic/Mechanic Shift"),
    Enraged     UMETA(DisplayName = "Enraged — Timer Expired, Max Difficulty"),
};

UENUM(BlueprintType)
enum class EBossMechanic : uint8
{
    None               UMETA(DisplayName = "None"),
    TelegraphedAoE     UMETA(DisplayName = "Telegraphed AoE — Ground circle, dodge or die"),
    TelegraphedCone    UMETA(DisplayName = "Telegraphed Cone — Frontal, sidestep"),
    TelegraphedLine    UMETA(DisplayName = "Telegraphed Line — Charge/laser, dodge sideways"),
    AddPhase           UMETA(DisplayName = "Add Phase — Summon minions, burn adds or boss"),
    TankSwap           UMETA(DisplayName = "Tank Swap — 2 tanks required, debuff swap at stacks"),
    MechanicAssignment UMETA(DisplayName = "Mechanic Assignment — Party members assigned to pillars/levers"),
    SpreadMechanic      UMETA(DisplayName = "Spread — Party spreads, avoid chaining damage"),
    StackMechanic       UMETA(DisplayName = "Stack — Party stacks, shared damage"),
    SilenceMechanic     UMETA(DisplayName = "Silence — No spells for duration, melee only"),
    ArenaChange         UMETA(DisplayName = "Arena Change — Floor collapses or environment shifts"),
    TimeManipulation    UMETA(DisplayName = "Time Manipulation — Boss slows/reverses time"),
    FractureShift       UMETA(DisplayName = "Fracture Shift — Arena shifts to alternate reality"),
    DPSCheck            UMETA(DisplayName = "DPS Check — Burn boss or wipe"),
    HealCheck           UMETA(DisplayName = "Heal Check — Survive massive AoE, heal through it"),
    Untargetable        UMETA(DisplayName = "Untargetable — Boss immune, handle mechanic first"),
};

USTRUCT(BlueprintType)
struct FBossPhaseConfig
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) EBossPhase Phase = EBossPhase::Phase1;
    UPROPERTY(BlueprintReadWrite) float HPThreshold = 100.0f; // % HP when this phase starts
    UPROPERTY(BlueprintReadWrite) float DamageMultiplier = 1.0f;
    UPROPERTY(BlueprintReadWrite) float AttackSpeedMultiplier = 1.0f;
    UPROPERTY(BlueprintReadWrite) float MovementSpeedMultiplier = 1.0f;
    UPROPERTY(BlueprintReadWrite) TArray<EBossMechanic> Mechanics;
    UPROPERTY(BlueprintReadWrite) float EnrageTimer = -1.0f; // -1 = no enrage in this phase
    UPROPERTY(BlueprintReadWrite) FString PhaseName = "";
    UPROPERTY(BlueprintReadWrite) FString TransitionCinematic = ""; // Cinematic name for transition
    UPROPERTY(BlueprintReadWrite) FString MusicCue = ""; // Music cue ID for this phase
};

USTRUCT(BlueprintType)
struct FBossAttack
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) FString AttackName = "";
    UPROPERTY(BlueprintReadWrite) EBossMechanic TelegraphType = EBossMechanic::None;
    UPROPERTY(BlueprintReadWrite) float Damage = 50.0f;
    UPROPERTY(BlueprintReadWrite) float TelegraphDuration = 2.0f; // Seconds of warning before hit
    UPROPERTY(BlueprintReadWrite) float CastTime = 1.0f;
    UPROPERTY(BlueprintReadWrite) float Cooldown = 8.0f;
    UPROPERTY(BlueprintReadWrite) float Range = 500.0f;
    UPROPERTY(BlueprintReadWrite) float Radius = 300.0f; // 0 = single target
    UPROPERTY(BlueprintReadWrite) bool bIsAvoidable = true;
    UPROPERTY(BlueprintReadWrite) bool bRequiresTank = false; // Must be tanked/mitigated
    UPROPERTY(BlueprintReadWrite) bool bRequiresHealer = false; // Requires healing through
    UPROPERTY(BlueprintReadWrite) bool bRequiresInterrupt = false; // Must be interrupted
    UPROPERTY(BlueprintReadWrite) bool bAppliesDebuff = false;
    UPROPERTY(BlueprintReadWrite) EWeaponStatusEffect DebuffType = EWeaponStatusEffect::None;
    UPROPERTY(BlueprintReadWrite) float DebuffDuration = 5.0f;
    UPROPERTY(BlueprintReadWrite) bool bIsPhaseTransitionAttack = false;
    UPROPERTY(BlueprintReadWrite) FString VFXAsset = ""; // Niagara system path
    UPROPERTY(BlueprintReadWrite) FString SoundCue = ""; // Sound cue path
};

USTRUCT(BlueprintType)
struct FBoss
{
    GENERATED_BODY()

    // Identity
    UPROPERTY(BlueprintReadWrite) int32 BossID = 0;
    UPROPERTY(BlueprintReadWrite) FString BossName = "";
    UPROPERTY(BlueprintReadWrite) FString Title = ""; // e.g. "The Fractured Sovereign"
    UPROPERTY(BlueprintReadWrite) FString Description = "";
    UPROPERTY(BlueprintReadWrite) FString ArenaName = "";
    UPROPERTY(BlueprintReadWrite) FString ZoneName = "";

    // Stats
    UPROPERTY(BlueprintReadWrite) float MaxHealth = 50000.0f;
    UPROPERTY(BlueprintReadWrite) float Armor = 100.0f;
    UPROPERTY(BlueprintReadWrite) float Resistance = 50.0f; // Magic resistance
    UPROPERTY(BlueprintReadWrite) float MovementSpeed = 400.0f;

    // Difficulty
    UPROPERTY(BlueprintReadWrite) int32 RecommendedPlayerLevel = 1;
    UPROPERTY(BlueprintReadWrite) int32 MinPartySize = 1;
    UPROPERTY(BlueprintReadWrite) int32 MaxPartySize = 10;
    UPROPERTY(BlueprintReadWrite) bool bIsRaidBoss = false;

    // Phases
    UPROPERTY(BlueprintReadWrite) TArray<FBossPhaseConfig> Phases;
    UPROPERTY(BlueprintReadWrite) int32 CurrentPhaseIndex = 0;

    // Attacks (per phase)
    TArray<TArray<FBossAttack>> PhaseAttacks; // [Phase][Attack]

    // Enrage
    UPROPERTY(BlueprintReadWrite) float EnrageTimerTotal = 600.0f; // 10 min default
    UPROPERTY(BlueprintReadWrite) float EnrageTimerRemaining = 600.0f;
    UPROPERTY(BlueprintReadWrite) bool bIsEnraged = false;

    // Loot
    UPROPERTY(BlueprintReadWrite) TArray<FLootDrop> GuaranteedDrops;
    UPROPERTY(BlueprintReadWrite) TArray<FLootDrop> RandomDrops;
    UPROPERTY(BlueprintReadWrite) int32 MinDrops = 2;
    UPROPERTY(BlueprintReadWrite) int32 MaxDrops = 5;

    // Fracture mechanic
    UPROPERTY(BlueprintReadWrite) bool bUsesFractureShift = false;
    UPROPERTY(BlueprintReadWrite) int32 FracturePhaseTrigger = 3; // Phase that triggers fracture

    // Quest flags
    UPROPERTY(BlueprintReadWrite) FString RequiredQuestFlag = ""; // Quest that unlocks this boss
    UPROPERTY(BlueprintReadWrite) FString KillQuestFlag = ""; // Quest flag set on kill

    // Visual/Audio
    UPROPERTY(BlueprintReadWrite) FString CharacterModelPath = "";
    UPROPERTY(BlueprintReadWrite) FString MusicTrack = "";
    UPROPERTY(BlueprintReadWrite) FString IntroCinematic = "";
    UPROPERTY(BlueprintReadWrite) FString DeathCinematic = "";
};

UCLASS(Blueprintable)
class ECLIPSEROTF_API UEclipseBossDirector : public UObject
{
    GENERATED_BODY()

public:
    UEclipseBossDirector();

    // Initialize all 14 bosses from V6
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Boss")
    void InitializeAllBosses();

    // Get boss by ID
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Boss")
    FBoss GetBoss(int32 BossID) const;

    // Get all bosses
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Boss")
    TArray<FBoss> GetAllBosses() const;

    // Get boss count
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Boss")
    int32 GetBossCount() const;

    // Get bosses by zone
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Boss")
    TArray<FBoss> GetBossesByZone(const FString& ZoneName) const;

    // Get raid bosses only
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Boss")
    TArray<FBoss> GetRaidBosses() const;

    // === COMBAT SIMULATION ===

    // Start a boss encounter
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Boss")
    void StartEncounter(int32 BossID, int32 PartySize);

    // Update boss state (call every frame)
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Boss")
    void UpdateBoss(float DeltaTime, float PlayerHP, float TankHP, float HealerHP);

    // Boss takes damage
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Boss")
    float BossTakeDamage(float Damage, bool bIsCrit, bool bIsMagic);

    // Get current phase
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Boss")
    EBossPhase GetCurrentPhase() const;

    // Get current HP percentage
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Boss")
    float GetHPPct() const;

    // Get current enrage timer
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Boss")
    float GetEnrageTimer() const;

    // Is boss dead?
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Boss")
    bool IsBossDead() const;

    // Is boss enraged?
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Boss")
    bool IsEnraged() const;

    // Get current phase attacks
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Boss")
    TArray<FBossAttack> GetCurrentPhaseAttacks() const;

    // Get current active mechanic
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Boss")
    EBossMechanic GetActiveMechanic() const;

    // Get next telegraphed attack
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Boss")
    FBossAttack GetNextAttack() const;

    // Get boss loot on death
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Boss")
    TArray<FLootDrop> GetLoot() const;

    // Reset boss for retry
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Boss")
    void ResetBoss();

    // Get active boss
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Boss")
    FBoss GetActiveBoss() const;

    // Is encounter active?
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Boss")
    bool IsEncounterActive() const;

private:
    TArray<FBoss> AllBosses;
    FBoss ActiveBoss;
    bool bEncounterActive = false;
    float CurrentHP = 0.0f;
    int32 CurrentPhaseIdx = 0;
    EBossMechanic ActiveMechanic = EBossMechanic::None;
    FBossAttack NextAttack;
    float AttackTimer = 0.0f;
    float MechanicTimer = 0.0f;
    bool bBossDead = false;

    void InitSolaraBosses();
    void InitVaelwoodBosses();
    void InitMorvathBosses();
    void InitAetherisBosses();
    void InitNhalorBosses();
    void InitUmbryssBosses();

    void CheckPhaseTransition();
    void RollNextAttack();
};
