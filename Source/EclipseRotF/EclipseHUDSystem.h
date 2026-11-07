// Eclipse HUD System - Health, Stamina, Mana, Rage, Aether, Fracture Flow
// Per V7: native on-screen HUD, cast bars, cooldowns, debuffs, DR categories, combat logs

#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EclipseHUDSystem.generated.h"

// EResourceType is defined in EclipseCharacter.h

UENUM(BlueprintType)
enum class EHUDWidget : uint8
{
    HealthBar        UMETA(DisplayName = "Health Bar - bottom center"),
    StaminaBar       UMETA(DisplayName = "Stamina Bar - bottom center, under health"),
    ManaBar          UMETA(DisplayName = "Mana Bar - bottom center, under stamina"),
    RageBar          UMETA(DisplayName = "Rage Bar - bottom center, fills red"),
    AetherBar        UMETA(DisplayName = "Aether Bar - bottom center, fills violet"),
    FlowMeter        UMETA(DisplayName = "Fracture Flow Meter - circular gauge, bottom center"),
    CastBar          UMETA(DisplayName = "Cast Bar - center screen, spell cast progress"),
    CooldownIcons    UMETA(DisplayName = "Cooldown Icons - bottom center, ability cooldowns"),
    DebuffTracker    UMETA(DisplayName = "Debuff Tracker - top center, active debuffs"),
    DRTracker        UMETA(DisplayName = "DR Tracker - top center, diminishing returns"),
    CombatLog        UMETA(DisplayName = "Combat Log - right side, scrollable"),
    TargetInfo       UMETA(DisplayName = "Target Info - top center, enemy health + weak points"),
    PartyFrame       UMETA(DisplayName = "Party Frame - left side, 5-player group"),
    RaidFrame        UMETA(DisplayName = "Raid Frame - left side, 10-player group"),
    BossHealth       UMETA(DisplayName = "Boss Health - top center, large bar"),
    Minimap          UMETA(DisplayName = "Minimap - top right"),
    InventoryGrid    UMETA(DisplayName = "Inventory Grid - tab to open"),
    CharacterSheet   UMETA(DisplayName = "Character Sheet - tab to open"),
    SkillTree        UMETA(DisplayName = "Skill Tree - tab to open"),
    ChatBox          UMETA(DisplayName = "Chat Box - bottom left, 8 channels"),
    Notification     UMETA(DisplayName = "Notifications - top right, loot/exp/quest"),
};

USTRUCT(BlueprintType)
struct FResourceBar
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) EResourceType Type = EResourceType::Health;
    UPROPERTY(BlueprintReadWrite) float Current = 100.0f;
    UPROPERTY(BlueprintReadWrite) float Max = 100.0f;
    UPROPERTY(BlueprintReadWrite) float RegenRate = 5.0f; // per second
    UPROPERTY(BlueprintReadWrite) float RegenDelay = 2.0f; // delay after use
    UPROPERTY(BlueprintReadWrite) FLinearColor BarColor = FLinearColor::White;
    UPROPERTY(BlueprintReadWrite) bool bVisible = true;
};

USTRUCT(BlueprintType)
struct FCooldownEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) FString AbilityName = "";
    UPROPERTY(BlueprintReadWrite) float CooldownRemaining = 0.0f;
    UPROPERTY(BlueprintReadWrite) float CooldownMax = 5.0f;
    UPROPERTY(BlueprintReadWrite) int32 SlotIndex = 0;
    UPROPERTY(BlueprintReadWrite) UTexture2D* Icon = nullptr;
};

USTRUCT(BlueprintType)
struct FDebuffEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) FString DebuffName = "";
    UPROPERTY(BlueprintReadWrite) float DurationRemaining = 0.0f;
    UPROPERTY(BlueprintReadWrite) float Stacks = 1.0f;
    UPROPERTY(BlueprintReadWrite) EWeaponStatusEffect ElementType = EWeaponStatusEffect::None;
    UPROPERTY(BlueprintReadWrite) FLinearColor IconColor = FLinearColor::Red;
};

UCLASS(Blueprintable)
class ECLIPSEROTF_API UEclipseHUDSystem : public UObject
{
    GENERATED_BODY()

public:
    UEclipseHUDSystem();

    // Resource bars
    UFUNCTION(BlueprintCallable, Category = "Eclipse|HUD")
    void UpdateResource(EResourceType Type, float Current, float Max);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|HUD")
    FResourceBar GetResource(EResourceType Type) const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|HUD")
    void SetResourceRegen(EResourceType Type, float RegenRate, float RegenDelay);

    // Fracture Flow meter
    UFUNCTION(BlueprintCallable, Category = "Eclipse|HUD")
    void UpdateFlow(float Current, float Max);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|HUD")
    bool IsFlowFull() const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|HUD")
    void TriggerFlowFinisher();

    // Cooldowns
    UFUNCTION(BlueprintCallable, Category = "Eclipse|HUD")
    void StartCooldown(int32 Slot, const FString& AbilityName, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|HUD")
    float GetCooldownRemaining(int32 Slot) const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|HUD")
    bool IsOnCooldown(int32 Slot) const;

    // Debuffs
    UFUNCTION(BlueprintCallable, Category = "Eclipse|HUD")
    void AddDebuff(const FString& Name, float Duration, EWeaponStatusEffect Element, float Stacks = 1);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|HUD")
    void RemoveDebuff(const FString& Name);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|HUD")
    TArray<FDebuffEntry> GetActiveDebuffs() const;

    // Cast bar
    UFUNCTION(BlueprintCallable, Category = "Eclipse|HUD")
    void StartCastBar(const FString& SpellName, float CastTime);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|HUD")
    void UpdateCastBar(float CurrentTime);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|HUD")
    void CancelCastBar();

    UFUNCTION(BlueprintCallable, Category = "Eclipse|HUD")
    bool IsCasting() const;

    // Combat log
    UFUNCTION(BlueprintCallable, Category = "Eclipse|HUD")
    void AddCombatLogEntry(const FString& Entry);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|HUD")
    TArray<FString> GetCombatLog(int32 MaxEntries = 20) const;

    // Target info
    UFUNCTION(BlueprintCallable, Category = "Eclipse|HUD")
    void SetTargetInfo(const FString& Name, float Health, float MaxHealth, EWeakPointType WeakPoint, bool bIsBoss);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|HUD")
    void ClearTargetInfo();

    // Party/Raid
    UFUNCTION(BlueprintCallable, Category = "Eclipse|HUD")
    void SetPartySize(int32 Size);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|HUD")
    void UpdatePartyMember(int32 Index, const FString& Name, float Health, float MaxHealth, int32 Role);

    // Boss health
    UFUNCTION(BlueprintCallable, Category = "Eclipse|HUD")
    void SetBossHealth(const FString& BossName, float Current, float Max);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|HUD")
    void ShowBossHealth(bool bShow);

    // HUD visibility
    UFUNCTION(BlueprintCallable, Category = "Eclipse|HUD")
    void SetHUDVisible(bool bVisible);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|HUD")
    void SetWidgetVisible(EHUDWidget Widget, bool bVisible);

    // Notification
    UFUNCTION(BlueprintCallable, Category = "Eclipse|HUD")
    void ShowNotification(const FString& Text, float Duration = 3.0f);

private:
    TMap<EResourceType, FResourceBar> Resources;
    TArray<FCooldownEntry> Cooldowns;
    TArray<FDebuffEntry> ActiveDebuffs;
    TArray<FString> CombatLog;
    bool bCastInProgress = false;
    FString CurrentCastSpell = "";
    float CastProgress = 0.0f;
    float CastMaxTime = 0.0f;
    float FlowCurrent = 0.0f;
    float FlowMax = 100.0f;
    bool bHUDVisible = true;
    bool bBossHealthVisible = false;
    FString BossName = "";
    float BossHealthCurrent = 0.0f;
    float BossHealthMax = 100.0f;

    void InitializeResources();
};
