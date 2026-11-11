# ECLIPSE — Asset & Code Inventory

*Generated 2026-11-08 (plan `slate_heron_ledger_v1`). Asset facts come from asset-registry and Blueprint-status reads earlier in this session; the editor's Python bridge was unreachable during the write, so counts were not re-read at write time. Read-only — nothing was changed.*

## 1. Totals

| Item | Count | Notes |
|---|---|---|
| `/Game` assets | 562 | incl. ObjectRedirectors |
| Production C++ headers | 41 | `Source/EclipseRotF/` + `FrontEnd/` (canon §3 previously said 35) |
| Slice C++ headers | 19 | `Source/EclipseRotF/Aura/Slices/` (AetherbornCombo 9, AetherbornComboTiming 10) |
| Maps | 4 | see §4 |
| Blueprints / Widget BPs / Anim BPs | 14 / 8 / 1 | all `BS_UP_TO_DATE` except `WBP_KeyBindings` (**BS_ERROR**) |
| DataTables / DataAssets / Behavior Trees / Blackboards | **0** | verified absent |
| Sound assets | **0** | verified absent |
| Niagara systems | 1 | `/Game/VFX/NS_SolaraDustMotes` |
| C++ automation tests | **0** | no `IMPLEMENT_SIMPLE_AUTOMATION_TEST` / spec |

## 2. Production gameplay Blueprints

| Asset | Parent | Key settings (verified) |
|---|---|---|
| `/Game/Blueprints/BP_EclipseGameMode` | `AEclipseGameMode` | Default pawn `BP_AetherbornHero`; HUD/PC = engine defaults. Set as `GlobalDefaultGameMode` |
| `/Game/Blueprints/BP_AetherbornHero` | `AEclipseCharacter` | Mesh `SKM_Manny_Simple` + `ABP_Unarmed`; `HeroMesh` = `AETHERBORN_HERO_AAA` (hidden at BeginPlay); `LightAttackMontages` = AM_Light_01..03, `HeavyAttackMontage` = AM_Heavy |
| `/Game/Blueprints/BP_Pickup` | `Actor` | placed in Stage 1 arena |

## 3. UI widgets

| Asset | Status |
|---|---|
| `/Game/UI/WBP_EclipseHUD` | up to date |
| `/Game/UI/WBP_ResourceBar` | up to date |
| `/Game/UI/WBP_ActionSlot` | up to date (log shows an earlier failed compile, since fixed) |
| `/Game/UI/WBP_WeaponSlot` | up to date |
| `/Game/UI/WBP_KeyBindings` | **BS_ERROR — does not compile** |
| `/Game/Aura/Slices/AetherbornCombo/UI/WBP_AetherbornComboHUD` | Slice only |
| `/Game/Aura/Slices/AetherbornComboTiming/UI/WBP_AetherbornComboTimingHUD`, `WBP_AetherbornComboTimingTuning` | Slice only |

## 4. Maps

| Map | Role | Verified state |
|---|---|---|
| `/Game/Maps/Lvl_Stage1_Arena` | `GameDefaultMap` + `EditorStartupMap` | 5 actors: StaticMeshActor, PlayerStart, EclipseTargetDummy, SkeletalMeshActor, BP_Pickup_C. **No light, no sky.** No GameMode override |
| `/Game/Maps/Lvl_SolaraCathedral` | environment prototype | not opened in this audit |
| `/Game/Aura/Slices/AetherbornCombo/Maps/L_AetherbornCombo` | Slice | — |
| `/Game/Aura/Slices/AetherbornComboTiming/Maps/L_AetherbornComboTiming` | Slice | — |

## 5. Input

- `/Game/Input/`: `IA_Move, IA_Look, IA_LightAttack, IA_HeavyAttack, IA_Dodge, IA_Block, IA_Sprint, IA_LockOn, IA_Ability1..4, IA_Ultimate, IA_WeaponArt, IA_ToggleKeybinds` (15) + `IMC_Default`.
- Slice-only: `IMC_AetherbornCombo` (+2 IA), `IMC_AetherbornComboTiming` (+3 IA).

## 6. Animation

| Asset | Skeleton | Notifies |
|---|---|---|
| `/Game/Animations/Combat/AM_Light_01` | SK_Mannequin | Eclipse Hit Window, Eclipse Combo Window |
| `AM_Light_02`, `AM_Light_03` | SK_Mannequin | not re-checked |
| `AM_Heavy` | SK_Mannequin | Eclipse Hit Window |
| `AM_Dodge`, `AM_Cast_01`, `AM_HitReact_Med` | SK_Mannequin | none |
| `AM_HitReact_Lgt`, `AM_HitReact_Hvy` | SK_Mannequin | not re-checked |
| `/Game/Characters/Mannequins/` | template | Manny/Quinn, `ABP_Unarmed`, BlendSpace, Control Rigs, pistol/rifle/unarmed sets |
| `/Game/Aura/Slices/AetherbornComboTiming/Anims/` | AetherbornHero_Skeleton | 4 sequences + 4 montages (retargeted) |
| IK | — | `IK_AetherbornHero`, `IK_AetherbornSource`, `RTG_AetherbornHero` |

No Block / Parry source animation exists. No Motion Matching / Pose Search content.

## 7. Characters

**Rigged (skeletal):** `/Game/AuraModelGen/AetherbornHero/AetherbornHero/SkeletalMeshes/AetherbornHero` (used by the Timing Slice), `/Game/AuraModelGen/Aetherborn_1/Aetherborn/SkeletalMeshes/Aetherborn_1` (no known consumer), Manny/Quinn.

**Static, unrigged:** `/Game/AuraModelGen/Aetherborn/Aetherborn`, `/Game/AuraModelGen/AetherbornStormlord/AetherbornStormlord`, and 62 folders under `/Game/Characters/` (each = 1 StaticMesh + 1 MI + 3 textures):

| Group | Folders |
|---|---|
| Canon ancestries | AETHERBORN_* (8 variants), SOLARI(+HQ), KAZARI(+HQ), DRAVEN(+HQ), GRIMSTARR(+HQ), NEREYTH, MYCELIAR(+HQ), FORGEBORN(+HQ), HELLSWORN(+HQ), NECREVAN(+HQ) |
| Canon classes | BULWARK_HQ, DUSKBLADE_HQ, ARCANIST_HQ/SHEET, RIFTWALKER(+HQ), DREADKNIGHT_HQ, TEMPLAR(+HQ), SOULBINDER(+HQ), STORMCALLER(+HQ), ORACLE(+HQ), ALCHEMIST_HQ/REF_AAA, BERSERKER(+HQ), SHADOWBLADE_HQ, SPELLBLADE_HQ, NECROTURGE_HQ, EARTHWEAVER_HQ |
| **Not canon names (C10)** | NECROMANCER(+HQ, REF_AAA), DRUID(+HQ), BARBARIAN_AAA, SORCERESS_AAA, KNIGHT_REF_AAA, ASSASSIN_HQ, MUSCULAR_AAA, ASHFALLEN, DUSKBORN, STORMFANG — NECROMANCER/DRUID predate the `ecdf880` rename to Necroturge/Earthweaver |
| Misfiled | SOLARA_CATHEDRAL_AAA (environment, in `/Characters`) |
| **Missing in UE** | Ancestries Vyrael, Tharuk, Umbralis; classes Wildwarden, Warthane (Blender `.mtl` outputs existed for VYRAEL/UMBRALIS/WARTHANE/WILDWARDEN but are now deleted in the working tree) |

## 8. Materials, VFX, textures

- `/Game/Generated_Materials/M_AfterImage_Ghost` (perfect-dodge afterimage, canon §4).
- `/Game/Generated_VFX/Aetherborn/`: `M_AB_Lightning`, `M_AB_SoftGlow`, `T_LightningArc_A/B`, `T_LightningWing_A/B` — no Niagara system uses them.
- `/Game/AuraModelGen/Material_AuraModelGen_Lit`.
- Slice materials: `/Game/Aura/Slices/AetherbornCombo/Materials/*` (2 masters + 13 MIs), textures `T_AetherbornCombo_*`.

## 9. Hygiene findings

| Finding | Paths |
|---|---|
| ObjectRedirectors | `/Game/AuraModelGen/Aetherborn/{tripo_mat_727f0bd9, tripo_model_basecolor}`, `/Game/AuraModelGen/AetherbornStormlord/{tripo_mat_f54c1f6a, tripo_model_basecolor}`, `/Game/AuraModelGen/AetherbornHero/.../Materials/tripo_material_df15840e-…`, `/Game/AuraModelGen/Aetherborn_1/.../Materials/tripo_material_f6b8a96b-…` |
| Duplicate texture | `/Game/Generated_Images/T_AetherbornCombo_StoneTile` vs `/Game/Aura/Slices/AetherbornCombo/Textures/T_AetherbornCombo_StoneTile` |
| Stale audit | `ECLIPSE_PROJECT_AUDIT.json` (2026-11-06) says 12 ancestries, no Blueprints/levels — historical only |
| Version control | `*.uasset` / `*.umap` are gitignored; newest C++ is untracked (see `ECLIPSE_THREAD_AUDIT.md` §3) |

## 10. Plugins / modules (verified)

`Build.cs`: Core, CoreUObject, Engine, InputCore, EnhancedInput, Niagara, NiagaraCore, GameplayTags, GameplayTasks, **GameplayAbilities** (present but unused); private Slate, SlateCore, UMG, AIModule. `.uproject` enables Aura, Niagara, EnhancedInput, ModelContextProtocol, PythonScriptPlugin, AccumulationDOF, BinkMedia, TextureShare, StaticMeshEditorModeling, ShaderToolkit.

## 11. Non-Unreal project files

Root directives (13 .md/.txt/.json, see `ECLIPSE_CANON.md` §1b), `Docs/` (combat directive v1.1, Solara Cathedral design), `Content/Python/` (Stage 1 + Blender pipeline scripts, 33 .py), `ancestry_packs/` (120 files), `HANDOFF_FORGE_ENVIRONMENT/` (16), `HANDOFF_FORGE_TRAILER/` (43).
