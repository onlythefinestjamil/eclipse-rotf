// Eclipse Raid Director — Full implementation
// 5-player dungeons + 10-player raids. Encounter flow: Start → Update → Wipe/Victory → Loot.
// Links BossDirector (combat) and PartySystem (party composition / AI slots).

#include "EclipseRaidDirector.h"

// ---------------------------------------------------------------------------
// File-scope helpers
// ---------------------------------------------------------------------------

static FEncounterConfig MakeEncounter(int32 ID, const FString& Name, EEncounterType Type,
	int32 BossID, const FString& Zone, const FString& Dungeon, int32 Level,
	float TimeLimit = -1.f, int32 MaxRevives = 3, bool bFracture = false)
{
	FEncounterConfig E;
	E.EncounterID = ID;
	E.EncounterName = Name;
	E.Type = Type;
	E.BossID = BossID;
	E.ZoneName = Zone;
	E.DungeonName = Dungeon;
	E.RecommendedLevel = Level;
	E.TimeLimit = TimeLimit;
	E.MaxRevives = MaxRevives;
	E.bAllowAICompanions = true;
	E.bAllowHumanReplacement = true;
	E.bIsFractureEncounter = bFracture;
	return E;
}

// ===========================================================================
// Construction
// ===========================================================================

UEclipseRaidDirector::UEclipseRaidDirector()
{
	// Sub-systems are created lazily in InitializeEncounters()
}

void UEclipseRaidDirector::InitializeEncounters()
{
	AllEncounters.Empty(14);

	// --- Solara (2 dungeons + 1 raid) ---
	AllEncounters.Add(MakeEncounter(101, TEXT("Emberforge Crucible"), EEncounterType::Dungeon, 1,
		TEXT("Solara"), TEXT("Emberforge Crucible"), 20, 600.f, 3));
	AllEncounters.Add(MakeEncounter(102, TEXT("Pyre Sanctum"), EEncounterType::Dungeon, 2,
		TEXT("Solara"), TEXT("Pyre Sanctum"), 25, 600.f, 3));
	AllEncounters.Add(MakeEncounter(103, TEXT("Solarion's Apex"), EEncounterType::Raid, 3,
		TEXT("Solara"), TEXT("Solarion's Apex"), 30, 900.f, 5, false));

	// --- Vaelwood (1 dungeon + 1 raid) ---
	AllEncounters.Add(MakeEncounter(104, TEXT("Rootbound Grove"), EEncounterType::Dungeon, 4,
		TEXT("Vaelwood"), TEXT("Rootbound Grove"), 35, 600.f, 3));
	AllEncounters.Add(MakeEncounter(105, TEXT("Thornheart Sanctum"), EEncounterType::Raid, 5,
		TEXT("Vaelwood"), TEXT("Thornheart Sanctum"), 40, 900.f, 5, false));

	// --- Morvath (1 dungeon + 1 raid) ---
	AllEncounters.Add(MakeEncounter(106, TEXT("Choir of Wails"), EEncounterType::Dungeon, 6,
		TEXT("Morvath"), TEXT("Choir of Wails"), 45, 600.f, 3));
	AllEncounters.Add(MakeEncounter(107, TEXT("Hollow Throne"), EEncounterType::Raid, 7,
		TEXT("Morvath"), TEXT("Hollow Throne"), 50, 900.f, 5, false));

	// --- Aetheris (2 dungeons + 1 raid) ---
	AllEncounters.Add(MakeEncounter(108, TEXT("Gale-Temple Spires"), EEncounterType::Dungeon, 8,
		TEXT("Aetheris"), TEXT("Gale-Temple Spires"), 55, 600.f, 3));
	AllEncounters.Add(MakeEncounter(109, TEXT("Stormcage Pinnacle"), EEncounterType::Dungeon, 9,
		TEXT("Aetheris"), TEXT("Stormcage Pinnacle"), 58, 600.f, 3));
	AllEncounters.Add(MakeEncounter(110, TEXT("Sky-Split Threshold"), EEncounterType::Raid, 10,
		TEXT("Aetheris"), TEXT("Sky-Split Threshold"), 60, 900.f, 5, false));

	// --- Nhalor (1 dungeon + 1 raid) ---
	AllEncounters.Add(MakeEncounter(111, TEXT("Velnyra's Veil"), EEncounterType::Dungeon, 11,
		TEXT("Nhalor"), TEXT("Velnyra's Veil"), 63, 600.f, 3));
	AllEncounters.Add(MakeEncounter(112, TEXT("Frozen Maw Abyss"), EEncounterType::Raid, 12,
		TEXT("Nhalor"), TEXT("Frozen Maw Abyss"), 65, 900.f, 5, false));

	// --- Umbryss (1 dungeon + 1 raid — world finale, fracture mechanics) ---
	AllEncounters.Add(MakeEncounter(113, TEXT("Nocthyra's Loom"), EEncounterType::Dungeon, 13,
		TEXT("Umbryss"), TEXT("Nocthyra's Loom"), 68, 600.f, 3, true));
	AllEncounters.Add(MakeEncounter(114, TEXT("Fracture Core"), EEncounterType::Raid, 14,
		TEXT("Umbryss"), TEXT("Fracture Core"), 70, 1200.f, 5, true));

	// Wire up subsystems
	if (!BossDirector)
	{
		BossDirector = NewObject<UEclipseBossDirector>();
	}
	BossDirector->InitializeAllBosses();

	if (!PartySystem)
	{
		PartySystem = NewObject<UEclipsePartySystem>();
	}

	State = EEncounterState::Inactive;
}

// ===========================================================================
// Queries
// ===========================================================================

TArray<FEncounterConfig> UEclipseRaidDirector::GetAllEncounters() const
{
	return AllEncounters;
}

FEncounterConfig UEclipseRaidDirector::GetEncounter(int32 EncounterID) const
{
	for (const FEncounterConfig& E : AllEncounters)
	{
		if (E.EncounterID == EncounterID) return E;
	}
	return FEncounterConfig();
}

TArray<FEncounterConfig> UEclipseRaidDirector::GetEncountersByType(EEncounterType Type) const
{
	TArray<FEncounterConfig> Result;
	for (const FEncounterConfig& E : AllEncounters)
	{
		if (E.Type == Type) Result.Add(E);
	}
	return Result;
}

int32 UEclipseRaidDirector::GetEncounterCount() const
{
	return AllEncounters.Num();
}

// ===========================================================================
// Encounter flow
// ===========================================================================

bool UEclipseRaidDirector::StartEncounter(int32 EncounterID, int32 PartySize)
{
	// Find the encounter config
	bool bFound = false;
	for (const FEncounterConfig& E : AllEncounters)
	{
		if (E.EncounterID == EncounterID)
		{
			ActiveEncounter = E;
			bFound = true;
			break;
		}
	}
	if (!bFound) return false;

	// Validate party size against encounter type
	int32 ExpectedSize = 1;
	switch (ActiveEncounter.Type)
	{
	case EEncounterType::Dungeon: ExpectedSize = 5; break;
	case EEncounterType::Raid:    ExpectedSize = 10; break;
	default: ExpectedSize = 1; break;
	}
	if (PartySize < 1) PartySize = 1;
	if (PartySize > ExpectedSize) PartySize = ExpectedSize;

	// Initialize the party
	if (PartySystem)
	{
		EGroupType GroupType = EGroupType::Solo;
		switch (ActiveEncounter.Type)
		{
		case EEncounterType::Dungeon: GroupType = EGroupType::Party; break;
		case EEncounterType::Raid:    GroupType = EGroupType::Raid;  break;
		default: GroupType = EGroupType::Solo; break;
		}
		PartySystem->InitializeGroup(GroupType);
		// Fill any remaining empty slots with AI companions
		PartySystem->FillEmptySlotsWithAI();
	}

	// Start the boss encounter via BossDirector
	if (BossDirector)
	{
		BossDirector->StartEncounter(ActiveEncounter.BossID, PartySize);
	}

	// Reset encounter state
	State = EEncounterState::InProgress;
	EncounterTimer = 0.0f;
	RevivesUsed = 0;
	DeathCount = 0;
	bBossDown = false;
	LastResult = FEncounterResult();

	return true;
}

void UEclipseRaidDirector::UpdateEncounter(float DeltaTime)
{
	if (State != EEncounterState::InProgress) return;

	// Advance encounter clock
	EncounterTimer += DeltaTime;

	// Time-limit check
	if (ActiveEncounter.TimeLimit > 0.0f && EncounterTimer >= ActiveEncounter.TimeLimit)
	{
		// Time's up — force enrage / wipe
		State = EEncounterState::Wipe;
		LastResult.bVictory = false;
		LastResult.Duration = EncounterTimer;
		LastResult.EncounterName = ActiveEncounter.EncounterName;
		LastResult.BossID = ActiveEncounter.BossID;
		LastResult.Deaths = DeathCount;
		LastResult.RevivesUsed = RevivesUsed;
		if (PartySystem)
		{
			LastResult.PartySize = PartySystem->GetPartySize();
			LastResult.AICount = PartySystem->GetAIcount();
			LastResult.HumanCount = PartySystem->GetHumanCount();
		}
		return;
	}

	// Derive aggregate HP values for the boss director update
	float PlayerHP = 100.f, TankHP = 100.f, HealerHP = 100.f;
	if (PartySystem)
	{
		float TotalHP = 0.f, TotalMaxHP = 0.f;
		for (const FPartyMember& M : PartySystem->GetPartyMembers())
		{
			TotalHP += M.bIsAlive ? M.Health : 0.f;
			TotalMaxHP += M.MaxHealth;
			if (M.Role == EClassRole::Tank) TankHP = M.bIsAlive ? (M.Health / FMath::Max(1.f, M.MaxHealth)) * 100.f : 0.f;
			if (M.Role == EClassRole::Healer) HealerHP = M.bIsAlive ? (M.Health / FMath::Max(1.f, M.MaxHealth)) * 100.f : 0.f;
		}
		if (TotalMaxHP > 0.f) PlayerHP = (TotalHP / TotalMaxHP) * 100.f;
	}

	// Drive the boss
	if (BossDirector)
	{
		BossDirector->UpdateBoss(DeltaTime, PlayerHP, TankHP, HealerHP);

		// Check for boss death → victory
		if (BossDirector->IsBossDead() && !bBossDown)
		{
			OnBossDefeated();
			return;
		}

		// Check for party wipe
		if (PartySystem && PartySystem->IsPartyWiped() && RevivesUsed >= ActiveEncounter.MaxRevives)
		{
			State = EEncounterState::Wipe;
			LastResult.bVictory = false;
			LastResult.Duration = EncounterTimer;
			LastResult.EncounterName = ActiveEncounter.EncounterName;
			LastResult.BossID = ActiveEncounter.BossID;
			LastResult.Deaths = DeathCount;
			LastResult.RevivesUsed = RevivesUsed;
			LastResult.PartySize = PartySystem->GetPartySize();
			LastResult.AICount = PartySystem->GetAIcount();
			LastResult.HumanCount = PartySystem->GetHumanCount();
			return;
		}
	}
}

EEncounterState UEclipseRaidDirector::GetState() const
{
	return State;
}

FEncounterConfig UEclipseRaidDirector::GetActiveEncounter() const
{
	return ActiveEncounter;
}

FEncounterResult UEclipseRaidDirector::GetResult() const
{
	return LastResult;
}

void UEclipseRaidDirector::PlayerDowned()
{
	if (State != EEncounterState::InProgress) return;
	DeathCount++;

	if (PartySystem && PartySystem->IsPartyWiped())
	{
		// Party wiped — check if revives are still available
		if (RevivesUsed >= ActiveEncounter.MaxRevives)
		{
			State = EEncounterState::Wipe;
			LastResult.bVictory = false;
			LastResult.Duration = EncounterTimer;
			LastResult.EncounterName = ActiveEncounter.EncounterName;
			LastResult.BossID = ActiveEncounter.BossID;
			LastResult.Deaths = DeathCount;
			LastResult.RevivesUsed = RevivesUsed;
			LastResult.PartySize = PartySystem->GetPartySize();
			LastResult.AICount = PartySystem->GetAIcount();
			LastResult.HumanCount = PartySystem->GetHumanCount();
		}
	}
}

bool UEclipseRaidDirector::RevivePlayer()
{
	if (State != EEncounterState::InProgress) return false;
	if (RevivesUsed >= ActiveEncounter.MaxRevives) return false;
	RevivesUsed++;
	// (PartySystem restores the downed member's alive state in the game layer)
	return true;
}

bool UEclipseRaidDirector::IsWipe() const
{
	return (State == EEncounterState::Wipe);
}

void UEclipseRaidDirector::OnBossDefeated()
{
	bBossDown = true;
	State = EEncounterState::Victory;

	// Distribute loot from the boss
	if (BossDirector)
	{
		LastResult.LootDistributed = BossDirector->GetLoot();
	}

	// Populate result
	LastResult.bVictory = true;
	LastResult.Duration = EncounterTimer;
	LastResult.EncounterName = ActiveEncounter.EncounterName;
	LastResult.BossID = ActiveEncounter.BossID;
	LastResult.Deaths = DeathCount;
	LastResult.RevivesUsed = RevivesUsed;
	if (PartySystem)
	{
		LastResult.PartySize = PartySystem->GetPartySize();
		LastResult.AICount = PartySystem->GetAIcount();
		LastResult.HumanCount = PartySystem->GetHumanCount();
	}

	// Transition to Complete after loot is distributed
	State = EEncounterState::Complete;
}

int32 UEclipseRaidDirector::GetRemainingRevives() const
{
	return FMath::Max(0, ActiveEncounter.MaxRevives - RevivesUsed);
}

float UEclipseRaidDirector::GetTimeRemaining() const
{
	if (ActiveEncounter.TimeLimit <= 0.0f) return -1.0f;
	return FMath::Max(0.0f, ActiveEncounter.TimeLimit - EncounterTimer);
}

float UEclipseRaidDirector::GetDuration() const
{
	return EncounterTimer;
}

void UEclipseRaidDirector::ResetEncounter()
{
	State = EEncounterState::Inactive;
	ActiveEncounter = FEncounterConfig();
	LastResult = FEncounterResult();
	EncounterTimer = 0.0f;
	RevivesUsed = 0;
	DeathCount = 0;
	bBossDown = false;
	if (BossDirector) BossDirector->ResetBoss();
}

bool UEclipseRaidDirector::ReplaceAIWithHuman(int32 Slot, const FString& PlayerName)
{
	if (!PartySystem) return false;
	if (!ActiveEncounter.bAllowHumanReplacement) return false;
	return PartySystem->ReplaceAIWithHuman(Slot, PlayerName);
}

bool UEclipseRaidDirector::ReplaceHumanWithAI(int32 Slot)
{
	if (!PartySystem) return false;
	if (!ActiveEncounter.bAllowAICompanions) return false;
	return PartySystem->ReplaceHumanWithAI(Slot);
}

UEclipsePartySystem* UEclipseRaidDirector::GetPartySystem() const
{
	return PartySystem;
}
