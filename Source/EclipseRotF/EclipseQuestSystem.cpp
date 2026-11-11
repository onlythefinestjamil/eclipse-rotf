// Eclipse Quest System — Full implementation
// 128+ quests: main campaign, side, class, faction, companion, boss-unlock, fracture, hidden
// Branching choices with consequences, faction reputation, world flags, save/load.

#include "EclipseQuestSystem.h"
#include "EclipseCharacter.h" // EClass (was previously reached only through unity-build adjacency)

// ---------------------------------------------------------------------------
// File-scope helpers — compact quest/objective/choice construction
// ---------------------------------------------------------------------------

static FQuest Q(int32 ID, const FString& Name, EQuestType Type, const FString& Zone,
	const FString& Giver, int32 Level, int32 XP, int32 Gold, const FString& Desc = TEXT(""))
{
	FQuest q;
	q.QuestID = ID;
	q.QuestName = Name;
	q.Description = Desc;
	q.Type = Type;
	q.State = EQuestState::Locked;
	q.Zone = Zone;
	q.GiverNPC = Giver;
	q.TurnInNPC = Giver;
	q.RequiredLevel = Level;
	q.XPReward = XP;
	q.GoldReward = Gold;
	return q;
}

static FQuest& Obj(FQuest& q, EObjectiveType Type, const FString& Desc, int32 Required,
	const FString& Target = TEXT(""), const FString& TargetZone = TEXT(""), bool bOptional = false)
{
	FQuestObjective o;
	o.ObjectiveID = q.Objectives.Num();
	o.Description = Desc;
	o.Type = Type;
	o.RequiredAmount = Required;
	o.CurrentAmount = 0;
	o.TargetName = Target;
	o.TargetZone = TargetZone;
	o.bIsOptional = bOptional;
	o.bIsComplete = false;
	q.Objectives.Add(o);
	return q;
}

static FQuest& Ch(FQuest& q, int32 ChoiceID, const FString& Prompt, const FString& ChoiceText,
	const FString& Consequence, int32 NextQuestID = -1, const FString& Flag = TEXT(""),
	float RepChange = 0.f, const FString& Faction = TEXT(""), bool bLocks = false,
	const TArray<int32>& LockedIDs = TArray<int32>())
{
	FQuestChoice c;
	c.ChoiceID = ChoiceID;
	c.PromptText = Prompt;
	c.ChoiceText = ChoiceText;
	c.ConsequenceText = Consequence;
	c.NextQuestID = NextQuestID;
	c.SetFlag = Flag;
	c.FactionRepChange = RepChange;
	c.FactionName = Faction;
	c.bLocksOtherQuests = bLocks;
	c.LockedQuestIDs = LockedIDs;
	q.bHasChoice = true;
	q.Choices.Add(c);
	return q;
}

static FQuest& Prereq(FQuest& q, const TArray<int32>& IDs)
{
	q.PrerequisiteQuestIDs = IDs;
	return q;
}

static FQuest& FlagReq(FQuest& q, const FString& Flag)
{
	q.RequiredFlag = Flag;
	return q;
}

static FQuest& Rewards(FQuest& q, const TArray<int32>& Items, float RepReward = 0.f,
	const FString& Faction = TEXT(""))
{
	q.ItemRewards = Items;
	q.FactionRepReward = RepReward;
	q.FactionName = Faction;
	return q;
}

static FQuest& WorldState(FQuest& q, const FString& Change)
{
	q.bAffectsWorldState = true;
	q.WorldStateChange = Change;
	return q;
}

static FQuest& Fracture(FQuest& q, const FString& Zone)
{
	q.bIsFractureQuest = true;
	q.FractureZone = Zone;
	return q;
}

// ===========================================================================
// Construction
// ===========================================================================

UEclipseQuestSystem::UEclipseQuestSystem()
{
}

void UEclipseQuestSystem::InitializeAllQuests()
{
	AllQuests.Empty(135);
	QuestStates.Empty();
	FactionReputation.Empty();
	WorldFlags.Empty();

	// Initialize 6 factions at neutral (0) reputation
	static const TCHAR* FactionNames[] = {
		TEXT("Solaran Dawn"), TEXT("Vaelwood Circle"), TEXT("Morvath Covenant"),
		TEXT("Aetheryan Ascendancy"), TEXT("Nhalor Pact"), TEXT("Umbryss Veil")
	};
	for (const TCHAR* FName : FactionNames)
	{
		FactionReputation.Add(FString(FName), 0.0f);
	}

	InitMainCampaign();
	InitSideQuests();
	InitClassQuests();
	InitFactionQuests();
	InitCompanionQuests();
	InitBossQuests();
	InitFractureQuests();
	InitHiddenQuests();

	// Build the quest-state cache from AllQuests
	for (const FQuest& q : AllQuests)
	{
		QuestStates.Add(q.QuestID, q.State);
	}

	// First campaign quest is available from the start
	if (AllQuests.Num() > 0)
	{
		AllQuests[0].State = EQuestState::Available;
		QuestStates[AllQuests[0].QuestID] = EQuestState::Available;
	}
}

// ---------------------------------------------------------------------------
// Main Campaign — 12 quests, locked canon, branching choices with consequences
// ---------------------------------------------------------------------------

void UEclipseQuestSystem::InitMainCampaign()
{
	// 1 — The Fracture Begins
	{
		FQuest q = Q(1, TEXT("The Fracture Begins"), EQuestType::MainCampaign,
			TEXT("Solara"), TEXT("Elder Pyrean"), 1, 500, 100,
			TEXT("The sky split over Solara. Elder Pyrean sends you to investigate the first fracture."));
		Obj(q, EObjectiveType::TalkToNPC, TEXT("Speak with Elder Pyrean"), 1, TEXT("Elder Pyrean"), TEXT("Solara"));
		Obj(q, EObjectiveType::ReachLocation, TEXT("Reach the Fracture Site"), 1, TEXT("Fracture Site"), TEXT("Solara"));
		Obj(q, EObjectiveType::KillTarget, TEXT("Slay the Voidlings at the fracture"), 5, TEXT("Voidling"), TEXT("Solara"));
		AllQuests.Add(q);
	}

	// 2 — Embers of Solara
	{
		FQuest q = Q(2, TEXT("Embers of Solara"), EQuestType::MainCampaign,
			TEXT("Solara"), TEXT("Elder Pyrean"), 3, 800, 150,
			TEXT("The fracture is widening. Pyrean believes the Emberforge Crucible holds answers."));
		Prereq(q, {1});
		Obj(q, EObjectiveType::ReachLocation, TEXT("Enter the Emberforge Crucible"), 1, TEXT("Emberforge Crucible"), TEXT("Solara"));
		Obj(q, EObjectiveType::CollectItems, TEXT("Collect Fracture Shards"), 3, TEXT("Fracture Shard"), TEXT("Solara"));
		Obj(q, EObjectiveType::TalkToNPC, TEXT("Report to Forge-Master Kael"), 1, TEXT("Forge-Master Kael"), TEXT("Solara"));
		AllQuests.Add(q);
	}

	// 3 — The Ember-Forged (boss unlock + kill)
	{
		FQuest q = Q(3, TEXT("The Ember-Forged"), EQuestType::MainCampaign,
			TEXT("Solara"), TEXT("Forge-Master Kael"), 5, 1200, 300,
			TEXT("Ashen Karthul guards the crucible's core. Destroy it to seal the first fracture."));
		Prereq(q, {2});
		Obj(q, EObjectiveType::DefeatBoss, TEXT("Defeat Ashen Karthul"), 1, TEXT("Ashen Karthul"), TEXT("Solara"));
		Obj(q, EObjectiveType::ActivateObject, TEXT("Seal the Emberforge Core"), 1, TEXT("Emberforge Core"), TEXT("Solara"));
		WorldState(q, TEXT("Solara fracture sealed"));
		AllQuests.Add(q);
	}

	// 4 — Into Vaelwood
	{
		FQuest q = Q(4, TEXT("Into Vaelwood"), EQuestType::MainCampaign,
			TEXT("Vaelwood"), TEXT("Scout Lyrael"), 8, 600, 100,
			TEXT("A second fracture appeared in Vaelwood. Scout Lyrael awaits at the forest's edge."));
		Prereq(q, {3});
		Obj(q, EObjectiveType::ReachLocation, TEXT("Travel to Vaelwood"), 1, TEXT("Vaelwood Border"), TEXT("Vaelwood"));
		Obj(q, EObjectiveType::TalkToNPC, TEXT("Speak with Scout Lyrael"), 1, TEXT("Scout Lyrael"), TEXT("Vaelwood"));
		Obj(q, EObjectiveType::EscortNPC, TEXT("Escort Lyrael through the Thorned Pass"), 1, TEXT("Scout Lyrael"), TEXT("Vaelwood"));
		AllQuests.Add(q);
	}

	// 5 — Heart of Thorns (branching choice)
	{
		FQuest q = Q(5, TEXT("Heart of Thorns"), EQuestType::MainCampaign,
			TEXT("Vaelwood"), TEXT("Earthweaver Thaenor"), 12, 1500, 400,
			TEXT("The Heart of Thorns pulses with the fracture's energy. Earthweaver Thaenor offers two paths."));
		Prereq(q, {4});
		Obj(q, EObjectiveType::CollectItems, TEXT("Gather Verdant Essences"), 5, TEXT("Verdant Essence"), TEXT("Vaelwood"));
		Obj(q, EObjectiveType::ChooseOption, TEXT("Choose how to deal with the Heart"), 1);
		Ch(q, 1, TEXT("How do you handle the Heart of Thorns?"),
			TEXT("Purify the Heart"), TEXT("The Heart is cleansed; Vaelwood begins to heal."),
			6, TEXT("Vaelwood_Purified"), 25.f, TEXT("Vaelwood Circle"));
		Ch(q, 2, TEXT("How do you handle the Heart of Thorns?"),
			TEXT("Corrupt the Heart"), TEXT("The Heart's power is harnessed; Vaelwood darkens."),
			6, TEXT("Vaelwood_Corrupted"), 25.f, TEXT("Umbryss Veil"), true, {101});
		AllQuests.Add(q);
	}

	// 6 — The Living Sanctum (boss unlock + kill)
	{
		FQuest q = Q(6, TEXT("The Living Sanctum"), EQuestType::MainCampaign,
			TEXT("Vaelwood"), TEXT("Earthweaver Thaenor"), 15, 2000, 500,
			TEXT("Sylvanthal, the Heart of Thorns given form, must be defeated to seal the second fracture."));
		Prereq(q, {5});
		Obj(q, EObjectiveType::DefeatBoss, TEXT("Defeat Sylvanthal, Heart of Thorns"), 1, TEXT("Sylvanthal"), TEXT("Vaelwood"));
		Obj(q, EObjectiveType::ActivateObject, TEXT("Seal the Thornheart Sanctum"), 1, TEXT("Thornheart Core"), TEXT("Vaelwood"));
		WorldState(q, TEXT("Vaelwood fracture sealed"));
		AllQuests.Add(q);
	}

	// 7 — The Hollow Call (branching choice with major consequences)
	{
		FQuest q = Q(7, TEXT("The Hollow Call"), EQuestType::MainCampaign,
			TEXT("Morvath"), TEXT("Wraith-Caller Serin"), 20, 1800, 400,
			TEXT("The dead are waking in Morvath. Wraith-Caller Serin offers a choice: ally with the hollow or banish them."));
		Prereq(q, {6});
		Obj(q, EObjectiveType::ReachLocation, TEXT("Enter the Hollow Court"), 1, TEXT("Hollow Court"), TEXT("Morvath"));
		Obj(q, EObjectiveType::SurviveTime, TEXT("Survive the Wailing Choir's song"), 60, TEXT("Wailing Choir"), TEXT("Morvath"));
		Obj(q, EObjectiveType::ChooseOption, TEXT("Decide the fate of the Hollow"), 1);
		Ch(q, 1, TEXT("The Hollow King offers an alliance. Do you accept?"),
			TEXT("Ally with the Hollow"), TEXT("Lord Morvath lends his undead to your cause."),
			8, TEXT("Morvath_Allied"), 50.f, TEXT("Morvath Covenant"), true, {102, 103});
		Ch(q, 2, TEXT("The Hollow King offers an alliance. Do you accept?"),
			TEXT("Banish the Hollow"), TEXT("The hollow are laid to rest; Morvath is freed."),
			8, TEXT("Morvath_Banished"), -30.f, TEXT("Morvath Covenant"), false);
		AllQuests.Add(q);
	}

	// 8 — The Hollow Throne (boss unlock + kill)
	{
		FQuest q = Q(8, TEXT("The Hollow Throne"), EQuestType::MainCampaign,
			TEXT("Morvath"), TEXT("Wraith-Caller Serin"), 25, 2500, 600,
			TEXT("Lord Morvath, the Hollow King, sits upon his throne. End his reign to seal the third fracture."));
		Prereq(q, {7});
		Obj(q, EObjectiveType::DefeatBoss, TEXT("Defeat Lord Morvath, the Hollow King"), 1, TEXT("Lord Morvath"), TEXT("Morvath"));
		Obj(q, EObjectiveType::ActivateObject, TEXT("Claim the Hollow Crown"), 1, TEXT("Hollow Crown"), TEXT("Morvath"));
		WorldState(q, TEXT("Morvath fracture sealed"));
		AllQuests.Add(q);
	}

	// 9 — Ascension
	{
		FQuest q = Q(9, TEXT("Ascension"), EQuestType::MainCampaign,
			TEXT("Aetheris"), TEXT("Sky-Watcher Aelis"), 30, 2000, 500,
			TEXT("The fractures have reached the sky. Ascend the Gale-Temple Spires to reach the Aetheryan Ascendancy."));
		Prereq(q, {8});
		Obj(q, EObjectiveType::ReachLocation, TEXT("Climb the Gale-Temple Spires"), 1, TEXT("Gale-Temple Spires"), TEXT("Aetheris"));
		Obj(q, EObjectiveType::DefendLocation, TEXT("Defend the Spire against the Stormcaged"), 1, TEXT("Spire Summit"), TEXT("Aetheris"));
		Obj(q, EObjectiveType::TalkToNPC, TEXT("Speak with Sky-Watcher Aelis"), 1, TEXT("Sky-Watcher Aelis"), TEXT("Aetheris"));
		AllQuests.Add(q);
	}

	// 10 — Sky-Splitter (boss unlock + kill)
	{
		FQuest q = Q(10, TEXT("Sky-Splitter"), EQuestType::MainCampaign,
			TEXT("Aetheris"), TEXT("Sky-Watcher Aelis"), 35, 3000, 700,
			TEXT("Aetheryon, the Sky-Splitter, tears the heavens apart. Slay the dragon-god to seal the fourth fracture."));
		Prereq(q, {9});
		Obj(q, EObjectiveType::DefeatBoss, TEXT("Defeat Aetheryon, the Sky-Splitter"), 1, TEXT("Aetheryon"), TEXT("Aetheris"));
		Obj(q, EObjectiveType::ActivateObject, TEXT("Close the Sky-Split"), 1, TEXT("Sky-Rift"), TEXT("Aetheris"));
		WorldState(q, TEXT("Aetheris fracture sealed"));
		AllQuests.Add(q);
	}

	// 11 — The Endless Cold (branching choice)
	{
		FQuest q = Q(11, TEXT("The Endless Cold"), EQuestType::MainCampaign,
			TEXT("Nhalor"), TEXT("Frost-Speaker Vael"), 40, 2500, 600,
			TEXT("The final fractures are in Nhalor and Umbryss. Frost-Speaker Vael warns of the Frozen Maw."));
		Prereq(q, {10});
		Obj(q, EObjectiveType::ReachLocation, TEXT("Brave the Nhalor Wastes"), 1, TEXT("Nhalor Wastes"), TEXT("Nhalor"));
		Obj(q, EObjectiveType::SurviveTime, TEXT("Survive the Endless Winter"), 90, TEXT("Endless Winter"), TEXT("Nhalor"));
		Obj(q, EObjectiveType::DefeatBoss, TEXT("Defeat Nhalor, the Frozen Maw"), 1, TEXT("Nhalor"), TEXT("Nhalor"));
		Obj(q, EObjectiveType::ChooseOption, TEXT("Choose how to enter Umbryss"), 1);
		Ch(q, 1, TEXT("How will you enter the Fracture Core?"),
			TEXT("Rally all factions"), TEXT("The factions unite for the final assault."),
			12, TEXT("Factions_United"), 20.f, TEXT("Solaran Dawn"));
		Ch(q, 2, TEXT("How will you enter the Fracture Core?"),
			TEXT("Enter alone"), TEXT("You face the Eclipsed Sovereign with only your party."),
			12, TEXT("Lone_Wolf"), 0.f, TEXT(""));
		AllQuests.Add(q);
	}

	// 12 — End of All Things (final boss, final choice)
	{
		FQuest q = Q(12, TEXT("End of All Things"), EQuestType::MainCampaign,
			TEXT("Umbryss"), TEXT("The Eclipsed Sovereign"), 50, 10000, 5000,
			TEXT("The Eclipsed Sovereign consumed the Eclipse's raw power. End it, and the world is saved from being consumed."));
		Prereq(q, {11});
		Obj(q, EObjectiveType::ReachLocation, TEXT("Enter the Fracture Core"), 1, TEXT("Fracture Core"), TEXT("Umbryss"));
		Obj(q, EObjectiveType::DefeatBoss, TEXT("Defeat the Eclipsed Sovereign"), 1, TEXT("The Eclipsed Sovereign"), TEXT("Umbryss"));
		Obj(q, EObjectiveType::ChooseOption, TEXT("Decide the fate of the world"), 1);
		Ch(q, 1, TEXT("The fracture closes. What do you do with its power?"),
			TEXT("Mend the world"), TEXT("The fractures heal; the world is made whole."),
			-1, TEXT("World_Mended"), 100.f, TEXT("Solaran Dawn"));
		Ch(q, 2, TEXT("The fracture closes. What do you do with its power?"),
			TEXT("Embrace the fracture"), TEXT("You become the new Sovereign; the world is forever changed."),
			-1, TEXT("World_Fractured"), 100.f, TEXT("Umbryss Veil"), true, {});
		WorldState(q, TEXT("Final fracture resolved"));
		Fracture(q, TEXT("Umbryss"));
		AllQuests.Add(q);
	}
}

// ---------------------------------------------------------------------------
// Side Quests — 32 quests across all 6 zones
// ---------------------------------------------------------------------------

void UEclipseQuestSystem::InitSideQuests()
{
	// Solara side quests (6)
	auto AddSide = [&](int32 ID, const FString& Name, const FString& Zone, const FString& Giver,
		int32 Level, int32 XP, int32 Gold, EObjectiveType ObjType, const FString& ObjDesc,
		int32 Req, const FString& Target, const FString& Desc = TEXT(""))
	{
		FQuest q = Q(ID, Name, EQuestType::SideQuest, Zone, Giver, Level, XP, Gold, Desc);
		Obj(q, ObjType, ObjDesc, Req, Target, Zone);
		AllQuests.Add(q);
	};

	AddSide(100, TEXT("Lost Embers"), TEXT("Solara"), TEXT("Villager Mara"), 2, 200, 50, EObjectiveType::CollectItems, TEXT("Collect Lost Embers"), 5, TEXT("Lost Ember"), TEXT("Mara lost her embers in the forge fire."));
	AddSide(101, TEXT("The Smuggler's Path"), TEXT("Solara"), TEXT("Rogue Kaelen"), 4, 300, 80, EObjectiveType::ReachLocation, TEXT("Find the smuggler's tunnel"), 1, TEXT("Smuggler's Tunnel"), TEXT("A secret path beneath the crucible."));
	AddSide(102, TEXT("Flame-Touched"), TEXT("Solara"), TEXT("Priestess Solara"), 6, 400, 100, EObjectiveType::KillTarget, TEXT("Slay Flame Wraiths"), 8, TEXT("Flame Wraith"), TEXT("Wraiths born of the Solara fracture."));
	AddSide(103, TEXT("The First Forge"), TEXT("Solara"), TEXT("Forge-Master Kael"), 8, 500, 150, EObjectiveType::CraftItem, TEXT("Craft a Dawnforged Blade"), 1, TEXT("Dawnforged Blade"), TEXT("Reforge an ancient blade."));
	AddSide(104, TEXT("Ashen Remains"), TEXT("Solara"), TEXT("Historian Vael"), 10, 600, 200, EObjectiveType::DiscoverSecret, TEXT("Find the Ashen Tomb"), 1, TEXT("Ashen Tomb"), TEXT("A forgotten tomb beneath the crucible."));
	AddSide(105, TEXT("Sun-Priest's Vigil"), TEXT("Solara"), TEXT("Sun-Priest Ael"), 12, 700, 250, EObjectiveType::DefendLocation, TEXT("Defend the Sun-Temple"), 1, TEXT("Sun-Temple"), TEXT("Defend the temple from void-touched."));

	// Vaelwood side quests (6)
	AddSide(106, TEXT("The Mossy Path"), TEXT("Vaelwood"), TEXT("Herbalist Fen"), 9, 350, 80, EObjectiveType::CollectItems, TEXT("Gather Moon-Moss"), 6, TEXT("Moon-Moss"), TEXT("Fen needs moss for her remedies."));
	AddSide(107, TEXT("Thorned Wings"), TEXT("Vaelwood"), TEXT("Ranger Sylv"), 11, 400, 100, EObjectiveType::KillTarget, TEXT("Slay Thorn-Wasps"), 10, TEXT("Thorn-Wasp"), TEXT("The wasps infest the grove."));
	AddSide(108, TEXT("The Heart-Tree"), TEXT("Vaelwood"), TEXT("Earthweaver Thaenor"), 13, 500, 120, EObjectiveType::TalkToNPC, TEXT("Commune with the Heart-Tree"), 1, TEXT("Heart-Tree"), TEXT("Speak with the ancient tree."));
	AddSide(109, TEXT("Root-Deep"), TEXT("Vaelwood"), TEXT("Herbalist Fen"), 15, 600, 150, EObjectiveType::DiscoverSecret, TEXT("Find the Root-Deep Cavern"), 1, TEXT("Root-Deep Cavern"), TEXT("A hidden cavern beneath the grove."));
	AddSide(110, TEXT("The Blooming Path"), TEXT("Vaelwood"), TEXT("Gardener Lyra"), 17, 700, 200, EObjectiveType::CollectItems, TEXT("Collect Verdant Blooms"), 8, TEXT("Verdant Bloom"), TEXT("Rare blooms for the garden."));
	AddSide(111, TEXT("Thornheart Memories"), TEXT("Vaelwood"), TEXT("Historian Vael"), 18, 800, 250, EObjectiveType::DiscoverSecret, TEXT("Find the Thornheart Archive"), 1, TEXT("Thornheart Archive"), TEXT("Records of the old sanctum."));

	// Morvath side quests (5)
	AddSide(112, TEXT("The Quietest Voice"), TEXT("Morvath"), TEXT("Wraith-Caller Serin"), 22, 500, 120, EObjectiveType::KillTarget, TEXT("Silence the Echoing Dead"), 12, TEXT("Echoing Dead"), TEXT("The dead won't stop singing."));
	AddSide(113, TEXT("A Soul For A Soul"), TEXT("Morvath"), TEXT("Spirit-Warder Kael"), 24, 600, 150, EObjectiveType::EscortNPC, TEXT("Escort the Bound Soul"), 1, TEXT("Bound Soul"), TEXT("Guide a soul to its rest."));
	AddSide(114, TEXT("The Hollow Archive"), TEXT("Morvath"), TEXT("Historian Vael"), 26, 700, 200, EObjectiveType::DiscoverSecret, TEXT("Find the Hollow Archive"), 1, TEXT("Hollow Archive"), TEXT("The Hollow King's records."));
	AddSide(115, TEXT("Choir Practice"), TEXT("Morvath"), TEXT("Choir-Mother Vel"), 28, 800, 250, EObjectiveType::SurviveTime, TEXT("Survive the Choir's Rehearsal"), 45, TEXT("Rehearsal"), TEXT("Endure the choir's song."));
	AddSide(116, TEXT("The Last Wail"), TEXT("Morvath"), TEXT("Wraith-Caller Serin"), 29, 900, 300, EObjectiveType::DefeatBoss, TEXT("Defeat the Wailing Choir"), 1, TEXT("The Wailing Choir"), TEXT("Silence the choir for good."));

	// Aetheris side quests (6)
	AddSide(117, TEXT("Wind-Borne"), TEXT("Aetheris"), TEXT("Sky-Watcher Aelis"), 31, 600, 150, EObjectiveType::ReachLocation, TEXT("Reach the Wind-Platform"), 1, TEXT("Wind-Platform"), TEXT("Ascend the spires."));
	AddSide(118, TEXT("Static Charged"), TEXT("Aetheris"), TEXT("Storm-Caller Vael"), 33, 700, 180, EObjectiveType::KillTarget, TEXT("Slay Storm-Elementals"), 10, TEXT("Storm-Elemental"), TEXT("Elementals from the fracture."));
	AddSide(119, TEXT("The Cage Keys"), TEXT("Aetheris"), TEXT("Warden Thael"), 35, 800, 200, EObjectiveType::CollectItems, TEXT("Collect Stormcage Keys"), 3, TEXT("Stormcage Key"), TEXT("Keys to the old cages."));
	AddSide(120, TEXT("Sky-Fallen"), TEXT("Aetheris"), TEXT("Historian Vael"), 37, 900, 250, EObjectiveType::DiscoverSecret, TEXT("Find the Sky-Fallen Ruins"), 1, TEXT("Sky-Fallen Ruins"), TEXT("Ruins that fell from the sky."));
	AddSide(121, TEXT("The Tempest's Eye"), TEXT("Aetheris"), TEXT("Storm-Caller Vael"), 39, 1000, 300, EObjectiveType::SurviveTime, TEXT("Survive the Tempest's Eye"), 60, TEXT("Tempest's Eye"), TEXT("Endure the storm's heart."));
	AddSide(122, TEXT("Stormcage Breach"), TEXT("Aetheris"), TEXT("Warden Thael"), 40, 1100, 350, EObjectiveType::DefeatBoss, TEXT("Defeat the Stormcage Warden"), 1, TEXT("The Stormcage Warden"), TEXT("Breach the cage."));

	// Nhalor side quests (5)
	AddSide(123, TEXT("Frozen Tears"), TEXT("Nhalor"), TEXT("Frost-Speaker Vael"), 41, 700, 180, EObjectiveType::CollectItems, TEXT("Collect Frozen Tears"), 5, TEXT("Frozen Tear"), TEXT("Tears of the frozen dead."));
	AddSide(124, TEXT("The Cold Road"), TEXT("Nhalor"), TEXT("Wanderer Kael"), 43, 800, 200, EObjectiveType::ReachLocation, TEXT("Find the Cold Road"), 1, TEXT("Cold Road"), TEXT("A road through the frozen waste."));
	AddSide(125, TEXT("Icebound"), TEXT("Nhalor"), TEXT("Frost-Speaker Vael"), 45, 900, 250, EObjectiveType::KillTarget, TEXT("Slay Frost-Giants"), 6, TEXT("Frost-Giant"), TEXT("Giants of the deep cold."));
	AddSide(126, TEXT("The Widow's Gift"), TEXT("Nhalor"), TEXT("Widow Velnyra"), 46, 1000, 300, EObjectiveType::TalkToNPC, TEXT("Speak with the Glacial Widow"), 1, TEXT("Velnyra"), TEXT("The widow offers a chilling gift."));
	AddSide(127, TEXT("Frozen Echoes"), TEXT("Nhalor"), TEXT("Historian Vael"), 47, 1100, 350, EObjectiveType::DiscoverSecret, TEXT("Find the Frozen Echo"), 1, TEXT("Frozen Echo"), TEXT("An echo trapped in ice."));

	// Umbryss side quests (4)
	AddSide(128, TEXT("Shadow Stitching"), TEXT("Umbryss"), TEXT("Shade-Mother Nocthyra"), 50, 1000, 300, EObjectiveType::CollectItems, TEXT("Collect Shadow-Silk"), 8, TEXT("Shadow-Silk"), TEXT("Silk from Nocthyra's loom."));
	AddSide(129, TEXT("The Fractured Path"), TEXT("Umbryss"), TEXT("Wanderer Kael"), 52, 1200, 400, EObjectiveType::ReachLocation, TEXT("Find the Fractured Path"), 1, TEXT("Fractured Path"), TEXT("A path through the fracture."));
	AddSide(130, TEXT("Void-Touched"), TEXT("Umbryss"), TEXT("Void-Priestess Vel"), 54, 1500, 500, EObjectiveType::KillTarget, TEXT("Slay Void-Horrors"), 15, TEXT("Void-Horror"), TEXT("Horrors from the fracture's depths."));
	AddSide(131, TEXT("The Last Shadow"), TEXT("Umbryss"), TEXT("Shade-Mother Nocthyra"), 56, 2000, 700, EObjectiveType::DefeatBoss, TEXT("Defeat the Shade-Mother"), 1, TEXT("Shade-Mother Nocthyra"), TEXT("End the Shade-Mother's weaving."));
}

// ---------------------------------------------------------------------------
// Class Quests — 17 quests, one per class
// ---------------------------------------------------------------------------

void UEclipseQuestSystem::InitClassQuests()
{
	struct ClassQuestDef { EClass Class; FString Name; FString Giver; FString Target; };
	ClassQuestDef Defs[17] = {
		{EClass::Bulwark,       TEXT("The Unbreakable Wall"),    TEXT("Captain Korvan"),  TEXT("Bulwark Master")},
		{EClass::Duskblade, TEXT("Shadow's Edge"),           TEXT("Night-Blade Sera"), TEXT("Umbral Master")},
		{EClass::Arcanist,       TEXT("The Arcane Thesis"),        TEXT("Archmage Vael"),   TEXT("Arcanist Master")},
		{EClass::Riftwalker,     TEXT("Between Worlds"),          TEXT("Rift-Walker Kael"), TEXT("Riftwalker Master")},
		{EClass::Dreadknight,    TEXT("The Blood Oath"),          TEXT("Dread-Lord Morvan"), TEXT("Dreadknight Master")},
		{EClass::Templar,        TEXT("Shield of Faith"),          TEXT("High-Priestess Aela"), TEXT("Templar Master")},
		{EClass::Soulbinder,     TEXT("Bound Souls"),             TEXT("Soul-Binder Thael"), TEXT("Soulbinder Master")},
		{EClass::Wildwarden,     TEXT("The Wild Hunt"),           TEXT("Warden Lyrael"),   TEXT("Wildwarden Master")},
		{EClass::Stormcaller,    TEXT("Voice of the Storm"),       TEXT("Storm-Priest Vael"), TEXT("Stormcaller Master")},
		{EClass::Oracle,        TEXT("Eyes of Fate"),             TEXT("Oracle Serin"),     TEXT("Oracle Master")},
		{EClass::Alchemist,     TEXT("The Philosopher's Stone"),  TEXT("Alchemist Kael"),   TEXT("Alchemist Master")},
		{EClass::Berserker,     TEXT("Rage Unleashed"),           TEXT("Berserker Korvan"),  TEXT("Berserker Master")},
		{EClass::Shadowblade,   TEXT("The Silent Cut"),           TEXT("Shadow-Blade Sera"), TEXT("Shadowblade Master")},
		{EClass::Spellblade,    TEXT("Steel and Sorcery"),         TEXT("Spell-Blade Vael"),  TEXT("Spellblade Master")},
		{EClass::Warthane,       TEXT("The War-Banner"),           TEXT("Warthane Thael"),    TEXT("Warthane Master")},
		{EClass::Necroturge,   TEXT("The Final Command"),         TEXT("Necroturge Morvan"), TEXT("Necroturge Master")},
		{EClass::Earthweaver,         TEXT("Shape of the Wild"),         TEXT("Archdruid Lyrael"),  TEXT("Earthweaver Master")},
	};

	for (int32 i = 0; i < 17; i++)
	{
		int32 QID = 200 + i;
		const ClassQuestDef& d = Defs[i];
		FQuest q = Q(QID, d.Name, EQuestType::ClassQuest,
			TEXT("Solara"), d.Giver, 10 + i * 2, 800 + i * 50, 200 + i * 20,
			FString::Printf(TEXT("Prove your worth as a %s to unlock your class mastery."), *d.Name));
		Prereq(q, {1}); // requires main campaign start
		Obj(q, EObjectiveType::TalkToNPC, FString::Printf(TEXT("Speak with %s"), *d.Giver), 1, d.Giver, TEXT("Solara"));
		Obj(q, EObjectiveType::KillTarget, TEXT("Slay class challengers"), 5, d.Target, TEXT("Solara"));
		Obj(q, EObjectiveType::DefeatBoss, TEXT("Defeat your class master in ritual combat"), 1, d.Target, TEXT("Solara"));
		Rewards(q, {QID * 10}, 10.f, TEXT("Solaran Dawn"));
		AllQuests.Add(q);
	}
}

// ---------------------------------------------------------------------------
// Faction Quests — 6 factions × 6 = 36 quests
// ---------------------------------------------------------------------------

void UEclipseQuestSystem::InitFactionQuests()
{
	struct FactionDef { FString Name; FString Giver; FString Zone; };
	FactionDef Factions[6] = {
		{TEXT("Solaran Dawn"),          TEXT("Dawn-Commander Korvan"), TEXT("Solara")},
		{TEXT("Vaelwood Circle"),       TEXT("Circle Elder Thaenor"),  TEXT("Vaelwood")},
		{TEXT("Morvath Covenant"),      TEXT("Covenant-Binder Serin"), TEXT("Morvath")},
		{TEXT("Aetheryan Ascendancy"),  TEXT("Ascendant Aelis"),        TEXT("Aetheris")},
		{TEXT("Nhalor Pact"),           TEXT("Pact-Speaker Vael"),     TEXT("Nhalor")},
		{TEXT("Umbryss Veil"),          TEXT("Veil-Mother Nocthyra"),  TEXT("Umbryss")},
	};

	for (int32 f = 0; f < 6; f++)
	{
		const FactionDef& fd = Factions[f];
		for (int32 i = 0; i < 6; i++)
		{
			int32 QID = 300 + f * 6 + i;
			FQuest q = Q(QID,
				FString::Printf(TEXT("%s: Rank %d"), *fd.Name, i + 1),
				EQuestType::FactionQuest, fd.Zone, fd.Giver,
				5 + f * 5 + i * 3, 500 + i * 200, 100 + i * 50,
				FString::Printf(TEXT("Earn favor with the %s."), *fd.Name));
			Prereq(q, {1});
			// Escalating objectives
			if (i < 2)
				Obj(q, EObjectiveType::KillTarget, FString::Printf(TEXT("Slay enemies for the %s"), *fd.Name), 10 + i * 5, TEXT("Faction Enemy"), fd.Zone);
			else if (i < 4)
				Obj(q, EObjectiveType::CollectItems, FString::Printf(TEXT("Gather supplies for the %s"), *fd.Name), 5 + i * 2, TEXT("Faction Supply"), fd.Zone);
			else
				Obj(q, EObjectiveType::DefendLocation, FString::Printf(TEXT("Defend the %s stronghold"), *fd.Name), 1, TEXT("Faction Stronghold"), fd.Zone);

			Rewards(q, {}, 50.f + i * 25.f, fd.Name);
			q.bAffectsNPCRelationship = true;
			q.NPCRelationshipChange = FString::Printf(TEXT("%s reputation +%d"), *fd.Name, i + 1);
			AllQuests.Add(q);
		}
	}
}

// ---------------------------------------------------------------------------
// Companion Quests — 6 companion loyalty arcs
// ---------------------------------------------------------------------------

void UEclipseQuestSystem::InitCompanionQuests()
{
	struct CompDef { FString Name; FString Giver; FString Zone; FString Arc; };
	CompDef Companions[6] = {
		{TEXT("Lyrael's Vow"),    TEXT("Lyrael"),       TEXT("Vaelwood"),   TEXT("Lyrael seeks to heal the grove she failed to protect.")},
		{TEXT("Korvan's Redemption"), TEXT("Korvan"),   TEXT("Solara"),      TEXT("Korvan seeks redemption for his fallen company.")},
		{TEXT("Serin's Silence"),  TEXT("Serin"),       TEXT("Morvath"),     TEXT("Serin seeks to silence the wail that haunts her.")},
		{TEXT("Aelis's Ascent"),   TEXT("Aelis"),       TEXT("Aetheris"),    TEXT("Aelis seeks to reach the highest spire.")},
		{TEXT("Vael's Warmth"),    TEXT("Vael"),        TEXT("Nhalor"),      TEXT("Vael seeks to end the cold that took her family.")},
		{TEXT("Nocthyra's Thread"), TEXT("Nocthyra"),    TEXT("Umbryss"),     TEXT("Nocthyra seeks to unweave the shadow that binds her.")},
	};

	for (int32 i = 0; i < 6; i++)
	{
		int32 QID = 400 + i;
		const CompDef& c = Companions[i];
		FQuest q = Q(QID, c.Name, EQuestType::CompanionQuest, c.Zone, c.Giver,
			15 + i * 5, 1200 + i * 200, 300 + i * 50, c.Arc);
		Prereq(q, {3 + i}); // unlock gradually through campaign
		Obj(q, EObjectiveType::TalkToNPC, FString::Printf(TEXT("Speak with %s"), *c.Giver), 1, c.Giver, c.Zone);
		Obj(q, EObjectiveType::EscortNPC, FString::Printf(TEXT("Escort %s to their past"), *c.Giver), 1, c.Giver, c.Zone);
		Obj(q, EObjectiveType::DefeatBoss, FString::Printf(TEXT("Help %s face their demon"), *c.Giver), 1, TEXT("Companion Demon"), c.Zone);
		Obj(q, EObjectiveType::ChooseOption, TEXT("Decide their fate"), 1);
		Ch(q, 1, FString::Printf(TEXT("How do you help %s?"), *c.Giver),
			TEXT("Support them fully"), TEXT("Their loyalty is sealed; they become a permanent companion."),
			-1, FString::Printf(TEXT("%s_Loyal"), *c.Giver), 50.f, TEXT("Solaran Dawn"));
		Ch(q, 2, FString::Printf(TEXT("How do you help %s?"), *c.Giver),
			TEXT("Let them go"), TEXT("They depart, at peace; you walk the path alone."),
			-1, FString::Printf(TEXT("%s_Departed"), *c.Giver), 0.f, TEXT(""));
		q.bAffectsNPCRelationship = true;
		q.NPCRelationshipChange = FString::Printf(TEXT("%s loyalty arc"), *c.Giver);
		AllQuests.Add(q);
	}
}

// ---------------------------------------------------------------------------
// Boss Unlock Quests — 14 quests, one per boss
// ---------------------------------------------------------------------------

void UEclipseQuestSystem::InitBossQuests()
{
	struct BossDef { FString Name; FString Zone; int32 Level; };
	BossDef Bosses[14] = {
		{TEXT("Ashen Karthul"),                 TEXT("Solara"),   20},
		{TEXT("Pyre-Warden Velkor"),            TEXT("Solara"),   25},
		{TEXT("Solarion, the Dawn-Bringer"),    TEXT("Solara"),   30},
		{TEXT("Mossgrasp the Encompassing"),    TEXT("Vaelwood"), 35},
		{TEXT("Sylvanthal, Heart of Thorns"),   TEXT("Vaelwood"), 40},
		{TEXT("The Wailing Choir"),             TEXT("Morvath"),  45},
		{TEXT("Lord Morvath, the Hollow King"), TEXT("Morvath"),  50},
		{TEXT("Zephyrian Gale-Caller"),         TEXT("Aetheris"), 55},
		{TEXT("The Stormcage Warden"),          TEXT("Aetheris"), 58},
		{TEXT("Aetheryon, the Sky-Splitter"),    TEXT("Aetheris"), 60},
		{TEXT("Glacial Widow Velnyra"),          TEXT("Nhalor"),   63},
		{TEXT("Nhalor, the Frozen Maw"),        TEXT("Nhalor"),   65},
		{TEXT("Shade-Mother Nocthyra"),         TEXT("Umbryss"),  68},
		{TEXT("The Eclipsed Sovereign"), TEXT("Umbryss"), 70},
	};

	for (int32 i = 0; i < 14; i++)
	{
		int32 QID = 500 + i;
		const BossDef& b = Bosses[i];
		FQuest q = Q(QID, FString::Printf(TEXT("Unlocks: %s"), *b.Name), EQuestType::BossQuest,
			b.Zone, TEXT("Wandering Scholar"), b.Level, 1000 + i * 100, 200 + i * 50,
			FString::Printf(TEXT("Discover the means to challenge %s."), *b.Name));
		Prereq(q, {1 + i / 2}); // unlock gradually
		Obj(q, EObjectiveType::DiscoverSecret, FString::Printf(TEXT("Find %s's arena"), *b.Name), 1, b.Name + TEXT(" Arena"), b.Zone);
		Obj(q, EObjectiveType::CollectItems, TEXT("Collect Fracture Keys"), 3, TEXT("Fracture Key"), b.Zone);
		Obj(q, EObjectiveType::ActivateObject, FString::Printf(TEXT("Open %s's arena"), *b.Name), 1, b.Name + TEXT(" Seal"), b.Zone);
		Rewards(q, {QID}, 20.f, TEXT("Solaran Dawn"));
		AllQuests.Add(q);
	}
}

// ---------------------------------------------------------------------------
// Fracture Quests — 6 quests that alter the world
// ---------------------------------------------------------------------------

void UEclipseQuestSystem::InitFractureQuests()
{
	struct FracDef { FString Name; FString Zone; FString Flag; };
	FracDef Fracs[6] = {
		{TEXT("The Solara Fracture"),  TEXT("Solara"),   TEXT("Fracture_Solara")},
		{TEXT("The Vaelwood Fracture"), TEXT("Vaelwood"), TEXT("Fracture_Vaelwood")},
		{TEXT("The Morvath Fracture"),  TEXT("Morvath"),  TEXT("Fracture_Morvath")},
		{TEXT("The Aetheris Fracture"), TEXT("Aetheris"), TEXT("Fracture_Aetheris")},
		{TEXT("The Nhalor Fracture"),  TEXT("Nhalor"),   TEXT("Fracture_Nhalor")},
		{TEXT("The Umbryss Fracture"),  TEXT("Umbryss"),  TEXT("Fracture_Umbryss")},
	};

	for (int32 i = 0; i < 6; i++)
	{
		int32 QID = 600 + i;
		const FracDef& f = Fracs[i];
		FQuest q = Q(QID, f.Name, EQuestType::FractureQuest, f.Zone, TEXT("Fracture-Watcher"),
			20 + i * 8, 2000 + i * 500, 500 + i * 100,
			FString::Printf(TEXT("The fracture in %s is spreading. Close it before it's too late."), *f.Zone));
		Prereq(q, {3 + i * 2}); // unlock through campaign
		Obj(q, EObjectiveType::ReachLocation, FString::Printf(TEXT("Reach the %s fracture"), *f.Zone), 1, TEXT("Fracture Point"), f.Zone);
		Obj(q, EObjectiveType::SurviveTime, TEXT("Survive the fracture's instability"), 120, TEXT("Fracture Instability"), f.Zone);
		Obj(q, EObjectiveType::ActivateObject, TEXT("Seal the fracture"), 1, TEXT("Fracture Seal"), f.Zone);
		Obj(q, EObjectiveType::ChooseOption, TEXT("Decide how to seal it"), 1);
		Ch(q, 1, TEXT("How do you seal the fracture?"),
			TEXT("Seal it cleanly"), TEXT("The fracture is mended; the zone stabilizes."),
			-1, f.Flag + TEXT("_Sealed"), 30.f, TEXT("Solaran Dawn"));
		Ch(q, 2, TEXT("How do you seal the fracture?"),
			TEXT("Harness its power"), TEXT("The fracture is bound to you; the zone changes forever."),
			-1, f.Flag + TEXT("_Harnessed"), 30.f, TEXT("Umbryss Veil"), true, {700 + i});
		WorldState(q, FString::Printf(TEXT("%s fracture altered"), *f.Zone));
		Fracture(q, f.Zone);
		AllQuests.Add(q);
	}
}

// ---------------------------------------------------------------------------
// Hidden Quests — 12 secret discovery quests
// ---------------------------------------------------------------------------

void UEclipseQuestSystem::InitHiddenQuests()
{
	struct HiddenDef { FString Name; FString Zone; int32 Level; };
	HiddenDef Hiddens[12] = {
		{TEXT("The Ashen Library"),   TEXT("Solara"),   15},
		{TEXT("The Sunken Forge"),     TEXT("Solara"),   18},
		{TEXT("The Root-Mind"),       TEXT("Vaelwood"), 22},
		{TEXT("The Singing Stone"),   TEXT("Vaelwood"), 25},
		{TEXT("The Hollow Child"),    TEXT("Morvath"),  30},
		{TEXT("The Unfinished Song"), TEXT("Morvath"),  33},
		{TEXT("The Floating Tower"),  TEXT("Aetheris"), 38},
		{TEXT("The Storm's Heart"),   TEXT("Aetheris"), 41},
		{TEXT("The Frozen Garden"),   TEXT("Nhalor"),   44},
		{TEXT("The Ice-Mirror"),      TEXT("Nhalor"),   47},
		{TEXT("The Shadow Loom"),     TEXT("Umbryss"),  55},
		{TEXT("The Fracture's Edge"), TEXT("Umbryss"),  60},
	};

	for (int32 i = 0; i < 12; i++)
	{
		int32 QID = 700 + i;
		const HiddenDef& h = Hiddens[i];
		FQuest q = Q(QID, h.Name, EQuestType::HiddenQuest, h.Zone, TEXT("???"),
			h.Level, 3000, 1000,
			FString::Printf(TEXT("A hidden secret in %s. Find it if you can."), *h.Zone));
		// Hidden quests have no prerequisites — they're discovered by exploration
		Obj(q, EObjectiveType::DiscoverSecret, FString::Printf(TEXT("Discover %s"), *h.Name), 1, h.Name, h.Zone);
		Obj(q, EObjectiveType::KillTarget, TEXT("Slay the secret guardian"), 1, TEXT("Secret Guardian"), h.Zone);
		Obj(q, EObjectiveType::ActivateObject, TEXT("Claim the hidden treasure"), 1, TEXT("Hidden Treasure"), h.Zone);
		Rewards(q, {QID * 100, QID * 100 + 1}, 0.f, TEXT(""));
		AllQuests.Add(q);
	}
}

// ===========================================================================
// Quest flow
// ===========================================================================

bool UEclipseQuestSystem::AcceptQuest(int32 QuestID, int32 PlayerLevel, const TArray<int32>& CompletedQuestIDs)
{
	for (FQuest& q : AllQuests)
	{
		if (q.QuestID != QuestID) continue;
		if (q.State != EQuestState::Locked && q.State != EQuestState::Available) return false;
		if (!ArePrerequisitesMet(QuestID, PlayerLevel, CompletedQuestIDs)) return false;
		q.State = EQuestState::Active;
		q.CurrentObjectiveIndex = 0;
		QuestStates[QuestID] = EQuestState::Active;
		return true;
	}
	return false;
}

void UEclipseQuestSystem::UpdateObjective(int32 QuestID, int32 ObjectiveID, int32 Amount)
{
	for (FQuest& q : AllQuests)
	{
		if (q.QuestID != QuestID) continue;
		if (q.State != EQuestState::Active) return;
		for (FQuestObjective& o : q.Objectives)
		{
			if (o.ObjectiveID == ObjectiveID)
			{
				o.CurrentAmount = FMath::Min(o.CurrentAmount + Amount, o.RequiredAmount);
				if (o.CurrentAmount >= o.RequiredAmount && !o.bIsComplete)
				{
					o.bIsComplete = true;
					// Auto-advance objective index
					if (q.CurrentObjectiveIndex < q.Objectives.Num() - 1)
					{
						q.CurrentObjectiveIndex++;
					}
				}
				break;
			}
		}
		break;
	}
}

void UEclipseQuestSystem::CompleteObjective(int32 QuestID, int32 ObjectiveID)
{
	for (FQuest& q : AllQuests)
	{
		if (q.QuestID != QuestID) continue;
		if (q.State != EQuestState::Active) return;
		for (FQuestObjective& o : q.Objectives)
		{
			if (o.ObjectiveID == ObjectiveID)
			{
				o.CurrentAmount = o.RequiredAmount;
				o.bIsComplete = true;
				break;
			}
		}
		// Advance to next incomplete objective
		bool bAllComplete = true;
		for (int32 i = 0; i < q.Objectives.Num(); i++)
		{
			if (!q.Objectives[i].bIsComplete)
			{
				q.CurrentObjectiveIndex = i;
				bAllComplete = false;
				break;
			}
		}
		if (bAllComplete) q.CurrentObjectiveIndex = q.Objectives.Num();
		break;
	}
}

bool UEclipseQuestSystem::MakeChoice(int32 QuestID, int32 ChoiceID)
{
	for (FQuest& q : AllQuests)
	{
		if (q.QuestID != QuestID) continue;
		if (q.State != EQuestState::Active) return false;
		for (const FQuestChoice& c : q.Choices)
		{
			if (c.ChoiceID != ChoiceID) continue;
			q.SelectedChoiceID = ChoiceID;
			// Apply consequences: world flag
			if (!c.SetFlag.IsEmpty())
			{
				SetWorldFlag(c.SetFlag, true);
			}
			// Apply consequences: faction reputation
			if (!c.FactionName.IsEmpty() && c.FactionRepChange != 0.0f)
			{
				AddFactionRep(c.FactionName, c.FactionRepChange);
			}
			// Apply consequences: lock other quests
			if (c.bLocksOtherQuests)
			{
				for (int32 LockedID : c.LockedQuestIDs)
				{
					for (FQuest& OtherQ : AllQuests)
					{
						if (OtherQ.QuestID == LockedID)
						{
							OtherQ.State = EQuestState::Locked;
							QuestStates[LockedID] = EQuestState::Locked;
							break;
						}
					}
				}
			}
			// Mark the "Choose" objective complete
			for (FQuestObjective& o : q.Objectives)
			{
				if (o.Type == EObjectiveType::ChooseOption && !o.bIsComplete)
				{
					o.bIsComplete = true;
					o.CurrentAmount = o.RequiredAmount;
					if (q.CurrentObjectiveIndex < q.Objectives.Num() - 1)
						q.CurrentObjectiveIndex++;
					break;
				}
			}
			return true;
		}
		break;
	}
	return false;
}

bool UEclipseQuestSystem::TurnInQuest(int32 QuestID)
{
	for (FQuest& q : AllQuests)
	{
		if (q.QuestID != QuestID) continue;
		if (q.State != EQuestState::Active) return false;
		// Verify all non-optional objectives are complete
		for (const FQuestObjective& o : q.Objectives)
		{
			if (!o.bIsOptional && !o.bIsComplete) return false;
		}
		q.State = EQuestState::Completed;
		QuestStates[QuestID] = EQuestState::Completed;
		// Apply world state change
		if (q.bAffectsWorldState && !q.WorldStateChange.IsEmpty())
		{
			SetWorldFlag(q.WorldStateChange, true);
		}
		// Apply fracture zone change
		if (q.bIsFractureQuest && !q.FractureZone.IsEmpty())
		{
			SetWorldFlag(FString::Printf(TEXT("Fracture_Altered_%s"), *q.FractureZone), true);
		}
		// Apply faction reputation reward
		if (!q.FactionName.IsEmpty() && q.FactionRepReward != 0.0f)
		{
			AddFactionRep(q.FactionName, q.FactionRepReward);
		}
		return true;
	}
	return false;
}

void UEclipseQuestSystem::AbandonQuest(int32 QuestID)
{
	for (FQuest& q : AllQuests)
	{
		if (q.QuestID != QuestID) continue;
		if (q.State != EQuestState::Active) return;
		q.State = EQuestState::Abandoned;
		QuestStates[QuestID] = EQuestState::Abandoned;
		// Reset objective progress
		for (FQuestObjective& o : q.Objectives)
		{
			o.CurrentAmount = 0;
			o.bIsComplete = false;
		}
		q.CurrentObjectiveIndex = 0;
		break;
	}
}

void UEclipseQuestSystem::FailQuest(int32 QuestID)
{
	for (FQuest& q : AllQuests)
	{
		if (q.QuestID != QuestID) continue;
		q.State = EQuestState::Failed;
		QuestStates[QuestID] = EQuestState::Failed;
		// A failed main campaign quest may have world consequences
		if (q.bAffectsWorldState && !q.WorldStateChange.IsEmpty())
		{
			SetWorldFlag(q.WorldStateChange + TEXT("_Failed"), true);
		}
		break;
	}
}

bool UEclipseQuestSystem::ArePrerequisitesMet(int32 QuestID, int32 PlayerLevel, const TArray<int32>& CompletedQuestIDs) const
{
	for (const FQuest& q : AllQuests)
	{
		if (q.QuestID != QuestID) continue;
		if (PlayerLevel < q.RequiredLevel) return false;
		// Check prerequisite quest IDs
		for (int32 PreID : q.PrerequisiteQuestIDs)
		{
			if (!CompletedQuestIDs.Contains(PreID)) return false;
		}
		// Check required world flag
		if (!q.RequiredFlag.IsEmpty())
		{
			const bool* pFlag = WorldFlags.Find(q.RequiredFlag);
			if (!pFlag || !*pFlag) return false;
		}
		return true;
	}
	return false;
}

EQuestState UEclipseQuestSystem::GetQuestState(int32 QuestID) const
{
	for (const FQuest& q : AllQuests)
	{
		if (q.QuestID == QuestID) return q.State;
	}
	return EQuestState::Locked;
}

FQuestObjective UEclipseQuestSystem::GetCurrentObjective(int32 QuestID) const
{
	for (const FQuest& q : AllQuests)
	{
		if (q.QuestID != QuestID) continue;
		if (q.Objectives.IsValidIndex(q.CurrentObjectiveIndex))
		{
			return q.Objectives[q.CurrentObjectiveIndex];
		}
		break;
	}
	return FQuestObjective();
}

// ===========================================================================
// Queries
// ===========================================================================

TArray<FQuest> UEclipseQuestSystem::GetAllQuests() const
{
	return AllQuests;
}

FQuest UEclipseQuestSystem::GetQuest(int32 QuestID) const
{
	for (const FQuest& q : AllQuests)
	{
		if (q.QuestID == QuestID) return q;
	}
	return FQuest();
}

TArray<FQuest> UEclipseQuestSystem::GetActiveQuests() const
{
	TArray<FQuest> Result;
	for (const FQuest& q : AllQuests)
	{
		if (q.State == EQuestState::Active) Result.Add(q);
	}
	return Result;
}

TArray<FQuest> UEclipseQuestSystem::GetAvailableQuests(int32 PlayerLevel, const TArray<int32>& CompletedQuestIDs) const
{
	TArray<FQuest> Result;
	for (const FQuest& q : AllQuests)
	{
		if (q.State == EQuestState::Locked || q.State == EQuestState::Available)
		{
			if (ArePrerequisitesMet(q.QuestID, PlayerLevel, CompletedQuestIDs))
			{
				FQuest Copy = q;
				Copy.State = EQuestState::Available;
				Result.Add(Copy);
			}
		}
	}
	return Result;
}

TArray<int32> UEclipseQuestSystem::GetCompletedQuestIDs() const
{
	TArray<int32> Result;
	for (const FQuest& q : AllQuests)
	{
		if (q.State == EQuestState::Completed) Result.Add(q.QuestID);
	}
	return Result;
}

int32 UEclipseQuestSystem::GetQuestCount() const
{
	return AllQuests.Num();
}

TArray<FQuest> UEclipseQuestSystem::GetQuestsByType(EQuestType Type) const
{
	TArray<FQuest> Result;
	for (const FQuest& q : AllQuests)
	{
		if (q.Type == Type) Result.Add(q);
	}
	return Result;
}

TArray<FQuest> UEclipseQuestSystem::GetQuestsByZone(const FString& ZoneName) const
{
	TArray<FQuest> Result;
	for (const FQuest& q : AllQuests)
	{
		if (q.Zone == ZoneName) Result.Add(q);
	}
	return Result;
}

// ===========================================================================
// Faction reputation
// ===========================================================================

float UEclipseQuestSystem::GetFactionRep(const FString& FactionName) const
{
	const float* pVal = FactionReputation.Find(FactionName);
	return pVal ? *pVal : 0.0f;
}

void UEclipseQuestSystem::AddFactionRep(const FString& FactionName, float Amount)
{
	float* pVal = FactionReputation.Find(FactionName);
	if (pVal)
	{
		*pVal += Amount;
	}
	else
	{
		FactionReputation.Add(FactionName, Amount);
	}
}

TMap<FString, float> UEclipseQuestSystem::GetAllFactionReps() const
{
	return FactionReputation;
}

// ===========================================================================
// World state / flags
// ===========================================================================

TMap<FString, bool> UEclipseQuestSystem::GetWorldFlags() const
{
	return WorldFlags;
}

void UEclipseQuestSystem::SetWorldFlag(const FString& FlagName, bool Value)
{
	WorldFlags.Add(FlagName, Value);
}

bool UEclipseQuestSystem::GetWorldFlag(const FString& FlagName) const
{
	const bool* pVal = WorldFlags.Find(FlagName);
	return pVal ? *pVal : false;
}

// ===========================================================================
// Save / Load
// ===========================================================================

TMap<int32, EQuestState> UEclipseQuestSystem::GetAllQuestStates() const
{
	TMap<int32, EQuestState> Result;
	for (const FQuest& q : AllQuests)
	{
		Result.Add(q.QuestID, q.State);
	}
	return Result;
}

void UEclipseQuestSystem::LoadQuestStates(const TMap<int32, EQuestState>& InStates)
{
	for (FQuest& q : AllQuests)
	{
		const EQuestState* pState = InStates.Find(q.QuestID);
		if (pState)
		{
			q.State = *pState;
			QuestStates[q.QuestID] = *pState;
		}
	}
}
