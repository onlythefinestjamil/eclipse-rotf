# ECLIPSE V9 — HERMES DIRECTOR PROMPT FOR AURA
# Saved: November 7, 2026 1:00 AM EST
# Status: AUTHORITATIVE — supersedes all prior directives for Aura integration

## CORE PRINCIPLES
- Preserve confirmed decisions: 13 ancestries, 17 classes, original Aetherborn
- No direct copying of Elden Ring, Wukong, WoW assets
- Study principles: grounded anticipation, responsive evasion, purposeful recovery
- Inspect before promising. Plan before executing. Test before claiming done.

## AURA WORKFLOW (per task)
1. Ask (inspect current state)
2. Plan (propose changes, list files)
3. Review planned changes
4. Execute narrowly in Agent
5. Compile
6. PIE test
7. Record video/logs
8. Autopilot where applicable
9. Hermes independently review + regress
10. Merge only after acceptance

## COMBAT QUALITY TARGET
- Camera-relative locomotion, stop/turn/pivot transitions
- Sprint, jump, dodge, perfect dodge, block, perfect parry
- Guard counters, stagger/poise, light/heavy combos
- Charge attacks, aerial moves, lock-on, weapon arts
- Spellcasting, interrupts, resource consumption, cast bars
- Action bars, configurable GCD, explicit animation cancel windows
- Dodge: 8-direction grounded, 100-160ms input buffer, 2.5-4m travel
- Dodge: 24-34 frames total, 8-13 invulnerable frames @ 60 FPS (EXPERIMENTAL)
- No universal cancel — dodge only from idle/recovery/approved windows

## ANIMATION + MODEL DIRECTION
- Original animation keys with grounded weight
- Clear start, anticipation, hip/shoulder mechanics, planting
- Test muscular Black male Aetherborn with locs
- Luminous fractures + matching energy wings per approved imagery
- Afro-textured hairstyles protected from clipping/flattening
- Generated models are REFERENCES until topology/rig/skinning/LOD passes

## STAGED BUILD QUEUE
- Stage 1: One playable class (Bulwark), Aetherborn test character, dodge + perfect dodge, one enemy with 3 telegraphed attacks, stamina bar, damage logging
- Stage 2: Attack combo windows, block/parry/guard counter, stagger, enemy reactions, camera/impact feedback, deterministic hit events
- Stage 3: Arcanist spellcasting (cast bar, mana, interrupts), VFX on surfaces, healer NPC
- Stage 4: Multiplayer 2-client predicted movement/dodge/combat, latency tests, PvP DR
- Stage 5: Second race rig, expanded weapons, 5-player dungeon + 10-player raid samples
- Stage 6: Scale to 17 classes, unique animation/VFX families, automated combat tests

## ACCEPTANCE CRITERIA
- Video clips in neutral light + actual combat
- Animation timing overlays, movement/velocity traces
- Stamina/cooldown logs, screenshots of changed Blueprints/C++
- Automated test report, profiling captures
- Repeat at 30/60 FPS, controller + KB/mouse, different camera yaw
- Test: slopes, obstacles, lock-on on/off, low stamina, during recovery, stunned, near ledges, 2 network clients with latency

## AURA TASK FORMAT (per feature)
1. Reference bible IDs
2. Expected outcome
3. Current project/asset context
4. Constraints and state machine
5. Files to inspect
6. Files allowed to alter
7. Replication authority
8. VFX/audio/animation requirements
9. PIE verification cases
10. Deliverables
