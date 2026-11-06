// Eclipse Action Bar + Keybind System
// Per V7: native on-screen HUD, cast bars, cooldowns, keybinds, macros
// Per user: action bar with keybinds for all abilities, spells, items, macros

#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EclipseActionBar.generated.h"

#include "EclipseCharacter.h" // For EClass
#include "EclipseClassAbilities.h" // For EAbilityType
#include "EclipseSpellSystem.h" // For ESpellTier, FSpell
#include "EclipseWeaponSwap.h" // For EWeaponStatusEffect
#include "EclipseChatSystem.h" // For macros

UENUM(BlueprintType)
enum class EActionBarSlot : uint8
{
    Slot1   UMETA(DisplayName = "Slot 1 - Key 1"),
    Slot2   UMETA(DisplayName = "Slot 2 - Key 2"),
    Slot3   UMETA(DisplayName = "Slot 3 - Key 3"),
    Slot4   UMETA(DisplayName = "Slot 4 - Key 4"),
    Slot5   UMETA(DisplayName = "Slot 5 - Key 5"),
    Slot6   UMETA(DisplayName = "Slot 6 - Key 6"),
    Slot7   UMETA(DisplayName = "Slot 7 - Key 7"),
    Slot8   UMETA(DisplayName = "Slot 8 - Key 8"),
    Slot9   UMETA(DisplayName = "Slot 9 - Key 9"),
    Slot10  UMETA(DisplayName = "Slot 10 - Key 0"),
    Slot11  UMETA(DisplayName = "Slot 11 - Key Q"),
    Slot12  UMETA(DisplayName = "Slot 12 - Key E"),
    Slot13  UMETA(DisplayName = "Slot 13 - Key R"),
    Slot14  UMETA(DisplayName = "Slot 14 - Key F"),
    Slot15  UMETA(DisplayName = "Slot 15 - Key V"),
    Slot16  UMETA(DisplayName = "Slot 16 - Shift+1"),
    Slot17  UMETA(DisplayName = "Slot 17 - Shift+2"),
    Slot18  UMETA(DisplayName = "Slot 18 - Shift+3"),
    Slot19  UMETA(DisplayName = "Slot 19 - Shift+Q"),
    Slot20  UMETA(DisplayName = "Slot 20 - Shift+E"),
};

UENUM(BlueprintType)
enum class EActionType : uint8
{
    Spell          UMETA(DisplayName = "Spell - cast a spell from spellbook"),
    Ability         UMETA(DisplayName = "Ability - class ability (light/heavy/charged/weapon art)"),
    Item           UMMETA(DisplayName = "Item - use consumable (potion, scroll, reagent)"),
    Macro          UMETA(DisplayName = "Macro - execute a chat macro"),
    WeaponSwap     UMETA(DisplayName = "Weapon Swap - switch weapon slots"),
    Emote           UMETA(DisplayName = "Emote - character animation"),
    Utility         UMETA(DisplayName = "Utility - mount, toy, profession tool"),
};

USTRUCT(BlueprintType)
struct FActionBarEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) EActionBarSlot Slot = EActionBarSlot::Slot1;
    UProperty(BlueprintReadWrite) EActionType ActionType = EActionType::Spell;
    UProperty(BlueprintReadWrite) int32 ActionID = 0; // Spell ID, Ability type, Item ID, or Macro ID
    UProperty(BlueprintReadWrite) FString DisplayName = "";
    UProperty(BlueprintReadWrite) FString Keybind = "";
    UProperty(BlueprintReadWrite) FString IconPath = "";
    UProperty(BlueprintReadWrite) float CooldownRemaining = 0.0f;
    UProperty(BlueprintReadWrite) float CooldownMax = 0.0f;
    UProperty(BlueprintReadWrite) bool bIsOnCooldown = false;
    UProperty(BlueprintReadWrite) bool bIsUsable = true;
    UProperty(BlueprintReadWrite) float ResourceCost = 0.0f;
    UProperty(BlueprintReadWrite) EResourceType ResourceType = EResourceType::Mana;
    UProperty(BlueprintReadWrite) bool bRequiresFlow = false;
    UProperty(BlueprintReadWrite) float FlowCost = 0.0f;
};

USTRUCT(BlueprintType)
struct FKeybind
{
    GENERATED_BODY()

    UProperty(BlueprintReadWrite) FString Action = ""; // e.g. "CastSlot1", "Dodge", "Parry"
    UProperty(BlueprintReadWrite) FString Key = ""; // e.g. "1", "Q", "Space", "LMB"
    UProperty(BlueprintReadWrite) FString Modifier = ""; // e.g. "Shift", "Ctrl", "Alt"
    UProperty(BlueprintReadWrite) FString Label = ""; // e.g. "Fireball", "Dodge Roll"
    UProperty(BlueprintReadWrite) bool bRebindable = true;
};

UCLASS(Blueprintable)
class ECLIPSEROTF_API UEclipseActionBar : public UObject
{
    GENERATED_BODY()

public:
    UEclipseActionBar();

    // === ACTION BAR ===

    // Initialize action bar for a class
    UFUNCTION(BlueprintCallable, Category = "Eclipse|ActionBar")
    void InitializeForClass(EClass CharacterClass);

    // Set a slot to a spell
    UFUNCTION(BlueprintCallable, Category = "Eclipse|ActionBar")
    void SetSpellSlot(EActionBarSlot Slot, int32 SpellID);

    // Set a slot to an ability
    UFUNCTION(BlueprintCallable, Category = "Eclipse|ActionBar")
    void SetAbilitySlot(EActionBarSlot Slot, EAbilityType AbilityType);

    // Set a slot to an item
    UFUNCTION(BlueprintCallable, Category = "Eclipse|ActionBar")
    void SetItemSlot(EActionBarSlot Slot, int32 ItemID, const FString& ItemName);

    // Set a slot to a macro
    UFUNCTION(BlueprintCallable, Category = "Eclipse|ActionBar")
    void SetMacroSlot(EActionBarSlot Slot, int32 MacroID, const FString& MacroName);

    // Set a slot to weapon swap
    UFUNCTION(BlueprintCallable, Category = "Eclipse|ActionBar")
    void SetWeaponSwapSlot(EActionBarSlot Slot);

    // Clear a slot
    UFUNCTION(BlueprintCallable, Category = "Eclipse|ActionBar")
    void ClearSlot(EActionBarSlot Slot);

    // Get all action bar entries
    UFUNCTION(BlueprintCallable, Category = "Eclipse|ActionBar")
    TArray<FActionBarEntry> GetActionBar() const;

    // Get specific slot
    UFUNCTION(BlueprintCallable, Category = "Eclipse|ActionBar")
    FActionBarEntry GetSlot(EActionBarSlot Slot) const;

    // Activate a slot (player pressed the key)
    UFUNCTION(BlueprintCallable, Category = "Eclipse|ActionBar")
    bool ActivateSlot(EActionBarSlot Slot, float CurrentFlow, float CurrentResource);

    // Start cooldown on a slot
    UFUNCTION(BlueprintCallable, Category = "Eclipse|ActionBar")
    void StartCooldown(EActionBarSlot Slot, float Duration);

    // Update cooldowns (call every frame)
    UFUNCTION(BlueprintCallable, Category = "Eclipse|ActionBar")
    void UpdateCooldowns(float DeltaTime);

    // Is slot on cooldown?
    UFUNCTION(BlueprintCallable, Category = "Eclipse|ActionBar")
    bool IsOnCooldown(EActionBarSlot Slot) const;

    // Get cooldown remaining
    UFUNCTION(BlueprintCallable, Category = "Eclipse|ActionBar")
    float GetCooldownRemaining(EActionBarSlot Slot) const;

    // === KEYBINDS ===

    // Initialize default keybinds
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Keybinds")
    void InitializeDefaultKeybinds();

    // Rebind a key
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Keybinds")
    bool RebindKey(const FString& Action, const FString& NewKey, const FString& Modifier = "");

    // Get all keybinds
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Keybinds")
    TArray<FKeybind> GetAllKeybinds() const;

    // Get keybind for an action
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Keybinds")
    FKeybind GetKeybind(const FString& Action) const;

    // Reset to defaults
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Keybinds")
    void ResetKeybinds();

    // === COMBAT ACTIONS (manual, not on action bar) ===

    // These are the manual combat actions from the DamageRotation system
    // They have their own keybinds separate from the action bar

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Keybinds")
    FString GetCombatKeybinds() const;

private:
    TArray<FActionBarEntry> ActionBarSlots; // 20 slots
    TArray<FKeybind> Keybinds;

    // Combat action keybinds (separate from action bar)
    void InitializeCombatKeybinds();

    // Slot index helper
    int32 SlotToIndex(EActionBarSlot Slot) const;
    EActionBarSlot IndexToSlot(int32 Index) const;
};
