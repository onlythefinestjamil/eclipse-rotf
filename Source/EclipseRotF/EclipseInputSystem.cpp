// Eclipse Input System Implementation
// GCD governs abilities — basic attacks, dodges, blocks stay responsive
// WoW-style action bar with 20 slots + combat keybinds

#include "EclipseInputSystem.h"
#include "Engine/Engine.h"

UEclipseInputSystem::UEclipseInputSystem()
{
    GCDRemaining = 0.0f;
    bGCDActive = false;
    AnimationLockRemaining = 0.0f;
    bAnimationLocked = false;
    ActionBar = nullptr;
    InitializeActions();
}

void UEclipseInputSystem::InitializeActions()
{
    // === LOCOMOTION (always responsive, never GCD locked) ===
    AllActions.Add({"MoveForward", "W", "", "Move forward", true, true, EInputContext::Locomotion, false, false, true});
    AllActions.Add({"MoveBackward", "S", "", "Move backward", true, true, EInputContext::Locomotion, false, false, true});
    AllActions.Add({"MoveLeft", "A", "", "Strafe left", true, true, EInputContext::Locomotion, false, false, true});
    AllActions.Add({"MoveRight", "D", "", "Strafe right", true, true, EInputContext::Locomotion, false, false, true});
    AllActions.Add({"Jump", "Space", "", "Jump", false, true, EInputContext::Locomotion, false, false, true});
    AllActions.Add({"Sprint", "Shift", "", "Sprint (hold)", true, true, EInputContext::Locomotion, false, false, true});
    AllActions.Add({"CameraLook", "Mouse", "", "Camera look", true, false, EInputContext::Locomotion, false, false, true});

    // === COMBAT (basic attacks bypass GCD, abilities respect GCD) ===
    AllActions.Add({"LightAttack", "LMB", "", "Light attack — fast, low damage, NOT GCD locked", false, true, EInputContext::Combat, false, true, false});
    AllActions.Add({"HeavyAttack", "RMB", "", "Heavy attack — committed, high damage, NOT GCD locked but animation locked", false, true, EInputContext::Combat, false, true, false});
    AllActions.Add({"ChargedAttack", "RMB", "Hold", "Charged attack — hold RMB, very high damage", true, true, EInputContext::Combat, false, true, false});
    AllActions.Add({"WeaponArt", "MMB", "", "Weapon art — L2, unique per weapon, GCD locked", false, true, EInputContext::Combat, true, false, false});
    AllActions.Add({"Dodge", "Space", "", "Dodge roll — i-frames, NOT GCD locked, reactive", false, true, EInputContext::Combat, false, false, true});
    AllActions.Add({"Parry", "MMB", "Ctrl", "Parry — 0.15s window, NOT GCD locked, reactive", false, true, EInputContext::Combat, false, false, true});
    AllActions.Add({"Block", "MMB", "Hold", "Block — hold, drains stamina, NOT GCD locked, reactive", true, true, EInputContext::Combat, false, false, true});
    AllActions.Add({"GuardCounter", "RMB", "Shift", "Guard counter — block then heavy, GCD locked", false, true, EInputContext::Combat, true, false, false});
    AllActions.Add({"TargetLock", "Tab", "", "Toggle target lock", false, true, EInputContext::Combat, false, false, true});
    AllActions.Add({"TargetCycle", "Tab", "Shift", "Cycle locked target", false, true, EInputContext::Combat, false, false, true});
    AllActions.Add({"SheatheWeapon", "X", "", "Sheathe/unsheathe weapon", false, true, EInputContext::Combat, false, false, true});
    AllActions.Add({"TwoHandMode", "T", "", "Toggle two-handed grip", false, true, EInputContext::Combat, false, false, true});

    // === ACTION BAR SLOTS (1-20, all GCD locked except combat basics) ===
    // Slots 1-10: keys 1-0
    FString NumKeys[] = {"1","2","3","4","5","6","7","8","9","0"};
    for (int32 i = 0; i < 10; i++)
    {
        FEclipseInputAction Action;
        Action.ActionName = FString::Printf(TEXT("ActionBarSlot%d"), i + 1);
        Action.DefaultKey = NumKeys[i];
        Action.Description = FString::Printf(TEXT("Action bar slot %d — spell/ability, GCD locked"), i + 1);
        Action.bIsRebindable = true;
        Action.Context = EInputContext::Spellcasting;
        Action.bIsGCDLocked = true;
        Action.bIsBasicAttack = false;
        Action.bIsReactive = false;
        AllActions.Add(Action);
    }
    
    // Slots 11-15: Q, E, R, F, V
    FString LetterKeys[] = {"Q","E","R","F","V"};
    for (int32 i = 0; i < 5; i++)
    {
        FEclipseInputAction Action;
        Action.ActionName = FString::Printf(TEXT("ActionBarSlot%d"), i + 11);
        Action.DefaultKey = LetterKeys[i];
        Action.Description = FString::Printf(TEXT("Action bar slot %d — spell/ability, GCD locked"), i + 11);
        Action.bIsRebindable = true;
        Action.Context = EInputContext::Spellcasting;
        Action.bIsGCDLocked = true;
        AllActions.Add(Action);
    }
    
    // Slots 16-20: Shift+1/2/3/Q/E
    FString ShiftKeys[] = {"1","2","3","Q","E"};
    for (int32 i = 0; i < 5; i++)
    {
        FEclipseInputAction Action;
        Action.ActionName = FString::Printf(TEXT("ActionBarSlot%d"), i + 16);
        Action.DefaultKey = ShiftKeys[i];
        Action.Modifier = "Shift";
        Action.Description = FString::Printf(TEXT("Action bar slot %d — ultimate/item/macro, GCD locked"), i + 16);
        Action.bIsRebindable = true;
        Action.Context = EInputContext::Spellcasting;
        Action.bIsGCDLocked = true;
        AllActions.Add(Action);
    }

    // === PARTY COMMANDS (F1-F8, always responsive) ===
    AllActions.Add({"PartyFollow", "F1", "", "Party: Follow me", false, true, EInputContext::PartyCommand, false, false, true});
    AllActions.Add({"PartyAttack", "F2", "", "Party: Attack my target", false, true, EInputContext::PartyCommand, false, false, true});
    AllActions.Add({"PartyHold", "F3", "", "Party: Hold position", false, true, EInputContext::PartyCommand, false, false, true});
    AllActions.Add({"PartySpread", "F4", "", "Party: Spread out", false, true, EInputContext::PartyCommand, false, false, true});
    AllActions.Add({"PartyRetreat", "F5", "", "Party: Retreat", false, true, EInputContext::PartyCommand, false, false, true});
    AllActions.Add({"PartyFocusBoss", "F6", "", "Party: Focus boss", false, true, EInputContext::PartyCommand, false, false, true});
    AllActions.Add({"PartyStack", "F7", "", "Party: Stack up", false, true, EInputContext::PartyCommand, false, false, true});
    AllActions.Add({"PartyMechanic", "F8", "", "Party: Handle mechanic", false, true, EInputContext::PartyCommand, false, false, true});

    // === UI TOGGLES (always responsive) ===
    AllActions.Add({"ToggleInventory", "I", "", "Inventory", false, true, EInputContext::UI, false, false, true});
    AllActions.Add({"ToggleCharacter", "C", "", "Character sheet", false, true, EInputContext::UI, false, false, true});
    AllActions.Add({"ToggleSkills", "K", "", "Skills/talents", false, true, EInputContext::UI, false, false, true});
    AllActions.Add({"ToggleMap", "M", "", "Map", false, true, EInputContext::UI, false, false, true});
    AllActions.Add({"ToggleQuests", "J", "", "Quest log", false, true, EInputContext::UI, false, false, true});
    AllActions.Add({"ToggleParty", "P", "", "Party frame", false, true, EInputContext::UI, false, false, true});
    AllActions.Add({"ToggleChat", "Enter", "", "Chat", false, true, EInputContext::UI, false, false, true});
    AllActions.Add({"ToggleSettings", "Esc", "", "Settings menu", false, true, EInputContext::UI, false, false, true});
    AllActions.Add({"ToggleActionBar", "B", "", "Edit action bar", false, true, EInputContext::UI, false, false, true});

    UE_LOG(LogTemp, Log, TEXT("[Input] %d actions initialized — GCD locks abilities, basic attacks/dodge/block stay responsive"), AllActions.Num());
}

TArray<FEclipseInputAction> UEclipseInputSystem::GetAllActions() const
{
    return AllActions;
}

TArray<FEclipseInputAction> UEclipseInputSystem::GetActionsByContext(EInputContext Context) const
{
    TArray<FEclipseInputAction> Result;
    for (const auto& Action : AllActions)
    {
        if (Action.Context == Context) Result.Add(Action);
    }
    return Result;
}

FString UEclipseInputSystem::GetKeybindReference() const
{
    FString Ref = "=== ECLIPSE KEYBINDS ===\n\n";
    
    Ref += "COMBAT (basic attacks bypass GCD):\n";
    Ref += "  LMB: Light Attack (responsive)\n";
    Ref += "  RMB: Heavy Attack (animation committed)\n";
    Ref += "  Hold RMB: Charged Attack\n";
    Ref += "  MMB: Weapon Art (GCD locked)\n";
    Ref += "  Space: Dodge Roll (reactive, i-frames)\n";
    Ref += "  Ctrl+MMB: Parry (reactive, 0.15s window)\n";
    Ref += "  Hold MMB: Block (reactive, stamina drain)\n";
    Ref += "  Shift+RMB: Guard Counter (GCD locked)\n";
    Ref += "  Tab: Target Lock / Shift+Tab: Cycle\n";
    Ref += "  X: Sheathe Weapon | T: Two-Hand Mode\n\n";
    
    Ref += "ACTION BAR (1-0, Q E R F V, Shift+1-3, Shift+Q-E):\n";
    Ref += "  1-0: Slots 1-10 (GCD locked)\n";
    Ref += "  Q E R F V: Slots 11-15 (GCD locked)\n";
    Ref += "  Shift+1/2/3/Q/E: Slots 16-20 (GCD locked)\n";
    Ref += "  B: Edit action bar\n\n";
    
    Ref += "PARTY COMMANDS (F1-F8):\n";
    Ref += "  F1: Follow | F2: Attack | F3: Hold | F4: Spread\n";
    Ref += "  F5: Retreat | F6: Focus Boss | F7: Stack | F8: Mechanic\n\n";
    
    Ref += "MOVEMENT (always responsive):\n";
    Ref += "  WASD: Move | Space: Jump | Shift: Sprint | Mouse: Camera\n\n";
    
    Ref += "UI:\n";
    Ref += "  I: Inventory | C: Character | K: Skills | M: Map\n";
    Ref += "  J: Quests | P: Party | Enter: Chat | Esc: Settings\n\n";
    
    Ref += "GCD RULES:\n";
    Ref += "  GCD (1.0s default) locks: spells, abilities, weapon arts\n";
    Ref += "  GCD does NOT lock: light/heavy attacks, dodge, block, parry\n";
    Ref += "  Heavy attacks lock input during animation commitment\n";
    
    return Ref;
}

FString UEclipseInputSystem::GetMappingContextName(EInputContext Context) const
{
    switch (Context)
    {
    case EInputContext::Menu: return TEXT("IMC_Menu");
    case EInputContext::Locomotion: return TEXT("IMC_Locomotion");
    case EInputContext::Combat: return TEXT("IMC_Combat");
    case EInputContext::Spellcasting: return TEXT("IMC_Spellcasting");
    case EInputContext::PartyCommand: return TEXT("IMC_PartyCommand");
    case EInputContext::UI: return TEXT("IMC_UI");
    case EInputContext::Dialogue: return TEXT("IMC_Dialogue");
    default: return TEXT("IMC_Default");
    }
}

// === GCD SYSTEM ===

void UEclipseInputSystem::TriggerGCD(float Duration)
{
    if (Duration <= 0.0f) Duration = GCDDuration;
    GCDRemaining = Duration;
    bGCDActive = true;
    UE_LOG(LogTemp, Log, TEXT("[Input/GCD] GCD triggered: %.2fs"), Duration);
}

void UEclipseInputSystem::UpdateGCD(float DeltaTime)
{
    if (bGCDActive)
    {
        GCDRemaining -= DeltaTime;
        if (GCDRemaining <= 0.0f)
        {
            GCDRemaining = 0.0f;
            bGCDActive = false;
        }
    }
    
    if (bAnimationLocked)
    {
        AnimationLockRemaining -= DeltaTime;
        if (AnimationLockRemaining <= 0.0f)
        {
            AnimationLockRemaining = 0.0f;
            bAnimationLocked = false;
        }
    }
}

bool UEclipseInputSystem::IsGCDActive() const
{
    return bGCDActive;
}

float UEclipseInputSystem::GetGCDRemaining() const
{
    return GCDRemaining;
}

bool UEclipseInputSystem::CanUseAction(const FString& ActionName) const
{
    // Find the action
    for (const auto& Action : AllActions)
    {
        if (Action.ActionName == ActionName)
        {
            // Basic attacks and reactive actions bypass GCD
            if (Action.bIsBasicAttack || Action.bIsReactive)
            {
                // But still check animation lock — can't act during heavy commitment
                if (bAnimationLocked) return false;
                return true;
            }
            
            // GCD-locked actions check GCD
            if (Action.bIsGCDLocked)
            {
                if (bGCDActive) return false;
                if (bAnimationLocked) return false;
                return true;
            }
            
            // Non-GCD, non-basic — check animation lock only
            if (bAnimationLocked) return false;
            return true;
        }
    }
    return false;
}

bool UEclipseInputSystem::TryUseAction(const FString& ActionName)
{
    if (!CanUseAction(ActionName))
    {
        UE_LOG(LogTemp, Log, TEXT("[Input] Action blocked: %s (GCD: %s, AnimLock: %s)"),
            *ActionName, bGCDActive ? TEXT("YES") : TEXT("NO"),
            bAnimationLocked ? TEXT("YES") : TEXT("NO"));
        return false;
    }
    
    // Find the action and trigger GCD if it's GCD-locked
    for (const auto& Action : AllActions)
    {
        if (Action.ActionName == ActionName)
        {
            if (Action.bIsGCDLocked && !Action.bIsBasicAttack)
            {
                TriggerGCD(GCDDuration);
            }
            UE_LOG(LogTemp, Log, TEXT("[Input] Action used: %s (GCD triggered: %s)"),
                *ActionName, Action.bIsGCDLocked ? TEXT("YES") : TEXT("NO"));
            return true;
        }
    }
    return false;
}

// === ACTION BAR INTEGRATION ===

bool UEclipseInputSystem::TryActivateSlot(EActionBarSlot Slot)
{
    if (!ActionBar) return false;
    
    // Action bar slots are GCD-locked (spells/abilities)
    if (bGCDActive)
    {
        UE_LOG(LogTemp, Log, TEXT("[Input] Action bar slot %d blocked by GCD"), (int32)Slot);
        return false;
    }
    
    if (bAnimationLocked)
    {
        UE_LOG(LogTemp, Log, TEXT("[Input] Action bar slot %d blocked by animation lock"), (int32)Slot);
        return false;
    }
    
    // Activate the slot through the action bar system
    bool bActivated = ActionBar->ActivateSlot(Slot, 100.0f, 100.0f); // TODO: pass real Flow + resource
    
    if (bActivated)
    {
        // Trigger GCD for the activated spell/ability
        TriggerGCD(GCDDuration);
        UE_LOG(LogTemp, Log, TEXT("[Input] Action bar slot %d activated — GCD triggered"), (int32)Slot);
    }
    
    return bActivated;
}

void UEclipseInputSystem::BindActionBar(UEclipseActionBar* InActionBar)
{
    ActionBar = InActionBar;
    UE_LOG(LogTemp, Log, TEXT("[Input] Action bar bound to input system"));
}

// === ANIMATION LOCK ===

void UEclipseInputSystem::SetAnimationLock(float Duration)
{
    AnimationLockRemaining = Duration;
    bAnimationLocked = true;
    UE_LOG(LogTemp, Log, TEXT("[Input] Animation lock set: %.2fs (heavy attack commitment)"), Duration);
}

bool UEclipseInputSystem::IsAnimationLocked() const
{
    return bAnimationLocked;
}

void UEclipseInputSystem::ClearAnimationLock()
{
    AnimationLockRemaining = 0.0f;
    bAnimationLocked = false;
    UE_LOG(LogTemp, Log, TEXT("[Input] Animation lock cleared"));
}
