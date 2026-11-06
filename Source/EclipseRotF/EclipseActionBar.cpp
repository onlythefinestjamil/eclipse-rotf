// Eclipse ActionBar Implementation
// 20 slots, keybinds, combat actions, macros, weapon swap

#include "EclipseActionBar.h"
#include "Engine/Engine.h"

UEclipseActionBar::UEclipseActionBar()
{
    ActionBarSlots.SetNum(20);
    for (int32 i = 0; i < 20; i++)
    {
        ActionBarSlots[i].Slot = (EActionBarSlot)i;
        ActionBarSlots[i].bIsUsable = false;
        ActionBarSlots[i].CooldownRemaining = 0.0f;
    }
    InitializeDefaultKeybinds();
    InitializeCombatKeybinds();
}

int32 UEclipseActionBar::SlotToIndex(EActionBarSlot Slot) const
{
    return (int32)Slot;
}

EActionBarSlot UEclipseActionBar::IndexToSlot(int32 Index) const
{
    if (Index >= 0 && Index < 20)
        return (EActionBarSlot)Index;
    return EActionBarSlot::Slot1;
}

void UEclipseActionBar::InitializeForClass(EClass CharacterClass)
{
    // Clear all slots
    for (auto& Slot : ActionBarSlots)
    {
        Slot.bIsUsable = false;
        Slot.ActionID = 0;
        Slot.DisplayName = "";
        Slot.CooldownRemaining = 0.0f;
        Slot.bIsOnCooldown = false;
    }

    // Default keybinds per slot
    FString SlotKeys[] = {"1","2","3","4","5","6","7","8","9","0","Q","E","R","F","V","Shift+1","Shift+2","Shift+3","Shift+Q","Shift+E"};
    FString SlotKeyMods[] = {"","","","","","","","","","","","","","","Shift","Shift","Shift","Shift","Shift"};
    FString SlotKeyKeys[] = {"1","2","3","4","5","6","7","8","9","0","Q","E","R","F","V","1","2","3","Q","E"};

    for (int32 i = 0; i < 20; i++)
    {
        ActionBarSlots[i].Keybind = SlotKeys[i];
    }

    // Assign class-specific spells/abilities to slots based on class
    // Slots 1-5: Basic abilities (class signature)
    // Slots 6-10: Spells (class spells)
    // Slots 11-15: Advanced abilities + utility
    // Slots 16-20: Ultimate, weapon art, items, macros, weapon swap

    switch (CharacterClass)
    {
    case EClass::Vanguard:
        ActionBarSlots[0].ActionType = EActionType::Ability;
        ActionBarSlots[0].ActionID = (int32)EAbilityType::Ability1;
        ActionBarSlots[0].DisplayName = "Shield Bash";
        ActionBarSlots[0].bIsUsable = true;
        ActionBarSlots[0].ResourceCost = 20.0f;
        ActionBarSlots[0].ResourceType = EResourceType::Stamina;

        ActionBarSlots[1].ActionType = EActionType::Ability;
        ActionBarSlots[1].ActionID = (int32)EAbilityType::GuardCounter;
        ActionBarSlots[1].DisplayName = "Guard Counter";
        ActionBarSlots[1].bIsUsable = true;
        ActionBarSlots[1].ResourceCost = 25.0f;
        ActionBarSlots[1].ResourceType = EResourceType::Stamina;

        ActionBarSlots[2].ActionType = EActionType::Ability;
        ActionBarSlots[2].ActionID = (int32)EAbilityType::Ability2;
        ActionBarSlots[2].DisplayName = "Shield Wall";
        ActionBarSlots[2].bIsUsable = true;
        ActionBarSlots[2].ResourceCost = 40.0f;
        ActionBarSlots[2].ResourceType = EResourceType::Stamina;

        ActionBarSlots[3].ActionType = EActionType::Ability;
        ActionBarSlots[3].ActionID = (int32)EAbilityType::WeaponArt;
        ActionBarSlots[3].DisplayName = "Shield Charge";
        ActionBarSlots[3].bIsUsable = true;
        ActionBarSlots[3].ResourceCost = 30.0f;
        ActionBarSlots[3].ResourceType = EResourceType::Stamina;

        ActionBarSlots[4].ActionType = EActionType::Ability;
        ActionBarSlots[4].ActionID = (int32)EAbilityType::Ability3;
        ActionBarSlots[4].DisplayName = "War Banner";
        ActionBarSlots[4].bIsUsable = true;

        // Ultimate
        ActionBarSlots[15].ActionType = EActionType::Ability;
        ActionBarSlots[15].ActionID = (int32)EAbilityType::Ultimate;
        ActionBarSlots[15].DisplayName = "Sunbreaker Slam";
        ActionBarSlots[15].bIsUsable = true;
        ActionBarSlots[15].bRequiresFlow = true;
        ActionBarSlots[15].FlowCost = 100.0f;
        break;

    case EClass::UmbralAssassin:
        ActionBarSlots[0].DisplayName = "Shadow Step";
        ActionBarSlots[0].bIsUsable = true;
        ActionBarSlots[0].ResourceCost = 15.0f;
        ActionBarSlots[0].ResourceType = EResourceType::Stamina;

        ActionBarSlots[1].DisplayName = "Backstab";
        ActionBarSlots[1].bIsUsable = true;
        ActionBarSlots[1].ResourceCost = 20.0f;
        ActionBarSlots[1].ResourceType = EResourceType::Stamina;

        ActionBarSlots[2].DisplayName = "Smoke Bomb";
        ActionBarSlots[2].bIsUsable = true;

        ActionBarSlots[3].DisplayName = "Voidrender Daggers";
        ActionBarSlots[3].bIsUsable = true;

        ActionBarSlots[15].DisplayName = "Veilfang Massacre";
        ActionBarSlots[15].bIsUsable = true;
        ActionBarSlots[15].bRequiresFlow = true;
        ActionBarSlots[15].FlowCost = 100.0f;
        break;

    case EClass::Arcanist:
        ActionBarSlots[0].DisplayName = "Aether Lance";
        ActionBarSlots[0].bIsUsable = true;
        ActionBarSlots[0].ResourceCost = 20.0f;
        ActionBarSlots[0].ResourceType = EResourceType::Mana;

        ActionBarSlots[1].DisplayName = "Frost Nova";
        ActionBarSlots[1].bIsUsable = true;
        ActionBarSlots[1].ResourceCost = 30.0f;
        ActionBarSlots[1].ResourceType = EResourceType::Mana;

        ActionBarSlots[2].DisplayName = "Meteor Storm";
        ActionBarSlots[2].bIsUsable = true;
        ActionBarSlots[2].ResourceCost = 40.0f;
        ActionBarSlots[2].ResourceType = EResourceType::Mana;

        ActionBarSlots[3].DisplayName = "Sigil of Fracture";
        ActionBarSlots[3].bIsUsable = true;

        ActionBarSlots[15].DisplayName = "Astral Annihilation";
        ActionBarSlots[15].bIsUsable = true;
        ActionBarSlots[15].bRequiresFlow = true;
        ActionBarSlots[15].FlowCost = 100.0f;
        break;

    // Other classes follow the same pattern - their abilities from EclipseClassAbilities
    default:
        // Generic setup for remaining classes
        for (int32 i = 0; i < 5; i++)
        {
            ActionBarSlots[i].bIsUsable = true;
            ActionBarSlots[i].DisplayName = FString::Printf(TEXT("Ability %d"), i + 1);
        }
        ActionBarSlots[15].DisplayName = "Ultimate";
        ActionBarSlots[15].bIsUsable = true;
        ActionBarSlots[15].bRequiresFlow = true;
        ActionBarSlots[15].FlowCost = 100.0f;
        break;
    }

    // Slot 16-19: Weapon swap + items + macros
    ActionBarSlots[16].ActionType = EActionType::WeaponSwap;
    ActionBarSlots[16].DisplayName = "Weapon Swap";
    ActionBarSlots[16].bIsUsable = true;

    ActionBarSlots[17].ActionType = EActionType::Item;
    ActionBarSlots[17].ActionID = 1;
    ActionBarSlots[17].DisplayName = "Health Potion";
    ActionBarSlots[17].bIsUsable = true;

    ActionBarSlots[18].ActionType = EActionType::Item;
    ActionBarSlots[18].ActionID = 2;
    ActionBarSlots[18].DisplayName = "Stamina Flask";
    ActionBarSlots[18].bIsUsable = true;

    ActionBarSlots[19].ActionType = EActionType::Macro;
    ActionBarSlots[19].ActionID = 1;
    ActionBarSlots[19].DisplayName = "LFM - Raid";
    ActionBarSlots[19].bIsUsable = true;

    UE_LOG(LogTemp, Log, TEXT("[ActionBar] Initialized for class: %s (20 slots)"), *UEnum::GetValueAsString(CharacterClass));
}

void UEclipseActionBar::SetSpellSlot(EActionBarSlot Slot, int32 SpellID)
{
    int32 Index = SlotToIndex(Slot);
    if (ActionBarSlots.IsValidIndex(Index))
    {
        ActionBarSlots[Index].ActionType = EActionType::Spell;
        ActionBarSlots[Index].ActionID = SpellID;
        ActionBarSlots[Index].bIsUsable = true;
        UE_LOG(LogTemp, Log, TEXT("[ActionBar] Slot %d set to spell ID %d"), Index, SpellID);
    }
}

void UEclipseActionBar::SetAbilitySlot(EActionBarSlot Slot, EAbilityType AbilityType)
{
    int32 Index = SlotToIndex(Slot);
    if (ActionBarSlots.IsValidIndex(Index))
    {
        ActionBarSlots[Index].ActionType = EActionType::Ability;
        ActionBarSlots[Index].ActionID = (int32)AbilityType;
        ActionBarSlots[Index].bIsUsable = true;
    }
}

void UEclipseActionBar::SetItemSlot(EActionBarSlot Slot, int32 ItemID, const FString& ItemName)
{
    int32 Index = SlotToIndex(Slot);
    if (ActionBarSlots.IsValidIndex(Index))
    {
        ActionBarSlots[Index].ActionType = EActionType::Item;
        ActionBarSlots[Index].ActionID = ItemID;
        ActionBarSlots[Index].DisplayName = ItemName;
        ActionBarSlots[Index].bIsUsable = true;
    }
}

void UEclipseActionBar::SetMacroSlot(EActionBarSlot Slot, int32 MacroID, const FString& MacroName)
{
    int32 Index = SlotToIndex(Slot);
    if (ActionBarSlots.IsValidIndex(Index))
    {
        ActionBarSlots[Index].ActionType = EActionType::Macro;
        ActionBarSlots[Index].ActionID = MacroID;
        ActionBarSlots[Index].DisplayName = MacroName;
        ActionBarSlots[Index].bIsUsable = true;
    }
}

void UEclipseActionBar::SetWeaponSwapSlot(EActionBarSlot Slot)
{
    int32 Index = SlotToIndex(Slot);
    if (ActionBarSlots.IsValidIndex(Index))
    {
        ActionBarSlots[Index].ActionType = EActionType::WeaponSwap;
        ActionBarSlots[Index].DisplayName = "Weapon Swap";
        ActionBarSlots[Index].bIsUsable = true;
    }
}

void UEclipseActionBar::ClearSlot(EActionBarSlot Slot)
{
    int32 Index = SlotToIndex(Slot);
    if (ActionBarSlots.IsValidIndex(Index))
    {
        ActionBarSlots[Index].bIsUsable = false;
        ActionBarSlots[Index].ActionID = 0;
        ActionBarSlots[Index].DisplayName = "";
        ActionBarSlots[Index].bIsOnCooldown = false;
    }
}

TArray<FActionBarEntry> UEclipseActionBar::GetActionBar() const
{
    return ActionBarSlots;
}

FActionBarEntry UEclipseActionBar::GetSlot(EActionBarSlot Slot) const
{
    int32 Index = SlotToIndex(Slot);
    if (ActionBarSlots.IsValidIndex(Index))
        return ActionBarSlots[Index];
    return FActionBarEntry();
}

bool UEclipseActionBar::ActivateSlot(EActionBarSlot Slot, float CurrentFlow, float CurrentResource)
{
    int32 Index = SlotToIndex(Slot);
    if (!ActionBarSlots.IsValidIndex(Index)) return false;
    if (!ActionBarSlots[Index].bIsUsable) return false;
    if (ActionBarSlots[Index].bIsOnCooldown) return false;

    // Check Flow cost
    if (ActionBarSlots[Index].bRequiresFlow && CurrentFlow < ActionBarSlots[Index].FlowCost)
    {
        UE_LOG(LogTemp, Log, TEXT("[ActionBar] Not enough Flow for %s (need %.0f, have %.0f)"),
            *ActionBarSlots[Index].DisplayName, ActionBarSlots[Index].FlowCost, CurrentFlow);
        return false;
    }

    // Check resource cost
    if (CurrentResource < ActionBarSlots[Index].ResourceCost)
    {
        UE_LOG(LogTemp, Log, TEXT("[ActionBar] Not enough resource for %s (need %.0f)"),
            *ActionBarSlots[Index].DisplayName, ActionBarSlots[Index].ResourceCost);
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("[ActionBar] ACTIVATED: %s (Slot %d)"), *ActionBarSlots[Index].DisplayName, Index);
    return true;
}

void UEclipseActionBar::StartCooldown(EActionBarSlot Slot, float Duration)
{
    int32 Index = SlotToIndex(Slot);
    if (ActionBarSlots.IsValidIndex(Index))
    {
        ActionBarSlots[Index].bIsOnCooldown = true;
        ActionBarSlots[Index].CooldownRemaining = Duration;
        ActionBarSlots[Index].CooldownMax = Duration;
    }
}

void UEclipseActionBar::UpdateCooldowns(float DeltaTime)
{
    for (auto& Slot : ActionBarSlots)
    {
        if (Slot.bIsOnCooldown)
        {
            Slot.CooldownRemaining -= DeltaTime;
            if (Slot.CooldownRemaining <= 0.0f)
            {
                Slot.bIsOnCooldown = false;
                Slot.CooldownRemaining = 0.0f;
            }
        }
    }
}

bool UEclipseActionBar::IsOnCooldown(EActionBarSlot Slot) const
{
    int32 Index = SlotToIndex(Slot);
    if (ActionBarSlots.IsValidIndex(Index))
        return ActionBarSlots[Index].bIsOnCooldown;
    return false;
}

float UEclipseActionBar::GetCooldownRemaining(EActionBarSlot Slot) const
{
    int32 Index = SlotToIndex(Slot);
    if (ActionBarSlots.IsValidIndex(Index))
        return ActionBarSlots[Index].CooldownRemaining;
    return 0.0f;
}

// === KEYBINDS ===

void UEclipseActionBar::InitializeDefaultKeybinds()
{
    Keybinds.Empty();

    // Action bar slots 1-10 (keys 1-0)
    FString NumberKeys[] = {"1","2","3","4","5","6","7","8","9","0"};
    for (int32 i = 0; i < 10; i++)
    {
        FKeybind kb;
        kb.Action = FString::Printf(TEXT("CastSlot%d"), i + 1);
        kb.Key = NumberKeys[i];
        kb.Label = FString::Printf(TEXT("Action Bar Slot %d"), i + 1);
        Keybinds.Add(kb);
    }

    // Action bar slots 11-15 (Q, E, R, F, V)
    FString LetterKeys[] = {"Q","E","R","F","V"};
    for (int32 i = 0; i < 5; i++)
    {
        FKeybind kb;
        kb.Action = FString::Printf(TEXT("CastSlot%d"), i + 11);
        kb.Key = LetterKeys[i];
        kb.Label = FString::Printf(TEXT("Action Bar Slot %d"), i + 11);
        Keybinds.Add(kb);
    }

    // Shift modifiers for slots 16-20
    FString ShiftKeys[] = {"1","2","3","Q","E"};
    for (int32 i = 0; i < 5; i++)
    {
        FKeybind kb;
        kb.Action = FString::Printf(TEXT("CastSlot%d"), i + 16);
        kb.Key = ShiftKeys[i];
        kb.Modifier = "Shift";
        kb.Label = FString::Printf(TEXT("Action Bar Slot %d"), i + 16);
        Keybinds.Add(kb);
    }
}

void UEclipseActionBar::InitializeCombatKeybinds()
{
    // Manual combat actions (not on action bar, separate keybinds)
    FKeybind CombatBinds[] = {
        {"LightAttack", "LMB", "", "Light Attack (R1)", true},
        {"HeavyAttack", "RMB", "", "Heavy Attack (R2)", true},
        {"ChargedAttack", "RMB", "Hold", "Charged Attack (Hold R2)", true},
        {"WeaponArt", "MMB", "", "Weapon Art (L2)", true},
        {"Dodge", "Space", "", "Dodge Roll", true},
        {"Parry", "MMB", "Ctrl", "Parry", true},
        {"Block", "MMB", "Hold", "Block (Hold L1)", true},
        {"GuardCounter", "RMB", "Shift", "Guard Counter", true},
        {"Sprint", "Shift", "Hold", "Sprint", true},
        {"Jump", "Space", "Ctrl", "Jump", true},
        {"AerialAttack", "LMB", "Air", "Aerial Attack", true},
        {"TargetLock", "Tab", "", "Toggle Target Lock", true},
        {"TargetCycle", "Tab", "Shift", "Cycle Targets", true},
        {"AimSpell", "RMB", "Hold", "Aim Spell", true},
        {"SheatheWeapon", "X", "", "Sheathe/Unsheathe Weapon", true},
        {"TwoHandMode", "T", "", "Toggle Two-Handed Grip", true},
    };

    for (const auto& Bind : CombatBinds)
    {
        Keybinds.Add(Bind);
    }

    // Party/raid commands
    FKeybind PartyBinds[] = {
        {"PartyCommandFollow", "F1", "", "Party: Follow Me", true},
        {"PartyCommandAttack", "F2", "", "Party: Attack My Target", true},
        {"PartyCommandHold", "F3", "", "Party: Hold Position", true},
        {"PartyCommandSpread", "F4", "", "Party: Spread Out", true},
        {"PartyCommandRetreat", "F5", "", "Party: Retreat", true},
        {"PartyCommandFocus", "F6", "", "Party: Focus Boss", true},
        {"PartyCommandStack", "F7", "", "Party: Stack Up", true},
        {"PartyCommandMechanic", "F8", "", "Party: Handle Mechanic", true},
    };

    for (const auto& Bind : PartyBinds)
    {
        Keybinds.Add(Bind);
    }

    // UI toggles
    FKeybind UIBinds[] = {
        {"ToggleInventory", "I", "", "Inventory", true},
        {"ToggleCharacter", "C", "", "Character Sheet", true},
        {"ToggleSkills", "K", "", "Skills/Talents", true},
        {"ToggleMap", "M", "", "Map", true},
        {"ToggleQuestLog", "J", "", "Quest Log", true},
        {"ToggleParty", "P", "", "Party Frame", true},
        {"ToggleChat", "Enter", "", "Chat", true},
        {"ToggleSettings", "Esc", "", "Settings Menu", true},
        {"ToggleActionBar", "=", "", "Edit Action Bar", true},
        {"ToggleKeybinds", "Backspace", "", "Keybind Settings", true},
    };

    for (const auto& Bind : UIBinds)
    {
        Keybinds.Add(Bind);
    }

    UE_LOG(LogTemp, Log, TEXT("[ActionBar] %d keybinds initialized"), Keybinds.Num());
}

bool UEclipseActionBar::RebindKey(const FString& Action, const FString& NewKey, const FString& Modifier)
{
    for (auto& kb : Keybinds)
    {
        if (kb.Action == Action)
        {
            kb.Key = NewKey;
            kb.Modifier = Modifier;
            UE_LOG(LogTemp, Log, TEXT("[ActionBar] Rebound %s to %s%s"), *Action, Modifier.IsEmpty() ? TEXT("") : *(Modifier + TEXT("+")), *NewKey);
            return true;
        }
    }
    return false;
}

TArray<FKeybind> UEclipseActionBar::GetAllKeybinds() const
{
    return Keybinds;
}

FKeybind UEclipseActionBar::GetKeybind(const FString& Action) const
{
    for (const auto& kb : Keybinds)
    {
        if (kb.Action == Action)
            return kb;
    }
    return FKeybind();
}

void UEclipseActionBar::ResetKeybinds()
{
    InitializeDefaultKeybinds();
    InitializeCombatKeybinds();
    UE_LOG(LogTemp, Log, TEXT("[ActionBar] Keybinds reset to defaults"));
}

FString UEclipseActionBar::GetCombatKeybinds() const
{
    FString Result = "COMBAT KEYBINDS:\n";
    Result += "  LMB: Light Attack\n";
    Result += "  RMB: Heavy Attack / Hold: Charged Attack\n";
    Result += "  MMB: Weapon Art / Ctrl+MMB: Parry / Hold MMB: Block\n";
    Result += "  Space: Dodge / Ctrl+Space: Jump\n";
    Result += "  Shift: Sprint\n";
    Result += "  Tab: Target Lock / Shift+Tab: Cycle Targets\n";
    Result += "\nACTION BAR (1-0, Q E R F V, Shift+1-3, Shift+Q-E):\n";
    Result += "  1-0: Slots 1-10\n";
    Result += "  Q E R F V: Slots 11-15\n";
    Result += "  Shift+1/2/3/Q/E: Slots 16-20\n";
    Result += "\nPARTY COMMANDS (F1-F8):\n";
    Result += "  F1: Follow Me\n  F2: Attack My Target\n  F3: Hold Position\n";
    Result += "  F4: Spread Out\n  F5: Retreat\n  F6: Focus Boss\n";
    Result += "  F7: Stack Up\n  F8: Handle Mechanic\n";
    Result += "\nUI TOGGLES:\n";
    Result += "  I: Inventory\n  C: Character\n  K: Skills\n  M: Map\n";
    Result += "  J: Quests\n  P: Party\n  Enter: Chat\n  Esc: Settings\n";
    return Result;
}
