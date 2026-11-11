# ECLIPSE — Integration Roadmap

*Generated 2026-11-08 (plan `slate_heron_ledger_v1`). These are proposals only: nothing here has been applied, and nothing is deleted or overwritten. Priority: a polished, playable **single-player** RPG before any MMO infrastructure (V9 build queue, canon §2).*

## 1. Phases

| Phase | Meaning | Contents |
|---|---|---|
| **A — Working in Eclipse** | Compiles and on the runtime path | Movement/camera, light/heavy combo, block/parry, dodge + perfect dodge + afterimage, cast bar, stagger, six Aetherborn abilities, target dummy, pickup, HUD widgets (except key binds), Enhanced Input, enums (13 ancestries / 17 classes / 11 affinities / 5 resources / 7 rarities), visual config |
| **B — Created, not integrated** | Compiles; nothing references it | Quest, Talent, Spell, ClassAbilities, CompanionAI, RaidDirector→Boss/Party, InputSystem→ActionBar, GameFeel, VisualPipeline class, SolaraLevel, DamageRotation; the VerticalSlice-only set (DR, Loot, EnemyAI, Save, HUDSystem, Targeting, WeaponSwap, Chat, ClassRoles) |
| **C — Partial** | Some layers exist | FrontEnd (C++, no widgets), EnemyAI (logic, no actor), key rebinding (widget broken), Solara Cathedral level, rigged `AetherbornHero` (Slice only), lightning VFX (no Niagara), weapon socket (no mesh), the two native combo Slices |
| **D — Designed, not built** | Docs/canon only | Mounts, dual affinities, crafting/blacksmith, economy/auction/trade/mail/guilds, inventory, leveling, dialogue, PvP arenas, atmospheric presence, appearance customization, missing ancestry/class models |
| **E — Future expansion** | MMO layer | Replication, 2-client multiplayer, PvP DR live, 5-player dungeons / 10-player raids at scale, realm transfers, chat, social |

## 2. Conflicts and recommendations

| # | Conflict | Keep | Combine | Needs decision |
|---|---|---|---|---|
| C1 | `AEclipseGameMode` (live, wires nothing) vs `AEclipseVerticalSlice` (wires 11, unused) | `AEclipseGameMode` | Move systems in one at a time; keep VerticalSlice as reference | — |
| C2 | 5 combo implementations with different timings | `UEclipseCombatComponent` = authority | Slices mirror it | Which timing set (memory `slice-timing-authority`) |
| C3 | 3 saves: `UEclipseSaveSystem` (4 slots), `UEclipseCharacterSaveGame` (8), `UEclipseKeybindSaveGame` | One character save + separate settings save | — | 4 vs 8 slots |
| C4 | HUD: `UEclipseHUDSystem` / `UEclipseActionBar` UObjects vs C++ widgets vs WBPs | WBPs on C++ widget bases | Port useful logic from the UObjects | — |
| C5 | `UEclipseInputSystem` + `EInputAction` vs Enhanced Input | Enhanced Input | Port the GCD rule into the ability component | — |
| C6 | `UEclipseEnemyAI` has no actor | Enemy Character + AIController hosting it | — | — |
| C7 | Overlapping enums (`EGameModeType`/`EGameModeContext`; `EWeaponType` with no weapons) | — | Shared `EclipseTypes.h` | — |
| C8 | Affinity pairs: Bible V6 = 45, canon hypothesis = 10 pairing affinities, git `8c4a8c9` message = "10", `ECLIPSE_DEFINITIVE_CANON_V9.txt` §5 = **11 affinities, 55 combinations** | — | — | **USER** |
| C9 | Stale `ECLIPSE_PROJECT_AUDIT.json` | Mark historical | — | — |
| C10 | Non-canon art folders (NECROMANCER, DRUID, BARBARIAN, SORCERESS, KNIGHT, ASSASSIN, MUSCULAR, ASHFALLEN, DUSKBORN, STORMFANG) | — | — | **USER**: map to classes or mark reference |
| C11 | Redirectors, duplicate stone texture, ~60 unrigged statics | — | Fix up redirectors | — |
| C12 | GAS linked but unused vs custom ability component | — | — | **USER**: custom for single-player, or migrate before scaling to 17 classes |
| C13 | `ECLIPSE_DEFINITIVE_CANON_V9.txt`, `ECLIPSE_V6_COMPLETE_HANDOFF.txt`, Hermes handoffs and `ECLIPSE_AGENT_REGISTRY.json` are not in the canon §1b register | — | Register them | **USER**: their precedence |
| C14 | `*.uasset`/`*.umap` are gitignored and the newest C++ is untracked | — | — | **USER**: asset source control (Git LFS or Perforce) before integration starts |

## 3. Integration sequence (one system per step)

| Step | Work | Phase | Gate to move on |
|---|---|---|---|
| 0a | Commit the current untracked C++ and docs as a baseline; decide C14 | — | Clean `git status` |
| 0b | Fix `WBP_KeyBindings`; fix up redirectors; add a light and sky to `Lvl_Stage1_Arena` | C→A | All BPs up to date; arena visible in PIE |
| 1 | V9 stage 1: Bulwark on the production pawn, enemy actor with 3 telegraphed attacks (C6), stamina bar, damage log | B→A | PIE: the enemy telegraphs and hits; stamina drains |
| 2 | Consolidate HUD/action bar (C4), then save (C3) | B→A | Save → quit → load restores the character |
| 3 | Combo windows, parry, guard counter, stagger, camera/impact (C2, GameFeel merge) | B→A | Canon §2 constants hold in PIE |
| 4 | Arcanist via `UEclipseSpellSystem`; healer via `UEclipseCompanionAI` + Party | B→A | Spell lands; companion heals |
| 5 | FrontEnd widgets + save/load (plan `amber_quill_lantern`) | C→A | New character → enter world → reload |
| Gate | Spend gate (canon §2): packaged Windows build with a playable character, companion, enemy, spellcasting, save/load | — | — |
| Later | Talents (after C8), loot/inventory, quests, mounts, boss/raid, then Phase E | D/E | — |

## 4. Per-step verification checklist

1. C++ compiles (Live Coding; rebuild only for new modules).
2. Every touched Blueprint is `BS_UP_TO_DATE`.
3. PIE in `Lvl_Stage1_Arena`, checking the step's log tags (`[DAMAGE]`, `[ANIM]`, …).
4. Video or screenshot evidence.
5. Automation test where applicable (none exist yet: writing the first is P1).
6. Packaged Windows build at the end of each stage.
7. Update `ECLIPSE_CANON.md` (constants) and project memory (decisions).

## 5. Rollback rule

- Code and docs: one git commit per step; undo with `git revert <sha>`.
- **Assets are not in git (C14).** Until that is decided, copy the touched `Content/` folders to a dated backup before each step.
- Nothing is deleted; superseded systems are left in place and marked deprecated in this file.

## 6. Backlog

- **P0:** step 0a baseline commit; C14 decision; fix `WBP_KeyBindings`; light the arena; C1; enemy actor (C6).
- **P1:** C3/C4 consolidation; `EclipseTypes.h` (C7); first automation tests for the canon §2 combat constants; register the documents from C13.
- **P2:** wire Spell, Companion, FrontEnd; add notifies to the remaining montages; retarget the production montages onto `AetherbornHero`.
- **Decisions pending:** C2 timing set, C3 slot count, C8, C10, C12, C13, C14.
