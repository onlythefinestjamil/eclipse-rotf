// Eclipse Game Mode — Online + Offline Modes
// Per V4: Story campaign (1-4 co-op), Online PvE, PvP, Offline solo
// Mimics Elden Ring's offline/online structure + WoW's group systems

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "EclipseGameMode.generated.h"

UENUM(BlueprintType)
enum class EGameModeType : uint8
{
    OfflineStory    UMETA(DisplayName = "Offline — Solo Story Campaign"),
    OnlineCoop      UMETA(DisplayName = "Online — Co-op Story (1-4 players)"),
    OnlinePvE       UMETA(DisplayName = "Online — PvE Dungeons/Raids"),
    OnlinePvP       UMETA(DisplayName = "Online — PvP Arena/Battleground"),
    OfflineCouch    UMETA(DisplayName = "Offline — Couch Co-op (Split-screen)"),
    OfflineTraining UMETA(DisplayName = "Offline — Training/Practice"),
};

UENUM(BlueprintType)
enum class EMovementStyle : uint8
{
    Wukong          UMETA(DisplayName = "Wukong — Fluid staff combat, fast transitions, aerial combos"),
    EldenRing       UMETA(DisplayName = "Elden Ring — Weighted commitment, stamina management, deliberate timing"),
    WoW             UMETA(DisplayName = "WoW — Rotation-based, GCD, ability priority, resource management"),
    Hybrid          UMETA(DisplayName = "Hybrid — Wukong fluidity + Elden Ring weight + WoW depth"),
};

UCLASS(Blueprintable)
class ECLIPSEROTF_API AEclipseGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AEclipseGameMode();

    // Current mode
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Mode")
    EGameModeType CurrentMode = EGameModeType::OfflineStory;

    // Movement style (what Aura mimics)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Movement")
    EMovementStyle MovementStyle = EMovementStyle::Hybrid;

    // Online/Offline state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Network")
    bool bIsOnline = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Network")
    bool bIsDedicatedServer = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Network")
    int32 MaxPlayers = 4; // Story co-op max

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Network")
    int32 CurrentPlayers = 1;

    // Couch co-op
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Couch")
    bool bIsCouchCoop = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Couch")
    int32 CouchPlayers = 1; // 1-4 local

    // Movement mimicry targets (for Aura)
    // Wukong: fluid staff combat, fast transitions, aerial combos, transform mechanics
    // Elden Ring: weighted attacks, stamina commitment, dodge i-frames, parry windows, poise
    // WoW: GCD rotations, ability priority, resource management, threat/aggro, roles

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Mode")
    void SetGameMode(EGameModeType Mode);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Mode")
    void SetOnline(bool bOnline);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Movement")
    void SetMovementStyle(EMovementStyle Style);

    // Get movement parameters for current style
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Movement")
    float GetDodgeIFrameDuration() const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Movement")
    float GetParryWindow() const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Movement")
    float GetAttackCommitment() const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Movement")
    float GetGCDForCurrentStyle() const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Movement")
    bool CanCancelAnimation() const;

protected:
    virtual void BeginPlay() override;

    // Movement style parameters
    float Wukong_DodgeIFrames = 0.33f;   // Fast, generous
    float Wukong_ParryWindow = 0.15f;     // Quick
    float Wukong_Commitment = 0.3f;       // Low — can cancel most attacks
    float Wukong_GCD = 0.8f;             // Fast transitions

    float EldenRing_DodgeIFrames = 0.27f; // Precise, must time
    float EldenRing_ParryWindow = 0.12f;  // Tight
    float EldenRing_Commitment = 0.7f;    // High — committed to attack
    float EldenRing_GCD = 1.2f;          // Deliberate

    float WoW_DodgeIFrames = 0.2f;        // Minimal — not primary defense
    float WoW_ParryWindow = 0.1f;         // Very tight
    float WoW_Commitment = 0.9f;         // Very high — ability locked
    float WoW_GCD = 1.5f;                // Standard GCD

    // Hybrid = best attributes from all three combined
    // Wukong: fluid transitions, aerial combos, cancel windows, generous dodge
    // Elden Ring: weighted impact, poise/stagger, precise parry, stamina commitment, weapon arts
    // WoW: GCD rotations, ability priority, resource management, threat/aggro, role synergy

    float Hybrid_DodgeIFrames = 0.32f;      // Wukong's generosity + Elden Ring's precision
    float Hybrid_ParryWindow = 0.15f;        // Wukong's window — readable but tight
    float Hybrid_Commitment = 0.45f;        // Wukong fluidity (can cancel windup) + Elden Ring weight (committed during active)
    float Hybrid_GCD = 1.0f;               // WoW's rotation structure + Wukong's speed
    float Hybrid_StaminaRegen = 25.0f;     // Elden Ring's stamina management
    float Hybrid_AerialCombo = true;       // Wukong's aerial combat
    float Hybrid_WeaponArts = true;        // Elden Ring's L2 weapon skills
    float Hybrid_RotationPriority = true;   // WoW's ability priority system
    float Hybrid_ThreatSystem = true;      // WoW's aggro management
    float Hybrid_RoleSynergy = true;       // WoW's class synergies
    float Hybrid_TransformMechanics = true; // Wukong's transform stance changes
    float Hybrid_PoiseBreak = true;        // Elden Ring's poise/stagger system
    float Hybrid_BloodborneDash = false;    // Quickstep option (unlockable)
};
