// Eclipse Boss Director — Full implementation
// V6: 14 bosses across 6 zones (Solara, Vaelwood, Morvath, Aetheris, Nhalor, Umbryss)
// Each boss: 3-4 phases, telegraphed attacks per phase, enrage timers, loot tables
// Combat simulation: StartEncounter, UpdateBoss, BossTakeDamage, phase transitions

#include "EclipseBossDirector.h"

// ---------------------------------------------------------------------------
// File-scope helpers — reduce verbosity of boss/phase/attack/loot construction
// ---------------------------------------------------------------------------

static FBossAttack MakeAttack(const FString& Name, EBossMechanic Telegraph,
	float Damage, float TelegraphDur, float CastTime, float Cooldown,
	float Range, float Radius, bool bAvoidable, bool bReqTank,
	bool bReqHealer, bool bReqInterrupt, bool bAppliesDebuff = false,
	EWeaponStatusEffect Debuff = EWeaponStatusEffect::None,
	float DebuffDur = 5.0f, bool bPhaseTransition = false,
	const FString& VFX = TEXT(""), const FString& Sound = TEXT(""))
{
	FBossAttack A;
	A.AttackName = Name;
	A.TelegraphType = Telegraph;
	A.Damage = Damage;
	A.TelegraphDuration = TelegraphDur;
	A.CastTime = CastTime;
	A.Cooldown = Cooldown;
	A.Range = Range;
	A.Radius = Radius;
	A.bIsAvoidable = bAvoidable;
	A.bRequiresTank = bReqTank;
	A.bRequiresHealer = bReqHealer;
	A.bRequiresInterrupt = bReqInterrupt;
	A.bAppliesDebuff = bAppliesDebuff;
	A.DebuffType = Debuff;
	A.DebuffDuration = DebuffDur;
	A.bIsPhaseTransitionAttack = bPhaseTransition;
	A.VFXAsset = VFX;
	A.SoundCue = Sound;
	return A;
}

static FBossPhaseConfig MakePhase(EBossPhase Phase, float HPThreshold,
	float DmgMult, float AtkSpdMult, float MoveSpdMult, const FString& Name,
	const TArray<EBossMechanic>& Mechanics, float EnrageTimer = -1.0f,
	const FString& Cinematic = TEXT(""), const FString& Music = TEXT(""))
{
	FBossPhaseConfig P;
	P.Phase = Phase;
	P.HPThreshold = HPThreshold;
	P.DamageMultiplier = DmgMult;
	P.AttackSpeedMultiplier = AtkSpdMult;
	P.MovementSpeedMultiplier = MoveSpdMult;
	P.Mechanics = Mechanics;
	P.EnrageTimer = EnrageTimer;
	P.PhaseName = Name;
	P.TransitionCinematic = Cinematic;
	P.MusicCue = Music;
	return P;
}

static FDropEntry MakeDrop(const FString& Name, ERarity Rarity, float Chance,
	int32 ItemLevel, bool bWeapon = false, bool bArmor = false,
	bool bSoulbound = true)
{
	FDropEntry D;
	D.ItemName = Name;
	D.Rarity = Rarity;
	D.DropType = EDropType::Universal;
	D.Binding = bSoulbound ? EBindingType::Soulbound : EBindingType::BindOnEquip;
	D.DropChance = Chance;
	D.ItemLevel = ItemLevel;
	D.bIsWeapon = bWeapon;
	D.bIsArmor = bArmor;
	return D;
}

// Seed a FBoss with identity + stats; returns reference for chaining
static FBoss& SeedBoss(FBoss& B, int32 ID, const FString& Name, const FString& Title,
	const FString& Zone, const FString& Arena, float HP, float Armor, float Resist,
	int32 Level, bool bRaid, const FString& Desc)
{
	B.BossID = ID;
	B.BossName = Name;
	B.Title = Title;
	B.ZoneName = Zone;
	B.ArenaName = Arena;
	B.Description = Desc;
	B.MaxHealth = HP;
	B.Armor = Armor;
	B.Resistance = Resist;
	B.MovementSpeed = 400.0f;
	B.RecommendedPlayerLevel = Level;
	B.MinPartySize = 1;
	B.MaxPartySize = bRaid ? 10 : 5;
	B.bIsRaidBoss = bRaid;
	B.EnrageTimerTotal = bRaid ? 720.0f : 480.0f; // 12 min raid, 8 min dungeon
	B.EnrageTimerRemaining = B.EnrageTimerTotal;
	B.MinDrops = bRaid ? 3 : 2;
	B.MaxDrops = bRaid ? 6 : 4;
	B.RequiredQuestFlag = FString::Printf(TEXT("BossUnlock_%d"), ID);
	B.KillQuestFlag = FString::Printf(TEXT("BossKill_%d"), ID);
	B.CharacterModelPath = FString::Printf(TEXT("Bosses/Model_%d"), ID);
	B.MusicTrack = FString::Printf(TEXT("Music/Boss_%s"), *Zone);
	B.IntroCinematic = FString::Printf(TEXT("Cinematics/BossIntro_%d"), ID);
	B.DeathCinematic = FString::Printf(TEXT("Cinematics/BossDeath_%d"), ID);
	return B;
}

// Append a phase + its attacks to a boss
static void AddPhase(FBoss& B, const FBossPhaseConfig& Phase, const TArray<FBossAttack>& Attacks)
{
	B.Phases.Add(Phase);
	B.PhaseAttacks.Add(Attacks);
}

// ===========================================================================
// Construction
// ===========================================================================

UEclipseBossDirector::UEclipseBossDirector()
{
	// Bosses are initialized explicitly via InitializeAllBosses()
}

void UEclipseBossDirector::InitializeAllBosses()
{
	AllBosses.Empty(14);
	InitSolaraBosses();
	InitVaelwoodBosses();
	InitMorvathBosses();
	InitAetherisBosses();
	InitNhalorBosses();
	InitUmbryssBosses();
}

// ---------------------------------------------------------------------------
// Zone: Solara (sun / fire / ember) — 3 bosses
// ---------------------------------------------------------------------------

void UEclipseBossDirector::InitSolaraBosses()
{
	// Boss 1 — Ashen Karthul (Dungeon, 3 phases)
	{
		FBoss B;
		SeedBoss(B, 1, TEXT("Ashen Karthul"), TEXT("The Ember-Forged"),
			TEXT("Solara"), TEXT("Emberforge Crucible"), 85000, 120, 60, 20, false,
			TEXT("A colossal construct of living magma, forged in the first age of Solara."));

		AddPhase(B, MakePhase(EBossPhase::Phase1, 100.f, 1.0f, 1.0f, 1.0f,
			TEXT("Opening — Molten Stance"), { EBossMechanic::TelegraphedCone, EBossMechanic::TankSwap }),
			{
				MakeAttack(TEXT("Cleaving Slag"), EBossMechanic::TelegraphedCone, 120, 1.5f, 0.8f, 6, 400, 0, true, true, false, false),
				MakeAttack(TEXT("Magma Spit"), EBossMechanic::TelegraphedAoE, 90, 2.0f, 1.0f, 10, 800, 300, true, false, false, false),
				MakeAttack(TEXT("Heat Slam"), EBossMechanic::None, 160, 0.5f, 1.2f, 8, 300, 0, false, true, true, false, true, EWeaponStatusEffect::Fire, 6.f),
			});
		AddPhase(B, MakePhase(EBossPhase::Phase2, 75.f, 1.3f, 1.2f, 1.1f,
			TEXT("Escalation — Cracking Core"), { EBossMechanic::TelegraphedAoE, EBossMechanic::AddPhase }),
			{
				MakeAttack(TEXT("Ember Burst"), EBossMechanic::TelegraphedAoE, 140, 2.5f, 1.5f, 12, 900, 350, true, false, true, false),
				MakeAttack(TEXT("Summon Cinderlings"), EBossMechanic::AddPhase, 0, 1.0f, 2.0f, 20, 600, 400, true, false, false, false),
				MakeAttack(TEXT("Slag Whip"), EBossMechanic::TelegraphedLine, 110, 1.8f, 1.0f, 7, 600, 0, true, false, false, false),
			});
		AddPhase(B, MakePhase(EBossPhase::Phase3, 50.f, 1.6f, 1.4f, 1.2f,
			TEXT("Desperation — Core Overload"), { EBossMechanic::DPSCheck, EBossMechanic::ArenaChange }),
			{
				MakeAttack(TEXT("Supernova Slam"), EBossMechanic::TelegraphedAoE, 220, 3.0f, 2.0f, 15, 1200, 500, true, false, true, false, false, EWeaponStatusEffect::None, 5.f, true),
				MakeAttack(TEXT("Floor Collapse"), EBossMechanic::ArenaChange, 80, 2.0f, 1.0f, 25, 1000, 600, true, false, false, false),
				MakeAttack(TEXT("Frenzy Strikes"), EBossMechanic::None, 90, 0.3f, 0.4f, 3, 250, 0, false, true, true, false, true, EWeaponStatusEffect::Fire, 4.f),
			});

		B.GuaranteedDrops = {
			MakeDrop(TEXT("Emberforged Greataxe"), ERarity::Epic, 100.f, 20, true, false),
			MakeDrop(TEXT("Slagcore Breastplate"), ERarity::Rare, 100.f, 20, false, true),
		};
		B.RandomDrops = {
			MakeDrop(TEXT("Magma-Core Ring"), ERarity::Rare, 35.f, 20),
			MakeDrop(TEXT("Cinderling Charm"), ERarity::Uncommon, 50.f, 20),
			MakeDrop(TEXT("Emberforge Helm"), ERarity::Epic, 12.f, 20, false, true),
			MakeDrop(TEXT("Karthus, Ember of the First Flame"), ERarity::Legendary, 3.f, 20, true),
			MakeDrop(TEXT("Forgestone Shard"), ERarity::Uncommon, 60.f, 20, false, false, false),
		};
		AllBosses.Add(B);
	}

	// Boss 2 — Pyre-Warden Velkor (Dungeon, 3 phases)
	{
		FBoss B;
		SeedBoss(B, 2, TEXT("Pyre-Warden Velkor"), TEXT("Keeper of the Eternal Flame"),
			TEXT("Solara"), TEXT("Pyre Sanctum"), 110000, 140, 80, 25, false,
			TEXT("Warden of Solara's holiest flame; turns intruders to ash."));

		AddPhase(B, MakePhase(EBossPhase::Phase1, 100.f, 1.0f, 1.0f, 1.0f,
			TEXT("Opening — Eternal Guard"), { EBossMechanic::TankSwap, EBossMechanic::TelegraphedLine }),
			{
				MakeAttack(TEXT("Warden's Halberd"), EBossMechanic::TelegraphedLine, 130, 1.5f, 1.0f, 5, 700, 0, true, true, false, false),
				MakeAttack(TEXT("Brand the Unfaithful"), EBossMechanic::TelegraphedCone, 100, 2.0f, 1.2f, 9, 500, 300, true, false, false, false, true, EWeaponStatusEffect::Fire, 8.f),
				MakeAttack(TEXT("Shield Bash"), EBossMechanic::None, 80, 0.5f, 0.8f, 6, 250, 0, false, true, false, false),
			});
		AddPhase(B, MakePhase(EBossPhase::Phase2, 70.f, 1.4f, 1.2f, 1.1f,
			TEXT("Escalation — Flame Ramparts"), { EBossMechanic::SpreadMechanic, EBossMechanic::TelegraphedAoE }),
			{
				MakeAttack(TEXT("Pillar of Fire"), EBossMechanic::TelegraphedAoE, 170, 2.5f, 1.5f, 11, 900, 300, true, false, true, false, true, EWeaponStatusEffect::Fire, 6.f),
				MakeAttack(TEXT("Scatter-Scorch"), EBossMechanic::SpreadMechanic, 90, 1.0f, 0.8f, 7, 1200, 200, true, false, false, false),
				MakeAttack(TEXT("Warden's Wrath"), EBossMechanic::TelegraphedCone, 150, 2.0f, 1.3f, 10, 600, 400, true, true, false, false),
			});
		AddPhase(B, MakePhase(EBossPhase::Phase3, 40.f, 1.8f, 1.5f, 1.2f,
			TEXT("Desperation — Last Light"), { EBossMechanic::HealCheck, EBossMechanic::DPSCheck }),
			{
				MakeAttack(TEXT("Sanctum Blaze"), EBossMechanic::HealCheck, 250, 3.0f, 2.0f, 14, 1500, 700, false, false, true, false, false, EWeaponStatusEffect::None, 5.f, true),
				MakeAttack(TEXT("Final Verdict"), EBossMechanic::TelegraphedLine, 200, 2.0f, 1.5f, 9, 800, 0, true, true, false, true),
			});

		B.GuaranteedDrops = {
			MakeDrop(TEXT("Pyre-Warden's Halberd"), ERarity::Epic, 100.f, 25, true),
			MakeDrop(TEXT("Eternal Flame Sigil"), ERarity::Rare, 100.f, 25, false, false),
		};
		B.RandomDrops = {
			MakeDrop(TEXT("Sanctum Guard Boots"), ERarity::Rare, 30.f, 25, false, true),
			MakeDrop(TEXT("Warden's Pauldrons"), ERarity::Epic, 15.f, 25, false, true),
			MakeDrop(TEXT("Flamebrand Talisman"), ERarity::Uncommon, 55.f, 25),
			MakeDrop(TEXT("Velkor's Eternal Flame"), ERarity::Legendary, 4.f, 25, true),
		};
		AllBosses.Add(B);
	}

	// Boss 3 — Solarion, the Dawn-Bringer (Raid, 4 phases) — Solara finale
	{
		FBoss B;
		SeedBoss(B, 3, TEXT("Solarion, the Dawn-Bringer"), TEXT("Light of the First Sun"),
			TEXT("Solara"), TEXT("Solarion's Apex"), 420000, 200, 150, 30, true,
			TEXT("The eldest living avatar of the sun. His radiance scorches all shadow from the world."));
		B.bUsesFractureShift = false;
		B.FracturePhaseTrigger = 3;

		AddPhase(B, MakePhase(EBossPhase::Phase1, 100.f, 1.0f, 1.0f, 1.0f,
			TEXT("Opening — Radiance"), { EBossMechanic::TankSwap, EBossMechanic::TelegraphedCone, EBossMechanic::SpreadMechanic }),
			{
				MakeAttack(TEXT("Dawn Cleave"), EBossMechanic::TelegraphedCone, 180, 2.0f, 1.2f, 6, 600, 500, true, true, false, false),
				MakeAttack(TEXT("Solar Lance"), EBossMechanic::TelegraphedLine, 240, 2.5f, 1.5f, 10, 1000, 0, true, false, false, true),
				MakeAttack(TEXT("Radiant Scatter"), EBossMechanic::SpreadMechanic, 120, 1.5f, 1.0f, 8, 1500, 250, true, false, false, false, true, EWeaponStatusEffect::Holy, 6.f),
			});
		AddPhase(B, MakePhase(EBossPhase::Phase2, 75.f, 1.3f, 1.2f, 1.1f,
			TEXT("Escalation — Crowned Sun"), { EBossMechanic::TelegraphedAoE, EBossMechanic::AddPhase, EBossMechanic::MechanicAssignment }),
			{
				MakeAttack(TEXT("Corona Burst"), EBossMechanic::TelegraphedAoE, 280, 3.0f, 2.0f, 14, 1400, 600, true, false, true, false),
				MakeAttack(TEXT("Summon Dawnborn"), EBossMechanic::AddPhase, 0, 1.0f, 2.5f, 30, 800, 500, true, false, false, false),
				MakeAttack(TEXT("Sun-Pillar Assignment"), EBossMechanic::MechanicAssignment, 200, 2.0f, 1.5f, 16, 1000, 400, true, false, false, false),
				MakeAttack(TEXT("Lance Flurry"), EBossMechanic::TelegraphedLine, 200, 1.5f, 1.0f, 5, 900, 0, true, false, false, false),
			});
		AddPhase(B, MakePhase(EBossPhase::Phase3, 50.f, 1.6f, 1.4f, 1.2f,
			TEXT("Desperation — Eclipse Warning"), { EBossMechanic::ArenaChange, EBossMechanic::StackMechanic, EBossMechanic::TimeManipulation }),
			{
				MakeAttack(TEXT("Supernova"), EBossMechanic::TelegraphedAoE, 400, 4.0f, 3.0f, 20, 2000, 900, true, false, true, false, false, EWeaponStatusEffect::None, 5.f, true),
				MakeAttack(TEXT("Time Dilation"), EBossMechanic::TimeManipulation, 0, 2.0f, 2.0f, 35, 2000, 800, true, false, false, false),
				MakeAttack(TEXT("Solar Stack"), EBossMechanic::StackMechanic, 300, 2.0f, 1.5f, 12, 500, 300, false, false, true, false, true, EWeaponStatusEffect::Holy, 8.f),
			});
		AddPhase(B, MakePhase(EBossPhase::Phase4, 25.f, 2.0f, 1.7f, 1.3f,
			TEXT("Final Stand — Last Light"), { EBossMechanic::DPSCheck, EBossMechanic::HealCheck, EBossMechanic::Untargetable }, 600.f,
			TEXT("Cinematics/Solarion_FinalStand"), TEXT("Music/Solarion_Phase4")),
			{
				MakeAttack(TEXT("Zenith Cataclysm"), EBossMechanic::DPSCheck, 500, 5.0f, 3.5f, 18, 2500, 1200, false, false, true, false, false, EWeaponStatusEffect::None, 5.f, true),
				MakeAttack(TEXT("Wrath of Dawn"), EBossMechanic::HealCheck, 450, 3.0f, 2.0f, 10, 1800, 800, false, false, true, false, true, EWeaponStatusEffect::Fire, 10.f),
				MakeAttack(TEXT("Become Unreachable"), EBossMechanic::Untargetable, 0, 2.0f, 1.0f, 22, 1500, 600, true, false, false, false),
				MakeAttack(TEXT("Dawnbreaker Combo"), EBossMechanic::TelegraphedCone, 350, 2.0f, 1.5f, 6, 700, 500, true, true, false, false),
			});

		B.GuaranteedDrops = {
			MakeDrop(TEXT("Solarion's Crown"), ERarity::Legendary, 100.f, 30, false, true),
			MakeDrop(TEXT("Dawn-Bringer's Greatsword"), ERarity::Legendary, 100.f, 30, true),
			MakeDrop(TEXT("Radiant Core"), ERarity::Mythic, 25.f, 30, false, false),
		};
		B.RandomDrops = {
			MakeDrop(TEXT("Sunforged Plate"), ERarity::Epic, 40.f, 30, false, true),
			MakeDrop(TEXT("Crown-Light Ring"), ERarity::Epic, 30.f, 30),
			MakeDrop(TEXT("Dawnborn Blade"), ERarity::Rare, 55.f, 30, true),
			MakeDrop(TEXT("Solarion's Ashes"), ERarity::Legendary, 8.f, 30),
			MakeDrop(TEXT("Eclipse-Shard"), ERarity::Mythic, 2.f, 30),
		};
		AllBosses.Add(B);
	}
}

// ---------------------------------------------------------------------------
// Zone: Vaelwood (nature / thorns / life) — 2 bosses
// ---------------------------------------------------------------------------

void UEclipseBossDirector::InitVaelwoodBosses()
{
	// Boss 4 — Mossgrasp the Encompassing (Dungeon, 3 phases)
	{
		FBoss B;
		SeedBoss(B, 4, TEXT("Mossgrasp the Encompassing"), TEXT("Ancient of the Verdant Deep"),
			TEXT("Vaelwood"), TEXT("Rootbound Grove"), 130000, 160, 70, 35, false,
			TEXT("An ancient treant whose roots drink from the world's blood."));

		AddPhase(B, MakePhase(EBossPhase::Phase1, 100.f, 1.0f, 1.0f, 0.8f,
			TEXT("Opening — Slumbering Roots"), { EBossMechanic::TelegraphedAoE, EBossMechanic::TankSwap }),
			{
				MakeAttack(TEXT("Root Lash"), EBossMechanic::TelegraphedLine, 110, 1.5f, 1.0f, 7, 700, 0, true, true, false, false),
				MakeAttack(TEXT("Spore Cloud"), EBossMechanic::TelegraphedAoE, 80, 2.5f, 1.5f, 12, 1000, 400, true, false, false, false, true, EWeaponStatusEffect::Poison, 10.f),
				MakeAttack(TEXT("Grasping Vines"), EBossMechanic::None, 70, 1.0f, 1.0f, 9, 600, 300, true, false, false, false, true, EWeaponStatusEffect::Earth, 5.f),
			});
		AddPhase(B, MakePhase(EBossPhase::Phase2, 70.f, 1.3f, 1.2f, 1.0f,
			TEXT("Escalation — Blooming Rage"), { EBossMechanic::AddPhase, EBossMechanic::TelegraphedCone }),
			{
				MakeAttack(TEXT("Thorn Spray"), EBossMechanic::TelegraphedCone, 160, 2.0f, 1.3f, 10, 600, 450, true, false, false, false, true, EWeaponStatusEffect::Poison, 8.f),
				MakeAttack(TEXT("Summon Sproutlings"), EBossMechanic::AddPhase, 0, 1.0f, 2.0f, 22, 800, 500, true, false, false, false),
				MakeAttack(TEXT("Regrowth"), EBossMechanic::None, -50, 2.0f, 2.0f, 25, 300, 300, false, false, false, false),
			});
		AddPhase(B, MakePhase(EBossPhase::Phase3, 45.f, 1.6f, 1.4f, 1.1f,
			TEXT("Desperation — Overgrowth"), { EBossMechanic::ArenaChange, EBossMechanic::HealCheck }),
			{
				MakeAttack(TEXT("Strangling Canopy"), EBossMechanic::ArenaChange, 200, 3.0f, 2.0f, 16, 1500, 800, true, false, true, false, true, EWeaponStatusEffect::Earth, 12.f),
				MakeAttack(TEXT("Frenzy Root"), EBossMechanic::TelegraphedLine, 180, 1.5f, 1.0f, 5, 800, 0, true, true, false, false),
			});

		B.GuaranteedDrops = {
			MakeDrop(TEXT("Rootbound Maul"), ERarity::Epic, 100.f, 35, true),
			MakeDrop(TEXT("Verdant Bark Shield"), ERarity::Rare, 100.f, 35, false, true),
		};
		B.RandomDrops = {
			MakeDrop(TEXT("Mossheart Pendant"), ERarity::Rare, 35.f, 35),
			MakeDrop(TEXT("Sproutling Charm"), ERarity::Uncommon, 50.f, 35),
			MakeDrop(TEXT("Thorned Greaves"), ERarity::Epic, 15.f, 35, false, true),
			MakeDrop(TEXT("Mossgrasp's Heartwood"), ERarity::Legendary, 4.f, 35, true),
		};
		AllBosses.Add(B);
	}

	// Boss 5 — Sylvanthal, Heart of Thorns (Raid, 4 phases)
	{
		FBoss B;
		SeedBoss(B, 5, TEXT("Sylvanthal, Heart of Thorns"), TEXT("The Living Sanctum"),
			TEXT("Vaelwood"), TEXT("Thornheart Sanctum"), 460000, 180, 160, 40, true,
			TEXT("The living heart of Vaelwood; a pulsing throne of thorns that bleeds sap and fury."));
		B.FracturePhaseTrigger = 3;

		AddPhase(B, MakePhase(EBossPhase::Phase1, 100.f, 1.0f, 1.0f, 1.0f,
			TEXT("Opening — Heartbeat"), { EBossMechanic::TankSwap, EBossMechanic::TelegraphedAoE, EBossMechanic::SpreadMechanic }),
			{
				MakeAttack(TEXT("Pulse of Thorns"), EBossMechanic::TelegraphedAoE, 200, 2.5f, 1.5f, 10, 1200, 500, true, false, true, false),
				MakeAttack(TEXT("Heart-Lance"), EBossMechanic::TelegraphedLine, 260, 2.0f, 1.2f, 7, 1000, 0, true, true, false, true),
				MakeAttack(TEXT("Sap Spray"), EBossMechanic::SpreadMechanic, 130, 1.5f, 1.0f, 8, 1600, 250, true, false, false, false, true, EWeaponStatusEffect::Poison, 8.f),
			});
		AddPhase(B, MakePhase(EBossPhase::Phase2, 75.f, 1.3f, 1.2f, 1.1f,
			TEXT("Escalation — Thicket"), { EBossMechanic::AddPhase, EBossMechanic::MechanicAssignment, EBossMechanic::TelegraphedCone }),
			{
				MakeAttack(TEXT("Thorn Field"), EBossMechanic::ArenaChange, 300, 3.0f, 2.0f, 18, 2000, 900, true, false, true, false, true, EWeaponStatusEffect::Earth, 10.f),
				MakeAttack(TEXT("Pollen Charge"), EBossMechanic::MechanicAssignment, 180, 2.0f, 1.5f, 14, 900, 400, true, false, false, false),
				MakeAttack(TEXT("Summon Thornborn"), EBossMechanic::AddPhase, 0, 1.0f, 2.5f, 28, 1000, 600, true, false, false, false),
				MakeAttack(TEXT("Cleave of Thorns"), EBossMechanic::TelegraphedCone, 220, 1.8f, 1.2f, 6, 700, 500, true, true, false, false),
			});
		AddPhase(B, MakePhase(EBossPhase::Phase3, 50.f, 1.6f, 1.4f, 1.2f,
			TEXT("Desperation — Bleeding Heart"), { EBossMechanic::StackMechanic, EBossMechanic::TimeManipulation, EBossMechanic::HealCheck }),
			{
				MakeAttack(TEXT("Blood-Sap Storm"), EBossMechanic::StackMechanic, 350, 2.5f, 2.0f, 13, 600, 400, false, false, true, false, true, EWeaponStatusEffect::Poison, 12.f),
				MakeAttack(TEXT("Overgrowth Surge"), EBossMechanic::TelegraphedAoE, 320, 3.0f, 2.5f, 16, 1800, 700, true, false, true, false),
				MakeAttack(TEXT("Root-Slow"), EBossMechanic::TimeManipulation, 0, 2.0f, 1.5f, 30, 2000, 1000, true, false, false, false),
			});
		AddPhase(B, MakePhase(EBossPhase::Phase4, 25.f, 2.0f, 1.7f, 1.3f,
			TEXT("Final Stand — Withering Bloom"), { EBossMechanic::DPSCheck, EBossMechanic::ArenaChange }, 660.f,
			TEXT("Cinematics/Sylvanthal_FinalStand"), TEXT("Music/Sylvanthal_Phase4")),
			{
				MakeAttack(TEXT("Bloom of Ruin"), EBossMechanic::DPSCheck, 500, 5.0f, 3.5f, 20, 2500, 1200, false, false, true, false, false, EWeaponStatusEffect::None, 5.f, true),
				MakeAttack(TEXT("Thornmaw Devour"), EBossMechanic::TelegraphedCone, 380, 2.0f, 1.5f, 7, 800, 600, true, true, false, false, true, EWeaponStatusEffect::Poison, 10.f),
				MakeAttack(TEXT("Sanctum Collapse"), EBossMechanic::ArenaChange, 280, 3.0f, 2.0f, 25, 2200, 1000, true, false, false, false),
			});

		B.GuaranteedDrops = {
			MakeDrop(TEXT("Thornheart Crown"), ERarity::Legendary, 100.f, 40, false, true),
			MakeDrop(TEXT("Sylvanthal's Briar Spear"), ERarity::Legendary, 100.f, 40, true),
			MakeDrop(TEXT("Verdant Heart-Core"), ERarity::Mythic, 22.f, 40, false, false),
		};
		B.RandomDrops = {
			MakeDrop(TEXT("Thornweave Robe"), ERarity::Epic, 35.f, 40, false, true),
			MakeDrop(TEXT("Sapstone Ring"), ERarity::Epic, 30.f, 40),
			MakeDrop(TEXT("Briarbark Gauntlets"), ERarity::Rare, 50.f, 40, false, true),
			MakeDrop(TEXT("Sylvanthal's Last Bloom"), ERarity::Legendary, 7.f, 40),
			MakeDrop(TEXT("Thorn-Shard"), ERarity::Mythic, 2.f, 40),
		};
		AllBosses.Add(B);
	}
}

// ---------------------------------------------------------------------------
// Zone: Morvath (death / hollow / void) — 2 bosses
// ---------------------------------------------------------------------------

void UEclipseBossDirector::InitMorvathBosses()
{
	// Boss 6 — The Wailing Choir (Dungeon, 3 phases)
	{
		FBoss B;
		SeedBoss(B, 6, TEXT("The Wailing Choir"), TEXT("Chorus of the Damned"),
			TEXT("Morvath"), TEXT("Choir of Wails"), 140000, 90, 180, 45, false,
			TEXT("A chorus of the hollowed dead, whose song tears the soul from the body."));

		AddPhase(B, MakePhase(EBossPhase::Phase1, 100.f, 1.0f, 1.0f, 1.0f,
			TEXT("Opening — Dirge"), { EBossMechanic::SilenceMechanic, EBossMechanic::TelegraphedAoE }),
			{
				MakeAttack(TEXT("Wailing Note"), EBossMechanic::TelegraphedAoE, 130, 2.0f, 1.5f, 9, 1100, 450, true, false, true, false, true, EWeaponStatusEffect::Soul, 6.f),
				MakeAttack(TEXT("Hush"), EBossMechanic::SilenceMechanic, 0, 1.5f, 1.0f, 18, 1500, 1000, true, false, false, false),
				MakeAttack(TEXT("Soul Rip"), EBossMechanic::None, 160, 0.5f, 1.0f, 7, 400, 0, false, false, false, false, true, EWeaponStatusEffect::Soul, 8.f),
			});
		AddPhase(B, MakePhase(EBossPhase::Phase2, 70.f, 1.4f, 1.3f, 1.1f,
			TEXT("Escalation — Crescendo"), { EBossMechanic::AddPhase, EBossMechanic::SpreadMechanic, EBossMechanic::TelegraphedCone }),
			{
				MakeAttack(TEXT("Dissonant Chord"), EBossMechanic::TelegraphedCone, 190, 2.0f, 1.3f, 10, 700, 500, true, false, false, false, true, EWeaponStatusEffect::Soul, 8.f),
				MakeAttack(TEXT("Summon Hollowed"), EBossMechanic::AddPhase, 0, 1.0f, 2.0f, 25, 900, 600, true, false, false, false),
				MakeAttack(TEXT("Echoing Wail"), EBossMechanic::SpreadMechanic, 110, 1.5f, 1.0f, 8, 1600, 300, true, false, false, false),
			});
		AddPhase(B, MakePhase(EBossPhase::Phase3, 40.f, 1.8f, 1.5f, 1.2f,
			TEXT("Desperation — Final Refrain"), { EBossMechanic::HealCheck, EBossMechanic::SilenceMechanic }),
			{
				MakeAttack(TEXT("Funeral March"), EBossMechanic::HealCheck, 280, 3.0f, 2.5f, 15, 1800, 900, false, false, true, false, true, EWeaponStatusEffect::Soul, 12.f),
				MakeAttack(TEXT("Silent Aria"), EBossMechanic::SilenceMechanic, 0, 2.0f, 1.5f, 22, 2000, 1200, true, false, false, false),
				MakeAttack(TEXT("Choir Frenzy"), EBossMechanic::TelegraphedAoE, 240, 2.0f, 1.5f, 6, 1200, 500, true, false, true, false),
			});

		B.GuaranteedDrops = {
			MakeDrop(TEXT("Choir's Echoing Staff"), ERarity::Epic, 100.f, 45, true),
			MakeDrop(TEXT("Hollowsong Cowl"), ERarity::Rare, 100.f, 45, false, true),
		};
		B.RandomDrops = {
			MakeDrop(TEXT("Soul-Chime Ring"), ERarity::Rare, 35.f, 45),
			MakeDrop(TEXT("Wailstone Charm"), ERarity::Uncommon, 50.f, 45),
			MakeDrop(TEXT("Choir-Mother's Veil"), ERarity::Epic, 14.f, 45, false, true),
			MakeDrop(TEXT("Hymn of the Damned"), ERarity::Legendary, 4.f, 45, true),
		};
		AllBosses.Add(B);
	}

	// Boss 7 — Lord Morvath, the Hollow King (Raid, 4 phases) — Morvath finale
	{
		FBoss B;
		SeedBoss(B, 7, TEXT("Lord Morvath, the Hollow King"), TEXT("Sovereign of the Hollow Court"),
			TEXT("Morvath"), TEXT("Hollow Throne"), 500000, 220, 200, 50, true,
			TEXT("The sovereign of death itself. His crown drinks the light; his throne devours souls."));
		B.FracturePhaseTrigger = 3;

		AddPhase(B, MakePhase(EBossPhase::Phase1, 100.f, 1.0f, 1.0f, 1.0f,
			TEXT("Opening — Court of the Hollow"), { EBossMechanic::TankSwap, EBossMechanic::TelegraphedLine, EBossMechanic::SpreadMechanic }),
			{
				MakeAttack(TEXT("Decree Blade"), EBossMechanic::TelegraphedLine, 240, 2.0f, 1.2f, 7, 1000, 0, true, true, false, false),
				MakeAttack(TEXT("Hollow Scythe"), EBossMechanic::TelegraphedCone, 200, 2.0f, 1.3f, 9, 700, 500, true, true, false, false, true, EWeaponStatusEffect::Soul, 8.f),
				MakeAttack(TEXT("Shroud Pulse"), EBossMechanic::SpreadMechanic, 150, 1.5f, 1.0f, 8, 1600, 250, true, false, false, false, true, EWeaponStatusEffect::Shadow, 6.f),
			});
		AddPhase(B, MakePhase(EBossPhase::Phase2, 75.f, 1.3f, 1.2f, 1.1f,
			TEXT("Escalation — The Crown Drinks"), { EBossMechanic::AddPhase, EBossMechanic::MechanicAssignment, EBossMechanic::SilenceMechanic }),
			{
				MakeAttack(TEXT("Soul Harvest"), EBossMechanic::TelegraphedAoE, 300, 3.0f, 2.0f, 14, 1500, 600, true, false, true, false, true, EWeaponStatusEffect::Soul, 10.f),
				MakeAttack(TEXT("Summon Hollowed Knights"), EBossMechanic::AddPhase, 0, 1.0f, 2.5f, 30, 1000, 600, true, false, false, false),
				MakeAttack(TEXT("Throne-Chain"), EBossMechanic::MechanicAssignment, 250, 2.5f, 1.5f, 16, 1200, 500, true, false, false, false),
				MakeAttack(TEXT("Mute"), EBossMechanic::SilenceMechanic, 0, 1.5f, 1.0f, 20, 2000, 1200, true, false, false, false),
			});
		AddPhase(B, MakePhase(EBossPhase::Phase3, 50.f, 1.6f, 1.4f, 1.2f,
			TEXT("Desperation — Hollowing"), { EBossMechanic::ArenaChange, EBossMechanic::TimeManipulation, EBossMechanic::TankSwap }),
			{
				MakeAttack(TEXT("Hollow Crown Burst"), EBossMechanic::TelegraphedAoE, 400, 3.5f, 2.5f, 18, 2000, 800, true, false, true, false, false, EWeaponStatusEffect::None, 5.f, true),
				MakeAttack(TEXT("Reversal of Fate"), EBossMechanic::TimeManipulation, 0, 2.5f, 2.0f, 35, 2500, 1200, true, false, false, false),
				MakeAttack(TEXT("Soul-Rend Combo"), EBossMechanic::TelegraphedLine, 320, 1.5f, 1.0f, 5, 900, 0, true, true, false, false, true, EWeaponStatusEffect::Soul, 8.f),
			});
		AddPhase(B, MakePhase(EBossPhase::Phase4, 25.f, 2.0f, 1.7f, 1.3f,
			TEXT("Final Stand — The Hollow Throne"), { EBossMechanic::DPSCheck, EBossMechanic::Untargetable, EBossMechanic::HealCheck }, 720.f,
			TEXT("Cinematics/Morvath_FinalStand"), TEXT("Music/Morvath_Phase4")),
			{
				MakeAttack(TEXT("End of All Songs"), EBossMechanic::DPSCheck, 550, 5.0f, 3.5f, 18, 2800, 1300, false, false, true, false, false, EWeaponStatusEffect::None, 5.f, true),
				MakeAttack(TEXT("Throne's Shadow"), EBossMechanic::Untargetable, 0, 2.0f, 1.5f, 24, 2000, 800, true, false, false, false),
				MakeAttack(TEXT("Soul-Tithe"), EBossMechanic::HealCheck, 480, 3.0f, 2.0f, 10, 1800, 900, false, false, true, false, true, EWeaponStatusEffect::Soul, 12.f),
				MakeAttack(TEXT("Hollow Decree"), EBossMechanic::TelegraphedCone, 420, 2.0f, 1.5f, 6, 800, 600, true, true, false, false),
			});

		B.GuaranteedDrops = {
			MakeDrop(TEXT("Crown of the Hollow"), ERarity::Legendary, 100.f, 50, false, true),
			MakeDrop(TEXT("Morvath's Soul-Scythe"), ERarity::Legendary, 100.f, 50, true),
			MakeDrop(TEXT("Hollow Throne Core"), ERarity::Mythic, 20.f, 50, false, false),
		};
		B.RandomDrops = {
			MakeDrop(TEXT("Hollow-King Plate"), ERarity::Epic, 38.f, 50, false, true),
			MakeDrop(TEXT("Soul-Tithe Ring"), ERarity::Epic, 28.f, 50),
			MakeDrop(TEXT("Wraith-Silk Cloak"), ERarity::Rare, 50.f, 50, false, true),
			MakeDrop(TEXT("Morvath's Final Decree"), ERarity::Legendary, 6.f, 50, true),
			MakeDrop(TEXT("Hollow-Shard"), ERarity::Mythic, 2.f, 50),
		};
		AllBosses.Add(B);
	}
}

// ---------------------------------------------------------------------------
// Zone: Aetheris (wind / sky / aether) — 3 bosses
// ---------------------------------------------------------------------------

void UEclipseBossDirector::InitAetherisBosses()
{
	// Boss 8 — Zephyrian Gale-Caller (Dungeon, 3 phases)
	{
		FBoss B;
		SeedBoss(B, 8, TEXT("Zephyrian Gale-Caller"), TEXT("Voice of the Howling Heights"),
			TEXT("Aetheris"), TEXT("Gale-Temple Spires"), 150000, 80, 220, 55, false,
			TEXT("A storm-priest whose voice commands the winds of the upper aether."));

		AddPhase(B, MakePhase(EBossPhase::Phase1, 100.f, 1.0f, 1.0f, 1.2f,
			TEXT("Opening — Whispering Wind"), { EBossMechanic::TelegraphedAoE, EBossMechanic::SpreadMechanic }),
			{
				MakeAttack(TEXT("Gust Lance"), EBossMechanic::TelegraphedLine, 120, 1.5f, 1.0f, 6, 900, 0, true, false, false, false, true, EWeaponStatusEffect::Wind, 5.f),
				MakeAttack(TEXT("Zephyr Burst"), EBossMechanic::TelegraphedAoE, 100, 2.0f, 1.5f, 10, 1200, 400, true, false, false, false, true, EWeaponStatusEffect::Wind, 6.f),
				MakeAttack(TEXT("Scatter Gust"), EBossMechanic::SpreadMechanic, 80, 1.0f, 0.8f, 7, 1600, 250, true, false, false, false),
			});
		AddPhase(B, MakePhase(EBossPhase::Phase2, 70.f, 1.4f, 1.3f, 1.3f,
			TEXT("Escalation — Howling Heights"), { EBossMechanic::TelegraphedCone, EBossMechanic::MechanicAssignment, EBossMechanic::AddPhase }),
			{
				MakeAttack(TEXT("Cyclone Cleave"), EBossMechanic::TelegraphedCone, 180, 2.0f, 1.3f, 9, 700, 500, true, false, false, false, true, EWeaponStatusEffect::Lightning, 6.f),
				MakeAttack(TEXT("Wind-Platform"), EBossMechanic::MechanicAssignment, 160, 2.5f, 1.5f, 15, 1000, 500, true, false, false, false),
				MakeAttack(TEXT("Summon Galeborn"), EBossMechanic::AddPhase, 0, 1.0f, 2.0f, 24, 800, 500, true, false, false, false),
				MakeAttack(TEXT("Static Discharge"), EBossMechanic::TelegraphedAoE, 200, 2.0f, 1.5f, 12, 1400, 500, true, false, true, false, true, EWeaponStatusEffect::Lightning, 8.f),
			});
		AddPhase(B, MakePhase(EBossPhase::Phase3, 40.f, 1.8f, 1.5f, 1.4f,
			TEXT("Desperation — Hurricane"), { EBossMechanic::ArenaChange, EBossMechanic::HealCheck }),
			{
				MakeAttack(TEXT("Tempest Eye"), EBossMechanic::ArenaChange, 260, 3.0f, 2.5f, 16, 2000, 900, true, false, true, false, true, EWeaponStatusEffect::Wind, 10.f),
				MakeAttack(TEXT("Lightning Roar"), EBossMechanic::HealCheck, 290, 2.5f, 2.0f, 12, 1800, 800, false, false, true, false, true, EWeaponStatusEffect::Lightning, 10.f),
			});

		B.GuaranteedDrops = {
			MakeDrop(TEXT("Gale-Caller's Staff"), ERarity::Epic, 100.f, 55, true),
			MakeDrop(TEXT("Tempest-Woven Vestments"), ERarity::Rare, 100.f, 55, false, true),
		};
		B.RandomDrops = {
			MakeDrop(TEXT("Zephyr Ring"), ERarity::Rare, 35.f, 55),
			MakeDrop(TEXT("Static-Charged Charm"), ERarity::Uncommon, 50.f, 55),
			MakeDrop(TEXT("Gale-Temple Sandals"), ERarity::Epic, 15.f, 55, false, true),
			MakeDrop(TEXT("Zephyr's Howling Core"), ERarity::Legendary, 4.f, 55),
		};
		AllBosses.Add(B);
	}

	// Boss 9 — The Stormcage Warden (Dungeon, 3 phases)
	{
		FBoss B;
		SeedBoss(B, 9, TEXT("The Stormcage Warden"), TEXT("Jailer of the Tempest"),
			TEXT("Aetheris"), TEXT("Stormcage Pinnacle"), 170000, 200, 130, 58, false,
			TEXT("Warden of a prison built to hold the storm itself — now its master."));

		AddPhase(B, MakePhase(EBossPhase::Phase1, 100.f, 1.0f, 1.0f, 0.9f,
			TEXT("Opening — Imprisoned Storm"), { EBossMechanic::TankSwap, EBossMechanic::TelegraphedCone }),
			{
				MakeAttack(TEXT("Cage-Strike"), EBossMechanic::TelegraphedCone, 140, 1.5f, 1.0f, 6, 600, 400, true, true, false, false),
				MakeAttack(TEXT("Chain Lightning"), EBossMechanic::TelegraphedLine, 180, 2.0f, 1.2f, 10, 1000, 0, true, false, false, false, true, EWeaponStatusEffect::Lightning, 6.f),
				MakeAttack(TEXT("Bolt Barrage"), EBossMechanic::TelegraphedAoE, 120, 2.5f, 1.5f, 12, 1300, 400, true, false, false, false, true, EWeaponStatusEffect::Lightning, 7.f),
			});
		AddPhase(B, MakePhase(EBossPhase::Phase2, 65.f, 1.4f, 1.3f, 1.1f,
			TEXT("Escalation — Storm Released"), { EBossMechanic::AddPhase, EBossMechanic::MechanicAssignment, EBossMechanic::SpreadMechanic }),
			{
				MakeAttack(TEXT("Release the Tempest"), EBossMechanic::TelegraphedAoE, 260, 3.0f, 2.0f, 15, 1800, 700, true, false, true, false, true, EWeaponStatusEffect::Lightning, 8.f),
				MakeAttack(TEXT("Summon Stormcaged"), EBossMechanic::AddPhase, 0, 1.0f, 2.0f, 25, 900, 600, true, false, false, false),
				MakeAttack(TEXT("Cage-Platform"), EBossMechanic::MechanicAssignment, 200, 2.5f, 1.5f, 16, 1100, 500, true, false, false, false),
				MakeAttack(TEXT("Scatter-Bolt"), EBossMechanic::SpreadMechanic, 150, 1.5f, 1.0f, 8, 1700, 300, true, false, false, false),
			});
		AddPhase(B, MakePhase(EBossPhase::Phase3, 35.f, 1.8f, 1.5f, 1.2f,
			TEXT("Desperation — Cage Break"), { EBossMechanic::ArenaChange, EBossMechanic::DPSCheck }),
			{
				MakeAttack(TEXT("Stormcage Collapse"), EBossMechanic::ArenaChange, 320, 3.5f, 2.5f, 18, 2200, 1000, true, false, true, false, true, EWeaponStatusEffect::Lightning, 10.f),
				MakeAttack(TEXT("Final Bolt"), EBossMechanic::DPSCheck, 400, 4.0f, 3.0f, 14, 2400, 800, false, false, true, false, false, EWeaponStatusEffect::None, 5.f, true),
			});

		B.GuaranteedDrops = {
			MakeDrop(TEXT("Stormcage Warhammer"), ERarity::Epic, 100.f, 58, true),
			MakeDrop(TEXT("Tempest-Bound Plate"), ERarity::Rare, 100.f, 58, false, true),
		};
		B.RandomDrops = {
			MakeDrop(TEXT("Cage-Key Pendant"), ERarity::Rare, 35.f, 58),
			MakeDrop(TEXT("Static Greaves"), ERarity::Epic, 15.f, 58, false, true),
			MakeDrop(TEXT("Stormcaller's Ring"), ERarity::Uncommon, 50.f, 58),
			MakeDrop(TEXT("Warden's Tempest Core"), ERarity::Legendary, 4.f, 58, true),
		};
		AllBosses.Add(B);
	}

	// Boss 10 — Aetheryon, the Sky-Splitter (Raid, 4 phases) — Aetheris finale
	{
		FBoss B;
		SeedBoss(B, 10, TEXT("Aetheryon, the Sky-Splitter"), TEXT("Divider of Heavens"),
			TEXT("Aetheris"), TEXT("Sky-Split Threshold"), 520000, 150, 250, 60, true,
			TEXT("The dragon-god of the aether; his wings part the sky and the world alike."));
		B.FracturePhaseTrigger = 3;

		AddPhase(B, MakePhase(EBossPhase::Phase1, 100.f, 1.0f, 1.0f, 1.2f,
			TEXT("Opening — Ascension"), { EBossMechanic::TankSwap, EBossMechanic::TelegraphedCone, EBossMechanic::TelegraphedLine }),
			{
				MakeAttack(TEXT("Sky-Splitter Claw"), EBossMechanic::TelegraphedCone, 240, 2.0f, 1.2f, 6, 800, 600, true, true, false, false),
				MakeAttack(TEXT("Aether Lance"), EBossMechanic::TelegraphedLine, 280, 2.0f, 1.3f, 8, 1100, 0, true, false, false, false, true, EWeaponStatusEffect::Lightning, 6.f),
				MakeAttack(TEXT("Gust-Slam"), EBossMechanic::TelegraphedAoE, 180, 2.5f, 1.5f, 11, 1400, 500, true, false, true, false, true, EWeaponStatusEffect::Wind, 6.f),
			});
		AddPhase(B, MakePhase(EBossPhase::Phase2, 75.f, 1.3f, 1.2f, 1.2f,
			TEXT("Escalation — Storm-Wing"), { EBossMechanic::AddPhase, EBossMechanic::MechanicAssignment, EBossMechanic::SpreadMechanic, EBossMechanic::TelegraphedAoE }),
			{
				MakeAttack(TEXT("Aether-Burst"), EBossMechanic::TelegraphedAoE, 340, 3.0f, 2.0f, 14, 1800, 700, true, false, true, false, true, EWeaponStatusEffect::Lightning, 8.f),
				MakeAttack(TEXT("Summon Skyborn"), EBossMechanic::AddPhase, 0, 1.0f, 2.5f, 28, 1000, 600, true, false, false, false),
				MakeAttack(TEXT("Wind-Platform"), EBossMechanic::MechanicAssignment, 260, 2.5f, 1.5f, 15, 1200, 500, true, false, false, false),
				MakeAttack(TEXT("Static-Scatter"), EBossMechanic::SpreadMechanic, 160, 1.5f, 1.0f, 7, 1700, 300, true, false, false, false),
			});
		AddPhase(B, MakePhase(EBossPhase::Phase3, 50.f, 1.6f, 1.4f, 1.3f,
			TEXT("Desperation — Fracture-Sky"), { EBossMechanic::ArenaChange, EBossMechanic::TimeManipulation, EBossMechanic::HealCheck }),
			{
				MakeAttack(TEXT("Sky-Fracture"), EBossMechanic::ArenaChange, 420, 4.0f, 2.5f, 18, 2400, 1000, true, false, true, false, true, EWeaponStatusEffect::Lightning, 10.f),
				MakeAttack(TEXT("Time-Stall"), EBossMechanic::TimeManipulation, 0, 2.5f, 2.0f, 32, 2500, 1200, true, false, false, false),
				MakeAttack(TEXT("Aether Storm"), EBossMechanic::HealCheck, 380, 3.0f, 2.0f, 12, 2000, 900, false, false, true, false, true, EWeaponStatusEffect::Wind, 10.f),
			});
		AddPhase(B, MakePhase(EBossPhase::Phase4, 25.f, 2.0f, 1.7f, 1.4f,
			TEXT("Final Stand — Heaven's End"), { EBossMechanic::DPSCheck, EBossMechanic::Untargetable, EBossMechanic::ArenaChange }, 660.f,
			TEXT("Cinematics/Aetheryon_FinalStand"), TEXT("Music/Aetheryon_Phase4")),
			{
				MakeAttack(TEXT("Zenith Storm"), EBossMechanic::DPSCheck, 560, 5.0f, 3.5f, 20, 2800, 1300, false, false, true, false, false, EWeaponStatusEffect::None, 5.f, true),
				MakeAttack(TEXT("Sky-Split Roost"), EBossMechanic::Untargetable, 0, 2.0f, 1.5f, 24, 2000, 800, true, false, false, false),
				MakeAttack(TEXT("Aether-Descent"), EBossMechanic::ArenaChange, 460, 3.0f, 2.0f, 14, 2200, 1000, true, true, false, false, true, EWeaponStatusEffect::Lightning, 10.f),
				MakeAttack(TEXT("Claw Flurry"), EBossMechanic::TelegraphedCone, 400, 1.5f, 1.0f, 5, 700, 600, true, true, false, false),
			});

		B.GuaranteedDrops = {
			MakeDrop(TEXT("Sky-Splitter's Aether-Scale"), ERarity::Legendary, 100.f, 60, false, true),
			MakeDrop(TEXT("Aetheryon's Talon-Glaive"), ERarity::Legendary, 100.f, 60, true),
			MakeDrop(TEXT("Aether Dragon-Core"), ERarity::Mythic, 20.f, 60, false, false),
		};
		B.RandomDrops = {
			MakeDrop(TEXT("Storm-Wing Plate"), ERarity::Epic, 38.f, 60, false, true),
			MakeDrop(TEXT("Aether-Charged Ring"), ERarity::Epic, 28.f, 60),
			MakeDrop(TEXT("Skyborn Glaive"), ERarity::Rare, 50.f, 60, true),
			MakeDrop(TEXT("Aetheryon's Last Wing"), ERarity::Legendary, 7.f, 60),
			MakeDrop(TEXT("Sky-Shard"), ERarity::Mythic, 2.f, 60),
		};
		AllBosses.Add(B);
	}
}

// ---------------------------------------------------------------------------
// Zone: Nhalor (ice / cold / frozen) — 2 bosses
// ---------------------------------------------------------------------------

void UEclipseBossDirector::InitNhalorBosses()
{
	// Boss 11 — Glacial Widow Velnyra (Dungeon, 3 phases)
	{
		FBoss B;
		SeedBoss(B, 11, TEXT("Glacial Widow Velnyra"), TEXT("Bride of the Endless Winter"),
			TEXT("Nhalor"), TEXT("Velnyra's Veil"), 190000, 130, 200, 63, false,
			TEXT("A widow-bride who married the winter itself; her kiss freezes the heart."));

		AddPhase(B, MakePhase(EBossPhase::Phase1, 100.f, 1.0f, 1.0f, 1.0f,
			TEXT("Opening — Frozen Vow"), { EBossMechanic::TelegraphedCone, EBossMechanic::SpreadMechanic }),
			{
				MakeAttack(TEXT("Kiss of Frost"), EBossMechanic::TelegraphedCone, 130, 2.0f, 1.2f, 8, 600, 400, true, false, false, false, true, EWeaponStatusEffect::Ice, 8.f),
				MakeAttack(TEXT("Ice-Lance"), EBossMechanic::TelegraphedLine, 170, 1.5f, 1.0f, 6, 1000, 0, true, false, false, false, true, EWeaponStatusEffect::Ice, 6.f),
				MakeAttack(TEXT("Frost-Scatter"), EBossMechanic::SpreadMechanic, 90, 1.5f, 1.0f, 9, 1500, 300, true, false, false, false, true, EWeaponStatusEffect::Ice, 5.f),
			});
		AddPhase(B, MakePhase(EBossPhase::Phase2, 65.f, 1.4f, 1.3f, 1.1f,
			TEXT("Escalation — Widening Cold"), { EBossMechanic::AddPhase, EBossMechanic::TelegraphedAoE, EBossMechanic::SilenceMechanic }),
			{
				MakeAttack(TEXT("Blizzard Veil"), EBossMechanic::TelegraphedAoE, 220, 3.0f, 2.0f, 13, 1600, 600, true, false, true, false, true, EWeaponStatusEffect::Ice, 10.f),
				MakeAttack(TEXT("Summon Frost-Widows"), EBossMechanic::AddPhase, 0, 1.0f, 2.0f, 24, 900, 600, true, false, false, false),
				MakeAttack(TEXT("Silent Embrace"), EBossMechanic::SilenceMechanic, 0, 2.0f, 1.5f, 20, 1800, 1000, true, false, false, false),
				MakeAttack(TEXT("Frozen Touch"), EBossMechanic::None, 180, 0.5f, 1.0f, 5, 300, 0, false, false, false, false, true, EWeaponStatusEffect::Ice, 10.f),
			});
		AddPhase(B, MakePhase(EBossPhase::Phase3, 35.f, 1.8f, 1.5f, 1.2f,
			TEXT("Desperation — Endless Winter"), { EBossMechanic::ArenaChange, EBossMechanic::HealCheck }),
			{
				MakeAttack(TEXT("Heart-Freeze"), EBossMechanic::HealCheck, 300, 3.5f, 2.5f, 16, 2000, 900, false, false, true, false, true, EWeaponStatusEffect::Ice, 12.f),
				MakeAttack(TEXT("Veil Collapse"), EBossMechanic::ArenaChange, 340, 3.0f, 2.0f, 18, 2200, 1000, true, false, false, false, true, EWeaponStatusEffect::Ice, 10.f),
			});

		B.GuaranteedDrops = {
			MakeDrop(TEXT("Velnyra's Frozen Veil"), ERarity::Epic, 100.f, 63, false, true),
			MakeDrop(TEXT("Widow's Ice-Daggers"), ERarity::Rare, 100.f, 63, true),
		};
		B.RandomDrops = {
			MakeDrop(TEXT("Frostbite Ring"), ERarity::Rare, 35.f, 63),
			MakeDrop(TEXT("Glacial Charm"), ERarity::Uncommon, 50.f, 63),
			MakeDrop(TEXT("Widow-Silk Cloak"), ERarity::Epic, 15.f, 63, false, true),
			MakeDrop(TEXT("Velnyra's Frozen Heart"), ERarity::Legendary, 4.f, 63),
		};
		AllBosses.Add(B);
	}

	// Boss 12 — Nhalor, the Frozen Maw (Raid, 4 phases) — Nhalor finale
	{
		FBoss B;
		SeedBoss(B, 12, TEXT("Nhalor, the Frozen Maw"), TEXT("Devourer of Warmth"),
			TEXT("Nhalor"), TEXT("Frozen Maw Abyss"), 540000, 260, 240, 65, true,
			TEXT("A primordial beast of the deep cold; its maw devours all warmth and life."));
		B.FracturePhaseTrigger = 3;

		AddPhase(B, MakePhase(EBossPhase::Phase1, 100.f, 1.0f, 1.0f, 1.0f,
			TEXT("Opening — Hungering Cold"), { EBossMechanic::TankSwap, EBossMechanic::TelegraphedCone, EBossMechanic::SpreadMechanic }),
			{
				MakeAttack(TEXT("Maw-Bite"), EBossMechanic::TelegraphedCone, 260, 2.0f, 1.2f, 6, 700, 600, true, true, false, false, true, EWeaponStatusEffect::Ice, 8.f),
				MakeAttack(TEXT("Glacial Lance"), EBossMechanic::TelegraphedLine, 300, 2.0f, 1.3f, 8, 1100, 0, true, false, false, false, true, EWeaponStatusEffect::Ice, 6.f),
				MakeAttack(TEXT("Frost-Scatter"), EBossMechanic::SpreadMechanic, 170, 1.5f, 1.0f, 7, 1700, 300, true, false, false, false, true, EWeaponStatusEffect::Ice, 5.f),
			});
		AddPhase(B, MakePhase(EBossPhase::Phase2, 75.f, 1.3f, 1.2f, 1.1f,
			TEXT("Escalation — Deep Winter"), { EBossMechanic::AddPhase, EBossMechanic::TelegraphedAoE, EBossMechanic::MechanicAssignment }),
			{
				MakeAttack(TEXT("Avalanche"), EBossMechanic::TelegraphedAoE, 360, 3.0f, 2.0f, 14, 1900, 700, true, false, true, false, true, EWeaponStatusEffect::Ice, 8.f),
				MakeAttack(TEXT("Summon Frost-Mawlings"), EBossMechanic::AddPhase, 0, 1.0f, 2.5f, 28, 1000, 600, true, false, false, false),
				MakeAttack(TEXT("Ice-Platform"), EBossMechanic::MechanicAssignment, 280, 2.5f, 1.5f, 15, 1200, 500, true, false, false, false),
				MakeAttack(TEXT("Frozen Roar"), EBossMechanic::TelegraphedCone, 320, 2.0f, 1.3f, 9, 800, 600, true, true, false, false),
			});
		AddPhase(B, MakePhase(EBossPhase::Phase3, 50.f, 1.6f, 1.4f, 1.2f,
			TEXT("Desperation — World-Skip"), { EBossMechanic::ArenaChange, EBossMechanic::TimeManipulation, EBossMechanic::SilenceMechanic }),
			{
				MakeAttack(TEXT("Permafrost Eruption"), EBossMechanic::ArenaChange, 440, 4.0f, 2.5f, 18, 2400, 1000, true, false, true, false, true, EWeaponStatusEffect::Ice, 10.f),
				MakeAttack(TEXT("Time-Freeze"), EBossMechanic::TimeManipulation, 0, 2.5f, 2.0f, 30, 2500, 1200, true, false, false, false),
				MakeAttack(TEXT("Hush of the Maw"), EBossMechanic::SilenceMechanic, 0, 2.0f, 1.5f, 22, 2000, 1200, true, false, false, false),
			});
		AddPhase(B, MakePhase(EBossPhase::Phase4, 25.f, 2.0f, 1.7f, 1.3f,
			TEXT("Final Stand — Absolute Zero"), { EBossMechanic::DPSCheck, EBossMechanic::HealCheck, EBossMechanic::Untargetable }, 660.f,
			TEXT("Cinematics/Nhalor_FinalStand"), TEXT("Music/Nhalor_Phase4")),
			{
				MakeAttack(TEXT("Heat Death"), EBossMechanic::DPSCheck, 580, 5.0f, 3.5f, 18, 2800, 1300, false, false, true, false, false, EWeaponStatusEffect::None, 5.f, true),
				MakeAttack(TEXT("Devour Warmth"), EBossMechanic::HealCheck, 500, 3.0f, 2.0f, 10, 1800, 900, false, false, true, false, true, EWeaponStatusEffect::Ice, 12.f),
				MakeAttack(TEXT("Frozen Slumber"), EBossMechanic::Untargetable, 0, 2.0f, 1.5f, 24, 2000, 800, true, false, false, false),
				MakeAttack(TEXT("Maw Frenzy"), EBossMechanic::TelegraphedCone, 440, 1.5f, 1.0f, 5, 700, 600, true, true, false, false, true, EWeaponStatusEffect::Ice, 10.f),
			});

		B.GuaranteedDrops = {
			MakeDrop(TEXT("Crown of the Frozen Maw"), ERarity::Legendary, 100.f, 65, false, true),
			MakeDrop(TEXT("Nhalor's Glacial Greatsword"), ERarity::Legendary, 100.f, 65, true),
			MakeDrop(TEXT("Frozen Maw Heart"), ERarity::Mythic, 20.f, 65, false, false),
		};
		B.RandomDrops = {
			MakeDrop(TEXT("Permafrost Plate"), ERarity::Epic, 38.f, 65, false, true),
			MakeDrop(TEXT("Glacier Ring"), ERarity::Epic, 28.f, 65),
			MakeDrop(TEXT("Frost-Maw Greaves"), ERarity::Rare, 50.f, 65, false, true),
			MakeDrop(TEXT("Nhalor's Last Breath"), ERarity::Legendary, 7.f, 65),
			MakeDrop(TEXT("Frost-Shard"), ERarity::Mythic, 2.f, 65),
		};
		AllBosses.Add(B);
	}
}

// ---------------------------------------------------------------------------
// Zone: Umbryss (shadow / fracture / void) — 2 bosses (zone finale = world finale)
// ---------------------------------------------------------------------------

void UEclipseBossDirector::InitUmbryssBosses()
{
	// Boss 13 — Shade-Mother Nocthyra (Dungeon, 3 phases)
	{
		FBoss B;
		SeedBoss(B, 13, TEXT("Shade-Mother Nocthyra"), TEXT("Weaver of the Final Dark"),
			TEXT("Umbryss"), TEXT("Nocthyra's Loom"), 220000, 110, 260, 68, false,
			TEXT("She weaves shadows into silk; her loom is the last dark before the end of light."));

		AddPhase(B, MakePhase(EBossPhase::Phase1, 100.f, 1.0f, 1.0f, 1.1f,
			TEXT("Opening — Threads of Dark"), { EBossMechanic::TelegraphedLine, EBossMechanic::SpreadMechanic }),
			{
				MakeAttack(TEXT("Silk-Lash"), EBossMechanic::TelegraphedLine, 150, 1.5f, 1.0f, 6, 1000, 0, true, false, false, false, true, EWeaponStatusEffect::Shadow, 6.f),
				MakeAttack(TEXT("Shade-Burst"), EBossMechanic::TelegraphedAoE, 130, 2.0f, 1.5f, 10, 1300, 400, true, false, false, false, true, EWeaponStatusEffect::Shadow, 7.f),
				MakeAttack(TEXT("Loom-Scatter"), EBossMechanic::SpreadMechanic, 100, 1.5f, 1.0f, 8, 1600, 300, true, false, false, false, true, EWeaponStatusEffect::Shadow, 5.f),
			});
		AddPhase(B, MakePhase(EBossPhase::Phase2, 65.f, 1.4f, 1.3f, 1.2f,
			TEXT("Escalation — Weaving Fate"), { EBossMechanic::AddPhase, EBossMechanic::MechanicAssignment, EBossMechanic::TelegraphedCone }),
			{
				MakeAttack(TEXT("Shade-Cocoon"), EBossMechanic::TelegraphedAoE, 240, 3.0f, 2.0f, 13, 1700, 600, true, false, true, false, true, EWeaponStatusEffect::Shadow, 8.f),
				MakeAttack(TEXT("Summon Silk-Shades"), EBossMechanic::AddPhase, 0, 1.0f, 2.0f, 24, 900, 600, true, false, false, false),
				MakeAttack(TEXT("Loom-Threads"), EBossMechanic::MechanicAssignment, 220, 2.5f, 1.5f, 15, 1200, 500, true, false, false, false),
				MakeAttack(TEXT("Web-Cleave"), EBossMechanic::TelegraphedCone, 200, 2.0f, 1.3f, 9, 700, 500, true, true, false, false),
			});
		AddPhase(B, MakePhase(EBossPhase::Phase3, 35.f, 1.8f, 1.5f, 1.3f,
			TEXT("Desperation — The Last Thread"), { EBossMechanic::ArenaChange, EBossMechanic::HealCheck, EBossMechanic::TimeManipulation }),
			{
				MakeAttack(TEXT("Final Weave"), EBossMechanic::ArenaChange, 360, 3.5f, 2.5f, 17, 2200, 1000, true, false, true, false, true, EWeaponStatusEffect::Shadow, 12.f),
				MakeAttack(TEXT("Unravel"), EBossMechanic::TimeManipulation, 0, 2.5f, 2.0f, 30, 2400, 1200, true, false, false, false),
				MakeAttack(TEXT("Shade Devour"), EBossMechanic::HealCheck, 340, 3.0f, 2.0f, 12, 1900, 800, false, false, true, false, true, EWeaponStatusEffect::Shadow, 10.f),
			});

		B.GuaranteedDrops = {
			MakeDrop(TEXT("Nocthyra's Silk-Scythe"), ERarity::Epic, 100.f, 68, true),
			MakeDrop(TEXT("Shade-Woven Vestments"), ERarity::Rare, 100.f, 68, false, true),
		};
		B.RandomDrops = {
			MakeDrop(TEXT("Loomstone Pendant"), ERarity::Rare, 35.f, 68),
			MakeDrop(TEXT("Silk-Charm"), ERarity::Uncommon, 50.f, 68),
			MakeDrop(TEXT("Shade-Mother's Veil"), ERarity::Epic, 15.f, 68, false, true),
			MakeDrop(TEXT("Nocthyra's Last Thread"), ERarity::Legendary, 4.f, 68),
		};
		AllBosses.Add(B);
	}

	// Boss 14 — Umbryss, the Fractured Sovereign (Raid, 4 phases) — WORLD FINALE
	{
		FBoss B;
		SeedBoss(B, 14, TEXT("Umbryss, the Fractured Sovereign"), TEXT("End of All Things"),
			TEXT("Umbryss"), TEXT("Fracture Core"), 800000, 300, 300, 70, true,
			TEXT("The fracture given form. Where Umbryss stands, the world breaks. The final sovereign of a dying age."));
		B.bUsesFractureShift = true;
		B.FracturePhaseTrigger = 3; // fracture shift triggers at phase 3 (50% HP)

		AddPhase(B, MakePhase(EBossPhase::Phase1, 100.f, 1.0f, 1.0f, 1.0f,
			TEXT("Opening — Sovereign's Gaze"), { EBossMechanic::TankSwap, EBossMechanic::TelegraphedLine, EBossMechanic::TelegraphedCone, EBossMechanic::SpreadMechanic }),
			{
				MakeAttack(TEXT("Fracture-Cleave"), EBossMechanic::TelegraphedCone, 300, 2.0f, 1.2f, 6, 800, 600, true, true, false, false, true, EWeaponStatusEffect::Shadow, 8.f),
				MakeAttack(TEXT("Sovereign's Lance"), EBossMechanic::TelegraphedLine, 340, 2.0f, 1.3f, 8, 1200, 0, true, false, false, false, true, EWeaponStatusEffect::Soul, 6.f),
				MakeAttack(TEXT("Void-Scatter"), EBossMechanic::SpreadMechanic, 200, 1.5f, 1.0f, 7, 1800, 300, true, false, false, false, true, EWeaponStatusEffect::Shadow, 6.f),
				MakeAttack(TEXT("Sovereign's Decree"), EBossMechanic::TelegraphedAoE, 260, 2.5f, 1.5f, 11, 1500, 500, true, false, true, false, true, EWeaponStatusEffect::Shadow, 8.f),
			});
		AddPhase(B, MakePhase(EBossPhase::Phase2, 75.f, 1.3f, 1.2f, 1.1f,
			TEXT("Escalation — The World Cracks"), { EBossMechanic::AddPhase, EBossMechanic::MechanicAssignment, EBossMechanic::TimeManipulation, EBossMechanic::TelegraphedAoE }),
			{
				MakeAttack(TEXT("Fracture Cascade"), EBossMechanic::TelegraphedAoE, 420, 3.0f, 2.0f, 14, 2000, 800, true, false, true, false, true, EWeaponStatusEffect::Shadow, 10.f),
				MakeAttack(TEXT("Summon Fracture-Shades"), EBossMechanic::AddPhase, 0, 1.0f, 2.5f, 28, 1100, 600, true, false, false, false),
				MakeAttack(TEXT("World-Platform"), EBossMechanic::MechanicAssignment, 340, 2.5f, 1.5f, 15, 1300, 500, true, false, false, false),
				MakeAttack(TEXT("Time-Fracture"), EBossMechanic::TimeManipulation, 0, 2.5f, 2.0f, 30, 2500, 1200, true, false, false, false),
				MakeAttack(TEXT("Void-Lance Flurry"), EBossMechanic::TelegraphedLine, 360, 1.5f, 1.0f, 5, 1000, 0, true, false, false, false, true, EWeaponStatusEffect::Soul, 6.f),
			});
		AddPhase(B, MakePhase(EBossPhase::Phase3, 50.f, 1.6f, 1.4f, 1.2f,
			TEXT("Desperation — Fracture Shift"), { EBossMechanic::FractureShift, EBossMechanic::ArenaChange, EBossMechanic::SilenceMechanic, EBossMechanic::HealCheck }),
			{
				MakeAttack(TEXT("Reality Fracture"), EBossMechanic::FractureShift, 500, 4.0f, 3.0f, 20, 2800, 1200, true, false, true, false, false, EWeaponStatusEffect::None, 5.f, true),
				MakeAttack(TEXT("World-Collapse"), EBossMechanic::ArenaChange, 460, 3.5f, 2.5f, 18, 2600, 1100, true, false, false, false, true, EWeaponStatusEffect::Shadow, 10.f),
				MakeAttack(TEXT("Silence of the Void"), EBossMechanic::SilenceMechanic, 0, 2.5f, 2.0f, 25, 2500, 1500, true, false, false, false),
				MakeAttack(TEXT("Sovereign's Hunger"), EBossMechanic::HealCheck, 480, 3.0f, 2.0f, 12, 2000, 900, false, false, true, false, true, EWeaponStatusEffect::Soul, 12.f),
			});
		AddPhase(B, MakePhase(EBossPhase::Phase4, 25.f, 2.0f, 1.7f, 1.4f,
			TEXT("Final Stand — End of All Things"), { EBossMechanic::DPSCheck, EBossMechanic::Untargetable, EBossMechanic::FractureShift, EBossMechanic::HealCheck }, 600.f,
			TEXT("Cinematics/Umbryss_FinalStand"), TEXT("Music/Umbryss_Phase4")),
			{
				MakeAttack(TEXT("End of All Things"), EBossMechanic::DPSCheck, 700, 5.0f, 4.0f, 16, 3200, 1500, false, false, true, false, false, EWeaponStatusEffect::None, 5.f, true),
				MakeAttack(TEXT("Final Fracture"), EBossMechanic::FractureShift, 600, 4.0f, 3.0f, 20, 3000, 1400, true, false, true, false, true, EWeaponStatusEffect::Shadow, 12.f),
				MakeAttack(TEXT("Sovereign Beyond"), EBossMechanic::Untargetable, 0, 2.0f, 1.5f, 22, 2200, 900, true, false, false, false),
				MakeAttack(TEXT("Void Devour"), EBossMechanic::HealCheck, 560, 3.0f, 2.0f, 8, 1900, 900, false, false, true, false, true, EWeaponStatusEffect::Soul, 14.f),
				MakeAttack(TEXT("Sovereign Combo"), EBossMechanic::TelegraphedCone, 520, 1.5f, 1.0f, 4, 700, 600, true, true, false, false, true, EWeaponStatusEffect::Shadow, 10.f),
			});

		B.GuaranteedDrops = {
			MakeDrop(TEXT("Crown of the Fractured"), ERarity::Mythic, 100.f, 70, false, true),
			MakeDrop(TEXT("Umbryss, Sovereign's End"), ERarity::Mythic, 100.f, 70, true),
			MakeDrop(TEXT("Fractured World-Core"), ERarity::Eclipse, 15.f, 70, false, false),
		};
		B.RandomDrops = {
			MakeDrop(TEXT("Fracture-Weave Plate"), ERarity::Legendary, 40.f, 70, false, true),
			MakeDrop(TEXT("Sovereign's Sigil"), ERarity::Legendary, 30.f, 70),
			MakeDrop(TEXT("Void-Touched Greaves"), ERarity::Epic, 50.f, 70, false, true),
			MakeDrop(TEXT("End-of-All-Things Shard"), ERarity::Mythic, 5.f, 70),
			MakeDrop(TEXT("Eclipse Core"), ERarity::Eclipse, 1.f, 70),
		};
		AllBosses.Add(B);
	}
}

// ===========================================================================
// Queries
// ===========================================================================

FBoss UEclipseBossDirector::GetBoss(int32 BossID) const
{
	for (const FBoss& B : AllBosses)
	{
		if (B.BossID == BossID) return B;
	}
	return FBoss();
}

TArray<FBoss> UEclipseBossDirector::GetAllBosses() const
{
	return AllBosses;
}

int32 UEclipseBossDirector::GetBossCount() const
{
	return AllBosses.Num();
}

TArray<FBoss> UEclipseBossDirector::GetBossesByZone(const FString& ZoneName) const
{
	TArray<FBoss> Result;
	for (const FBoss& B : AllBosses)
	{
		if (B.ZoneName == ZoneName) Result.Add(B);
	}
	return Result;
}

TArray<FBoss> UEclipseBossDirector::GetRaidBosses() const
{
	TArray<FBoss> Result;
	for (const FBoss& B : AllBosses)
	{
		if (B.bIsRaidBoss) Result.Add(B);
	}
	return Result;
}

// ===========================================================================
// Combat simulation
// ===========================================================================

void UEclipseBossDirector::StartEncounter(int32 BossID, int32 PartySize)
{
	// Find the boss
	bool bFound = false;
	for (const FBoss& B : AllBosses)
	{
		if (B.BossID == BossID)
		{
			ActiveBoss = B;
			bFound = true;
			break;
		}
	}
	if (!bFound) return;

	// Reset combat state
	CurrentHP = ActiveBoss.MaxHealth;
	CurrentPhaseIdx = 0;
	ActiveBoss.bIsEnraged = false;
	ActiveBoss.EnrageTimerRemaining = ActiveBoss.EnrageTimerTotal;
	bBossDead = false;
	bEncounterActive = true;
	ActiveMechanic = EBossMechanic::None;
	AttackTimer = 2.0f; // brief grace before first attack
	MechanicTimer = 0.0f;
	RollNextAttack();
}

void UEclipseBossDirector::UpdateBoss(float DeltaTime, float PlayerHP, float TankHP, float HealerHP)
{
	(void)PlayerHP; (void)TankHP; (void)HealerHP; // reserved for AI targeting decisions

	if (!bEncounterActive || bBossDead) return;

	// --- Enrage timer ---
	if (!ActiveBoss.bIsEnraged)
	{
		ActiveBoss.EnrageTimerRemaining -= DeltaTime;
		if (ActiveBoss.EnrageTimerRemaining <= 0.0f)
		{
			ActiveBoss.EnrageTimerRemaining = 0.0f;
			ActiveBoss.bIsEnraged = true;
			// Enrage: force final phase, max difficulty
			if (ActiveBoss.Phases.Num() > 0)
			{
				CurrentPhaseIdx = ActiveBoss.Phases.Num() - 1;
			}
			ActiveMechanic = EBossMechanic::DPSCheck;
			MechanicTimer = 5.0f;
			RollNextAttack();
			AttackTimer = 1.0f;
		}
	}

	// --- Active-mechanic telegraph window ---
	if (MechanicTimer > 0.0f)
	{
		MechanicTimer -= DeltaTime;
		if (MechanicTimer <= 0.0f)
		{
			ActiveMechanic = EBossMechanic::None;
		}
	}

	// --- Attack timer ---
	AttackTimer -= DeltaTime;
	if (AttackTimer <= 0.0f)
	{
		// Fire the queued attack — telegraph becomes active
		if (!NextAttack.AttackName.IsEmpty())
		{
			ActiveMechanic = NextAttack.TelegraphType;
			MechanicTimer = NextAttack.TelegraphDuration + NextAttack.CastTime;
		}
		// Queue the next attack and set its cooldown
		RollNextAttack();
		float AtkSpdMult = 1.0f;
		if (ActiveBoss.Phases.IsValidIndex(CurrentPhaseIdx))
		{
			AtkSpdMult = ActiveBoss.Phases[CurrentPhaseIdx].AttackSpeedMultiplier;
			if (ActiveBoss.bIsEnraged) AtkSpdMult *= 1.5f; // enrage speeds attacks
		}
		float Cd = (NextAttack.Cooldown > 0.0f) ? NextAttack.Cooldown : 5.0f;
		AttackTimer = Cd / FMath::Max(0.1f, AtkSpdMult);
	}

	CheckPhaseTransition();
}

float UEclipseBossDirector::BossTakeDamage(float Damage, bool bIsCrit, bool bIsMagic)
{
	if (!bEncounterActive || bBossDead) return 0.0f;

	float Mitigation = bIsMagic ? ActiveBoss.Resistance : ActiveBoss.Armor;
	// Diminishing-return mitigation: 1000/(1000+M)
	float Actual = Damage * (1000.0f / (1000.0f + Mitigation));
	if (bIsCrit) Actual *= 1.5f;

	CurrentHP -= Actual;
	if (CurrentHP <= 0.0f)
	{
		CurrentHP = 0.0f;
		bBossDead = true;
		bEncounterActive = false;
		ActiveMechanic = EBossMechanic::None;
	}
	else
	{
		CheckPhaseTransition();
	}
	return Actual;
}

void UEclipseBossDirector::CheckPhaseTransition()
{
	if (bBossDead || ActiveBoss.Phases.Num() == 0) return;
	float HPPct = GetHPPct();

	while (CurrentPhaseIdx < ActiveBoss.Phases.Num() - 1)
	{
		float NextThreshold = ActiveBoss.Phases[CurrentPhaseIdx + 1].HPThreshold;
		if (HPPct <= NextThreshold)
		{
			CurrentPhaseIdx++;
			const FBossPhaseConfig& NewPhase = ActiveBoss.Phases[CurrentPhaseIdx];
			// Activate the phase's signature mechanic
			if (NewPhase.Mechanics.Num() > 0)
			{
				ActiveMechanic = NewPhase.Mechanics[0];
			}
			MechanicTimer = 3.0f; // transition telegraph window
			// Re-roll attack for the new phase
			RollNextAttack();
			float AtkSpdMult = FMath::Max(0.1f, NewPhase.AttackSpeedMultiplier);
			float Cd = (NextAttack.Cooldown > 0.0f) ? NextAttack.Cooldown : 5.0f;
			AttackTimer = Cd / AtkSpdMult;
		}
		else
		{
			break;
		}
	}
}

void UEclipseBossDirector::RollNextAttack()
{
	if (!ActiveBoss.PhaseAttacks.IsValidIndex(CurrentPhaseIdx))
	{
		NextAttack = FBossAttack();
		return;
	}
	const TArray<FBossAttack>& Attacks = ActiveBoss.PhaseAttacks[CurrentPhaseIdx];
	if (Attacks.Num() == 0)
	{
		NextAttack = FBossAttack();
		return;
	}
	int32 Idx = FMath::RandRange(0, Attacks.Num() - 1);
	NextAttack = Attacks[Idx];
}

// ===========================================================================
// Combat queries
// ===========================================================================

EBossPhase UEclipseBossDirector::GetCurrentPhase() const
{
	if (ActiveBoss.bIsEnraged) return EBossPhase::Enraged;
	if (ActiveBoss.Phases.IsValidIndex(CurrentPhaseIdx))
	{
		return ActiveBoss.Phases[CurrentPhaseIdx].Phase;
	}
	return EBossPhase::Phase1;
}

float UEclipseBossDirector::GetHPPct() const
{
	if (ActiveBoss.MaxHealth <= 0.0f) return 0.0f;
	return (CurrentHP / ActiveBoss.MaxHealth) * 100.0f;
}

float UEclipseBossDirector::GetEnrageTimer() const
{
	return ActiveBoss.EnrageTimerRemaining;
}

bool UEclipseBossDirector::IsBossDead() const
{
	return bBossDead;
}

bool UEclipseBossDirector::IsEnraged() const
{
	return ActiveBoss.bIsEnraged;
}

TArray<FBossAttack> UEclipseBossDirector::GetCurrentPhaseAttacks() const
{
	if (ActiveBoss.PhaseAttacks.IsValidIndex(CurrentPhaseIdx))
	{
		return ActiveBoss.PhaseAttacks[CurrentPhaseIdx];
	}
	return TArray<FBossAttack>();
}

EBossMechanic UEclipseBossDirector::GetActiveMechanic() const
{
	return ActiveMechanic;
}

FBossAttack UEclipseBossDirector::GetNextAttack() const
{
	return NextAttack;
}

TArray<FDropEntry> UEclipseBossDirector::GetLoot() const
{
	TArray<FDropEntry> Result;
	// Guaranteed drops always
	for (const FDropEntry& D : ActiveBoss.GuaranteedDrops)
	{
		Result.Add(D);
	}
	// Random drops: roll each against its drop chance
	int32 DropsRolled = 0;
	for (const FDropEntry& D : ActiveBoss.RandomDrops)
	{
		if (DropsRolled >= ActiveBoss.MaxDrops) break;
		if (FMath::FRand() * 100.0f <= D.DropChance)
		{
			Result.Add(D);
			DropsRolled++;
		}
	}
	// Pad up to MinDrops if we rolled too few
	while (Result.Num() < ActiveBoss.MinDrops + (int32)ActiveBoss.GuaranteedDrops.Num()
		&& ActiveBoss.RandomDrops.Num() > 0)
	{
		int32 Idx = FMath::RandRange(0, ActiveBoss.RandomDrops.Num() - 1);
		Result.Add(ActiveBoss.RandomDrops[Idx]);
		if (Result.Num() >= ActiveBoss.MinDrops + ActiveBoss.GuaranteedDrops.Num()) break;
	}
	return Result;
}

void UEclipseBossDirector::ResetBoss()
{
	bEncounterActive = false;
	bBossDead = false;
	CurrentHP = 0.0f;
	CurrentPhaseIdx = 0;
	ActiveMechanic = EBossMechanic::None;
	AttackTimer = 0.0f;
	MechanicTimer = 0.0f;
	NextAttack = FBossAttack();
	ActiveBoss = FBoss();
}

FBoss UEclipseBossDirector::GetActiveBoss() const
{
	return ActiveBoss;
}

bool UEclipseBossDirector::IsEncounterActive() const
{
	return bEncounterActive;
}
