============================================
  ECLIPSE V6: DAY 1 VISUAL SLICE DIRECTIVE
  Momentum Labs | UE5.8
============================================

APPROVED DECISIONS:

01 · Hero: Aetherborn — muscular build, luminous fracture
    markings, ethereal energy wings. Preserve existing
    approved character design and inclusive customization.

02 · Environment: Solara Cathedral — exterior courtyard,
    surrounding ruins, Gothic cathedral interior, instanced
    boss chamber. Seamless navigation, Lumen lighting,
    Nanite geometry, volumetric fog.

03 · Boss: Cathedral-compatible canonical boss from V6's
    14-boss roster. Three phases, telegraphed attacks,
    destructible arena, temporal/fracture mechanics,
    memorable finishing animation.

04 · Combat: Hybrid — Wukong fluid combos + Elden Ring
    weight/commitment + WoW ability depth/GCD/cooldowns.
    Manual mastery: timed strikes, weak points, parries,
    perfect dodges. No guaranteed crits on every input.

05 · Computer: Single guided UE5 editor session. Queue
    all editor-dependent steps in order. Do not claim
    compilation/imports/packaging succeeded until verified.

COMBAT RULES (LOCKED):
- Manual mastery: timed crits, weak points, parries,
  perfect dodges — meaningful advantages, not guaranteed
- Hybrid GCD: GCD governs abilities; basic attacks,
  dodges, blocks, reactions stay responsive
- Animation quality: blending, locomotion matching, hit
  reactions, controlled cancels, animation notifies for
  damage timing
- Spell VFX: Niagara illuminates surfaces, communicates
  hitboxes, scales to avoid overwhelming visibility
- Companions: same combat framework as player — dodges,
  interrupts, cooldowns, boss mechanics
- Performance: 60 FPS target with scalability for Lumen,
  fog, particles, shadows

MANDATORY PROOF OF COMPLETION:
1. Successful Unreal compilation
2. Playable Aetherborn character
3. Fully functional combat and animations
4. Complete Solara Cathedral environment
5. Functioning three-phase boss
6. AI companion support
7. Working inventory, loot, and save/load
8. Packaged Windows executable
9. Actual gameplay screenshots/video + performance data

STATUS REPORTING: implemented → compiled → editor-tested →
packaged-tested. These are NOT interchangeable.

PRODUCTION PRINCIPLE: Building the eventual complete
Eclipse game, not a disposable demo. Reuse approved
technical and visual foundations across all future
content.

NEXT GATE: 10-minute playable experience > 100 untested
C++ files. Player must be able to: launch Eclipse,
explore Solara, use abilities, fight with companions,
defeat the boss, receive loot, save progress.

============================================
