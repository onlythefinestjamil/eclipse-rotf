# ECLIPSE — Master Feature Registry

*Generated 2026-11-08 (plan `slate_heron_ledger_v1`). Read-only audit; nothing was changed.*

**Status legend**
- **Design:** Locked (in `ECLIPSE_CANON.md`) · Bible (in a V6/V9 directive only) · Open (canon §5) · None
- **Impl:** Live (compiles and on the runtime path) · Orphan (compiles, nothing references it) · Partial · Slice (self-contained prototype) · None
- **Evidence:** V = verified by a project read this session · D = documents only (unverified)
- Nothing is "Tested" under the V6 definition (no packaged build, no automation tests). The only PIE verification on record is the perfect-dodge afterimage (canon §4).

Columns: Feature · Source · Description · Design · Impl · Paths · Deps · Readiness · Conflicts · Evidence · Next action

---

## 1. Character creation and races

| Feature | Source | Description | Design | Impl | Paths | Deps | Readiness | Conflicts | Ev | Next |
|---|---|---|---|---|---|---|---|---|---|---|
| 13 ancestries | `EclipseCharacter.h` | `EAncestry` enum | Locked | Live (data) | `Source/EclipseRotF/EclipseCharacter.h` | — | Ready | Old audit JSON says 12 (C9) | V | none |
| Character select / creation / loading screen | Plan `amber_quill_lantern` | Pick ancestry, name, 8 slots, async load | Bible | Partial (C++ only) | `Source/EclipseRotF/FrontEnd/` | Save (C3), UI | Blocked on widgets | 3 save paths (C3) | V | Build WBPs after C3 decision |
| Ancestry art | git `141aef6`…, `/Game/Characters/*` | Static meshes per ancestry | Bible | Partial | `/Game/Characters/SOLARI/StaticMeshes/SOLARI` etc. | Rigging | Not ready (unrigged) | Vyrael/Tharuk/Umbralis missing | V | Rig after spend gate |

Unverified (docs only): appearance customization (50+ hairstyles, skin slider) — git `52a99d5`.

## 2. Classes, subclasses, and specializations

| Feature | Source | Description | Design | Impl | Paths | Deps | Readiness | Conflicts | Ev | Next |
|---|---|---|---|---|---|---|---|---|---|---|
| 17 classes | `EclipseCharacter.h` | `EClass` enum | Locked | Live (data) | `Source/EclipseRotF/EclipseCharacter.h` | — | Ready | Legacy art names (C10) | V | none |
| Class roles | `EclipseClassRoles.h` | 14 roles, flexibility | Bible | Orphan (VerticalSlice only) | `Source/EclipseRotF/EclipseClassRoles.h` | Party | Needs C1 | — | V | Wire with companions |
| Class abilities | `EclipseClassAbilities.h` | 27 ability types | Bible | Orphan | `Source/EclipseRotF/EclipseClassAbilities.h` | Ability comp | Needs merge | Overlaps `UEclipseAbilityComponent` | V | Decide C12 |

Unverified: subclasses/specializations — no code found.

## 3. Character builds and talent trees

| Feature | Source | Description | Design | Impl | Paths | Deps | Readiness | Conflicts | Ev | Next |
|---|---|---|---|---|---|---|---|---|---|---|
| Talent trees | `EclipseTalentSystem.h` (git `63099ce`) | 5 layers, 129 nodes (33 Bulwark, 96 placeholders) | Open | Orphan | `Source/EclipseRotF/EclipseTalentSystem.h` | Affinities | **Blocked** | C8 | V | Await C8 |
| Dual affinities | Canon §2/§5, `ECLIPSE_DEFINITIVE_CANON_V9.txt` §5 | 11 affinities; pair count disputed | Open | None | — | Talents, Spells | Blocked | 45 vs 55 vs "10" (C8) | V | User decision |

## 4. Combat and movement

| Feature | Source | Description | Design | Impl | Paths | Deps | Readiness | Conflicts | Ev | Next |
|---|---|---|---|---|---|---|---|---|---|---|
| Movement / camera / sprint | Plan `ember_vale` | Enhanced Input third-person | Locked | Live | `/Game/Blueprints/BP_AetherbornHero`, `/Game/Input/IMC_Default` | — | Ready | C5 | V | none |
| Light/heavy combo, block, parry | `EclipseCombatComponent.h` | Timers + montage notifies | Locked | Live | `/Game/Animations/Combat/AM_Light_01`, `AM_Heavy` | Montages | Ready | 5 combo impls (C2) | V | Sync timings |
| Dodge + perfect dodge + afterimage | `EclipsePerfectDodgeComponent.h`, canon §4 | i-frames, slow-mo, ghosts | Locked | Live | `/Game/Generated_Materials/M_AfterImage_Ghost` | GameFeel | Ready | — | V (PIE) | none |
| Cast bar, stagger | memory `combat-animation-layer` | Cast authority, hit reactions | Bible | Live | `/Game/Animations/Combat/AM_Cast_01` | Ability comp | Ready | — | V | none |
| Combat math | `EclipseCombatSystem.h` | Attack/defence/crit types | Bible | Live (via component) | `Source/EclipseRotF/EclipseCombatSystem.h` | — | Ready | — | V | none |
| Damage rotation | `EclipseDamageRotation.h` | Rotation priority | Bible | Orphan | `Source/EclipseRotF/EclipseDamageRotation.h` | — | Low | `EInputAction` dup (C5) | V | Fold or retire |
| Game feel director | `EclipseGameFeel.h` | Hit-stop, shake, presets | Locked (constants) | Orphan | `Source/EclipseRotF/EclipseGameFeel.h` | — | Medium | Overlaps component feel | V | Merge into component |
| Combo Slices | Slices / plans `copper_harbor_bloom`, `tideglass_tempo` | Timing drills | — | Slice | `/Game/Aura/Slices/AetherbornCombo/Maps/L_AetherbornCombo`, `/Game/Aura/Slices/AetherbornComboTiming/Maps/L_AetherbornComboTiming` | — | Sandbox | C2 | V | Keep as sandboxes |

Unverified: lock-on behaviour (`IA_LockOn` exists; consumer not verified), poise.

## 5. Spells and abilities

| Feature | Source | Description | Design | Impl | Paths | Deps | Readiness | Conflicts | Ev | Next |
|---|---|---|---|---|---|---|---|---|---|---|
| Six Aetherborn abilities | Plan `ember_vale` | Costs, cooldowns, Q/E/R/F/ult | Bible | Live | `Source/EclipseRotF/EclipseAbilityComponent.h` | Resources | Ready | C12 | V | none |
| Spell system | `EclipseSpellSystem.h` | 34 tiers, 13 shapes | Bible | Orphan | `Source/EclipseRotF/EclipseSpellSystem.h` | Affinities | Medium | C8 | V | V9 stage 3 (Arcanist) |
| GAS | `Build.cs` | Module linked, unused | — | None | — | — | — | C12 | V | User decision |

## 6. Weapons, armor, loot, and gear sets

| Feature | Source | Description | Design | Impl | Paths | Deps | Readiness | Conflicts | Ev | Next |
|---|---|---|---|---|---|---|---|---|---|---|
| Weapon swap | `EclipseWeaponSwap.h` | 33 types, 16 combo styles | Bible | Orphan | `Source/EclipseRotF/EclipseWeaponSwap.h` | Meshes | Low | No weapon meshes | V | After stage 1 |
| Weapon attach | memory `combat-animation-layer` | `WeaponMesh` on `hand_r` | Bible | Live (empty) | `/Game/Blueprints/BP_AetherbornHero` | Mesh | Needs asset | — | V | Need a weapon mesh |
| Loot + rarity | `EclipseLootSystem.h` | 7 rarities, binding | Locked (rarity) | Orphan | `Source/EclipseRotF/EclipseLootSystem.h` | Inventory | Low | — | V | After inventory exists |
| Pickup | `BP_Pickup` | Placed actor | — | Live | `/Game/Blueprints/BP_Pickup` | — | — | — | V | none |

Unverified: armor, gear sets.

## 7. AI enemies and companions

| Feature | Source | Description | Design | Impl | Paths | Deps | Readiness | Conflicts | Ev | Next |
|---|---|---|---|---|---|---|---|---|---|---|
| Target dummy | `EclipseTargetDummy.h` | Damage receiver | — | Live | `/Game/Maps/Lvl_Stage1_Arena` | — | Ready | — | V | none |
| Enemy AI | `EclipseEnemyAI.h` | 12 types, 8 states (logic only) | Bible | Orphan | `Source/EclipseRotF/EclipseEnemyAI.h` | Actor + controller | Not ready | C6 | V | P0: enemy actor, 3 telegraphs |
| Companions | `EclipseCompanionAI.h` (git `e3bffa1`) | 21 personalities, fill party slots | Bible | Orphan | `Source/EclipseRotF/EclipseCompanionAI.h` | Party | Low | — | V | V9 stage 3 healer |

No Behavior Trees exist (verified).

## 8. Dungeons and raids

| Feature | Source | Description | Design | Impl | Paths | Deps | Readiness | Conflicts | Ev | Next |
|---|---|---|---|---|---|---|---|---|---|---|
| Raid director + party | `EclipseRaidDirector.h`, `EclipsePartySystem.h` | 14 encounters, groups | Bible | Orphan | `Source/EclipseRotF/EclipseRaidDirector.h` | Companions, Boss | Not ready | — | V | V9 stage 5 |

## 9. Boss mechanics

| Feature | Source | Description | Design | Impl | Paths | Deps | Readiness | Conflicts | Ev | Next |
|---|---|---|---|---|---|---|---|---|---|---|
| Boss director | `EclipseBossDirector.h` | 14 bosses, 4 phases, 26 mechanics | Bible | Orphan (via Raid) | `Source/EclipseRotF/EclipseBossDirector.h` | Enemy actor | Not ready | Bible V9.2 adds 5 bosses (D) | V | After enemy actor |

## 10. PvP arenas and battlegrounds

| Feature | Source | Description | Design | Impl | Paths | Deps | Readiness | Conflicts | Ev | Next |
|---|---|---|---|---|---|---|---|---|---|---|
| Diminishing returns | `EclipseDRSystem.h` | 6 CC categories | Bible | Orphan | `Source/EclipseRotF/EclipseDRSystem.h` | Multiplayer | Phase E | — | V | Defer |

Unverified: arenas, battlegrounds, duel circle (art plates only in `HANDOFF_FORGE_ENVIRONMENT/`).

## 11. World regions and realms

| Feature | Source | Description | Design | Impl | Paths | Deps | Readiness | Conflicts | Ev | Next |
|---|---|---|---|---|---|---|---|---|---|---|
| Stage 1 arena | config | Default map | — | Live (bare) | `/Game/Maps/Lvl_Stage1_Arena` | — | Needs lighting | No lights | V | P0 light/sky |
| Solara Cathedral | `Docs/Solara_Cathedral_Environment_Design_v1.md` | Environment prototype | Bible | Partial | `/Game/Maps/Lvl_SolaraCathedral`, `/Game/VFX/NS_SolaraDustMotes` | `AEclipseSolaraLevel` | Medium | — | V | Extract thread (audit §4) |

Unverified: 6 regions, 50 zones, realm transfers, world map (V6 registry / art plates).

## 12. Quests, story, and dialogue

| Feature | Source | Description | Design | Impl | Paths | Deps | Readiness | Conflicts | Ev | Next |
|---|---|---|---|---|---|---|---|---|---|---|
| Quest system | `EclipseQuestSystem.h` (git `446e5e8`) | 135 quests, 6 factions, branching | Bible | Orphan | `Source/EclipseRotF/EclipseQuestSystem.h` | UI, save | Low | — | V | After save consolidation |

Unverified: dialogue, story script.

## 13. Crafting, professions, and economy

Unverified (docs only): crafting/blacksmith, auction house, trade, mail, guilds — git `7a223f7` (Bible V9.5). **No code.**

## 14. Inventory and progression

| Feature | Source | Description | Design | Impl | Paths | Deps | Readiness | Conflicts | Ev | Next |
|---|---|---|---|---|---|---|---|---|---|---|
| Save system | `EclipseSaveSystem.h` | 4 slots | Bible | Orphan | `Source/EclipseRotF/EclipseSaveSystem.h` | — | Needs C3 | C3 | V | User decision C3 |
| Key-binding save | `EclipseKeybindSubsystem.h` | Persist rebinding | Bible | Partial | `/Game/UI/WBP_KeyBindings` | Widget | Broken | — | V | P0 fix widget |

Unverified: inventory, leveling, mount milestones (canon §4 targets only).

## 15. Multiplayer-ready architecture

No replication, RPCs or GAS use anywhere (verified). Chat (`EclipseChatSystem.h`, 8 channels) is an orphan. → Phase E.

## 16. UI, HUD, menus, and settings

| Feature | Source | Description | Design | Impl | Paths | Deps | Readiness | Conflicts | Ev | Next |
|---|---|---|---|---|---|---|---|---|---|---|
| HUD widgets | `/Game/UI` | HUD, resource bar, action/weapon slots | Bible | Live (partial) | `/Game/UI/WBP_EclipseHUD`, `/Game/UI/WBP_ResourceBar`, `/Game/UI/WBP_ActionSlot`, `/Game/UI/WBP_WeaponSlot` | Ability comp | Medium | C4 | V | Consolidate C4 |
| C++ HUD layers | `EclipseHUDSystem.h`, `EclipseActionBar.h`, `EclipseAbilityBarWidget.h` | 37 widget enum, 20-slot bar | Bible | Orphan | `Source/EclipseRotF/EclipseHUDSystem.h` | — | — | C4 | V | Retire or merge |
| Input/GCD system | `EclipseInputSystem.h` | 23 contexts, GCD | Bible | Orphan | `Source/EclipseRotF/EclipseInputSystem.h` | — | — | C5 | V | Keep Enhanced Input |

Unverified: settings menu, accessibility.

## 17. Character models, animations, VFX, and audio

| Feature | Source | Description | Design | Impl | Paths | Deps | Readiness | Conflicts | Ev | Next |
|---|---|---|---|---|---|---|---|---|---|---|
| Rigged hero | memory `aetherborn-hero-model` | Aura-generated skeletal mesh | — | Slice only | `/Game/AuraModelGen/AetherbornHero/AetherbornHero/SkeletalMeshes/AetherbornHero` | Retarget | Medium | Production shows Manny | V | Retarget production montages |
| Combat montages | Plan `sunlit_quill` | 9 montages on SK_Mannequin | — | Live | `/Game/Animations/Combat/AM_Light_01` | — | Ready | Notifies missing on most | V | Add notifies |
| Lightning VFX set | `/Game/Generated_VFX/Aetherborn` | Materials + textures | — | Partial (no Niagara) | `/Game/Generated_VFX/Aetherborn/Materials/M_AB_Lightning` | — | Low | — | V | Later |
| Audio | — | — | — | None | — | — | — | — | V | Later |

## 18. Performance and platform optimization

| Feature | Source | Description | Design | Impl | Paths | Deps | Readiness | Conflicts | Ev | Next |
|---|---|---|---|---|---|---|---|---|---|---|
| Visual pipeline | `EclipseVisualPipeline.h`, `DefaultEngine.ini` | Lumen, Nanite, VSM, TSR | Bible | Config live; class orphan | `Source/EclipseRotF/EclipseVisualPipeline.h` | — | Ready | — | V | Profile once arena is lit |

No packaged build exists, and no profiling capture has been taken.

## Other (not in the requested categories)

| Feature | Source | Impl | Paths | Notes | Ev |
|---|---|---|---|---|---|
| Vertical slice game mode | `EclipseVerticalSlice.h` | Orphan | `Source/EclipseRotF/EclipseVerticalSlice.h` | Integrator of 11 systems (C1) | V |
| Targeting / weak points | `EclipseTargetingSystem.h` | Orphan | `Source/EclipseRotF/EclipseTargetingSystem.h` | 7 modes, 11 weak points | V |
| Mounts | canon §4, memory `mount-system` | None | — | Traversal only; dedicated level | V (none built) |
| Atmospheric Presence | git `7d2caea` | None | — | Bible "signature feature" | D |
| Canon tooling | `ECLIPSE_CANON.md`, Skill `eclipse-canon` | Live | `ECLIPSE_CANON.md` | Thread entry point | V |
| Stage 1 Python pipeline | `Content/Python/stage1_*.py` | Scripts | `Content/Python/stage1_setup.py` | Lighting scripts exist yet the arena has no lights — check which map they targeted | V |
| Trailer / art handoffs | `HANDOFF_FORGE_TRAILER/`, `ancestry_packs/` | Reference | — | Not gameplay | V |
