# ECLIPSE — System Dependencies

*Generated 2026-11-08 (plan `slate_heron_ledger_v1`) from a static scan of `Source/EclipseRotF/`: a class counts as referenced when its name appears in another file. Read-only.*

## 1. Runtime map

```mermaid
flowchart TD
  CFG[DefaultEngine.ini GlobalDefaultGameMode] --> GM[BP_EclipseGameMode / AEclipseGameMode]
  GM --> Hero[BP_AetherbornHero / AEclipseCharacter]
  Hero --> Ability[UEclipseAbilityComponent]
  Hero --> Combat[UEclipseCombatComponent]
  Hero --> Dodge[UEclipsePerfectDodgeComponent]
  Combat --> CS[UEclipseCombatSystem]
  Notifies[AnimNotify HitWindow ComboWindow] --> Combat
  AbilityBar[UEclipseAbilityBarWidget] --> Ability
  AbilityBar --> Slot[UEclipseAbilitySlotWidget]
  Dummy[AEclipseTargetDummy placed in arena]

  VS[AEclipseVerticalSlice unused game mode] --> DR[UEclipseDRSystem]
  VS --> Loot[UEclipseLootSystem]
  VS --> Enemy[UEclipseEnemyAI]
  VS --> Save[UEclipseSaveSystem]
  VS --> HUDS[UEclipseHUDSystem]
  VS --> Tgt[UEclipseTargetingSystem]
  VS --> WS[UEclipseWeaponSwap]
  VS --> Chat[UEclipseChatSystem]
  VS --> Roles[UEclipseClassRoles]
  VS --> Rot[UEclipseDamageRotation]
  VS --> CS

  Raid[UEclipseRaidDirector] --> Boss[UEclipseBossDirector]
  Raid --> Party[UEclipsePartySystem]
  Input[UEclipseInputSystem] --> Bar[UEclipseActionBar]
  FE[UEclipseFrontEndSubsystem] --> CSave[UEclipseCharacterSaveGame]
  FE --> FES[UEclipseFrontEndSettings]

  style GM fill:#41a161,stroke:#228b22,color:#fff
  style Hero fill:#41a161,stroke:#228b22,color:#fff
  style VS fill:#d15858,stroke:#e74c3c,color:#fff
  style Raid fill:#b87700,stroke:#ff8c00,color:#fff
  style Input fill:#b87700,stroke:#ff8c00,color:#fff
  style FE fill:#b87700,stroke:#ff8c00,color:#fff
```

Green = live path. Red = unused integrator. Gold = unused cluster root.

## 2. Reference table

| Class | Base | Inbound refs (files) | On live path? |
|---|---|---|---|
| `AEclipseGameMode` | AGameModeBase | 0 (used via `BP_EclipseGameMode`) | **Yes** |
| `AEclipseCharacter` | ACharacter | 9 | **Yes** |
| `UEclipseCombatComponent` | UActorComponent | 7 | **Yes** |
| `UEclipseAbilityComponent` | UActorComponent | 5 | **Yes** |
| `UEclipsePerfectDodgeComponent` | UActorComponent | 3 | **Yes** |
| `UEclipseCombatSystem` | UObject | 4 | Yes (via combat component) |
| `UEclipseAnimNotify_HitWindow/_ComboWindow` | UAnimNotify | 0 (used on montages) | **Yes** |
| `AEclipseTargetDummy` | AActor | 0 (placed in map) | **Yes** |
| `UEclipseAbilityBarWidget` / `UEclipseAbilitySlotWidget` | UUserWidget | 0 / 2 | Unknown — no WBP parent verified |
| `AEclipseVerticalSlice` | AGameModeBase | 0 | No |
| DR, Loot, EnemyAI, Save, HUDSystem, Targeting, WeaponSwap, Chat, ClassRoles, DamageRotation | UObject | 2 each (VerticalSlice only) | No |
| `UEclipseRaidDirector` | UObject | 0 | No |
| `UEclipseBossDirector`, `UEclipsePartySystem` | UObject | 2 (RaidDirector only) | No |
| `UEclipseInputSystem` | UObject | 0 | No |
| `UEclipseActionBar` | UObject | 2 (InputSystem only) | No |
| `UEclipseQuestSystem`, `UEclipseTalentSystem`, `UEclipseSpellSystem`, `UEclipseClassAbilities`, `UEclipseCompanionAI`, `UEclipseGameFeel`, `UEclipseVisualPipeline` | UObject | 0 | No |
| `AEclipseSolaraLevel` | AActor | 0 | Unknown (may be placed in `Lvl_SolaraCathedral`) |
| `UEclipseKeybindSubsystem` | UGameInstanceSubsystem | 0 | Auto-created by engine; consumer `WBP_KeyBindings` is broken |
| `UEclipseFrontEndSubsystem` | UGameInstanceSubsystem | 0 | Auto-created; no widget consumes it |
| Slice classes (`AAetherbornCombo*`, `AAetherbornComboTiming*`) | various | 0 from production | Own maps only |

## 3. Cross-cutting dependencies

| Dependency | Consumers | Risk |
|---|---|---|
| Enums in `EclipseCharacter.h` (`EAncestry` 13, `EClass` 17, `EElementalAffinity` 11, `EResourceType`, `ERarityTier`) | almost every system | Changing affinity count (C8) touches Talent, Spell, ClassAbilities |
| `FGameFeelProfile` / `ECombatState` | Combat, PerfectDodge, AnimNotifies | Canon §2 constants live here |
| `SetGlobalTimeDilation` | Perfect dodge slow-mo, hit-stop | Must not run both at once (canon §2) |
| Enhanced Input `IMC_Default` | Character | `UEclipseInputSystem` duplicates it (C5) |
| `SK_Mannequin` skeleton | All production montages | `AetherbornHero` uses its own skeleton → retarget needed to swap the pawn body |
| Replication | none | Every system is single-player only today |
