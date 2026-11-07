============================================
  ECLIPSE: NEXT DEVELOPMENT DIRECTIVE
  Momentum Labs | Master Game Bible V6 | UE5.8
============================================

Master Game Bible V6 is the current authoritative design document.
Do not revert to V3 or earlier class rosters, abilities, combat rules,
or architectural decisions.

Continue building Eclipse as a complete, visually polished single-player
RPG, with AI companions for dungeons and raids and multiplayer-compatible
architecture for future expansion.

Before implementing new features, verify all changes against V6.
Preserve the current 17-class and 45 dual-affinity design unless V6
specifies otherwise.

Prioritize:
  1. EclipseBossDirector
  2. EclipseRaidDirector
  3. EclipseQuestSystem
  4. Visual production pipeline
  5. Integration testing

Do not modify files owned by the active Spell System subagent.
Coordinate shared interfaces before merging changes.

DEFINITION OF COMPLETE:
  - C++ compiles
  - Unreal loads the system
  - Gameplay works in a packaged build
  - Automated tests pass where applicable
  - Visual or gameplay evidence is provided

No system should be marked complete based solely on the existence
of source files.

============================================
BUDGET FRAMEWORK
============================================

Stage 1: Verify existing C++ systems and toolchain     $0-500
Stage 2: Build one highly polished playable area        $1K-5K
Stage 3: Expand bosses, AI, content workflows           $3K-10K
Stage 4: Add substantial world content and polish        $10K-30K+

These are incremental spending allowances, not guaranteed prices.
Assumes capable hardware, personal labor, and reusable assets.

CURRENT SPEND: $20/mo Meshy API only. No other spend approved.

GATE: Before any spend beyond $500, Hermes must demonstrate:
  - Working Windows build
  - Playable character with combat
  - One AI companion
  - One enemy
  - Functional spellcasting
  - Working save/load

Then add one finished boss and a visually polished environment.

That test proves whether AI-assisted development is genuinely
reducing cost or generating code that still needs debugging.

============================================
Momentum Labs / Illustrators Entertainment LLC
November 2026
============================================
