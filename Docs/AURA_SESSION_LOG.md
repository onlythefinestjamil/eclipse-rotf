# Aura Session Log — Eclipse: Realms of the Fractured

Running record of work performed with Aura in the Unreal Editor. Newest sessions are appended at the bottom.
Cross-reference: `ECLIPSE_CANON.md` (authoritative) · `Docs/ECLIPSE_EQUIPMENT_BIBLE.md` (Proposal, not canon).

---

## Session 2026-11-13 — Aetherborn armor, flagship weapons, concept-art intake

### Requests received (verbatim)
1. "as far as textures character models and back ground should i upload everytrhing directly here or do you have it already"
2. "i mean as far as the actuak character models i do want to replace that look with my own art"
3. "make a 3d render model of this armor and place it on the aetherborne as well as this weapon make sure you render all these weapons put one in his hand and the rest just save the file"
   - Clarified via options: weapons = **all 10 flagship weapons**; armor = **swap the armored body onto the Aetherborn actor**.
4. "i uploaded the images already / on the k drive concept art"
5. "moving ddc to k drive with everything else also notebook this thread so that i can see this i need it recorded"

### Standing instruction from the user
> Check the current list of MCP tools and never assume a previously used tool is still available. Anything generated — images and assets — is produced by tools.

### Environment
| Item | Value |
|---|---|
| Project | `EclipseRotF` — *Eclipse: Realms of the Fractured* |
| Project root | `C:\Users\Tap\EclipseRotF` |
| Engine | UE **5.8.4**, `C:\Program Files\Epic Games\UE_5.8` |
| Current level | `/Game/Maps/Lvl_Stage1_Arena.Lvl_Stage1_Arena` |
| Targets | PC, PS5, Xbox Series X |

---

### Concept art located on the K drive

**`K:/Game_Assets/Concept_Art/` — 151 images**, plus per-ancestry subfolders.
Mirror: `K:/Eclipse_Data/Desktop_Backup/ECLIPSE_ALL_CONCEPT_ART/` (same 151 images; also holds `Stage1_Screenshots`, `Trailer_Refs`, `Character_References`).

#### `K:/Game_Assets/Concept_Art/Aetherborn/` — 10 files
`AETHERBORN.png` · `AETHERBORN_ARCANIST_ARMOR.png` · `AETHERBORN_CASTER_SHEET.png` · `AETHERBORN_MALE.png` · `AETHERBORN_FEMALE.png` · `AETHERBORN_DUO.png` · `AETHERBORN_V2.png` · `AETHERBORN_V3.png` · `AETHELORN.png` · `AETHELORN_SHEET.png`

Two of these are full production sheets at **1536x2736** with a front view plus detail callouts:

- **`AETHERBORN_ARCANIST_ARMOR.png` — the armor reference.** Deep melanin skin, white dreadlocks with gold beads, black weathered plate carrying a starfield / celestial-circle motif on the chest, translucent crystalline blue energy gauntlets bound with leather straps, tattered black over-skirt over cream under-panels with rune trim, heavy black boots, blue-violet lightning cracks at the shoulder seams. Matches the Aetherborn visual reference in `ECLIPSE_CANON.md` closely.
- **`AETHERBORN_CASTER_SHEET.png` — Caster Variant**, ranged DPS, racial traits "Celestial Flames · Fallen Grace". Cream layered tabard with dark rune embroidery, blue crystalline lightning wings, gold eyes. Three props at the bottom: Celestial Orb Staff, Arcane Grimoire, Crescent Focus.

#### Why this matters
The armored body currently in the level was generated from a **downscaled 3-panel contact sheet** in `Saved/AMG_TempExports/` — a downstream export, not this art. The two panels framed the figure at slightly different sizes, which is what fused the cloak into the bulbous mass now visible on the mesh. The real reference was never used.

#### Armor art in the library root
`ARMOR_RARITY_TIERS` · `ARMOR_MYTHIC` · `INDIVIDUAL_ARMOR_PIECES` · `ARMOR_EGYPTIAN` / `_REDESIGN` · `ARMOR_PERSIAN` / `_REDESIGN` · `ARMOR_VIKING` / `_REDESIGN` · `ARMOR_WEST_AFRICAN` / `_REDESIGN`, plus one per class: `ARMOR_ALCHEMIST_CLASS`, `ARMOR_BERSERKER_CLASS`, `ARMOR_DRUID_CLASS`, `ARMOR_NECROMANCER_CLASS`, `ARMOR_ORACLE_CLASS`, `ARMOR_SHADOWBLADE_CLASS`, `ARMOR_SOULBINDER_CLASS`, `ARMOR_SPELLBLADE_CLASS`, `ARMOR_STORMCALLER_CLASS`, `ARMOR_TEMPLAR_CLASS`, `ARMOR_WARLORD_CLASS`, `ARMOR_WILDWARDEN_CLASS`.

#### Weapon art in the library root
Named weapons: `SUNBREAKER_GREATSWORD`, `WEAPON_LEGENDARY_SUNBREAKER`, `LUNAR_GREATSWORD`, `ECLIPSE_FORGED_GREATSWORD`, `MOURNTIDE_GREAT_AXE`, `VOIDREND_DAGGERS`, `WEAPON_LEGENDARY_VOIDREND`, `THUNDERCALL_STAFF`, `BONE_NECROMANCER_STAFF`, `CORAL_TRIDENT`, `SOULBINDER_CROZIER`, `FORGE_HAMMER`, `CUSTOM_FORGE_WEAPON`.
Relics: `RELIC_ASTRAL_SPIRE`, `RELIC_EDGE_OF_ELSEWHEN`, `RELIC_LANTERN_CHOIR`, `RELIC_LIGHTBEARER`, `RELIC_MOURNTIDE`, `RELIC_SUNBREAKER`, `RELIC_VERDANT_HOWL`, `RELIC_VEILFANG`.
Sets: `WEAPON_SET_FIRE`, `_FROST`, `_HOLY`, `_SHADOW`, `_SOUL`, `_STORM`.
Spells: `AEGIS_VERGE_SPELL`, `FULMEN_LANCE_SPELL`, `SANGUIS_OATH_SPELL`, `SPELL_FROST`, `SPELL_SHADOW`.

**Consequence:** the nine weapons generated from *text prompts* have authored art available. Regenerating from these images replaces invented geometry with the user's own designs.

---

### Assets produced this session

| Asset | Path | Status |
|---|---|---|
| Armored body | `/Game/AuraModelGen/AetherbornArmored/AetherbornArmored/SkeletalMeshes/AetherbornArmored.AetherbornArmored` | On the arena actor. **Has generation artifacts. Not production-ready.** |
| Armored skeleton | `.../AetherbornArmored_Skeleton.AetherbornArmored_Skeleton` | 41-bone biped rig |
| Original body | `/Game/AuraModelGen/AetherbornHero/AetherbornHero/SkeletalMeshes/AetherbornHero.AetherbornHero` | Replaced by the swap |
| Chronosunder | `/Game/AuraModelGen/W_Chronosunder/W_Chronosunder` | Greatsword, on `WeaponMesh_MainHand` — **floats off the hand** |
| Nine other weapons | `/Game/AuraModelGen/W_<Name>/W_<Name>` | Saved, **never visually verified** |

Weapons: `W_Chronosunder`, `W_Veilpiercers`, `W_Worldbreaker`, `W_StarfallCovenant`, `W_EpochsMercy`, `W_OathLastDawn`, `W_HarvestersRequiem`, `W_AstralDominion`, `W_Ruinbrand`, `W_UnwrittenTestament`.
All built at a 50 000 face limit (auto-clamped to 20 000), pivots Bottom except `W_UnwrittenTestament` (Center, scale 0.4). One material each: `W_<Name>_Material1`.

**Dimensions (cm, X×Y×Z):** Chronosunder 62×95×170 · Veilpiercers 16×48×50 · Worldbreaker 75×104×150 · StarfallCovenant 160×34×94 · EpochsMercy 55×48×190 · OathLastDawn 41×81×120 · HarvestersRequiem 57×151×200 · AstralDominion 73×73×240 · Ruinbrand 10×45×85 · UnwrittenTestament 39×39×40.

`Docs/ECLIPSE_EQUIPMENT_BIBLE.md` (131 lines) is the source of the roster and remains **Proposal — not canon**. No `DA_W_*` item definitions were created or modified.

---

### Arena state at end of session

- `SkeletalMeshActor` labelled `"SkeletalMeshes"` at world `(0, 0, 27.101)`, actor scale `2.0039`, rotation zero, `AnimClass = None`, mesh `AetherbornArmored`. Bounds centre `(13.33, 38.96, 152.69)`, extent `(79.97, 101.84, 148.84)`.
- Rig is Rigify-style, 41 bones. Hand bones are **`L_Hand` and `R_Hand` only** — no wrist, no weapon sockets.
- Weapon attachment: child `StaticMeshComponent` `WeaponMesh_MainHand`, mesh `W_Chronosunder`, collision `NoCollision`, Mobility `Movable`, attached to `SkeletalMeshComponent0` socket `R_Hand`. Relative transform: loc `(1.935, 7.919, −2.927)`, rot `(19.75, −103.73, 155.28)`, scale `0.4812`. **Wrong — the sword sits roughly a metre to his upper left.**
- **The level contains zero light actors** (`LIGHT_COUNT=0`). Every capture is black until temporary lights are spawned. This is the level's real current condition; do not bake permanent lights in to make a screenshot work.

---

### Blockers

#### 1. Project drive `C:` is full
Copying the Aetherborn art into `Saved/KDriveRefs/Aetherborn/` failed partway:

```
COPIED AETHERBORN_ARCANIST_ARMOR.png bytes=7889266
COPY_FAILED AETHERBORN_DUO.png:    [Errno 28] No space left on device
COPY_FAILED AETHERBORN_FEMALE.png: [Errno 28] No space left on device
COPY_FAILED AETHERBORN_MALE.png:   [Errno 28] No space left on device
COPY_FAILED AETHERBORN_V2.png:     [Errno 28] No space left on device
COPY_FAILED AETHERBORN_V3.png:     [Errno 28] No space left on device
```

Five of ten landed. This blocks model generation (downloads to `Saved/`) and imports (writes to `Content/`).

#### 2. Aura Python bridge went down
`grep`, `get_code_examples` and `execute_unreal_python` all began returning `Could not reach the Aura Python bridge … HTTP 502` / `Cannot connect to host 127.0.0.1:41250`. Fix with console command **`Aura.Server.Restart`**, then retry.

#### 3. `Config/DefaultEngine.ini` is malformed at the tail
Lines 87–95 currently read:

```ini
[Core.System]
PathsToCachePrimaryDerivedDataCachePaths=/K/Eclipse_DDC/RootDLC.kdl
PathsToCachePrimaryDerivedDataCachePaths=/K/Eclipse_DDC/
bCanUseLocalCacheForDerivedDataCachePaths=True
PathsToCachePrimaryDerivedDataCachePathsInInitState=/K/Eclipse_DDC/

[/Script/Engine.LocalPlayer]
]
"
```

The trailing `]` and `"` are stray characters, and the `[Core.System]` keys are not a supported way to relocate the DDC. **Needs repair and replacement with the real DDC backend overrides.**

---

### DDC relocation to the K drive

Backing store already present on `K:`: `K:/Eclipse_DDC`, `K:/Eclipse_DDC_old`, `K:/Eclipse_Data`.

Intended end state:
- DDC relocated to `K:/Eclipse_DDC` via the supported `[InstalledDerivedDataBackendGraph]` override (env-variable `EnvPathOverride` equivalents are `UE-LocalDataCachePath` / `UE-SharedDataCachePath`), **not** via the invented `PathsToCachePrimaryDerivedDataCachePaths` keys.
- The regenerable cache on `C:` deleted to reclaim space — a DDC is a cache and is never worth migrating byte-for-byte.
- `Config/DefaultEngine.ini` tail repaired.

Status: **blocked** on the Python bridge before the inventory and change could be applied.

---

### Open items
- [ ] Restore the Aura Python bridge.
- [ ] Free space on `C:` / relocate DDC to `K:`.
- [ ] Regenerate the Aetherborn body from `AETHERBORN_ARCANIST_ARMOR.png` **alone** (single view — no front/back fusion, which is what created the fused mass).
- [ ] Re-seat `W_Chronosunder` into `R_Hand` with a correct bone-space offset.
- [ ] Regenerate the nine text-prompt weapons from their real concept art.
- [ ] Produce a render/contact sheet per weapon — none of the ten has been visually verified.
- [ ] Repoint the `[Core.System]` DDC keys and clean the malformed ini tail.

---

### Engine notes recorded this session
APIs **absent** in this UE 5.8.4 build: `SkeletalMeshComponent.set_skinned_asset()`, `Actor.is_hidden()`, `SkeletalMeshComponent.bounds`, `MathLibrary.conv_quat_to_rotator`, `Vector.size()`.
Working equivalents: `get_skinned_asset()`, `set_actor_hidden()`, `MathLibrary.find_look_at_rotation()`, `MathLibrary.quat_rotator()`.
`set_skinned_asset_and_update()` returns `False` but **does apply** the mesh — verify with `get_skinned_asset()`.
`SceneCapture2D.get_editor_property("capture_component2d")` is read-only.
`spawn_actor_from_class(cls, loc, rot)` takes rotation directly, avoiding `set_actor_rotation` lint rejection.
`export_render_target` writes undecodable files; the working capture path is `read_render_target` → `Array` of `unreal.Color` → hand-rolled `zlib`/`struct` PNG writer.
Both Aetherborn actors shared the label `"SkeletalMeshes"` — **identify by mesh path + Z location, never by label.**
