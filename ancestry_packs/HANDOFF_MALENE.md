# HANDOFF → MALLENE — 13-ANCESTRY PRODUCTION PACKS (Nov 7, 2026)

Two packs, both from Jamil's production pipeline:

1. `Eclipse_13_Ancestries_Reference_and_Hermes_Prompts.zip` — START HERE.
   README_START_HERE.txt, the master directive (HERMES→AURA), the prior master prompt,
   MANIFEST.json, and GLOBAL_REFERENCES (roster collages + character art).

2. `Eclipse_13_Ancestries_Hermes_All_Tools_Full_Pipeline_v2.zip` — the full toolchain pack:
   13 ANCESTRIES folders, each with the identity-locked FRONT/BACK/3D prompt,
   a low-res roster preview (NOT a deliverable), STATUS.txt, and TOOLCHAIN_STAGES_AND_QA.txt.

Key directive points (from HERMES_READ_FIRST_MASTER_DIRECTIVE.txt):
- The 13 ancestry folders ARE the complete playable ancestry roster — not classes.
  Do not merge/swap/rename. Muscalar is an Aetherborn type, not a 14th ancestry.
- Previews are not deliverables. Each ancestry needs unique, consistent,
  high-resolution FRONT + BACK full-body references before modeling.
- Phases: ingestion → design lock → front-accepted-then-back → 3D blockout/sculpt/
  retopo/UV/bakes → rig/cloth/hair/collision → UE5 import/LODs → QA gate
  (front-back continuity, no prompt drift, finger/foot validity, versioned files).
- Filename schema: ECL_<Ancestry>_<Identity>_<VIEW>_FULLBODY_v001.png

— Forge (Mac), relaying for Jamil

## UPDATE — unpacked & browsable

Both packs are now ALSO unpacked in-repo (no unzipping needed):
- ancestry_packs/Eclipse_13_Ancestries_Hermes_Pack/       (reference + prompts — start here)
- ancestry_packs/Eclipse_13_Ancestries_Full_Toolchain_Pack/ (full toolchain pipeline)

The zips remain for intact transfer. Duplicate Downloads copies (" 2" / " 3") were ignored — one canonical copy each.

## UPDATE 2 — RUNWAY AGENT 13 CHARACTER CONTACT SHEETS

`Runway Agent - 13 generations.zip` — 13 high-res character contact sheets generated from the
global references, one per ancestry: Aetherborn (hero), Solari, Vyrael, Tharuk, Kazari, Umbralis,
Draven, Grimstarr, Nereyth, Myceliar, Forgeborn, Hellsworn, Necrevan.

USE: these are GENERATED CONTACT SHEETS from Runway's Agent — treat them as strong visual
starting points for Phase 1 (design lock), not as accepted front/back deliverables. Cross-check
each against its ancestry folder's identity-locked prompt before generating the accepted
FRONT/BACK references. Filenames map 1:1 to the ancestry folders.

## UPDATE 3 — THE 30-SECOND TRAILER (SEEDANCE 2.5 RENDER)

`eclipse_trailer_seedance_30s.mp4` — the full 30-second cinematic trailer rendered through
Seedance 2.5 from the canon trailer prompt (all 13 ancestries, hero lock, lore summary).

SPECS: 30.08s, 1920x1080, HEVC 10-bit, 24fps, 14.2 Mbps, stereo AAC — 51MB.

USE: this is the first full-length render — reference it for pacing, VO timing, and the
ancestry montage when we do the Runway Gen-4 / higher-fidelity pass. Extract individual
shot feedback from Jamil before any re-render.

## UPDATE 4 — EXPANDED CANON: 20 BOSS + ENEMY CONTACT SHEETS

`ancestry_packs/bosses/` — 20 Runway Agent contact sheets for the expanded-canon roster
(19 bosses + 1 enemy type): Abyssal Maw, Aether Titan, Ashen Karthul, Bramblevil, Cruxaris,
Fracture Cultist (enemy), Frostjaw, Glacier King, Ironcoil, Maw of Despair, Mycelial Queen,
Nethermind, Pyre-Warden Velkor, Skyshatter, Solarion, Stormcaller Vex, The Eclipsed Sovereign,
Veilweaver, Wyldheart, Zar-thul.

USE: matches the 24-boss Expanded Canon lore sheet. Same rule as the ancestry sheets —
design-lock starting points, validate each against the roster entry before accepted
front/back references. The Eclipsed Sovereign is the flagship boss.

## UPDATE 5 — 20 ENVIRONMENT / STAGE PLATES (RUNWAY TEXT-ONLY PASS)

`ancestry_packs/environments/` — 20 plates named STAGE_<name>_<view> per the schema:
- SOLARA CATHEDRAL (5): exterior_courtyard, entrance_lowangle, nave_interior_hero, stained_glass_detail, temporal_collapse_interior — MATCH THESE AGAINST THE IN-PROGRESS UE5 BUILD
- ASHEN FORGE (4): wide_establishing, walkway_highangle, anvil_row_floor, anvil_closeup — matches the canon forge concept
- Plus: gothic_city (aerial), desert_cathedral, forge_dungeon, duel_circle, cosmic_void_arena, shadow_wasteland, verdant_deep, abyssal_depths, frost_peaks, floating_isles, fracture_vista (aerial)

NOTE: these are TEXT-ONLY generations (no reference images attached). Valid design-lock
starting points for unnamed regions; for the Solara Cathedral and any stage already in UE5
production, hold final acceptance until the image-to-video pass with the 15 reference
images from HANDOFF_FORGE_ENVIRONMENT (still not pushed to GitHub as of this commit).

## UPDATE 6 — FULL BESTIARY + NPC ROSTER (26 sheets)

`ancestry_packs/enemies/` — 11 enemy types: Abyssal Scavenger, Blight Monstrosity, Eclipse
Berserker, Fracture Executioner, Frost Reaver, Mycelial Sporeling, Rift Sentinel, Solar
Inquisitor, Stonebound Brute, Thornling, Void Stalker.

`ancestry_packs/npcs/` — 15 NPCs: Aether Warden, Alchemist, Banker, Blacksmith, Bounty
Master, City Guard, Crafting Master, Faction Emissary, Guild Officer, Healer, Innkeeper,
Lorekeeper, Portal Guide, Quest Giver, Trade Vendor.

These complete the visual chain for Unreal/Aura: ancestries (13) + bosses (19+enemy) +
enemies (11) + NPCs (15) + stage plates (20) = the full aesthetics reference library.
Same rule: design-lock starting points — validate against lore entries before UE5/Aura
production passes.

## UPDATE 7 — SECOND WAVE: DUEL CIRCLE / SOVEREIGN RIFT / VERDANT DEEP / GOTHIC CITY / FRACTURE + FOUNDRY + 8 CLIPS

environments/ now 80 plates: adds full 4-shot sets for Duel Circle, Sovereigns Rift,
Verdant Deep, Gothic City, The Fracture + Forgeborn Foundry-Temple set + duplicate set
preserved (_dup2 suffix = identical second download; use either).

environment_clips/ — 8 VIDEO clips (image-to-video environment motion): gothic city
aerial drift, time-broken cathedral nave push-in, cosmic arena push-in, frost monastery,
desert cathedral facade crane, fractured continent aerial, submerged gothic city,
forge dungeon dolly (molten metal). These are MOTION references for level ambience,
matinee sequences, and the Gen-4 trailer pass.
