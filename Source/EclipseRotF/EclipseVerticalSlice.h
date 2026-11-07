// EclipseVerticalSlice.h - Ties all systems together for the first playable slice
// Per V7 Gate 2: rigged character, tested hair + armor, weapon animations, one cast/impact effect, playable level

#pragma once
#include "CoreMinimal.h"
#include "EclipseCharacter.h"
#include "GameFramework/GameModeBase.h"
#include "EclipseVerticalSlice.generated.h"

// Forward declarations
class UEclipseCombatSystem;
class UEclipseDamageRotation;
class UEclipseTargetingSystem;
class UEclipseWeaponSwap;
class UEclipseHUDSystem;
class UEclipseSaveSystem;
class UEclipseEnemyAI;
class UEclipseLootSystem;
class UEclipseClassRoles;
class UEclipseGameMode;
class UEclipseChatSystem;
class UEclipseDRSystem;

UENUM(BlueprintType)
enum class EVerticalSlicePhase : uint8
{
    PreProduction   UMETA(DisplayName = "Pre-Production - planning, no engine work"),
    CharacterSetup  UMETA(DisplayName = "Character Setup - rig, import, test"),
    LevelBuild      UMETA(DisplayName = "Level Build - Gothic City microlevel"),
    CombatSetup     UMETA(DisplayName = "Combat Setup - one enemy, one weapon, one spell"),
    HUDSetup         UMETA(DisplayName = "HUD Setup - health, flow, cooldowns"),
    SaveSetup        UMETA(DisplayName = "Save Setup - checkpoints, progression"),
    Playtest         UMETA(DisplayName = "Playtest - verify everything works"),
    Polish           UMETA(DisplayName = "Polish - lighting, VFX, audio"),
};

UCLASS(Blueprintable)
class ECLIPSEROTF_API AEclipseVerticalSlice : public AGameModeBase
{
    GENERATED_BODY()

public:
    AEclipseVerticalSlice();

    // Current phase
    UPROPERTY(BlueprintReadWrite, Category = "Eclipse|Slice")
    EVerticalSlicePhase CurrentPhase = EVerticalSlicePhase::PreProduction;

    // All systems
    UPROPERTY(BlueprintReadWrite, Category = "Eclipse|Slice")
    AEclipseCharacter* CharacterSystem = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Eclipse|Slice")
    UEclipseCombatSystem* CombatSystem = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Eclipse|Slice")
    UEclipseDamageRotation* DamageRotation = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Eclipse|Slice")
    UEclipseTargetingSystem* TargetingSystem = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Eclipse|Slice")
    UEclipseWeaponSwap* WeaponSwap = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Eclipse|Slice")
    UEclipseHUDSystem* HUDSystem = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Eclipse|Slice")
    UEclipseSaveSystem* SaveSystem = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Eclipse|Slice")
    UEclipseEnemyAI* EnemyAI = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Eclipse|Slice")
    UEclipseLootSystem* LootSystem = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Eclipse|Slice")
    UEclipseClassRoles* ClassRoles = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Eclipse|Slice")
    UEclipseChatSystem* ChatSystem = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Eclipse|Slice")
    UEclipseDRSystem* DRSystem = nullptr;

    // Initialize all systems
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Slice")
    void InitializeAllSystems();

    // Advance to next phase
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Slice")
    void AdvancePhase();

    // Get current phase as string
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Slice")
    FString GetCurrentPhaseName() const;

    // Vertical slice checklist
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Slice")
    FString GetChecklistStatus() const;

protected:
    virtual void BeginPlay() override;

private:
    void SetupCharacter();
    void SetupCombat();
    void SetupLevel();
    void SetupEnemy();
    void SetupHUD();
    void SetupSave();
    void SetupLoot();
};
