# Eclipse QA — Regression Test Specifications
**Generated:** 2026-11-12 by QA Director (cron)
**Status:** SPEC ONLY — no automation tests exist yet. These are the test cases that MUST pass before any milestone gate.

---

## 1. Combat System (EclipseCombatSystem + EclipseCombatComponent)
- **GCD-01:** TriggerGCD(1.5f) sets GCDRemaining=1.5; after 1.5s, IsOnGCD() returns false.
- **GCD-02:** SetGCDForClass for each of 17 classes sets correct GCD duration.
- **GCD-03:** Off-GCD abilities (dodge, block, parry) can execute while GCD active.
- **DODGE-01:** Perfect dodge within i-frame window triggers slow-mo + afterimage ghost.
- **DODGE-02:** Dodge outside i-frame window takes full damage.
- **PARRY-01:** Parry within parry window negates damage + applies stagger to attacker.
- **PARRY-02:** Parry outside window takes partial damage.
- **HITSTOP-01:** Hit-stop pauses game for configured duration on heavy hit.
- **HITSTOP-02:** Hit-stop does not affect UI animations.

## 2. Spell System (EclipseSpellSystem)
- **SPELL-01:** GetClassSpells(Bulwark) returns exactly 22 spells.
- **SPELL-02:** GetClassSpells for all 17 classes returns 22 each (374 total).
- **SPELL-03:** GetDualAffinitySpells() returns 46 spells.
- **SPELL-04:** GetDualAffinitySpell(Fire, Ice) returns "Thermal Shock".
- **SPELL-05:** All 45 unique element pairs have at least one dual affinity spell.
- **SPELL-06:** SpellIDs are sequential 0..N with no gaps.
- **SPELL-07:** No duplicate SpellNames across all 420 spells.

## 3. Talent System (EclipseTalentSystem)
- **TALENT-01:** AllTalents.Num() == 129.
- **TALENT-02:** Bulwark tree has 33 nodes across 5 layers.
- **TALENT-03:** Each of 16 placeholder classes has exactly 6 Class Foundation nodes.
- **TALENT-04:** PrerequisiteTalentIDs in Bulwark tree form a valid DAG (no cycles).
- **TALENT-05:** No orphan capstone (every Capstone has prerequisite chain to Foundation).
- **TALENT-06:** MaxTalentPoints == 30; cannot exceed.
- **TALENT-07:** AddSynergy creates valid bidirectional synergy links.
- **REGRESSION:** When placeholder trees are filled, re-run TALENT-04 through TALENT-06 for each class.

## 4. Boss Director (EclipseBossDirector)
- **BOSS-01:** AllBosses.Num() == 14.
- **BOSS-02:** Each boss has 3-4 phases with valid HP thresholds (100%→0%).
- **BOSS-03:** Each phase has at least 2 FBossAttack entries with TelegraphDuration > 0.
- **BOSS-04:** StartEncounter sets CurrentPhaseIndex=0 and Phase1 active.
- **BOSS-05:** CheckPhaseTransition advances phase when HP drops below threshold.
- **BOSS-06:** Enraged phase activates when EnrageTimer expires.
- **BOSS-07:** BossTakeDamage reduces HP and may trigger phase transition.
- **BOSS-08:** ResetBoss restores all boss state to Phase1.

## 5. Quest System (EclipseQuestSystem)
- **QUEST-01:** AllQuests.Num() == 135.
- **QUEST-02:** Main campaign has 12 quests (IDs 1-12) with linear prerequisites.
- **QUEST-03:** Side quests: 32 quests (IDs 100-131) across 6 zones.
- **QUEST-04:** Class quests: 17 quests (IDs 200-216), one per class.
- **QUEST-05:** Faction quests: 36 quests (IDs 300-335), 6 factions × 6 ranks.
- **QUEST-06:** Companion quests: 6 quests with branching Ch() choices (loyal vs departed).
- **QUEST-07:** Fracture quests: 6 quests with branching (sealed vs harnessed) + world flags.
- **QUEST-08:** Hidden quests: 12 quests with no prerequisites (discovery-based).
- **QUEST-09:** SetWorldFlag persists flag; FlagReq gates quest availability.
- **QUEST-10:** AbandonQuest resets quest state; FailQuest locks branching path.

## 6. Loot System (EclipseLootSystem)
- **LOOT-01:** 7 rarity tiers exist (Common→Eclipse) with escalating drop rates.
- **LOOT-02:** GenerateLoot respects LuckStat modifier.
- **LOOT-03:** GenerateBossLoot guarantees higher rarity floor than normal loot.
- **LOOT-04:** BoE items are tradeable until equipped; Soulbound items cannot trade.
- **LOOT-05:** ClassSpecific drops distribute appropriately across party.

## 7. Save System (EclipseSaveSystem)
- **SAVE-01:** SaveGame persists FPlayerSaveData to slot.
- **SAVE-02:** LoadGame restores exact saved state.
- **SAVE-03:** CreateCheckpoint stores location + Grace Site flag.
- **SAVE-04:** LoadToLastCheckpoint teleports to last Grace Site.
- **SAVE-05:** DeleteSave removes slot data.
- **SAVE-06:** DoesSaveExist returns correct boolean.

## 8. Companion AI (EclipseCompanionAI)
- **COMP-01:** 8 personalities produce distinct behavior profiles.
- **COMP-02:** 10 commands execute correct behavior (Follow, HoldPosition, AttackTarget, DefendAlly, Retreat, UseAbility, SpreadOut, StackUp, FocusBoss, HandleMechanic).
- **COMP-03:** ReplaceWithHuman swaps AI slot to human control; ReplaceWithAI reverses.
- **COMP-04:** CanBeReplacedByHuman returns true for AI companion slots.
- **COMP-05:** AI companion uses same combat framework (abilities, GCD, resources) as human.

## 9. Party/Raid System (EclipsePartySystem)
- **PARTY-01:** MaxPartySize for dungeon == 5.
- **PARTY-02:** MaxPartySize for raid == 10.
- **PARTY-03:** Solo player gets 4 AI companions for dungeon, 9 for raid.
- **PARTY-04:** AI companions fill Empty slots; human can replace AI slot.
- **PARTY-05:** Party disbands correctly when leader leaves.

## 10. MMO Infrastructure
- **MMO-01:** [BLOCKED] No Go gateway exists. Test cannot run.
- **MMO-02:** [BLOCKED] No PostgreSQL schema exists. Test cannot run.
- **MMO-03:** [BLOCKED] No Docker/K8s deployment exists. Test cannot run.
- **NOTE:** V6 Bible defers MMO infrastructure to post-single-player-polish phase. Not a current gate blocker, but must exist before online milestone.

## 11. Compilation Gate
- **COMPILE-01:** UnrealHeaderTool generates .generated.h for all UCLASS/USTRUCT files.
- **COMPILE-02:** Project compiles with zero errors in Development/Win64.
- **COMPILE-03:** WBP_KeyBindings compiles (currently BS_ERROR — BLOCKING).
- **COMPILE-04:** PIE launches without crash for Lvl_Stage1_Arena.
- **COMPILE-05:** Packaged build (Development) launches to main menu.
