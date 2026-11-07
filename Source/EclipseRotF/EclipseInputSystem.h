// Eclipse Enhanced Input System — Action definitions + mappings
// Per Day 1 Directive: hybrid combat, responsive movement, manual mastery
// Per user: WoW-style action bars + GCD system integrated

#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "EclipseActionBar.h"      // 20-slot action bar
#include "EclipseDamageRotation.h" // GCD, manual attacks, ability cooldowns
#include "EclipseInputSystem.generated.h"

UENUM(BlueprintType)
enum class EInputContext : uint8
{
    Menu           UMETA(DisplayName = "Menu — Navigation, selection, back"),
    Locomotion     UMETA(DisplayName = "Locomotion — Movement, camera, jump, sprint"),
    Combat          UMETA(DisplayName = "Combat — Light, heavy, dodge, parry, abilities"),
    Spellcasting    UMETA(DisplayName = "Spellcasting — Aim, cast, spell slots"),
    PartyCommand    UMETA(DisplayName = "Party Command — Issue orders to AI companions"),
    UI              UMETA(DisplayName = "UI — Inventory, map, quests, settings"),
    Dialogue        UMETA(DisplayName = "Dialogue — Talk, choose, skip"),
};

// GCD state — per Day 1 directive, GCD governs abilities but NOT basic attacks/dodge/block
UENUM(BlueprintType)
enum class EGCDState : uint8
{
    Ready         UMETA(DisplayName = "Ready — GCD available, all abilities usable"),
    Cooldown      UMETA(DisplayName = "On Cooldown — GCD active, abilities locked"),
    Hardlocked    UMETA(DisplayName = "Hardlocked — Animation commitment, nothing usable"),
};

USTRUCT(BlueprintType)
struct FEclipseInputAction
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) FString ActionName = "";
    UPROPERTY(BlueprintReadWrite) FString DefaultKey = "";
    UPROPERTY(BlueprintReadWrite) FString Modifier = "";
    UPROPERTY(BlueprintReadWrite) FString Description = "";
    UPROPERTY(BlueprintReadWrite) bool bIsContinuous = false;
    UPROPERTY(BlueprintReadWrite) bool bIsRebindable = true;
    UPROPERTY(BlueprintReadWrite) EInputContext Context = EInputContext::Locomotion;
    
    // GCD rules — per Day 1 directive
    UPROPERTY(BlueprintReadWrite) bool bIsGCDLocked = true;    // If true, GCD blocks this action
    UPROPERTY(BlueprintReadWrite) bool bIsBasicAttack = false;  // If true, NOT GCD locked — always responsive
    UPROPERTY(BlueprintReadWrite) bool bIsReactive = false;     // Dodge/block/parry — NOT GCD locked
};

UCLASS(Blueprintable)
class ECLIPSEROTF_API UEclipseInputSystem : public UObject
{
    GENERATED_BODY()

public:
    UEclipseInputSystem();

    // === INPUT ACTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Input")
    TArray<FEclipseInputAction> GetAllActions() const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Input")
    TArray<FEclipseInputAction> GetActionsByContext(EInputContext Context) const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Input")
    FString GetKeybindReference() const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Input")
    FString GetMappingContextName(EInputContext Context) const;

    // === GCD SYSTEM ===
    // Per Day 1: GCD governs eligible abilities. Basic attacks, dodges, blocks,
    // and appropriately flagged reactions remain responsive.
    
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Input|GCD")
    void TriggerGCD(float Duration);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Input|GCD")
    void UpdateGCD(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Input|GCD")
    bool IsGCDActive() const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Input|GCD")
    float GetGCDRemaining() const;

    // Can an action be used right now? (GCD check + animation lock check)
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Input|GCD")
    bool CanUseAction(const FString& ActionName) const;

    // Try to use an action — returns false if GCD-blocked
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Input|GCD")
    bool TryUseAction(const FString& ActionName);

    // === ACTION BAR INTEGRATION ===
    // Action bar slots 1-20 map to keys. GCD governs spell/ability slots.
    // Combat actions (light/heavy/dodge/parry) bypass GCD.
    
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Input|ActionBar")
    bool TryActivateSlot(EActionBarSlot Slot);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Input|ActionBar")
    void BindActionBar(UEclipseActionBar* InActionBar);

    // === ANIMATION LOCK ===
    // Heavy attacks have commitment — locks input during animation
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Input")
    void SetAnimationLock(float Duration);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Input")
    bool IsAnimationLocked() const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Input")
    void ClearAnimationLock();

private:
    TArray<FEclipseInputAction> AllActions;
    float GCDRemaining = 0.0f;
    float GCDDuration = 1.0f;
    bool bGCDActive = false;
    float AnimationLockRemaining = 0.0f;
    bool bAnimationLocked = false;
    UEclipseActionBar* ActionBar = nullptr;

    void InitializeActions();
};
