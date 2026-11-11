# ECLIPSE — EQUIPMENT DESIGN BIBLE (Phase 1)

**Status: PROPOSAL — not canon.** Registered in `ECLIPSE_CANON.md` §1b. Every item below is
`Proposed`. Every number is `Provisional`. Nothing here overrides `ECLIPSE_CANON.md` §2.

Plan: `Saved/.Aura/plans/gilded_moth_anvil_v1.md` · Last updated 2026-11-08

---

## 1. Canon audit (verified against `Source/EclipseRotF/EclipseCharacter.h`)

| Canon set | Source | Value |
|---|---|---|
| Classes (17) | `EClass` | Bulwark, Duskblade, Arcanist, Riftwalker, Dreadknight, Templar, Soulbinder, Wildwarden, Stormcaller, Oracle, Alchemist, Berserker, Shadowblade, Spellblade, Warthane, Necroturge, Earthweaver |
| Ancestries (13) | `EAncestry` | Aetherborn, Solari, Vyrael, Tharuk, Kazari, Umbralis, Draven, Grimstarr, Nereyth, Myceliar, Forgeborn, Hellsworn, Necrevan |
| Affinities (11) | `EElementalAffinity` | Fire, Ice, Lightning, Shadow, Holy, Water, Soul, Earth, Wind, Poison, Temporal |
| Rarity (7) | `ERarityTier` | Common, Uncommon, Rare, Epic, Legendary, Mythic, Eclipse |
| Resources (5) | `EResourceType` | Mana, Stamina, Rage, Reagents, Aether |
| Regions (6) | `ERegion` | Solara, Vaelwood, Morvath, Aetheris, Nhalor, Umbryss |

**Decisions taken for this bible (user, 2026-11-08):**
- Equipment uses **one primary affinity per item**. No secondary/dual affinity until canon §5 Q1 is resolved.
- Names from the directive that are **not canon** and are rejected: Flame, Frost, Storm, Radiance, Venom, Arcane, Void, Fracture (Shadow and Earth are canon and kept). "Fracture" survives only as Aetherborn art language and the `FractureFlow`/`Fracture*` ability names — never as an affinity.
- **GAS is used for equipment only** (attributes, equip effects, status). The 6 abilities, GCD and costs stay in `UEclipseAbilityComponent` (audit C12, partial).

**Conflicts found (report, do not resolve):**
- E1 `EWeaponStatusEffect` (`EclipseWeaponSwap.h`) has 10 entries and no Temporal; equipment uses `EElementalAffinity` instead.
- E2 The directive lists ~30 weapon families; code has 10 (`EWeaponType`). `OneHandAxe` and `Tome` were appended as **Proposed**. The rest stay design-only.
- E3 Attribute scaling (Str/Dex/Int…) has no canon attribute list — coefficients are deferred.
- E4 Loot sources: `EclipseLootSystem` exists but is unwired; acquisition below is descriptive only.

## 2. Taxonomy

| Family (`EWeaponType`) | Hands | Reach cm | Speed | Poise × | Identity | Status |
|---|---|---|---|---|---|---|
| Greatsword | 2H | 340 | slow | 1.25 | guard break | Implemented (enum) |
| Daggers | dual 1H | 220 | fast | 0.6 | combo/precision | Implemented (enum) |
| Spear | 2H | 420 | medium | 1.0 | reach/counter | Implemented (enum) |
| GreatAxe | 2H | 320 | slow | 1.3 | sweep/rage | Implemented (enum) |
| Hammer | 2H | 300 | slow | 1.8 | stagger/armor break | Implemented (enum) |
| Staff | 2H | 320 | medium | 0.8 | spell amp | Implemented (enum) |
| Bow | 2H | 1200 | medium | 0.7 | ranged precision | Implemented (enum) |
| Shield (sword+shield) | 1H+OH | 280 | medium | 1.0 | block/counter | Implemented (enum) |
| Scythe | 2H | 360 | medium | 1.1 | sweep/sustain | Implemented (enum) |
| FistWeapons | dual | 200 | fast | 0.7 | parry combos | Implemented (enum) |
| OneHandAxe | 1H | 270 | medium | 1.2 | armor shred | **Proposed** |
| Tome | OH | 600 | medium | 0.5 | spell utility | **Proposed** |

Slots (`EEquipmentSlot`): MainHand, OffHand, Head, Chest, Hands, Legs, Feet, Neck, Ring1, Ring2, Trinket.
Status labels (`EDesignStatus`): Proposed → Approved → InProduction → Blocked → Implemented → Tested → Complete.

## 3. Itemization formulas (Provisional)

- `WeaponPower = FamilyBase × (1 + 0.04 × (ItemLevel − 1)) × RarityMult`
- `RarityMult`: Common 1.00 · Uncommon 1.08 · Rare 1.18 · Epic 1.30 · Legendary 1.45 · Mythic 1.60 · Eclipse 1.75
- `RequiredLevel = ItemLevel − 5` (min 1)
- Combat push (implemented): the equipped item overwrites `UEclipseCombatComponent::LightDamage`,
  `HeavyDamage`, `AttackRange`. Light/Heavy values below are expressed against the current
  arena baseline (component default 40 / 90, 300 cm) so they can be felt on BerserkerDummy (500 HP).
- Crafted equipment target: within ±5% WeaponPower of same-ilvl boss drops; bosses keep unique passives.

## 4. Single-affinity status rules (implemented framework, effects Proposed)

Each hit adds the weapon's `StatusBuildupPerHit` to the target's `StatusBuildup` attribute.
At `StatusThreshold` (default 100) buildup resets to 0 and the target gains `Status.<Affinity>` for
`StatusDuration`, then `Status.Immune.<Affinity>` for `StatusImmunity` (blocks re-application → no lock loops).
PvP (future): duration × 0.5 and diminishing returns via the same immunity tag.

| Affinity | Status (Proposed name) | Proposed effect | Colour language |
|---|---|---|---|
| Fire | Burn | DoT | ember orange |
| Ice | Chill | move/attack slow | pale cyan |
| Lightning | Shock | brief interrupt on crit | white-violet |
| Shadow | Blight | healing reduction | ink black/purple |
| Holy | Judgment | bonus damage vs undead/demon | gold-white |
| Water | Soaked | amplifies Lightning (blocked: interaction = dual) | deep teal |
| Soul | Siphon | attacker regains resource on hit | spectral green |
| Earth | Sunder | armor reduction | ochre stone |
| Wind | Gale | knockback/pull | silver-green |
| Poison | Venom | stacking DoT | acid green |
| Temporal | Time Fracture | slow; no state rewind | bronze/silver clockwork |

Interaction matrix: **Blocked** by canon §5 Q1 (dual affinity). Only self-interactions (immunity) exist.

## 5. Flagship weapons (all `Proposed`, numbers `Provisional`)

| ID | Name | Family | Affinity | Rarity | iLvl/Req | Light | Heavy | Reach | Poise× | Crit | Buildup/hit | Dur/Immune s |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| W_Chronosunder | Chronosunder | Greatsword | Temporal | Legendary | 50/45 | 62 | 140 | 340 | 1.25 | 0.08 | 34 | 3/8 |
| W_Veilpiercers | Veilpiercers | Daggers | Shadow | Legendary | 50/45 | 32 | 70 | 220 | 0.6 | 0.18 | 15 | 4/10 |
| W_Worldbreaker | Worldbreaker | Hammer | Earth | Mythic | 55/50 | 70 | 165 | 300 | 1.8 | 0.04 | 25 | 5/12 |
| W_StarfallCovenant | Starfall Covenant | Bow | Holy | Legendary | 50/45 | 45 | 110 | 1200 | 0.7 | 0.15 | 20 | 4/10 |
| W_EpochsMercy | Epoch's Mercy | Staff | Temporal | Legendary | 50/45 | 35 | 80 | 320 | 0.8 | 0.06 | 20 | 3/8 |
| W_OathLastDawn | Oath of the Last Dawn | Shield | Holy | Legendary | 50/45 | 44 | 100 | 280 | 1.0 | 0.06 | 18 | 4/10 |
| W_HarvestersRequiem | Harvester's Requiem | Scythe | Soul | Legendary | 50/45 | 50 | 120 | 360 | 1.1 | 0.10 | 26 | 4/10 |
| W_AstralDominion | Astral Dominion | Spear | Lightning | Mythic | 55/50 | 55 | 130 | 420 | 1.0 | 0.12 | 22 | 2/8 |
| W_Ruinbrand | Ruinbrand | OneHandAxe | Fire | Legendary | 50/45 | 52 | 118 | 270 | 1.2 | 0.08 | 24 | 4/10 |
| W_UnwrittenTestament | The Unwritten Testament | Tome (OffHand) | Wind | Mythic | 55/50 | 30 | 75 | 600 | 0.5 | 0.10 | 20 | 3/8 |

Class suggestions (not restrictions; shared families stay cross-class): Chronosunder → Riftwalker/Bulwark;
Veilpiercers → Duskblade/Shadowblade; Worldbreaker → Warthane/Berserker; Starfall → Wildwarden;
Epoch's Mercy → Oracle; Oath → Templar/Bulwark; Harvester's → Necroturge/Dreadknight/Soulbinder;
Astral Dominion → Stormcaller/Spellblade; Ruinbrand → Berserker; Testament → Arcanist/Alchemist.
These are design pairings only — `AllowedClasses` is left empty on every asset.

### Per-item notes (lore/visual/awakening — all Proposed)
- **Chronosunder** — black steel, suspended bronze clock rings, celestial numerals. Awakening (3 stages, not implemented): Echo Strike replays a stored hit at 40% damage 0.6 s later, never re-procs.
- **Veilpiercers** — slender black blades, translucent edge segments; rewards perfect dodge (hook: `UEclipsePerfectDodgeComponent`).
- **Worldbreaker** — monumental hammer, suspended stone core; charged-slam shockwave (Proposed).
- **Starfall Covenant** — celestial-metal longbow with floating star ornaments; perfect-draw empowers next shot. Needs a projectile path; current build uses the melee sweep at 1200 cm as a stand-in.
- **Epoch's Mercy** — floating hourglass core cycling silver/gold; delayed healing echo (no history rewind).
- **Oath of the Last Dawn** — broad cathedral sword; guard-counter grants a ward (hook: `bGuardCounterOpen`).
- **Harvester's Requiem** — crescent blade with floating shards; Siphon sustain on marked targets.
- **Astral Dominion** — crystalline spear with orbiting rings; thrust momentum → armor-piercing finisher.
- **Ruinbrand** — asymmetrical split-core axe; successive heavies shred armor.
- **The Unwritten Testament** — floating grimoire, pages orbiting a crystal core; spell-combo charges (needs Spell system canon).

Acquisition (descriptive; loot system unwired): Legendary = dungeon boss or crafted equivalent; Mythic = raid/world boss
with bad-luck protection +1% per miss. Specific bosses/regions: **pending canon** (do not invent).

## 6. Unreal implementation (built this pass)

- `UEclipseItemDefinition : UPrimaryDataAsset` — `/Game/Data/Items/Weapons/DA_W_*` (balance data; art via soft refs).
- `UEclipseAttributeSet` — WeaponPower, CritChance, CritDamage, PoiseDamage, Armor, StatusBuildup, StatusThreshold.
- `UEclipseEquipmentComponent` on `AEclipseCharacter` — equip/unequip/cycle (`IA_CycleWeapon`, key T), applies
  `UEclipseGE_EquipStats` (infinite, SetByCaller), pushes combat stats, on-hit status via `UEclipseGE_Status`.
- `AEclipseCharacter` and `AEclipseTargetDummy` own a `UAbilitySystemComponent`.
- Logs: `[EQUIP]`, `[STATUS]`. Placeholder mesh `/Engine/BasicShapes/Cylinder` (no weapon meshes exist; spend gate).

## 7. Production gate (unchanged)
Before Phase D/E: resolve canon §5 Q1, define canon attributes, approve boss/region loot sources, and pass the spend gate.
