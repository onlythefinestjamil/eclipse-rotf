# ECLIPSE: REALMS OF THE FRACTURED — CANON

**The single entry point for every Aura thread. Read this before touching Eclipse.**

Purpose: this file exists because Aura threads do not share context. Only what is
written to disk, to project memory, or to a Skill travels between them. If a decision
lives only inside a conversation, the next thread is a stranger to it.

---

## 1. PRECEDENCE

When two sources disagree, the higher rule wins. Do not average them, and do not
silently pick one.

1. **The C++ source in `Source/EclipseRotF/`** is ground truth for *what exists*.
   An enum value, a constant or a component in code is real; a doc claiming otherwise
   is intent, not fact.
2. **`ECLIPSE_NEXT_DIRECTIVE.md` (Master Game Bible V6)** is authority on *design canon* —
   class roster, affinity model, progression.
3. **`ECLIPSE_V9_DIRECTIVE.md`** is authority on *Aura workflow and production process*
   ("supersedes all prior directives for Aura integration"). It agrees with V6 on the
   locked numbers: 13 ancestries, 17 classes.
4. **This file (`ECLIPSE_CANON.md`)** outranks every other document and every chat
   message for the specific constants listed in §2 and the decisions in §4.

If a request contradicts this file: **say so and stop.** Flag the contradiction.
Never resolve a canon conflict by inventing a value.

---

## 1b. DOCUMENT REGISTER

Every directive file at the project root, and how to treat it. A thread that opens one
of these without this file is reading intent, not canon.

| File | Status |
|---|---|
| `ECLIPSE_CANON.md` (this file) | **Outranks everything.** Single entry point. |
| `ECLIPSE_NEXT_DIRECTIVE.md` | Authoritative on **design canon** (Bible V6). Spend gate + definition of complete live here. |
| `ECLIPSE_V9_DIRECTIVE.md` | Authoritative on **Aura workflow/process**. Holds the staged build queue and the per-feature task format. |
| `ECLIPSE_8_DIRECTIVES.md` | V6 production directives. Reference for standards; covered by the two above. |
| `ECLIPSE_TALENT_DIRECTIVE.md` | Talent/affinity tree intent. **Blocked** by open question 1. |
| `ECLIPSE_DAY1_DIRECTIVE.md` | Historical startup brief. Reference only. |
| `ECLIPSE_INTEGRATED_PIPELINE.md` | Art/asset pipeline. Reference. |
| `ECLIPSE_STUDIO_ORCHESTRATOR.md` | Multi-agent orchestration. Reference. |
| `HANDOFF_FORGE_TRAILER.md` | Trailer asset handoff. Reference only, unrelated to gameplay. |
| `doc_6c08*_Master_Art_Production_Prompts.md` | Art prompt library. Reference. |
| `Docs/ECLIPSE_MASTER_FEATURE_REGISTRY.md` | Consolidation audit (2026-11-08): every feature, status, evidence. Reference — not canon. |
| `Docs/ECLIPSE_THREAD_AUDIT.md` | Threads/work streams evidenced on disk + extraction prompt. Reference. |
| `Docs/ECLIPSE_ASSET_INVENTORY.md` | Asset and code inventory. Reference. |
| `Docs/ECLIPSE_INTEGRATION_ROADMAP.md` | Phases A–E, conflicts C1–C14, integration order. Proposal — not canon. |
| `Docs/ECLIPSE_SYSTEM_DEPENDENCIES.md` | C++ system reference map. Reference. |
| `Docs/ECLIPSE_EQUIPMENT_BIBLE.md` | Equipment taxonomy, single-affinity status rules, 10 flagship weapons (2026-11-08). Proposal — not canon. |

Rule: if one of these contradicts §2 or §4, this file wins and the contradiction gets
reported, not resolved.

---

## 2. LOCKED CONSTANTS

Verified against source. Do not restate these from memory — re-verify with a read if
a change is suspected, then update this file.

### Classes — 17 (`EClass`, `Source/EclipseRotF/EclipseCharacter.h`)

Bulwark · Duskblade · Arcanist · Riftwalker · Dreadknight · Templar · Soulbinder ·
Wildwarden · Stormcaller · Oracle · Alchemist · Berserker · Shadowblade ·
Spellblade · Warthane · Necroturge · Earthweaver

There are **17**, not 12. Any document listing a generic fantasy archetype set
(Warrior / Paladin / Rogue / Mage…) is describing *archetypes*, not classes, and must
be reconciled to this list before use.

### Ancestries — 13 (`EAncestry`, same header)

Aetherborn · Solari · Vyrael · Tharuk · Kazari · Umbralis · Draven · Grimstarr ·
Nereyth · Myceliar · Forgeborn · Hellsworn · Necrevan

### Affinities — `EElementalAffinity` holds 11 names

Fire · Ice · Lightning · Shadow · Holy · Water · Soul · Earth · Wind · Poison · Temporal

Bible V6 specifies **45 dual-affinity pairings**. **These two facts are not yet
reconciled — see §5, open question 1.** Do not author pairings until it is resolved.

### Resources — 5 useful (`EResourceType`)

Mana · Stamina · Rage · Reagents · Aether  (enum also carries `None`)

### Rarity — 7 (`ERarityTier`)

Common · Uncommon · Rare · Epic · Legendary · Mythic · Eclipse

### Combat baseline (`FGameFeelProfile`, `ECombatState`)

States: `Idle, LightAttack, HeavyAttack, Dodging, Blocking`

| Constant | Value |
|---|---|
| `DodgeIFrameStart` / `DodgeIFrameEnd` | 0.05 s / 0.22 s |
| `DodgeDuration` / `DodgeCooldown` / `DodgeStaminaCost` | 0.32 s / 0.5 s / 20 |
| `ComboWindow` | 0.20 s |
| `PerfectParryWindow` | 0.08 s |
| `BlockDamageReduction` | 0.6 |
| `PerfectDodgeTimeDilation` / `PerfectDodgeSlowMoDuration` | 0.3 / 0.5 s |

Slow motion is counted in **real** seconds (`UWorld::GetRealTimeSeconds`), because
component delta is already dilated. Perfect dodge must not also call `DoHitStop` —
both drive `SetGlobalTimeDilation`.

### Build queue (V9, in order)

1. One playable class (Bulwark), Aetherborn test character, dodge + perfect dodge,
   one enemy with 3 telegraphed attacks, stamina bar, damage logging
2. Combo windows, block/parry/guard counter, stagger, enemy reactions, camera/impact
3. Arcanist spellcasting, surface VFX, healer NPC
4. 2-client multiplayer, latency tests, PvP DR
5. Second race rig, expanded weapons, 5-player dungeon + 10-player raid
6. Scale to 17 classes, unique animation/VFX families, automated combat tests

### Definition of complete (V6)

C++ compiles → Unreal loads the system → gameplay works in a packaged build →
automated tests pass where applicable → visual or gameplay evidence provided.
**No system is complete because its source files exist.**

### Spend gate (V6)

Current approved spend: $20/mo Meshy only. Past $500 requires a working Windows build
with a playable character, one AI companion, one enemy, spellcasting, and save/load.
Bulk asset generation does not precede that gate.

---

## 3. SYSTEMS THAT ACTUALLY EXIST

Authored and compiling in `Source/EclipseRotF/` (41 production headers incl. `FrontEnd/`, plus 19 Slice headers under `Aura/Slices/`; recounted 2026-11-08): character/movement/camera,
combat, abilities, perfect-dodge component, damage rotation, enemy AI, target dummy,
boss director, raid director, quest/party/loot/talent/spell systems, action bar, HUD,
save system, keybind subsystem, weapon swap, companion AI, targeting, game feel,
visual pipeline, class roles/abilities, anim notifies, chat, DR, vertical slice,
Solara level.

**Not authored — zero implementation:** mounts, creature meshes, water, sky/atmosphere
in the playable map, class-mount customization, progression/leveling.

## 4. DECISIONS TAKEN (with reasons)

**Perfect dodge afterimage** — 4 full-size ghosts, 45 cm spacing, 0.55 s life,
0.75 peak opacity, 18 cm/s rise, dissolve via `M_AfterImage_Ghost` +
`AfterImageOpacity` scalar. Ghost MIDs must be created with
`CreateDynamicMaterialInstance(slot, AfterImageMaterial)`; `CreateAndSetMaterialInstanceDynamic`
parents to the mesh's existing opaque material and silently ignores the fade parameter.
Ghosts must be moved with `SetActorLocationAndRotation` *after* the mesh becomes root —
a bare `AActor` has no root at spawn time and drops the transform. Verified in PIE.

**Mounts** — three independent domains (Land / Water / Flying), **traversal only**
(no mounted combat), **summoned on demand** (one key per domain, vanish on dismount),
player stays the possessed pawn (mounts attach; combat/ability/dodge components keep
working). Exercised in a **new dedicated level**, not `Lvl_Stage1_Arena`.
Rationale: traversal-only avoids touching the existing combat stack.

**Mount Forge axes** (supersede any earlier proposal):

```
EMountArmor    { Light, Royal, Warforged }
EMountEnergy   { None, VioletFracture, SolarRadiance, FrostShimmer }
EMountEvolution{ Base, Ascended, Mythic }
```

Collections: 60 core mounts + 4 flagship + one per class (**17**, not 12), counted
separately.

**Level milestones (design targets, not confirmed caps):** 15 first ground mount ·
20 armor/saddle/customization · 25 collection quests · 30 class mount · 35 aquatic ·
40 flight · 50+ class evolutions · endgame flagship.

## 5. OPEN — needs a human decision, do not invent

1. **Affinity arithmetic.** `EElementalAffinity` has 11 names; 45 unordered pairs
   requires exactly 10. Hypothesis: Temporal is a non-elemental magic that does not
   pair, leaving 45. Unconfirmed. Until confirmed, no dual-affinity data gets authored.
2. **Five unmounted classes.** `Warthane`, `Oracle`, `Shadowblade`, `Alchemist`,
   `Spellblade` have no class mount. Twelve provisional concepts cover the other twelve.
3. **Mount level name** — `Lvl_MountProving` is a working name, not canon.

## 6. HOW TO CONNECT A THREAD

At the start of any Eclipse thread, invoke the Skill:

```
/eclipse-canon
```

That skill (`.Aura/Skills/eclipse-canon/`) loads this file, reads project memory, and
refuses to author anything that contradicts §2 or §4. It also auto-loads when Aura
detects Eclipse feature work.

**At the end of a thread**, state the decision and ask for it to be recorded. Decisions
go to project memory (with the reason); constants go here. A conversation is not a
record.

Preamble if a Skill is unavailable — paste verbatim:

> Read ECLIPSE_CANON.md before touching Eclipse. It outranks other docs and chat.
> Flag contradictions instead of resolving them.

---

*Last updated: 2026-11-08. Update §2 only from verified source, §4 only from a decision
that was actually made, §5 only by removing items a human resolved.*
