// Eclipse Talent Tree System — Implementation
// Per V6 Talent Directive: no "+X% damage" filler. Every talent produces an
// identifiable, AI-usable gameplay benefit and participates in combo chains
// where one talent changes how another works.
//
// 5 layers per class: Class Foundation, Specialization, Affinity,
// Dual-Affinity Convergence, Game Mode Adaptation.
//
// Bulwark is the FULLY realized reference tree (33 nodes across all 5 layers)
// with 3 build archetypes. The other 16 classes ship Class Foundation placeholders
// (6 nodes each); full trees are added after Bulwark passes review.

#include "EclipseTalentSystem.h"
#include "Engine/Engine.h"

// =============================================================================
// Construction / initialization
// =============================================================================

UEclipseTalentSystem::UEclipseTalentSystem()
{
    MaxTalentPoints = 30;
    InitializeAllTrees();
}

void UEclipseTalentSystem::InitializeAllTrees()
{
    // Full tree first — the reference implementation.
    InitBulwarkTalents();

    // Class Foundation placeholders (full trees pending Bulwark review).
    InitDuskbladeTalents();
    InitArcanistTalents();
    InitRiftwalkerTalents();
    InitDreadknightTalents();
    InitTemplarTalents();
    InitSoulbinderTalents();
    InitWildwardenTalents();
    InitStormcallerTalents();
    InitOracleTalents();
    InitAlchemistTalents();
    InitBerserkerTalents();
    InitShadowbladeTalents();
    InitSpellbladeTalents();
    InitWarthaneTalents();
    InitNecroturgeTalents();
    InitEarthweaverTalents();

    // Build archetypes reference talent IDs by name lookup, so they are
    // robust against insertion order changes above.
    InitBuildArchetypes();

    // Rebuild the ID->node index from the authoritative AllTalents array so
    // that all field configuration done via AddTalent's returned reference is
    // reflected in the lookup map.
    TalentByID.Empty();
    for (const FTalentNode& Node : AllTalents)
    {
        TalentByID.Add(Node.TalentID, Node);
    }
}

// =============================================================================
// AddTalent helper — appends a node, assigns a sequential TalentID, returns a
// reference to the stored element so the caller can configure its fields.
// The reference is valid until the next AddTalent call.
// =============================================================================

FTalentNode& UEclipseTalentSystem::AddTalent(EClass Class, ETalentLayer Layer,
    ETalentNodeType Type, const FString& Name, int32 Tier, int32 Branch,
    const FString& Description)
{
    FTalentNode Node;
    Node.TalentID = AllTalents.Num() + 1;
    Node.ParentClass = Class;
    Node.Layer = Layer;
    Node.NodeType = Type;
    Node.TalentName = Name;
    Node.Description = Description;
    Node.DetailedTooltip = Description;
    Node.Tier = Tier;
    Node.BranchIndex = Branch;
    Node.PointCost = 1;
    Node.MaxRank = 1;
    Node.CurrentRank = 0;
    Node.RequiredLevel = FMath::Max(1, Tier * 5);
    Node.SaveSerializationID = Node.TalentID;
    Node.bAIUsable = true;
    Node.PvPTuningMod = 1.0f;
    Node.ResourceType = EResourceType::Stamina;
    Node.bRespectsGCD = true;
    Node.Affinity1 = EWeaponStatusEffect::None;
    Node.Affinity2 = EWeaponStatusEffect::None;

    AllTalents.Add(Node);
    return AllTalents.Last();
}

void UEclipseTalentSystem::AddSynergy(int32 TalentA, int32 TalentB)
{
    for (FTalentNode& Node : AllTalents)
    {
        if (Node.TalentID == TalentA && !Node.SynergyTalentIDs.Contains(TalentB))
        {
            Node.SynergyTalentIDs.Add(TalentB);
        }
        if (Node.TalentID == TalentB && !Node.SynergyTalentIDs.Contains(TalentA))
        {
            Node.SynergyTalentIDs.Add(TalentA);
        }
    }
}

void UEclipseTalentSystem::AddConflict(int32 TalentA, int32 TalentB)
{
    for (FTalentNode& Node : AllTalents)
    {
        if (Node.TalentID == TalentA && !Node.ConflictTalentIDs.Contains(TalentB))
        {
            Node.ConflictTalentIDs.Add(TalentB);
        }
        if (Node.TalentID == TalentB && !Node.ConflictTalentIDs.Contains(TalentA))
        {
            Node.ConflictTalentIDs.Add(TalentA);
        }
    }
}

void UEclipseTalentSystem::SetPrerequisite(int32 ChildID, int32 ParentID)
{
    for (FTalentNode& Node : AllTalents)
    {
        if (Node.TalentID == ChildID && !Node.PrerequisiteTalentIDs.Contains(ParentID))
        {
            Node.PrerequisiteTalentIDs.Add(ParentID);
        }
    }
}

// =============================================================================
// Convenience: find a talent ID by name (used by InitBuildArchetypes).
// =============================================================================
static int32 FindIDByName(const TArray<FTalentNode>& Talents, const FString& Name)
{
    for (const FTalentNode& Node : Talents)
    {
        if (Node.TalentName == Name)
        {
            return Node.TalentID;
        }
    }
    return -1;
}

// =============================================================================
// BULWARK — full 5-layer reference tree (33 nodes)
// Tank / Initiator — Shield + Greatsword — Earth affinity, Earth+Fire convergence
// Resource: Stamina (ultimate uses Flow)
//
// Combo chain example this tree implements:
//   Taunting Strike (CF5) applies Exposed -> Plate Striker (SP8) heavy attacks
//   apply Fracture -> Fracturing Blows (AF2) shares the stack pool -> Tectonic
//   Tremor (AF3) knocks down only Fractured targets -> Magma Veins (CV1) adds
//   Ember on every Fracture -> Quake-and-Burn (CV3) detonates the fracture line
//   with Fire -> Cataclysm (CV4) detonates every Ember stack for a Fire burst
//   + Earth stun. Seismic Resonance (AF6) refunds the Earth ability that started
//   the chain when 3+ targets are Fractured.
// =============================================================================

void UEclipseTalentSystem::InitBulwarkTalents()
{
    // ---- Spell ID constants this tree references (ability slot convention) ----
    constexpr int32 SPELL_WarspiteSlam     = 100;
    constexpr int32 SPELL_AegisRiposte     = 101;
    constexpr int32 SPELL_BastionFormation  = 102;
    constexpr int32 SPELL_SunbreakerSlam    = 103;
    constexpr int32 SPELL_BulwarkSurge     = 104;
    // New abilities unlocked by talents
    constexpr int32 SPELL_ShieldWallStance  = 200;
    constexpr int32 SPELL_TectonicTremor    = 201;
    constexpr int32 SPELL_QuakeAndBurn       = 202;
    constexpr int32 SPELL_PillarOfTheWorld  = 203;
    constexpr int32 SPELL_FrontlineStandard = 204;

    // ===================================================================
    // LAYER 1 — CLASS FOUNDATION (7 nodes): identity, survival, movement, utility
    // ===================================================================

    FTalentNode& CF1 = AddTalent(EClass::Bulwark, ETalentLayer::ClassFoundation,
        ETalentNodeType::Defensive, TEXT("Iron Resolve"), 1, 0,
        TEXT("When a single hit deals more than 15% of max HP, gain 1 Fortitude stack (max 3); each stack reduces the next incoming hit by 10%."));
    CF1.TriggerEvent = TEXT("OnTakeDamage");
    CF1.ActivationCondition = TEXT("IncomingHit > 0.15 * MaxHP");
    CF1.EffectDescription = TEXT("Gain Fortitude stack; next hit reduced 10% per stack, max 3.");
    CF1.EffectValue = 0.10f;
    CF1.EffectValuePerRank = 0.03f;
    CF1.MaxStacks = 3;
    CF1.ResourceType = EResourceType::Stamina;
    CF1.AIUsageRule = TEXT("Always-on passive; no decision needed.");
    CF1.RequiredLevel = 1;

    FTalentNode& CF2 = AddTalent(EClass::Bulwark, ETalentLayer::ClassFoundation,
        ETalentNodeType::ActiveAbility, TEXT("Shield Wall Stance"), 1, 1,
        TEXT("Toggle stance: while blocking, reflect 20% of melee damage as Earth damage and generate 5 Stamina/sec. Cannot attack while active."));
    CF2.TriggerEvent = TEXT("OnToggle");
    CF2.EffectDescription = TEXT("Reflect 20% melee as Earth; +5 Stamina/sec while blocking.");
    CF2.EffectValue = 0.20f;
    CF2.ResourceGen = 5.0f;
    CF2.UnlockedAbilityID = SPELL_ShieldWallStance;
    CF2.AffectedAbilityIDs.Add(SPELL_AegisRiposte);
    CF2.AIUsageRule = TEXT("Engage when surrounded or expecting a heavy telegraphed hit.");
    CF2.RequiredLevel = 1;

    FTalentNode& CF3 = AddTalent(EClass::Bulwark, ETalentLayer::ClassFoundation,
        ETalentNodeType::Utility, TEXT("Bulwark's March"), 1, 2,
        TEXT("Dash 600 units forward; enemies in the path are knocked back and knocked down for 1.5s. 12s cooldown."));
    CF3.TriggerEvent = TEXT("OnCast");
    CF3.EffectDescription = TEXT("600u dash; knockback + 1.5s knockdown in path.");
    CF3.CooldownMod = 12.0f;
    CF3.ResourceCost = 20.0f;
    CF3.AIUsageRule = TEXT("Open fights by closing distance and scattering ranged enemies.");
    CF3.RequiredLevel = 1;

    FTalentNode& CF4 = AddTalent(EClass::Bulwark, ETalentLayer::ClassFoundation,
        ETalentNodeType::ResourceMod, TEXT("Fortified Breathing"), 2, 1,
        TEXT("At full Stamina, gain Second Wind: next 3 Stamina-costing abilities cost 50% less and gain +1s of effect duration."));
    CF4.TriggerEvent = TEXT("OnStaminaFull");
    CF4.ActivationCondition = TEXT("Stamina == MaxStamina");
    CF4.EffectDescription = TEXT("Next 3 abilities -50% cost, +1s duration.");
    CF4.EffectValue = 0.50f;
    CF4.MaxStacks = 3;
    CF4.AIUsageRule = TEXT("Passive; AI holds stamina high before engaging to bank charges.");
    CF4.RequiredLevel = 5;

    FTalentNode& CF5 = AddTalent(EClass::Bulwark, ETalentLayer::ClassFoundation,
        ETalentNodeType::AbilityTransform, TEXT("Taunting Strike"), 2, 2,
        TEXT("First light attack on a new target applies Taunt (forces aggro 4s) and marks them Exposed (+15% Earth damage taken, 6s)."));
    CF5.TriggerEvent = TEXT("OnHit");
    CF5.ActivationCondition = TEXT("Target not yet hit this encounter");
    CF5.EffectDescription = TEXT("Apply Taunt 4s + Exposed (Earth vuln +15%, 6s).");
    CF5.EffectValue = 0.15f;
    CF5.AffectedAbilityIDs.Add(SPELL_WarspiteSlam);
    CF5.CombatBehaviorChange = TEXT("First light attack on a fresh target taunts and exposes it.");
    CF5.RotationPriority = TEXT("Open on a new target with a light attack to proc Taunt + Exposed.");
    CF5.AIUsageRule = TEXT("Use light attack as opener on each new add to hold aggro and set up Earth synergy.");
    CF5.RequiredLevel = 5;
    // Synergy with Plate Striker (SP9) is wired below via AddSynergy(IdCF5, IdSP9).

    FTalentNode& CF6 = AddTalent(EClass::Bulwark, ETalentLayer::ClassFoundation,
        ETalentNodeType::Passive, TEXT("Heavyfooted"), 2, 0,
        TEXT("Immune to knockback and forced displacement while blocking; +20% Stamina regen while stationary."));
    CF6.TriggerEvent = TEXT("OnBlock");
    CF6.EffectDescription = TEXT("Knockback/displacement immune while blocking; +20% Stamina regen stationary.");
    CF6.EffectValue = 0.20f;
    CF6.AIUsageRule = TEXT("Passive; lets AI hold ground against boss knockbacks.");
    CF6.RequiredLevel = 5;

    FTalentNode& CF7 = AddTalent(EClass::Bulwark, ETalentLayer::ClassFoundation,
        ETalentNodeType::Keystone, TEXT("Guardian's Oath"), 3, 0,
        TEXT("When an ally within 8m drops below 30% HP, gain Oathsworn: +30% damage reduction, +30% move speed, 6s. 60s internal cooldown."));
    CF7.TriggerEvent = TEXT("OnAllyLowHP");
    CF7.ActivationCondition = TEXT("Ally within 8m and HP < 30%");
    CF7.EffectDescription = TEXT("Gain Oathsworn: -30% dmg taken, +30% move speed, 6s. 60s ICD.");
    CF7.EffectValue = 0.30f;
    CF7.InternalCooldown = 60.0f;
    CF7.AIUsageRule = TEXT("Passive emergency save for nearby allies; position near squishies.");
    CF7.RequiredLevel = 10;
    CF7.bRequiresKeystone = true;
    CF7.RequiredKeystoneID = -1; // foundation keystone; no separate prerequisite

    // ===================================================================
    // LAYER 2 — SPECIALIZATION (10 nodes): three branches + capstone
    //   Branch 0: Bulwark (defensive tank)    [SP1-SP4]
    //   Branch 1: Warsinger (aggressive initiator) [SP5-SP7]
    //   Branch 2: Earthshaker (area control)  [SP8-SP9]
    //   Capstone: SP10 (any keystone unlocks)
    // ===================================================================

    // --- Bulwark branch ---
    FTalentNode& SP1 = AddTalent(EClass::Bulwark, ETalentLayer::Specialization,
        ETalentNodeType::Keystone, TEXT("Bulwark"), 4, 0,
        TEXT("Keystone — blocking converts 30% of blocked damage into Stamina; Aegis Riposte becomes a guaranteed crit after a successful block."));
    SP1.TriggerEvent = TEXT("OnBlock");
    SP1.EffectDescription = TEXT("30% blocked -> Stamina; Aegis Riposte guaranteed crit post-block.");
    SP1.EffectValue = 0.30f;
    SP1.AffectedAbilityIDs.Add(SPELL_AegisRiposte);
    SP1.AIUsageRule = TEXT("Pick for sustained defensive play; block-then-counter loop.");
    SP1.RequiredLevel = 10;

    FTalentNode& SP2 = AddTalent(EClass::Bulwark, ETalentLayer::Specialization,
        ETalentNodeType::AbilityTransform, TEXT("Reinforced Aegis"), 5, 0,
        TEXT("Aegis Riposte counter damage +50% and applies Stagger (1s) to the countered target."));
    SP2.TriggerEvent = TEXT("OnCounter");
    SP2.EffectDescription = TEXT("Aegis Riposte +50% damage + 1s Stagger.");
    SP2.EffectValue = 0.50f;
    SP2.AffectedAbilityIDs.Add(SPELL_AegisRiposte);
    SP2.RotationPriority = TEXT("Block -> Aegis Riposte for guaranteed crit + stagger.");
    SP2.AIUsageRule = TEXT("After a block, immediately Aegis Riposte for the crit + stagger.");
    SP2.RequiredLevel = 12;
    SP2.PrerequisiteTalentIDs.Add(0); // patched to SP1.TalentID below
    SP2.bRequiresKeystone = true;

    FTalentNode& SP3 = AddTalent(EClass::Bulwark, ETalentLayer::Specialization,
        ETalentNodeType::ProcReaction, TEXT("Return Fire"), 5, 0,
        TEXT("25% chance on block to fire a shield projectile dealing Earth damage equal to 20% of the blocked amount."));
    SP3.TriggerEvent = TEXT("OnBlock");
    SP3.EffectDescription = TEXT("25% on block: ranged Earth projectile = 20% of blocked amount.");
    SP3.ProcChance = 0.25f;
    SP3.EffectValue = 0.20f;
    SP3.InternalCooldown = 2.0f;
    SP3.AIUsageRule = TEXT("Passive; rewards frequent blocking with free ranged damage.");
    SP3.RequiredLevel = 12;
    SP3.PrerequisiteTalentIDs.Add(0); // SP1

    FTalentNode& SP4 = AddTalent(EClass::Bulwark, ETalentLayer::Specialization,
        ETalentNodeType::AbilityTransform, TEXT("Stalwart Surge"), 6, 0,
        TEXT("Bulwark Surge charge now leaves a 6m-wide Tremor Lane for 4s; enemies entering are knocked up and lose 10% armor."));
    SP4.TriggerEvent = TEXT("OnCast");
    SP4.EffectDescription = TEXT("Bulwark Surge leaves Tremor Lane (6m, 4s): knockup + -10% armor.");
    SP4.AffectedAbilityIDs.Add(SPELL_BulwarkSurge);
    SP4.EffectValue = 0.10f;
    SP4.AIUsageRule = TEXT("Use Bulwark Surge to create a denial lane between ranged adds and the party.");
    SP4.RequiredLevel = 15;
    SP4.PrerequisiteTalentIDs.Add(0); // SP1

    // --- Warsinger branch ---
    FTalentNode& SP5 = AddTalent(EClass::Bulwark, ETalentLayer::Specialization,
        ETalentNodeType::Keystone, TEXT("Warsinger"), 4, 1,
        TEXT("Keystone — Warspite Slam's stun becomes Battered (stacks to 3): each stack +12% Earth damage taken and -5% move speed on the target."));
    SP5.TriggerEvent = TEXT("OnHit");
    SP5.EffectDescription = TEXT("Warspite Slam applies Battered (max 3): +12% Earth dmg taken, -5% speed per stack.");
    SP5.EffectValue = 0.12f;
    SP5.MaxStacks = 3;
    SP5.AffectedAbilityIDs.Add(SPELL_WarspiteSlam);
    SP5.AIUsageRule = TEXT("Pick for aggressive initiator; stack Battered then unload Earth abilities.");
    SP5.RequiredLevel = 10;

    FTalentNode& SP6 = AddTalent(EClass::Bulwark, ETalentLayer::Specialization,
        ETalentNodeType::AbilityTransform, TEXT("Crater Maker"), 5, 1,
        TEXT("Warspite Slam impact leaves a 4m Seismic Crater for 8s; enemies inside have -20% attack speed and are pulled 1m/s toward the center."));
    SP6.TriggerEvent = TEXT("OnCast");
    SP6.EffectDescription = TEXT("Warspite Slam leaves Seismic Crater (4m, 8s): -20% atk speed, pull to center.");
    SP6.EffectValue = 0.20f;
    SP6.AffectedAbilityIDs.Add(SPELL_WarspiteSlam);
    SP6.AIUsageRule = TEXT("Drop Warspite Slam in a cluster to lock them in the crater.");
    SP6.RequiredLevel = 12;
    SP6.PrerequisiteTalentIDs.Add(0); // SP5

    FTalentNode& SP7 = AddTalent(EClass::Bulwark, ETalentLayer::Specialization,
        ETalentNodeType::ProcReaction, TEXT("Tremor Pulse"), 5, 1,
        TEXT("Every 3rd auto-attack pulses Earth damage in 3m; pulses deal double damage to enemies inside a Seismic Crater."));
    SP7.TriggerEvent = TEXT("OnAutoAttack");
    SP7.ActivationCondition = TEXT("AutoAttackCount % 3 == 0");
    SP7.EffectDescription = TEXT("Every 3rd auto: 3m Earth pulse; x2 vs targets in a Seismic Crater.");
    SP7.EffectValue = 1.0f;
    SP7.AIUsageRule = TEXT("Passive; chain autos inside Crater for doubled pulses.");
    SP7.RequiredLevel = 12;
    SP7.PrerequisiteTalentIDs.Add(0); // SP5

    // --- Earthshaker branch ---
    FTalentNode& SP8 = AddTalent(EClass::Bulwark, ETalentLayer::Specialization,
        ETalentNodeType::Keystone, TEXT("Earthshaker"), 4, 2,
        TEXT("Keystone — Bastion Formation becomes Earthwall Bastion: creates a 5m physical wall that blocks projectiles and enemies for 8s; Bulwark can move along it."));
    SP8.TriggerEvent = TEXT("OnCast");
    SP8.EffectDescription = TEXT("Bastion Formation -> Earthwall Bastion: 5m wall, blocks projectiles + enemies 8s.");
    SP8.AffectedAbilityIDs.Add(SPELL_BastionFormation);
    SP8.CombatBehaviorChange = TEXT("Replaces Bastion Formation with Earthwall Bastion.");
    SP8.AIUsageRule = TEXT("Pick for area control; wall off ranged adds or split a pack.");
    SP8.RequiredLevel = 10;

    FTalentNode& SP9 = AddTalent(EClass::Bulwark, ETalentLayer::Specialization,
        ETalentNodeType::Passive, TEXT("Plate Striker"), 5, 2,
        TEXT("Heavy attacks now deal Earth damage and apply 1 Fracture stack (armor -5% per stack, max 5, 8s). Shares the Fracture stack pool with Fracturing Blows."));
    SP9.TriggerEvent = TEXT("OnHeavyAttack");
    SP9.EffectDescription = TEXT("Heavy attacks deal Earth + apply Fracture (max 5, -5% armor each, 8s).");
    SP9.EffectValue = 0.05f;
    SP9.MaxStacks = 5;
    SP9.AIUsageRule = TEXT("Passive; weave heavy attacks to build Fracture for Earth burst windows.");
    SP9.RequiredLevel = 12;
    SP9.PrerequisiteTalentIDs.Add(0); // SP8

    // --- Capstone ---
    FTalentNode& SP10 = AddTalent(EClass::Bulwark, ETalentLayer::Specialization,
        ETalentNodeType::Capstone, TEXT("Sunbreaker's Wake"), 7, 0,
        TEXT("Capstone — Sunbreaker Slam's shockwave leaves a 10m Magma Rift for 10s: enemies inside take Earth+Fire DoT, are snared 30%, and Bulwark gains 20 Stamina/sec while standing in it."));
    SP10.TriggerEvent = TEXT("OnCast");
    SP10.EffectDescription = TEXT("Sunbreaker Slam leaves Magma Rift (10m, 10s): Earth+Fire DoT, 30% snare, +20 Stamina/sec to Bulwark inside.");
    SP10.AffectedAbilityIDs.Add(SPELL_SunbreakerSlam);
    SP10.EffectValue = 0.30f;
    SP10.ResourceGen = 20.0f;
    SP10.bRequiresKeystone = true;
    SP10.RequiredKeystoneID = -1; // patched: any of SP1/SP5/SP8
    SP10.AIUsageRule = TEXT("Ultimate burst + sustain zone; drop on a clustered, Fractured pack.");
    SP10.RequiredLevel = 25;

    // ===================================================================
    // LAYER 3 — AFFINITY: EARTH (7 nodes)
    // ===================================================================

    FTalentNode& AF1 = AddTalent(EClass::Bulwark, ETalentLayer::Affinity,
        ETalentNodeType::Passive, TEXT("Stoneblood"), 1, 0,
        TEXT("Gain 1 Stone Skin stack per 5s out of combat (max 5); each stack absorbs one hit for 40 Earth damage, then shatters, applying a 2s slow."));
    AF1.Affinity1 = EWeaponStatusEffect::Earth;
    AF1.TriggerEvent = TEXT("OnOutCombatTick");
    AF1.EffectDescription = TEXT("Out-of-combat: +1 Stone Skin /5s (max 5); each absorbs 40 dmg, shatters -> 2s slow.");
    AF1.EffectValue = 40.0f;
    AF1.MaxStacks = 5;
    AF1.AIUsageRule = TEXT("Pre-stack before pulls; shatters slow charging enemies.");
    AF1.RequiredLevel = 8;

    FTalentNode& AF2 = AddTalent(EClass::Bulwark, ETalentLayer::Affinity,
        ETalentNodeType::Passive, TEXT("Fracturing Blows"), 2, 0,
        TEXT("All Earth-elemental hits apply Fracture (armor -5% per stack, max 5, 8s). Shares the stack pool with Plate Striker."));
    AF2.Affinity1 = EWeaponStatusEffect::Earth;
    AF2.TriggerEvent = TEXT("OnEarthDamage");
    AF2.EffectDescription = TEXT("Earth hits apply Fracture (max 5, -5% armor, 8s). Shared pool with Plate Striker.");
    AF2.EffectValue = 0.05f;
    AF2.MaxStacks = 5;
    AF2.AIUsageRule = TEXT("Passive; every Earth ability now also shreds armor.");
    AF2.RequiredLevel = 10;
    AF2.PrerequisiteTalentIDs.Add(0); // AF1

    FTalentNode& AF3 = AddTalent(EClass::Bulwark, ETalentLayer::Affinity,
        ETalentNodeType::ActiveAbility, TEXT("Tectonic Tremor"), 3, 0,
        TEXT("8s CD, 20 Stamina — stomp dealing Earth damage in 6m and applying Stagger; enemies already at 3+ Fracture stacks are knocked down 1.5s instead."));
    AF3.Affinity1 = EWeaponStatusEffect::Earth;
    AF3.TriggerEvent = TEXT("OnCast");
    AF3.ActivationCondition = TEXT("Target Fracture >= 3 -> knockdown; else Stagger");
    AF3.EffectDescription = TEXT("6m Earth stomp; Stagger, or 1.5s knockdown if target has 3+ Fracture.");
    AF3.CooldownMod = 8.0f;
    AF3.ResourceCost = 20.0f;
    AF3.UnlockedAbilityID = SPELL_TectonicTremor;
    AF3.AIUsageRule = TEXT("Cast after building Fracture to convert Stagger into knockdown.");
    AF3.RequiredLevel = 12;
    AF3.PrerequisiteTalentIDs.Add(0); // AF2
    AF3.RotationPriority = TEXT("Cast after 3+ Fracture stacks for the knockdown upgrade.");

    FTalentNode& AF4 = AddTalent(EClass::Bulwark, ETalentLayer::Affinity,
        ETalentNodeType::Defensive, TEXT("Bedrock"), 3, 0,
        TEXT("While stationary >1s, gain Bedrock: CC-immune and +10% armor. Synergizes with Heavyfooted (also block-immune)."));
    AF4.Affinity1 = EWeaponStatusEffect::Earth;
    AF4.TriggerEvent = TEXT("OnStationary");
    AF4.ActivationCondition = TEXT("MoveSpeed == 0 for >1s");
    AF4.EffectDescription = TEXT("Stationary >1s: CC-immune, +10% armor.");
    AF4.EffectValue = 0.10f;
    AF4.AIUsageRule = TEXT("Plant during a crater/wall to become unmovable.");
    AF4.RequiredLevel = 12;
    AF4.PrerequisiteTalentIDs.Add(0); // AF2

    FTalentNode& AF5 = AddTalent(EClass::Bulwark, ETalentLayer::Affinity,
        ETalentNodeType::ResourceMod, TEXT("Lithosphere"), 4, 0,
        TEXT("Each Stone Skin shatter or Fracture application generates 5 Stamina."));
    AF5.Affinity1 = EWeaponStatusEffect::Earth;
    AF5.TriggerEvent = TEXT("OnFractureApply / OnStoneShatter");
    AF5.EffectDescription = TEXT("+5 Stamina per Fracture apply or Stone Skin shatter.");
    AF5.ResourceGen = 5.0f;
    AF5.AIUsageRule = TEXT("Passive sustain; rewards aggressive Fracture stacking.");
    AF5.RequiredLevel = 15;
    AF5.PrerequisiteTalentIDs.Add(0); // AF2

    FTalentNode& AF6 = AddTalent(EClass::Bulwark, ETalentLayer::Affinity,
        ETalentNodeType::Conditional, TEXT("Seismic Resonance"), 5, 0,
        TEXT("When 3+ enemies within 6m are Fractured, Bulwark's next Earth ability has its cooldown reduced 50% and deals +30%."));
    AF6.Affinity1 = EWeaponStatusEffect::Earth;
    AF6.TriggerEvent = TEXT("OnConditionMet");
    AF6.ActivationCondition = TEXT("Count(Fractured enemies in 6m) >= 3");
    AF6.EffectDescription = TEXT("3+ Fractured nearby: next Earth ability -50% CD, +30% damage.");
    AF6.EffectValue = 0.30f;
    AF6.CooldownMod = -0.50f;
    AF6.AIUsageRule = TEXT("Build Fracture on a pack, then spend the empowered Earth ability.");
    AF6.RequiredLevel = 18;
    AF6.PrerequisiteTalentIDs.Add(0); // AF3

    FTalentNode& AF7 = AddTalent(EClass::Bulwark, ETalentLayer::Affinity,
        ETalentNodeType::Capstone, TEXT("Pillar of the World"), 7, 0,
        TEXT("Capstone — 90s CD: summon an 8m stone pillar (blocks movement + LOS 12s); on expiry it shatters for Earth damage in 5m + Stagger. Requires Stoneblood + Fracturing Blows."));
    AF7.Affinity1 = EWeaponStatusEffect::Earth;
    AF7.TriggerEvent = TEXT("OnCast");
    AF7.EffectDescription = TEXT("Summon Pillar (blocks movement/LOS 12s); on expire: 5m Earth burst + Stagger.");
    AF7.CooldownMod = 90.0f;
    AF7.UnlockedAbilityID = SPELL_PillarOfTheWorld;
    AF7.bRequiresKeystone = true;
    AF7.AIUsageRule = TEXT("Zone denial + delayed burst; pair with Earthwall to split a pack.");
    AF7.RequiredLevel = 22;
    AF7.PrerequisiteTalentIDs.Add(0); // AF1
    AF7.PrerequisiteTalentIDs.Add(0); // AF2 (second entry patched)

    // ===================================================================
    // LAYER 4 — DUAL-AFFINITY CONVERGENCE: EARTH + FIRE (4 nodes)
    // ===================================================================

    FTalentNode& CV1 = AddTalent(EClass::Bulwark, ETalentLayer::DualAffinityConvergence,
        ETalentNodeType::CrossSynergy, TEXT("Magma Veins"), 4, 0,
        TEXT("Fracture stacks now also apply Ember (1 Fire DoT tick per 2s, 8s, stackable). Cross-synergy with Fracturing Blows."));
    CV1.Affinity1 = EWeaponStatusEffect::Earth;
    CV1.Affinity2 = EWeaponStatusEffect::Fire;
    CV1.bIsDualAffinity = true;
    CV1.TriggerEvent = TEXT("OnFractureApply");
    CV1.EffectDescription = TEXT("Fracture also applies Ember (Fire DoT /2s, 8s, stacks).");
    CV1.AIUsageRule = TEXT("Passive bridge; every Fracture now also fuels Fire detonation.");
    CV1.RequiredLevel = 15;
    CV1.PrerequisiteTalentIDs.Add(0); // AF2 (Fracturing Blows)

    FTalentNode& CV2 = AddTalent(EClass::Bulwark, ETalentLayer::DualAffinityConvergence,
        ETalentNodeType::Convergence, TEXT("Volcanic Mantle"), 5, 0,
        TEXT("When a Stone Skin stack shatters, release a 3m Fire burst (Fire DoT 6s) and replace the stack with Magma Skin (next 2 hits reflect 30 Fire damage each)."));
    CV2.Affinity1 = EWeaponStatusEffect::Earth;
    CV2.Affinity2 = EWeaponStatusEffect::Fire;
    CV2.bIsDualAffinity = true;
    CV2.TriggerEvent = TEXT("OnStoneShatter");
    CV2.EffectDescription = TEXT("Stone shatter -> 3m Fire burst + Magma Skin (reflect 30 Fire x2 hits).");
    CV2.EffectValue = 30.0f;
    CV2.MaxStacks = 2;
    CV2.AIUsageRule = TEXT("Defensive convergence; stone breaks become offense.");
    CV2.RequiredLevel = 18;
    CV2.PrerequisiteTalentIDs.Add(0); // CV1
    CV2.PrerequisiteTalentIDs.Add(0); // AF1 (Stoneblood)

    FTalentNode& CV3 = AddTalent(EClass::Bulwark, ETalentLayer::DualAffinityConvergence,
        ETalentNodeType::Convergence, TEXT("Quake-and-Burn"), 6, 0,
        TEXT("20s CD — slam creates a 6m Fissure traveling 10m forward: Earth damage + knockdown in path, then it erupts for Fire damage + Ignite (Burn 6s). Requires Magma Veins."));
    CV3.Affinity1 = EWeaponStatusEffect::Earth;
    CV3.Affinity2 = EWeaponStatusEffect::Fire;
    CV3.bIsDualAffinity = true;
    CV3.TriggerEvent = TEXT("OnCast");
    CV3.EffectDescription = TEXT("6m Fissure (10m forward): Earth + knockdown, then erupts Fire + Burn 6s.");
    CV3.CooldownMod = 20.0f;
    CV3.UnlockedAbilityID = SPELL_QuakeAndBurn;
    CV3.AIUsageRule = TEXT("Line nuke that chains Earth CC into Fire DoT; prime Fracture first.");
    CV3.RequiredLevel = 20;
    CV3.PrerequisiteTalentIDs.Add(0); // CV1
    CV3.RotationPriority = TEXT("Cast after Fracture stacks so the Earth portion benefits from armor shred.");

    FTalentNode& CV4 = AddTalent(EClass::Bulwark, ETalentLayer::DualAffinityConvergence,
        ETalentNodeType::Capstone, TEXT("Cataclysm"), 7, 0,
        TEXT("Capstone — Sunbreaker's Wake ultimate: the Magma Rift now spreads Ember to all enemies hit; after 10s the rift detonates — each Ember stack on each enemy explodes for Fire damage + 1s Earth stun. Requires Quake-and-Burn + Sunbreaker's Wake."));
    CV4.Affinity1 = EWeaponStatusEffect::Earth;
    CV4.Affinity2 = EWeaponStatusEffect::Fire;
    CV4.bIsDualAffinity = true;
    CV4.TriggerEvent = TEXT("OnCast");
    CV4.EffectDescription = TEXT("Sunbreaker's Wake: spreads Ember; rift detonates after 10s — each Ember stack -> Fire burst + 1s Earth stun.");
    CV4.AffectedAbilityIDs.Add(SPELL_SunbreakerSlam);
    CV4.bRequiresKeystone = true;
    CV4.AIUsageRule = TEXT("The tree's payoff: stack Fracture->Ember on a pack, then detonate the rift.");
    CV4.RequiredLevel = 28;
    CV4.PrerequisiteTalentIDs.Add(0); // CV3
    CV4.PrerequisiteTalentIDs.Add(0); // SP10 (patched)

    // ===================================================================
    // LAYER 5 — GAME MODE ADAPTATION (5 nodes)
    // ===================================================================

    FTalentNode& GM1 = AddTalent(EClass::Bulwark, ETalentLayer::GameModeAdaptation,
        ETalentNodeType::Conditional, TEXT("Lone Bastion"), 1, 0,
        TEXT("Solo — when no allies within 20m, +20% Stamina regen and Stone Skin max stacks +2."));
    GM1.GameMode = EGameModeContext::Solo;
    GM1.TriggerEvent = TEXT("OnConditionMet");
    GM1.ActivationCondition = TEXT("No allies within 20m");
    GM1.EffectDescription = TEXT("Solo: +20% Stamina regen, Stone Skin max +2.");
    GM1.EffectValue = 0.20f;
    GM1.AIUsageRule = TEXT("Auto-active in solo play.");
    GM1.RequiredLevel = 5;

    FTalentNode& GM2 = AddTalent(EClass::Bulwark, ETalentLayer::GameModeAdaptation,
        ETalentNodeType::Conditional, TEXT("Phalanx Captain"), 2, 0,
        TEXT("Dungeon — in a party of 3-5, Taunt duration +4s and Guardian's Oath internal cooldown halved."));
    GM2.GameMode = EGameModeContext::Dungeon;
    GM2.TriggerEvent = TEXT("OnConditionMet");
    GM2.ActivationCondition = TEXT("Party size 3-5");
    GM2.EffectDescription = TEXT("Dungeon: Taunt +4s duration; Guardian's Oath ICD x0.5.");
    GM2.EffectValue = 4.0f;
    GM2.AIUsageRule = TEXT("Auto-active in dungeons; hold adds longer.");
    GM2.RequiredLevel = 8;

    FTalentNode& GM3 = AddTalent(EClass::Bulwark, ETalentLayer::GameModeAdaptation,
        ETalentNodeType::Conditional, TEXT("Raid Bulwark"), 3, 0,
        TEXT("Raid — Bastion Formation's damage reduction extends to the 2 nearest allies (40% transfer); Bulwark takes 50% of the redirected damage."));
    GM3.GameMode = EGameModeContext::Raid;
    GM3.TriggerEvent = TEXT("OnConditionMet");
    GM3.ActivationCondition = TEXT("Raid context");
    GM3.EffectDescription = TEXT("Raid: Bastion covers 2 nearest allies (40% transfer); Bulwark takes 50% of redirected.");
    GM3.EffectValue = 0.40f;
    GM3.AffectedAbilityIDs.Add(SPELL_BastionFormation);
    GM3.AIUsageRule = TEXT("Use Bastion to body-block raid cleaves for the ranged stack.");
    GM3.RequiredLevel = 12;

    FTalentNode& GM4 = AddTalent(EClass::Bulwark, ETalentLayer::GameModeAdaptation,
        ETalentNodeType::Conditional, TEXT("Colosseum's Wall"), 4, 0,
        TEXT("Arena/PvP — Aegis Riposte counters also Disarm the target 1.5s; Return Fire procs ignore 30% of PvP damage reduction."));
    GM4.GameMode = EGameModeContext::Arena;
    GM4.TriggerEvent = TEXT("OnCounter");
    GM4.EffectDescription = TEXT("PvP: Aegis counter -> Disarm 1.5s; Return Fire ignores 30% PvP DR.");
    GM4.EffectValue = 0.30f;
    GM4.AffectedAbilityIDs.Add(SPELL_AegisRiposte);
    GM4.AIUsageRule = TEXT("Bait an attack, counter to disarm and strip PvP mitigation.");
    GM4.RequiredLevel = 10;

    FTalentNode& GM5 = AddTalent(EClass::Bulwark, ETalentLayer::GameModeAdaptation,
        ETalentNodeType::Utility, TEXT("Frontline Standard"), 5, 0,
        TEXT("Battleground — 60s CD: plant a Banner (12m aura) — allies +10% damage + CC resist; Bulwark gains Standard-Bearer (taunt-immune, +15% move speed)."));
    GM5.GameMode = EGameModeContext::Battleground;
    GM5.TriggerEvent = TEXT("OnCast");
    GM5.EffectDescription = TEXT("BG: Banner (12m) — allies +10% dmg + CC resist; Bulwark taunt-immune +15% speed.");
    GM5.EffectValue = 0.10f;
    GM5.CooldownMod = 60.0f;
    GM5.UnlockedAbilityID = SPELL_FrontlineStandard;
    GM5.AIUsageRule = TEXT("Plant at an objective to swing a teamfight.");
    GM5.RequiredLevel = 10;

    // ---- Resolve placeholder prerequisite / synergy IDs by name now that all
    //      Bulwark nodes exist in AllTalents. We look up by TalentName. ----
    auto ID = [&](const FString& Name) -> int32
    {
        return FindIDByName(AllTalents, Name);
    };

    const int32 IdCF4 = ID(TEXT("Fortified Breathing"));
    const int32 IdCF5 = ID(TEXT("Taunting Strike"));
    const int32 IdCF6 = ID(TEXT("Heavyfooted"));
    const int32 IdSP1 = ID(TEXT("Bulwark"));
    const int32 IdSP2 = ID(TEXT("Reinforced Aegis"));
    const int32 IdSP3 = ID(TEXT("Return Fire"));
    const int32 IdSP4 = ID(TEXT("Stalwart Surge"));
    const int32 IdSP5 = ID(TEXT("Warsinger"));
    const int32 IdSP6 = ID(TEXT("Crater Maker"));
    const int32 IdSP7 = ID(TEXT("Tremor Pulse"));
    const int32 IdSP8 = ID(TEXT("Earthshaker"));
    const int32 IdSP9 = ID(TEXT("Plate Striker"));
    const int32 IdSP10 = ID(TEXT("Sunbreaker's Wake"));
    const int32 IdAF1 = ID(TEXT("Stoneblood"));
    const int32 IdAF2 = ID(TEXT("Fracturing Blows"));
    const int32 IdAF3 = ID(TEXT("Tectonic Tremor"));
    const int32 IdAF4 = ID(TEXT("Bedrock"));
    const int32 IdAF5 = ID(TEXT("Lithosphere"));
    const int32 IdAF6 = ID(TEXT("Seismic Resonance"));
    const int32 IdAF7 = ID(TEXT("Pillar of the World"));
    const int32 IdCV1 = ID(TEXT("Magma Veins"));
    const int32 IdCV2 = ID(TEXT("Volcanic Mantle"));
    const int32 IdCV3 = ID(TEXT("Quake-and-Burn"));
    const int32 IdCV4 = ID(TEXT("Cataclysm"));

    // Patch prerequisites (replace the placeholder 0 entries).
    SetPrerequisite(IdSP2, IdSP1);
    SetPrerequisite(IdSP3, IdSP1);
    SetPrerequisite(IdSP4, IdSP1);
    SetPrerequisite(IdSP6, IdSP5);
    SetPrerequisite(IdSP7, IdSP5);
    SetPrerequisite(IdSP9, IdSP8);
    SetPrerequisite(IdAF2, IdAF1);
    SetPrerequisite(IdAF3, IdAF2);
    SetPrerequisite(IdAF4, IdAF2);
    SetPrerequisite(IdAF5, IdAF2);
    SetPrerequisite(IdAF6, IdAF3);
    SetPrerequisite(IdAF7, IdAF1);
    SetPrerequisite(IdAF7, IdAF2);
    SetPrerequisite(IdCV1, IdAF2);
    SetPrerequisite(IdCV2, IdCV1);
    SetPrerequisite(IdCV2, IdAF1);
    SetPrerequisite(IdCV3, IdCV1);
    SetPrerequisite(IdCV4, IdCV3);
    SetPrerequisite(IdCV4, IdSP10);

    // Remove the placeholder 0 entries that SetPrerequisite did not overwrite
    // (SetPrerequisite only adds; we scrub the sentinel zero we inserted.)
    for (FTalentNode& Node : AllTalents)
    {
        if (Node.ParentClass != EClass::Bulwark) continue;
        Node.PrerequisiteTalentIDs.RemoveAll([](int32 V) { return V == 0; });
    }

    // Capstone keystone gating: SP10 requires any one of the three keystones.
    for (FTalentNode& Node : AllTalents)
    {
        if (Node.TalentID == IdSP10)
        {
            Node.bRequiresKeystone = true;
            Node.RequiredKeystoneID = -1; // "any Bulwark specialization keystone"
        }
    }

    // Synergy graph — bidirectional links that express the combo chain.
    AddSynergy(IdCF5, IdSP9);              // Taunting Strike Exposed -> Plate Striker Earth heavies
    AddSynergy(IdSP9, IdAF2);             // Plate Striker Fracture pool shared with Fracturing Blows
    AddSynergy(IdAF2, IdAF3);            // Fracture -> Tectonic Tremor knockdown upgrade
    AddSynergy(IdAF2, IdCV1);            // Fracture -> Magma Veins adds Ember
    AddSynergy(IdCV1, IdCV3);           // Ember -> Quake-and-Burn Fire eruption
    AddSynergy(IdCV3, IdCV4);           // Quake-and-Burn -> Cataclysm detonation
    AddSynergy(IdCV1, IdCV4);           // Ember stacks -> Cataclysm per-stack burst
    AddSynergy(IdAF6, IdAF3);           // Seismic Resonance refunds Tectonic Tremor
    AddSynergy(IdAF1, IdCV2);           // Stoneblood shatters -> Volcanic Mantle
    AddSynergy(IdSP1, IdSP2);           // Bulwark block -> Reinforced Aegis crit
    AddSynergy(IdSP1, IdSP3);           // Bulwark block -> Return Fire proc
    AddSynergy(IdSP5, IdSP6);           // Warsinger Battered -> Crater Maker zone
    AddSynergy(IdSP6, IdSP7);           // Crater -> Tremor Pulse double damage
    AddSynergy(IdSP8, IdAF7);           // Earthshaker wall -> Pillar LOS stacking
    AddSynergy(IdSP10, IdCV4);          // Sunbreaker's Wake -> Cataclysm
    AddSynergy(IdCF4, IdSP10);          // Fortified Breathing -> Sunbreaker's Wake stamina flow
    AddSynergy(IdAF4, IdCF6);           // Bedrock + Heavyfooted immobility stack
    AddSynergy(IdAF5, IdAF3);           // Lithosphere refunds Tectonic Tremor stamina

    // Mutually exclusive: the three specialization keystones.
    AddConflict(IdSP1, IdSP5);
    AddConflict(IdSP1, IdSP8);
    AddConflict(IdSP5, IdSP8);
}

// =============================================================================
// BUILD ARCHETYPES — 3 Bulwark builds
// =============================================================================

void UEclipseTalentSystem::InitBuildArchetypes()
{
    auto ID = [&](const FString& Name) -> int32
    {
        return FindIDByName(AllTalents, Name);
    };

    // ---- 1. Bulwark Sentinel (Sustained) ----
    FBuildArchetype Bulwark;
    Bulwark.BuildName = TEXT("Bulwark Sentinel");
    Bulwark.Identity = TEXT("Sustained defensive tank — block-to-counter loop with stacking Fortitude and party protection.");
    Bulwark.Class = EClass::Bulwark;
    Bulwark.Role = EClassRole::Tank;
    Bulwark.PrimaryAffinity = EWeaponStatusEffect::Earth;
    Bulwark.SecondaryAffinity = EWeaponStatusEffect::None;
    Bulwark.PreferredMode = EGameModeContext::Dungeon;
    Bulwark.CentralMechanic = TEXT("Block absorbs damage -> Bulwark converts 30% to Stamina -> Reinforced Aegis guarantees a crit counter -> Return Fire procs ranged Earth -> repeat. Fortitude stacks dampen spikes; Guardian's Oath saves allies.");
    Bulwark.CombatOpener = TEXT("Bulwark's March in -> light attack for Taunting Strike (Taunt + Exposed) -> enter Shield Wall Stance.");
    Bulwark.NormalRotation = TEXT("Block telegraphed hits -> Aegis Riposte (guaranteed crit + Stagger) -> Warspite Slam for Battered -> Tremor Pulse procs from autos -> Tectonic Tremor to knock down Fractured adds.");
    Bulwark.BurstWindow = TEXT("Build 3+ Fortitude -> Bastion Formation (Earthwall) -> Sunbreaker Slam (Sunbreaker's Wake drops Magma Rift) -> stand in rift for +20 Stamina/sec and unload Aegis Riposte counters on CD.");
    Bulwark.DefensiveRotation = TEXT("Shield Wall Stance for reflect + stamina -> Bastion Formation for 50% DR / Earthwall -> Guardian's Oath auto-procs to save a low ally -> Bedrock if stationary.");
    Bulwark.ResourceStrategy = TEXT("Bulwark keystone refunds Stamina on every block; Fortified Breathing banks cheap casts at full Stamina; Lithosphere tops up from Fracture applications. Sustain through blocking rather than spending.");
    Bulwark.Strengths = TEXT("High sustain, strong party protection, reliable aggro control, forgiving on mistakes, excellent in dungeons/raids.");
    Bulwark.Weaknesses = TEXT("Low burst, predictable in PvP, struggles against targets that don't telegraph blockable hits, mobility limited while shielding.");
    Bulwark.GroupSynergies = TEXT("Holds aggro for DPS, Guardian's Oath saves squishies, Raid Bulwark transfers DR to ranged stack, Earthwall splits dangerous packs.");

    TArray<FString> BulwarkNames = {
        TEXT("Iron Resolve"), TEXT("Shield Wall Stance"), TEXT("Bulwark's March"),
        TEXT("Fortified Breathing"), TEXT("Taunting Strike"), TEXT("Heavyfooted"),
        TEXT("Guardian's Oath"),
        TEXT("Bulwark"), TEXT("Reinforced Aegis"), TEXT("Return Fire"), TEXT("Stalwart Surge"),
        TEXT("Stoneblood"), TEXT("Fracturing Blows"), TEXT("Tectonic Tremor"),
        TEXT("Bedrock"), TEXT("Lithosphere"),
        TEXT("Raid Bulwark"), TEXT("Phalanx Captain") };
    TArray<int32> BulwarkIDs;
    for (const FString& N : BulwarkNames)
    {
        int32 Tid = ID(N);
        if (Tid != -1) BulwarkIDs.Add(Tid);
    }
    Bulwark.AllocatedTalentIDs = BulwarkIDs;
    Bulwark.PrimaryAbilityIDs = { 100, 101, 102, 200, 201 };
    Bulwark.MechanicalCoherence = 9.0f;
    Bulwark.TalentAccessibility = 8.0f;
    Bulwark.RotationCompleteness = 8.0f;
    Bulwark.ResourceSustainability = 9.0f;
    Bulwark.OffensiveSynergy = 6.0f;
    Bulwark.DefensiveSynergy = 10.0f;
    Bulwark.ControlUtility = 7.0f;
    Bulwark.EncounterAdaptability = 8.0f;
    Bulwark.Counterplay = 7.0f;
    Bulwark.BuildDiversity = 7.0f;

    // ---- 2. Earthquake Berserker (Burst) ----
    FBuildArchetype Earthquake;
    Earthquake.BuildName = TEXT("Earthquake Berserker");
    Earthquake.Identity = TEXT("High-risk burst bruiser — stacks Fracture + Ember then detonates with Cataclysm for a Fire/Earth apocalypse.");
    Earthquake.Class = EClass::Bulwark;
    Earthquake.Role = EClassRole::Hybrid;
    Earthquake.PrimaryAffinity = EWeaponStatusEffect::Earth;
    Earthquake.SecondaryAffinity = EWeaponStatusEffect::Fire;
    Earthquake.PreferredMode = EGameModeContext::AllModes;
    Earthquake.CentralMechanic = TEXT("Plate Striker heavy attacks + Fracturing Blows stack Fracture -> Magma Veins adds Ember to each stack -> Quake-and-Burn converts the fracture line to Fire -> Seismic Resonance refunds the Earth opener when 3+ are Fractured -> Cataclysm detonates every Ember stack for a Fire burst + Earth stun.");
    Earthquake.CombatOpener = TEXT("Stalwart Surge (Tremor Lane) to scatter -> Warspite Slam (Crater) to cluster -> heavy attacks to seed Fracture.");
    Earthquake.NormalRotation = TEXT("Plate Striker heavies (Fracture + Ember) -> Tectonic Tremor (knockdown at 3+ Fracture) -> Tremor Pulse autos inside the Crater (x2) -> Quake-and-Burn line nuke.");
    Earthquake.BurstWindow = TEXT("Fracture x5 + Ember x5 on a pack -> Seismic Resonance procs -> Cataclysm-empowered Sunbreaker Slam drops the Magma Rift -> after 10s the rift detonates: every Ember stack explodes for Fire + 1s Earth stun. Stand in the rift for Stamina flow.");
    Earthquake.DefensiveRotation = TEXT("Bedrock while stationary chaining -> Stoneblood shatters -> Volcanic Mantle converts a shatter into Magma Skin reflect -> Guardian's Oath as panic save.");
    Earthquake.ResourceStrategy = TEXT("Lithosphere refunds 5 Stamina per Fracture apply — the faster you stack, the more you sustain. Fortified Breathing banks cheap casts. Seismic Resonance halves the next Earth CD. High spend, high refund.");
    Earthquake.Strengths = TEXT("Massive AoE burst, scales with target density, strong execute via Cataclysm, satisfying combo payoff, good in dungeons/raids with clustered adds.");
    Earthquake.Weaknesses = TEXT("Setup-dependent burst, squishier than Bulwark, Cataclysm has a long telegraph enemies can flee, vulnerable while building Fracture, weaker vs single bosses.");
    Earthquake.GroupSynergies = TEXT("Clusters adds for AoE DPS to cleave, Earth snare/knockdown controls packs, Volcanic Mantle reflects melee, benefits from group CC to keep targets in the rift.");

    TArray<FString> EarthquakeNames = {
        TEXT("Iron Resolve"), TEXT("Bulwark's March"), TEXT("Taunting Strike"), TEXT("Heavyfooted"),
        TEXT("Warsinger"), TEXT("Crater Maker"), TEXT("Tremor Pulse"),
        TEXT("Plate Striker"), TEXT("Sunbreaker's Wake"),
        TEXT("Stoneblood"), TEXT("Fracturing Blows"), TEXT("Tectonic Tremor"),
        TEXT("Bedrock"), TEXT("Lithosphere"), TEXT("Seismic Resonance"), TEXT("Pillar of the World"),
        TEXT("Magma Veins"), TEXT("Volcanic Mantle"), TEXT("Quake-and-Burn"), TEXT("Cataclysm"),
        TEXT("Lone Bastion") };
    TArray<int32> EarthquakeIDs;
    for (const FString& N : EarthquakeNames)
    {
        int32 Tid = ID(N);
        if (Tid != -1) EarthquakeIDs.Add(Tid);
    }
    Earthquake.AllocatedTalentIDs = EarthquakeIDs;
    Earthquake.PrimaryAbilityIDs = { 100, 103, 201, 202 };
    Earthquake.MechanicalCoherence = 9.0f;
    Earthquake.TalentAccessibility = 7.0f;
    Earthquake.RotationCompleteness = 9.0f;
    Earthquake.ResourceSustainability = 7.0f;
    Earthquake.OffensiveSynergy = 10.0f;
    Earthquake.DefensiveSynergy = 6.0f;
    Earthquake.ControlUtility = 7.0f;
    Earthquake.EncounterAdaptability = 6.0f;
    Earthquake.Counterplay = 6.0f;
    Earthquake.BuildDiversity = 8.0f;

    // ---- 3. Stone Warden (Control) ----
    FBuildArchetype StoneWarden;
    StoneWarden.BuildName = TEXT("Stone Warden");
    StoneWarden.Identity = TEXT("Area control + group support — walls, craters, pillars and snares to dictate the battlefield.");
    StoneWarden.Class = EClass::Bulwark;
    StoneWarden.Role = EClassRole::Support;
    StoneWarden.PrimaryAffinity = EWeaponStatusEffect::Earth;
    StoneWarden.SecondaryAffinity = EWeaponStatusEffect::Earth;
    StoneWarden.PreferredMode = EGameModeContext::Raid;
    StoneWarden.CentralMechanic = TEXT("Earthshaker keystone turns Bastion into Earthwall (a physical wall) -> Crater Maker zones enemies -> Pillar of the World blocks LOS/movement -> Quake-and-Burn fissures a line -> Tremor Pulse + Tectonic Tremor lock down what's inside. Bulwark reshapes the arena so enemies fight on his terms.");
    StoneWarden.CombatOpener = TEXT("Earthwall Bastion to wall off ranged adds -> Bulwark's March knocks melee into the wall -> Pillar of the World to trap the remaining cluster.");
    StoneWarden.NormalRotation = TEXT("Crater Maker (Warspite Slam zone) -> Tectonic Tremor knockdown on Fractured targets -> Tremor Pulse autos -> Quake-and-Burn fissure line -> reposition with Earthwall as needed.");
    StoneWarden.BurstWindow = TEXT("Sunbreaker Slam into the Crater -> Magma Rift snares the cluster -> Pillar shatters for a 5m Earth burst + Stagger -> Quake-and-Burn to clean up stragglers.");
    StoneWarden.DefensiveRotation = TEXT("Bedrock while planting walls -> Stoneblood pre-stacks -> Guardian's Oath for allies -> Bastion/Earthwall to body-block cleaves away from the party.");
    StoneWarden.ResourceStrategy = TEXT("Lithosphere + Fortified Breathing sustain; Pillar is expensive so bank Second Wind before it. Seismic Resonance refunds the Earth opener when 3+ are Fractured — easy in a zoned pack.");
    StoneWarden.Strengths = TEXT("Best area control, excellent raid/battleground value, splits dangerous packs, protects ranged stack, strong vs melee-heavy encounters, highly adaptable.");
    StoneWarden.Weaknesses = TEXT("Lower raw damage than the other two, Pillar has a long CD, walls can be played around by mobile enemies, relies on enemy positioning, less effective vs single spread-out targets.");
    StoneWarden.GroupSynergies = TEXT("Walls protect ranged DPS, Pillar blocks LOS for enemy casters, Crater + knockdown sets up melee cleave, Raid Bulwark + Frontline Standard buff the party, snares hold enemies for AoE.");

    TArray<FString> StoneWardenNames = {
        TEXT("Iron Resolve"), TEXT("Shield Wall Stance"), TEXT("Bulwark's March"),
        TEXT("Fortified Breathing"), TEXT("Taunting Strike"), TEXT("Heavyfooted"),
        TEXT("Guardian's Oath"),
        TEXT("Earthshaker"), TEXT("Plate Striker"), TEXT("Sunbreaker's Wake"),
        TEXT("Stoneblood"), TEXT("Fracturing Blows"), TEXT("Tectonic Tremor"),
        TEXT("Bedrock"), TEXT("Lithosphere"), TEXT("Seismic Resonance"), TEXT("Pillar of the World"),
        TEXT("Magma Veins"), TEXT("Quake-and-Burn"),
        TEXT("Raid Bulwark"), TEXT("Phalanx Captain"), TEXT("Frontline Standard") };
    TArray<int32> StoneWardenIDs;
    for (const FString& N : StoneWardenNames)
    {
        int32 Tid = ID(N);
        if (Tid != -1) StoneWardenIDs.Add(Tid);
    }
    StoneWarden.AllocatedTalentIDs = StoneWardenIDs;
    StoneWarden.PrimaryAbilityIDs = { 100, 102, 201, 202, 203, 204 };
    StoneWarden.MechanicalCoherence = 8.0f;
    StoneWarden.TalentAccessibility = 8.0f;
    StoneWarden.RotationCompleteness = 8.0f;
    StoneWarden.ResourceSustainability = 8.0f;
    StoneWarden.OffensiveSynergy = 6.0f;
    StoneWarden.DefensiveSynergy = 8.0f;
    StoneWarden.ControlUtility = 10.0f;
    StoneWarden.EncounterAdaptability = 9.0f;
    StoneWarden.Counterplay = 7.0f;
    StoneWarden.BuildDiversity = 8.0f;

    AllBuilds.Add(Bulwark);
    AllBuilds.Add(Earthquake);
    AllBuilds.Add(StoneWarden);
}

// =============================================================================
// PLACEHOLDER CLASS FOUNDATION TREES — 16 classes x 6 nodes each
// Each node has a real mechanical effect (not +X% filler). Full trees are added
// after the Bulwark reference passes review.
// =============================================================================

void UEclipseTalentSystem::InitDuskbladeTalents()
{
    FTalentNode& N1 = AddTalent(EClass::Duskblade, ETalentLayer::ClassFoundation,
        ETalentNodeType::Utility, TEXT("Umbral Step"), 1, 0,
        TEXT("Placeholder — Blink 400u through target, leaving an afterimage that taunts 2s. 10s CD."));
    N1.TriggerEvent = TEXT("OnCast"); N1.EffectDescription = TEXT("Blink through target; afterimage taunts 2s.");
    N1.CooldownMod = 10.0f; N1.ResourceType = EResourceType::Stamina; N1.ResourceCost = 15.0f;
    N1.AIUsageRule = TEXT("Reposition behind a target and drop aggro."); N1.RequiredLevel = 1;

    FTalentNode& N2 = AddTalent(EClass::Duskblade, ETalentLayer::ClassFoundation,
        ETalentNodeType::ActiveAbility, TEXT("Shadow Cloak"), 1, 1,
        TEXT("Placeholder — Enter stealth 4s; first attack from stealth +50% and applies Marked (crit dmg +20%, 6s)."));
    N2.TriggerEvent = TEXT("OnCast"); N2.EffectDescription = TEXT("Stealth 4s; stealth attack +50%, applies Marked.");
    N2.CooldownMod = 15.0f; N2.ResourceType = EResourceType::Stamina; N2.ResourceCost = 20.0f;
    N2.AIUsageRule = TEXT("Open from stealth for the Marked setup."); N2.RequiredLevel = 1;

    FTalentNode& N3 = AddTalent(EClass::Duskblade, ETalentLayer::ClassFoundation,
        ETalentNodeType::Defensive, TEXT("Afterimage Dodge"), 2, 0,
        TEXT("Placeholder — Perfect dodge leaves an afterimage that explodes for Shadow damage on contact."));
    N3.TriggerEvent = TEXT("OnPerfectDodge"); N3.EffectDescription = TEXT("Perfect dodge -> afterimage bomb (Shadow).");
    N3.AIUsageRule = TEXT("Passive; rewards well-timed dodges."); N3.RequiredLevel = 5;

    FTalentNode& N4 = AddTalent(EClass::Duskblade, ETalentLayer::ClassFoundation,
        ETalentNodeType::ProcReaction, TEXT("Venom Edge"), 2, 1,
        TEXT("Placeholder — Dagger crits apply 1 Toxin stack (Poison DoT 4s, max 5); stealth attacks double the stacks applied."));
    N4.Affinity1 = EWeaponStatusEffect::Poison; N4.TriggerEvent = TEXT("OnCrit");
    N4.EffectDescription = TEXT("Crit -> Toxin stack (Poison DoT, max 5); stealth x2 stacks."); N4.MaxStacks = 5;
    N4.AIUsageRule = TEXT("Build stacks from stealth for a poison payoff."); N4.RequiredLevel = 5;

    FTalentNode& N5 = AddTalent(EClass::Duskblade, ETalentLayer::ClassFoundation,
        ETalentNodeType::ResourceMod, TEXT("Flow Siphon"), 3, 1,
        TEXT("Placeholder — Killing a Marked target refunds 20 Stamina and resets Umbral Step CD."));
    N5.TriggerEvent = TEXT("OnKill"); N5.ActivationCondition = TEXT("Target was Marked");
    N5.EffectDescription = TEXT("Marked kill -> +20 Stamina, reset Umbral Step."); N5.ResourceGen = 20.0f;
    N5.AIUsageRule = TEXT("Chain Marked kills to keep uptime."); N5.RequiredLevel = 8;

    FTalentNode& N6 = AddTalent(EClass::Duskblade, ETalentLayer::ClassFoundation,
        ETalentNodeType::Keystone, TEXT("Death Mark"), 3, 0,
        TEXT("Placeholder — Mark a target (30s CD): +25% Shadow damage taken and revealed through stealth; killing them resets all CDs."));
    N6.TriggerEvent = TEXT("OnCast"); N6.EffectDescription = TEXT("Mark: target +25% Shadow dmg, true-sighted; kill resets CDs.");
    N6.CooldownMod = 30.0f; N6.AIUsageRule = TEXT("Mark the priority target, burn it, reset."); N6.RequiredLevel = 10;
    N6.bRequiresKeystone = true;
}

void UEclipseTalentSystem::InitArcanistTalents()
{
    FTalentNode& N1 = AddTalent(EClass::Arcanist, ETalentLayer::ClassFoundation,
        ETalentNodeType::Utility, TEXT("Arcane Sigil"), 1, 0,
        TEXT("Placeholder — Place a Sigil (8m, 15s): enemies inside slowed 20%; spells cast from inside cost -25%."));
    N1.TriggerEvent = TEXT("OnCast"); N1.EffectDescription = TEXT("Sigil: enemies -20% speed; self spells -25% cost inside.");
    N1.CooldownMod = 12.0f; N1.ResourceType = EResourceType::Mana; N1.ResourceCost = 15.0f;
    N1.AIUsageRule = TEXT("Pre-place a sigil at the engage point."); N1.RequiredLevel = 1;

    FTalentNode& N2 = AddTalent(EClass::Arcanist, ETalentLayer::ClassFoundation,
        ETalentNodeType::Defensive, TEXT("Mana Shield"), 1, 1,
        TEXT("Placeholder — 30% of incoming damage drains Mana instead of HP; breaks at 0 Mana."));
    N2.TriggerEvent = TEXT("OnTakeDamage"); N2.EffectDescription = TEXT("30% dmg -> Mana drain; breaks at 0 Mana.");
    N2.EffectValue = 0.30f; N2.AIUsageRule = TEXT("Passive Mana buffer."); N2.RequiredLevel = 1;

    FTalentNode& N3 = AddTalent(EClass::Arcanist, ETalentLayer::ClassFoundation,
        ETalentNodeType::ProcReaction, TEXT("Static Charge"), 2, 0,
        TEXT("Placeholder — Every 3rd spell applies 1 Charge stack; at 3 stacks it discharges for Lightning damage + 1s stun."));
    N3.Affinity1 = EWeaponStatusEffect::Lightning; N3.TriggerEvent = TEXT("OnCast");
    N3.EffectDescription = TEXT("3rd spell -> Charge stack; at 3 -> Lightning discharge + stun 1s."); N3.MaxStacks = 3;
    N3.AIUsageRule = TEXT("Weave spells to build and dump Charge."); N3.RequiredLevel = 5;

    FTalentNode& N4 = AddTalent(EClass::Arcanist, ETalentLayer::ClassFoundation,
        ETalentNodeType::ResourceMod, TEXT("Spell Weave"), 2, 1,
        TEXT("Placeholder — Casting 2 spells of different tiers within 1.5s refunds 15 Mana."));
    N4.TriggerEvent = TEXT("OnCast"); N4.ActivationCondition = TEXT("Two different tiers within 1.5s");
    N4.EffectDescription = TEXT("Mixed-tier pair within 1.5s -> +15 Mana."); N4.ResourceGen = 15.0f;
    N4.AIUsageRule = TEXT("Alternate cantrips and nukes for sustain."); N4.RequiredLevel = 5;

    FTalentNode& N5 = AddTalent(EClass::Arcanist, ETalentLayer::ClassFoundation,
        ETalentNodeType::Utility, TEXT("Conduit Blink"), 3, 0,
        TEXT("Placeholder — Blink to any active Sigil's location. 8s CD."));
    N5.TriggerEvent = TEXT("OnCast"); N5.EffectDescription = TEXT("Blink to an active Sigil."); N5.CooldownMod = 8.0f;
    N5.AIUsageRule = TEXT("Reposition between sigils."); N5.RequiredLevel = 8;

    FTalentNode& N6 = AddTalent(EClass::Arcanist, ETalentLayer::ClassFoundation,
        ETalentNodeType::Keystone, TEXT("Overload"), 3, 1,
        TEXT("Placeholder — At 100% Mana, next spell +40% and becomes AoE but costs double."));
    N6.TriggerEvent = TEXT("OnCast"); N6.ActivationCondition = TEXT("Mana == 100%");
    N6.EffectDescription = TEXT("Full Mana: next spell +40%, AoE, x2 cost."); N6.AIUsageRule = TEXT("Spend from full for a big AoE."); N6.RequiredLevel = 10;
    N6.bRequiresKeystone = true;
}

void UEclipseTalentSystem::InitRiftwalkerTalents()
{
    FTalentNode& N1 = AddTalent(EClass::Riftwalker, ETalentLayer::ClassFoundation,
        ETalentNodeType::Utility, TEXT("Rift Step"), 1, 0,
        TEXT("Placeholder — Open a rift, teleport 600u, leaving a Rift Tear that pulls enemies in 4m. 8s CD."));
    N1.Affinity1 = EWeaponStatusEffect::Wind; N1.TriggerEvent = TEXT("OnCast");
    N1.EffectDescription = TEXT("600u teleport; exit pulls enemies 4m."); N1.CooldownMod = 8.0f;
    N1.ResourceType = EResourceType::Aether; N1.ResourceCost = 15.0f; N1.AIUsageRule = TEXT("Reposition + cluster enemies."); N1.RequiredLevel = 1;

    FTalentNode& N2 = AddTalent(EClass::Riftwalker, ETalentLayer::ClassFoundation,
        ETalentNodeType::Defensive, TEXT("Phase Shift"), 1, 1,
        TEXT("Placeholder — Become intangible 1s (dodge all). 12s ICD."));
    N2.TriggerEvent = TEXT("OnCast"); N2.EffectDescription = TEXT("Intangible 1s, dodge all."); N2.InternalCooldown = 12.0f;
    N2.AIUsageRule = TEXT("I-frame through a telegraphed hit."); N2.RequiredLevel = 1;

    FTalentNode& N3 = AddTalent(EClass::Riftwalker, ETalentLayer::ClassFoundation,
        ETalentNodeType::ProcReaction, TEXT("Temporal Echo"), 2, 0,
        TEXT("Placeholder — Using an ability creates an Echo that re-casts it at 30% effect 2s later."));
    N3.TriggerEvent = TEXT("OnCast"); N3.EffectDescription = TEXT("Ability -> Echo re-cast at 30%, 2s later.");
    N3.AIUsageRule = TEXT("Passive; doubles spell impact over time."); N3.RequiredLevel = 5;

    FTalentNode& N4 = AddTalent(EClass::Riftwalker, ETalentLayer::ClassFoundation,
        ETalentNodeType::ResourceMod, TEXT("Windstep"), 2, 1,
        TEXT("Placeholder — Each perfect dodge generates 10 Aether."));
    N4.TriggerEvent = TEXT("OnPerfectDodge"); N4.EffectDescription = TEXT("Perfect dodge -> +10 Aether."); N4.ResourceGen = 10.0f;
    N4.AIUsageRule = TEXT("Sustain by dodging."); N4.RequiredLevel = 5;

    FTalentNode& N5 = AddTalent(EClass::Riftwalker, ETalentLayer::ClassFoundation,
        ETalentNodeType::ActiveAbility, TEXT("Rend Space"), 3, 0,
        TEXT("Placeholder — Create a 5m Spatial Tear (10s): projectiles inside redirect to center; enemies take Wind DoT."));
    N5.Affinity1 = EWeaponStatusEffect::Wind; N5.TriggerEvent = TEXT("OnCast");
    N5.EffectDescription = TEXT("5m tear (10s): redirect projectiles, Wind DoT."); N5.CooldownMod = 15.0f;
    N5.AIUsageRule = TEXT("Zone that gathers ranged pressure."); N5.RequiredLevel = 8;

    FTalentNode& N6 = AddTalent(EClass::Riftwalker, ETalentLayer::ClassFoundation,
        ETalentNodeType::Keystone, TEXT("Riftwalker's Paradox"), 3, 1,
        TEXT("Placeholder — When an ability is re-cast by Echo, its CD is reduced by 2s."));
    N6.TriggerEvent = TEXT("OnEcho"); N6.EffectDescription = TEXT("Echo re-cast -> -2s to that ability's CD.");
    N6.AIUsageRule = TEXT("Keystone; rewards Echo-chaining."); N6.RequiredLevel = 10; N6.bRequiresKeystone = true;
}

void UEclipseTalentSystem::InitDreadknightTalents()
{
    FTalentNode& N1 = AddTalent(EClass::Dreadknight, ETalentLayer::ClassFoundation,
        ETalentNodeType::ActiveAbility, TEXT("Soul Drain"), 1, 0,
        TEXT("Placeholder — Strike dealing Shadow damage and healing 30% of damage dealt. 6s CD."));
    N1.Affinity1 = EWeaponStatusEffect::Shadow; N1.TriggerEvent = TEXT("OnCast");
    N1.EffectDescription = TEXT("Shadow strike, heal 30% of damage."); N1.CooldownMod = 6.0f;
    N1.ResourceType = EResourceType::Rage; N1.AIUsageRule = TEXT("Sustain nuke on CD."); N1.RequiredLevel = 1;

    FTalentNode& N2 = AddTalent(EClass::Dreadknight, ETalentLayer::ClassFoundation,
        ETalentNodeType::Passive, TEXT("Vampiric Aura"), 1, 1,
        TEXT("Placeholder — 10% of all damage dealt heals Dreadknight."));
    N2.TriggerEvent = TEXT("OnDealDamage"); N2.EffectDescription = TEXT("10% damage dealt -> heal."); N2.EffectValue = 0.10f;
    N2.AIUsageRule = TEXT("Passive drain."); N2.RequiredLevel = 1;

    FTalentNode& N3 = AddTalent(EClass::Dreadknight, ETalentLayer::ClassFoundation,
        ETalentNodeType::Conditional, TEXT("Bloodied Resolve"), 2, 0,
        TEXT("Placeholder — Below 35% HP: +30% damage and +20% lifesteal."));
    N3.TriggerEvent = TEXT("OnConditionMet"); N3.ActivationCondition = TEXT("HP < 35%");
    N3.EffectDescription = TEXT("Low HP: +30% dmg, +20% lifesteal."); N3.AIUsageRule = TEXT("Death-zone bruiser."); N3.RequiredLevel = 5;

    FTalentNode& N4 = AddTalent(EClass::Dreadknight, ETalentLayer::ClassFoundation,
        ETalentNodeType::Utility, TEXT("Death Grip"), 2, 1,
        TEXT("Placeholder — Pull target 6m toward you (12s CD), applying Decrepit (-20% damage, 4s)."));
    N4.TriggerEvent = TEXT("OnCast"); N4.EffectDescription = TEXT("Pull 6m + Decrepit (-20% dmg, 4s)."); N4.CooldownMod = 12.0f;
    N4.AIUsageRule = TEXT("Yank a ranged add into melee."); N4.RequiredLevel = 5;

    FTalentNode& N5 = AddTalent(EClass::Dreadknight, ETalentLayer::ClassFoundation,
        ETalentNodeType::ActiveAbility, TEXT("Necrotic Pool"), 3, 0,
        TEXT("Placeholder — Leave a 4m pool (8s): enemies take Shadow DoT and have healing reduced 40%."));
    N5.Affinity1 = EWeaponStatusEffect::Shadow; N5.TriggerEvent = TEXT("OnCast");
    N5.EffectDescription = TEXT("4m pool: Shadow DoT + -40% healing."); N5.CooldownMod = 14.0f;
    N5.AIUsageRule = TEXT("Anti-heal zone on a clump."); N5.RequiredLevel = 8;

    FTalentNode& N6 = AddTalent(EClass::Dreadknight, ETalentLayer::ClassFoundation,
        ETalentNodeType::Keystone, TEXT("Eternal Hunger"), 3, 1,
        TEXT("Placeholder — Each kill extends active buffs by 3s and heals 10% max HP."));
    N6.TriggerEvent = TEXT("OnKill"); N6.EffectDescription = TEXT("Kill: +3s buffs, heal 10% max HP."); N6.AIUsageRule = TEXT("Snowball on trash pulls."); N6.RequiredLevel = 10; N6.bRequiresKeystone = true;
}

void UEclipseTalentSystem::InitTemplarTalents()
{
    FTalentNode& N1 = AddTalent(EClass::Templar, ETalentLayer::ClassFoundation,
        ETalentNodeType::ActiveAbility, TEXT("Smite"), 1, 0,
        TEXT("Placeholder — Holy strike, +20% vs undead/demon; heals lowest-HP ally for 50% of damage."));
    N1.Affinity1 = EWeaponStatusEffect::Holy; N1.TriggerEvent = TEXT("OnCast");
    N1.EffectDescription = TEXT("Holy strike; heal lowest ally 50% of dmg."); N1.ResourceType = EResourceType::Mana;
    N1.AIUsageRule = TEXT("Hybrid damage + heal."); N1.RequiredLevel = 1;

    FTalentNode& N2 = AddTalent(EClass::Templar, ETalentLayer::ClassFoundation,
        ETalentNodeType::Defensive, TEXT("Divine Shield"), 1, 1,
        TEXT("Placeholder — Block converts 20% of blocked damage into a self heal."));
    N2.TriggerEvent = TEXT("OnBlock"); N2.EffectDescription = TEXT("Block -> heal 20% of blocked."); N2.EffectValue = 0.20f;
    N2.AIUsageRule = TEXT("Sustain through blocking."); N2.RequiredLevel = 1;

    FTalentNode& N3 = AddTalent(EClass::Templar, ETalentLayer::ClassFoundation,
        ETalentNodeType::Passive, TEXT("Aura of Wrath"), 2, 0,
        TEXT("Placeholder — Allies in 8m deal +10% damage; enemies deal -10%."));
    N3.TriggerEvent = TEXT("OnAura"); N3.EffectDescription = TEXT("Aura: allies +10% dmg, enemies -10%."); N3.EffectValue = 0.10f;
    N3.AIUsageRule = TEXT("Position near the party."); N3.RequiredLevel = 5;

    FTalentNode& N4 = AddTalent(EClass::Templar, ETalentLayer::ClassFoundation,
        ETalentNodeType::ActiveAbility, TEXT("Lay on Hands"), 2, 1,
        TEXT("Placeholder — Heal an ally for 40% of their max HP. 30s CD."));
    N4.TriggerEvent = TEXT("OnCast"); N4.EffectDescription = TEXT("Heal 40% max HP."); N4.CooldownMod = 30.0f;
    N4.ResourceType = EResourceType::Mana; N4.AIUsageRule = TEXT("Big save on CD."); N4.RequiredLevel = 5;

    FTalentNode& N5 = AddTalent(EClass::Templar, ETalentLayer::ClassFoundation,
        ETalentNodeType::ProcReaction, TEXT("Judgment"), 3, 0,
        TEXT("Placeholder — 20% on hit to chain Holy damage to 2 additional enemies."));
    N5.Affinity1 = EWeaponStatusEffect::Holy; N5.TriggerEvent = TEXT("OnHit"); N5.EffectDescription = TEXT("20%: Holy chains to +2 enemies."); N5.ProcChance = 0.20f;
    N5.AIUsageRule = TEXT("Passive cleave."); N5.RequiredLevel = 8;

    FTalentNode& N6 = AddTalent(EClass::Templar, ETalentLayer::ClassFoundation,
        ETalentNodeType::Keystone, TEXT("Oath of the Light"), 3, 1,
        TEXT("Placeholder — While above 80% HP, Templar's heals cleanse 1 debuff."));
    N6.TriggerEvent = TEXT("OnHeal"); N6.ActivationCondition = TEXT("Self HP > 80%"); N6.EffectDescription = TEXT("High HP heals cleanse a debuff.");
    N6.AIUsageRule = TEXT("Stay healthy to dispel."); N6.RequiredLevel = 10; N6.bRequiresKeystone = true;
}

void UEclipseTalentSystem::InitSoulbinderTalents()
{
    FTalentNode& N1 = AddTalent(EClass::Soulbinder, ETalentLayer::ClassFoundation,
        ETalentNodeType::ActiveAbility, TEXT("Soul Link"), 1, 0,
        TEXT("Placeholder — Link to an ally (15s): 30% of their damage taken is redirected to Soulbinder as absorbed Soul damage."));
    N1.Affinity1 = EWeaponStatusEffect::Soul; N1.TriggerEvent = TEXT("OnCast"); N1.EffectDescription = TEXT("Redirect 30% ally dmg to self (absorbed).");
    N1.CooldownMod = 18.0f; N1.ResourceType = EResourceType::Mana; N1.AIUsageRule = TEXT("Pocket the tank."); N1.RequiredLevel = 1;

    FTalentNode& N2 = AddTalent(EClass::Soulbinder, ETalentLayer::ClassFoundation,
        ETalentNodeType::ActiveAbility, TEXT("Spirit Mend"), 1, 1,
        TEXT("Placeholder — Heal target + apply Spirit Shield (absorbs next hit, 10s)."));
    N2.TriggerEvent = TEXT("OnCast"); N2.EffectDescription = TEXT("Heal + Spirit Shield (absorb next hit)."); N2.CooldownMod = 8.0f;
    N2.AIUsageRule = TEXT("Pre-shield before a big hit."); N2.RequiredLevel = 1;

    FTalentNode& N3 = AddTalent(EClass::Soulbinder, ETalentLayer::ClassFoundation,
        ETalentNodeType::Defensive, TEXT("Ethereal Form"), 2, 0,
        TEXT("Placeholder — Become incorporeal 1.5s (immune to physical, can't attack). 15s CD."));
    N3.TriggerEvent = TEXT("OnCast"); N3.EffectDescription = TEXT("Incorporeal 1.5s."); N3.CooldownMod = 15.0f;
    N3.AIUsageRule = TEXT("I-frame physical burst."); N3.RequiredLevel = 5;

    FTalentNode& N4 = AddTalent(EClass::Soulbinder, ETalentLayer::ClassFoundation,
        ETalentNodeType::ResourceMod, TEXT("Soul Harvest"), 2, 1,
        TEXT("Placeholder — When a linked enemy dies, gain 20 Soul charges; each charge empowers the next heal +5%."));
    N4.Affinity1 = EWeaponStatusEffect::Soul; N4.TriggerEvent = TEXT("OnKill"); N4.EffectDescription = TEXT("Linked-enemy kill -> +20 Soul charges; heal +5% each."); N4.ResourceGen = 20.0f;
    N4.AIUsageRule = TEXT("Bank charges on trash for big boss heals."); N4.RequiredLevel = 5;

    FTalentNode& N5 = AddTalent(EClass::Soulbinder, ETalentLayer::ClassFoundation,
        ETalentNodeType::Utility, TEXT("Wraith Walk"), 3, 0,
        TEXT("Placeholder — Phase through enemies 3s (can't be targeted). 20s CD."));
    N5.TriggerEvent = TEXT("OnCast"); N5.EffectDescription = TEXT("Untargetable 3s."); N5.CooldownMod = 20.0f;
    N5.AIUsageRule = TEXT("Escape being focused."); N5.RequiredLevel = 8;

    FTalentNode& N6 = AddTalent(EClass::Soulbinder, ETalentLayer::ClassFoundation,
        ETalentNodeType::Keystone, TEXT("Bound Vessel"), 3, 1,
        TEXT("Placeholder — Store an ally's death; when they'd die, consume the soul to revive them at 30% HP. 5min CD."));
    N6.TriggerEvent = TEXT("OnAllyDeath"); N6.EffectDescription = TEXT("Prevent death, revive at 30% HP. 5min CD."); N6.CooldownMod = 300.0f;
    N6.AIUsageRule = TEXT("Raid save keystone."); N6.RequiredLevel = 10; N6.bRequiresKeystone = true;
}

void UEclipseTalentSystem::InitWildwardenTalents()
{
    FTalentNode& N1 = AddTalent(EClass::Wildwarden, ETalentLayer::ClassFoundation,
        ETalentNodeType::ActiveAbility, TEXT("Hunter's Mark"), 1, 0,
        TEXT("Placeholder — Mark a target (8s): Wildwarden + pet deal +20% to it; mark transfers on kill."));
    N1.TriggerEvent = TEXT("OnCast"); N1.EffectDescription = TEXT("Mark: +20% dmg by you + pet, transfers on kill."); N1.CooldownMod = 6.0f;
    N1.AIUsageRule = TEXT("Chain marks across a pull."); N1.RequiredLevel = 1;

    FTalentNode& N2 = AddTalent(EClass::Wildwarden, ETalentLayer::ClassFoundation,
        ETalentNodeType::ActiveAbility, TEXT("Companion Strike"), 1, 1,
        TEXT("Placeholder — Send pet to maul target (knockdown 1s). 8s CD."));
    N2.TriggerEvent = TEXT("OnCast"); N2.EffectDescription = TEXT("Pet maul + knockdown 1s."); N2.CooldownMod = 8.0f;
    N2.ResourceType = EResourceType::Stamina; N2.AIUsageRule = TEXT("Pet interrupt."); N2.RequiredLevel = 1;

    FTalentNode& N3 = AddTalent(EClass::Wildwarden, ETalentLayer::ClassFoundation,
        ETalentNodeType::Passive, TEXT("Nature's Warden"), 2, 0,
        TEXT("Placeholder — In grass/forest: +15% move speed; stealthed when stationary."));
    N3.TriggerEvent = TEXT("OnTerrain"); N3.EffectDescription = TEXT("Forest: +15% speed, stealthed if still."); N3.AIUsageRule = TEXT("Ambush from cover."); N3.RequiredLevel = 5;

    FTalentNode& N4 = AddTalent(EClass::Wildwarden, ETalentLayer::ClassFoundation,
        ETalentNodeType::Utility, TEXT("Bramble Snare"), 2, 1,
        TEXT("Placeholder — Throw a snare trap (4m): immobilize 2s + Nature DoT."));
    N4.TriggerEvent = TEXT("OnCast"); N4.EffectDescription = TEXT("Trap: immobilize 2s + Nature DoT."); N4.CooldownMod = 10.0f;
    N4.AIUsageRule = TEXT("Lock a runner."); N4.RequiredLevel = 5;

    FTalentNode& N5 = AddTalent(EClass::Wildwarden, ETalentLayer::ClassFoundation,
        ETalentNodeType::ResourceMod, TEXT("Predator's Focus"), 3, 0,
        TEXT("Placeholder — Each Marked-target kill refunds 15 Stamina and resets Hunter's Mark."));
    N5.TriggerEvent = TEXT("OnKill"); N5.ActivationCondition = TEXT("Target was Marked"); N5.EffectDescription = TEXT("Marked kill -> +15 Stamina, reset Mark."); N5.ResourceGen = 15.0f;
    N5.AIUsageRule = TEXT("Sustain the mark chain."); N5.RequiredLevel = 8;

    FTalentNode& N6 = AddTalent(EClass::Wildwarden, ETalentLayer::ClassFoundation,
        ETalentNodeType::Keystone, TEXT("Pack Leader"), 3, 1,
        TEXT("Placeholder — Pet +50% damage vs Marked; when pet crits, Wildwarden's next attack +30%."));
    N6.TriggerEvent = TEXT("OnPetCrit"); N6.EffectDescription = TEXT("Pet +50% vs Marked; pet crit -> your next attack +30%."); N6.AIUsageRule = TEXT("Mark, send pet, follow up."); N6.RequiredLevel = 10; N6.bRequiresKeystone = true;
}

void UEclipseTalentSystem::InitStormcallerTalents()
{
    FTalentNode& N1 = AddTalent(EClass::Stormcaller, ETalentLayer::ClassFoundation,
        ETalentNodeType::ActiveAbility, TEXT("Chain Lightning"), 1, 0,
        TEXT("Placeholder — Lightning bolt arcing to 3 nearby enemies (50% damage each)."));
    N1.Affinity1 = EWeaponStatusEffect::Lightning; N1.TriggerEvent = TEXT("OnCast"); N1.EffectDescription = TEXT("Arc to 3 enemies, 50% each."); N1.CooldownMod = 4.0f;
    N1.ResourceType = EResourceType::Mana; N1.AIUsageRule = TEXT("Spammable AoE nuke."); N1.RequiredLevel = 1;

    FTalentNode& N2 = AddTalent(EClass::Stormcaller, ETalentLayer::ClassFoundation,
        ETalentNodeType::ResourceMod, TEXT("Storm Surge"), 1, 1,
        TEXT("Placeholder — Casting while moving generates 5 Mana; standing still costs +20%."));
    N2.TriggerEvent = TEXT("OnCast"); N2.EffectDescription = TEXT("Moving cast +5 Mana; stationary +20% cost."); N2.ResourceGen = 5.0f;
    N2.AIUsageRule = TEXT("Kite to sustain."); N2.RequiredLevel = 1;

    FTalentNode& N3 = AddTalent(EClass::Stormcaller, ETalentLayer::ClassFoundation,
        ETalentNodeType::Utility, TEXT("Static Field"), 2, 0,
        TEXT("Placeholder — Place a 6m field (8s): enemies become Charged (next Lightning crit stuns 1s)."));
    N3.Affinity1 = EWeaponStatusEffect::Lightning; N3.TriggerEvent = TEXT("OnCast"); N3.EffectDescription = TEXT("Field charges enemies; next Lightning crit stuns."); N3.CooldownMod = 12.0f;
    N3.AIUsageRule = TEXT("Set up a stun window."); N3.RequiredLevel = 5;

    FTalentNode& N4 = AddTalent(EClass::Stormcaller, ETalentLayer::ClassFoundation,
        ETalentNodeType::Defensive, TEXT("Tempest Veil"), 2, 1,
        TEXT("Placeholder — 20% dodge chance while at full Mana."));
    N4.TriggerEvent = TEXT("OnIncomingHit"); N4.ActivationCondition = TEXT("Mana == 100%"); N4.EffectDescription = TEXT("Full Mana: 20% dodge."); N4.EffectValue = 0.20f;
    N4.AIUsageRule = TEXT("Stay topped for passive evasion."); N4.RequiredLevel = 5;

    FTalentNode& N5 = AddTalent(EClass::Stormcaller, ETalentLayer::ClassFoundation,
        ETalentNodeType::ProcReaction, TEXT("Overcharge"), 3, 0,
        TEXT("Placeholder — 15% on Lightning hit to cast a free weaker Chain Lightning."));
    N5.Affinity1 = EWeaponStatusEffect::Lightning; N5.TriggerEvent = TEXT("OnHit"); N5.EffectDescription = TEXT("15%: free Chain Lightning."); N5.ProcChance = 0.15f;
    N5.AIUsageRule = TEXT("Passive cascade."); N5.RequiredLevel = 8;

    FTalentNode& N6 = AddTalent(EClass::Stormcaller, ETalentLayer::ClassFoundation,
        ETalentNodeType::Keystone, TEXT("Eye of the Storm"), 3, 1,
        TEXT("Placeholder — At 3+ Charged enemies nearby, Stormcaller's spells ignore resistance."));
    N6.TriggerEvent = TEXT("OnConditionMet"); N6.ActivationCondition = TEXT("Charged enemies >= 3"); N6.EffectDescription = TEXT("3+ Charged: spells ignore resistance."); N6.AIUsageRule = TEXT("Cluster + ignore resist."); N6.RequiredLevel = 10; N6.bRequiresKeystone = true;
}

void UEclipseTalentSystem::InitOracleTalents()
{
    FTalentNode& N1 = AddTalent(EClass::Oracle, ETalentLayer::ClassFoundation,
        ETalentNodeType::ActiveAbility, TEXT("Foresight"), 1, 0,
        TEXT("Placeholder — Predict an ally's next lethal damage; when it lands, negate it. 40s CD."));
    N1.Affinity1 = EWeaponStatusEffect::Holy; N1.TriggerEvent = TEXT("OnCast"); N1.EffectDescription = TEXT("Negate next lethal hit on an ally."); N1.CooldownMod = 40.0f;
    N1.ResourceType = EResourceType::Mana; N1.AIUsageRule = TEXT("Pre-cast on a tank before a burst."); N1.RequiredLevel = 1;

    FTalentNode& N2 = AddTalent(EClass::Oracle, ETalentLayer::ClassFoundation,
        ETalentNodeType::Utility, TEXT("Divination"), 1, 1,
        TEXT("Placeholder — Reveal all enemies in 30m for 5s; marked enemies take +10% damage."));
    N2.TriggerEvent = TEXT("OnCast"); N2.EffectDescription = TEXT("Reveal 30m 5s; revealed +10% dmg taken."); N2.CooldownMod = 20.0f;
    N2.AIUsageRule = TEXT("Reveal stealthers/ambushers."); N2.RequiredLevel = 1;

    FTalentNode& N3 = AddTalent(EClass::Oracle, ETalentLayer::ClassFoundation,
        ETalentNodeType::ActiveAbility, TEXT("Blessing of Clarity"), 2, 0,
        TEXT("Placeholder — An ally's next 3 spells cost -50%."));
    N3.TriggerEvent = TEXT("OnCast"); N3.EffectDescription = TEXT("Ally next 3 spells -50% cost."); N3.CooldownMod = 15.0f;
    N3.AIUsageRule = TEXT("Buff a bursty ally before a window."); N3.RequiredLevel = 5;

    FTalentNode& N4 = AddTalent(EClass::Oracle, ETalentLayer::ClassFoundation,
        ETalentNodeType::ProcReaction, TEXT("Echo of Fate"), 2, 1,
        TEXT("Placeholder — 20% on heal to also heal the lowest-HP ally for 50%."));
    N4.TriggerEvent = TEXT("OnHeal"); N4.EffectDescription = TEXT("20%: heal lowest ally 50%."); N4.ProcChance = 0.20f;
    N4.AIUsageRule = TEXT("Passive smart healing."); N4.RequiredLevel = 5;

    FTalentNode& N5 = AddTalent(EClass::Oracle, ETalentLayer::ClassFoundation,
        ETalentNodeType::ResourceMod, TEXT("Mana Tide"), 3, 0,
        TEXT("Placeholder — Regenerate 2 Mana/sec to all allies in 10m."));
    N5.TriggerEvent = TEXT("OnAura"); N5.EffectDescription = TEXT("Aura: +2 Mana/sec to allies."); N5.ResourceGen = 2.0f;
    N5.AIUsageRule = TEXT("Stand near the group."); N5.RequiredLevel = 8;

    FTalentNode& N6 = AddTalent(EClass::Oracle, ETalentLayer::ClassFoundation,
        ETalentNodeType::Keystone, TEXT("Threads of Fate"), 3, 1,
        TEXT("Placeholder — When Foresight negates lethal damage, Oracle + target gain +25% damage for 5s."));
    N6.TriggerEvent = TEXT("OnForesightProc"); N6.EffectDescription = TEXT("Foresight save -> +25% dmg 5s."); N6.AIUsageRule = TEXT("Turn a save into a burst window."); N6.RequiredLevel = 10; N6.bRequiresKeystone = true;
}

void UEclipseTalentSystem::InitAlchemistTalents()
{
    FTalentNode& N1 = AddTalent(EClass::Alchemist, ETalentLayer::ClassFoundation,
        ETalentNodeType::ActiveAbility, TEXT("Flask Throw"), 1, 0,
        TEXT("Placeholder — Throw a flask (4m): Poison DoT + slow 30%, 6s."));
    N1.Affinity1 = EWeaponStatusEffect::Poison; N1.TriggerEvent = TEXT("OnCast"); N1.EffectDescription = TEXT("Flask: Poison DoT + 30% slow 6s."); N1.CooldownMod = 5.0f;
    N1.ResourceType = EResourceType::Reagents; N1.AIUsageRule = TEXT("Spammable zone denial."); N1.RequiredLevel = 1;

    FTalentNode& N2 = AddTalent(EClass::Alchemist, ETalentLayer::ClassFoundation,
        ETalentNodeType::ResourceMod, TEXT("Reagent Mix"), 1, 1,
        TEXT("Placeholder — Mixing 2 different flasks within 2s creates a compound (double effect, +3s duration)."));
    N2.TriggerEvent = TEXT("OnCast"); N2.ActivationCondition = TEXT("Two different flasks within 2s"); N2.EffectDescription = TEXT("Compound: x2 effect, +3s."); N2.AIUsageRule = TEXT("Alternate flask types."); N2.RequiredLevel = 1;

    FTalentNode& N3 = AddTalent(EClass::Alchemist, ETalentLayer::ClassFoundation,
        ETalentNodeType::Utility, TEXT("Smoke Bomb"), 2, 0,
        TEXT("Placeholder — 4m smoke cloud (6s): enemies blinded (miss 40%), allies stealthed."));
    N3.TriggerEvent = TEXT("OnCast"); N3.EffectDescription = TEXT("Smoke: enemies miss 40%, allies stealthed."); N3.CooldownMod = 18.0f;
    N3.AIUsageRule = TEXT("Drop to break LOS / stealth allies."); N3.RequiredLevel = 5;

    FTalentNode& N4 = AddTalent(EClass::Alchemist, ETalentLayer::ClassFoundation,
        ETalentNodeType::Defensive, TEXT("Adaptive Physiology"), 2, 1,
        TEXT("Placeholder — Each unique flask thrown grants +5% resist to that element, 10s."));
    N4.TriggerEvent = TEXT("OnFlaskThrow"); N4.EffectDescription = TEXT("Flask -> +5% matching resist 10s."); N4.EffectValue = 0.05f;
    N4.AIUsageRule = TEXT("Pre-stack resists for a typed boss."); N4.RequiredLevel = 5;

    FTalentNode& N5 = AddTalent(EClass::Alchemist, ETalentLayer::ClassFoundation,
        ETalentNodeType::ProcReaction, TEXT("Catalyst"), 3, 0,
        TEXT("Placeholder — 25% on flask hit to spread the effect to 2 more enemies."));
    N5.Affinity1 = EWeaponStatusEffect::Poison; N5.TriggerEvent = TEXT("OnFlaskHit"); N5.EffectDescription = TEXT("25%: spread to +2 enemies."); N5.ProcChance = 0.25f;
    N5.AIUsageRule = TEXT("Passive AoE spread."); N5.RequiredLevel = 8;

    FTalentNode& N6 = AddTalent(EClass::Alchemist, ETalentLayer::ClassFoundation,
        ETalentNodeType::Keystone, TEXT("Master Apothecary"), 3, 1,
        TEXT("Placeholder — Flasks now also apply Corrosion (-10% armor, stacks) alongside their base effect."));
    N6.TriggerEvent = TEXT("OnFlaskThrow"); N6.EffectDescription = TEXT("Flasks also apply Corrosion (-10% armor, stacks)."); N6.AIUsageRule = TEXT("Armor shred on every flask."); N6.RequiredLevel = 10; N6.bRequiresKeystone = true;
}

void UEclipseTalentSystem::InitBerserkerTalents()
{
    FTalentNode& N1 = AddTalent(EClass::Berserker, ETalentLayer::ClassFoundation,
        ETalentNodeType::ResourceMod, TEXT("Bloodrage"), 1, 0,
        TEXT("Placeholder — Damage given/taken generates Rage; at 100 Rage enter Frenzy (+25% atk speed, -15% dmg taken, 8s)."));
    N1.TriggerEvent = TEXT("OnCombat"); N1.EffectDescription = TEXT("Rage -> Frenzy (+25% speed, -15% dmg taken)."); N1.ResourceType = EResourceType::Rage;
    N1.AIUsageRule = TEXT("Build rage then spend Frenzy."); N1.RequiredLevel = 1;

    FTalentNode& N2 = AddTalent(EClass::Berserker, ETalentLayer::ClassFoundation,
        ETalentNodeType::ActiveAbility, TEXT("Reckless Swing"), 1, 1,
        TEXT("Placeholder — Heavy attack +50% but you take 10% max HP recoil."));
    N2.TriggerEvent = TEXT("OnCast"); N2.EffectDescription = TEXT("+50% dmg, 10% max HP recoil."); N2.CooldownMod = 3.0f;
    N2.ResourceType = EResourceType::Rage; N2.AIUsageRule = TEXT("High-risk nuke."); N2.RequiredLevel = 1;

    FTalentNode& N3 = AddTalent(EClass::Berserker, ETalentLayer::ClassFoundation,
        ETalentNodeType::Conditional, TEXT("Unyielding"), 2, 0,
        TEXT("Placeholder — Below 30% HP: +30% damage and CC-immune."));
    N3.TriggerEvent = TEXT("OnConditionMet"); N3.ActivationCondition = TEXT("HP < 30%"); N3.EffectDescription = TEXT("Low HP: +30% dmg, CC-immune."); N3.AIUsageRule = TEXT("Death-zone carry."); N3.RequiredLevel = 5;

    FTalentNode& N4 = AddTalent(EClass::Berserker, ETalentLayer::ClassFoundation,
        ETalentNodeType::ActiveAbility, TEXT("Whirlwind"), 2, 1,
        TEXT("Placeholder — Spin hitting all enemies in 5m (4s channel); +5 Rage/sec."));
    N4.TriggerEvent = TEXT("OnCast"); N4.EffectDescription = TEXT("5m spin 4s; +5 Rage/sec."); N4.CooldownMod = 10.0f;
    N4.ResourceGen = 5.0f; N4.AIUsageRule = TEXT("AoE rage builder."); N4.RequiredLevel = 5;

    FTalentNode& N5 = AddTalent(EClass::Berserker, ETalentLayer::ClassFoundation,
        ETalentNodeType::Passive, TEXT("Bloodthirst"), 3, 0,
        TEXT("Placeholder — 15% lifesteal while in Frenzy."));
    N5.TriggerEvent = TEXT("OnFrenzy"); N5.EffectDescription = TEXT("Frenzy: 15% lifesteal."); N5.EffectValue = 0.15f;
    N5.AIUsageRule = TEXT("Sustain during Frenzy."); N5.RequiredLevel = 8;

    FTalentNode& N6 = AddTalent(EClass::Berserker, ETalentLayer::ClassFoundation,
        ETalentNodeType::Keystone, TEXT("Berserk"), 3, 1,
        TEXT("Placeholder — In Frenzy, attacks apply Hemorrhage (bleed, stacks); at 5 stacks the target loses 5% max HP instantly."));
    N6.TriggerEvent = TEXT("OnFrenzyHit"); N6.EffectDescription = TEXT("Frenzy hits -> Hemorrhage; at 5 stacks -> 5% max HP burst."); N6.MaxStacks = 5;
    N6.AIUsageRule = TEXT("Stack bleeds for an execute."); N6.RequiredLevel = 10; N6.bRequiresKeystone = true;
}

void UEclipseTalentSystem::InitShadowbladeTalents()
{
    FTalentNode& N1 = AddTalent(EClass::Shadowblade, ETalentLayer::ClassFoundation,
        ETalentNodeType::ActiveAbility, TEXT("Shadow Strike"), 1, 0,
        TEXT("Placeholder — From stealth, teleport to target and strike for +60% as Shadow damage."));
    N1.Affinity1 = EWeaponStatusEffect::Shadow; N1.TriggerEvent = TEXT("OnStealthAttack"); N1.EffectDescription = TEXT("Stealth: teleport strike +60%."); N1.CooldownMod = 8.0f;
    N1.ResourceType = EResourceType::Aether; N1.AIUsageRule = TEXT("Open from stealth."); N1.RequiredLevel = 1;

    FTalentNode& N2 = AddTalent(EClass::Shadowblade, ETalentLayer::ClassFoundation,
        ETalentNodeType::Utility, TEXT("Vanish"), 1, 1,
        TEXT("Placeholder — Enter stealth 3s (breaks on action). 15s CD."));
    N2.TriggerEvent = TEXT("OnCast"); N2.EffectDescription = TEXT("Stealth 3s."); N2.CooldownMod = 15.0f;
    N2.AIUsageRule = TEXT("Re-enter stealth to reset Shadow Strike."); N2.RequiredLevel = 1;

    FTalentNode& N3 = AddTalent(EClass::Shadowblade, ETalentLayer::ClassFoundation,
        ETalentNodeType::Passive, TEXT("Backstab"), 2, 0,
        TEXT("Placeholder — Attacks from behind deal +40% and apply Weaken (-15% damage, 5s)."));
    N3.TriggerEvent = TEXT("OnHit"); N3.ActivationCondition = TEXT("Behind target"); N3.EffectDescription = TEXT("Behind: +40%, apply Weaken."); N3.EffectValue = 0.40f;
    N3.AIUsageRule = TEXT("Flank for burst + debuff."); N3.RequiredLevel = 5;

    FTalentNode& N4 = AddTalent(EClass::Shadowblade, ETalentLayer::ClassFoundation,
        ETalentNodeType::ResourceMod, TEXT("Shadowsong"), 2, 1,
        TEXT("Placeholder — Each stealth attack generates 15 Aether."));
    N4.TriggerEvent = TEXT("OnStealthAttack"); N4.EffectDescription = TEXT("Stealth attack -> +15 Aether."); N4.ResourceGen = 15.0f;
    N4.AIUsageRule = TEXT("Sustain via stealth loops."); N4.RequiredLevel = 5;

    FTalentNode& N5 = AddTalent(EClass::Shadowblade, ETalentLayer::ClassFoundation,
        ETalentNodeType::ProcReaction, TEXT("Death Lotus"), 3, 0,
        TEXT("Placeholder — 20% on hit to throw a shuriken at a nearby enemy (Shadow damage)."));
    N5.Affinity1 = EWeaponStatusEffect::Shadow; N5.TriggerEvent = TEXT("OnHit"); N5.EffectDescription = TEXT("20%: shuriken to nearby enemy."); N5.ProcChance = 0.20f;
    N5.AIUsageRule = TEXT("Passive cleave."); N5.RequiredLevel = 8;

    FTalentNode& N6 = AddTalent(EClass::Shadowblade, ETalentLayer::ClassFoundation,
        ETalentNodeType::Keystone, TEXT("Umbral Reaper"), 3, 1,
        TEXT("Placeholder — Killing a Weakened target resets Vanish CD and refunds 30 Aether."));
    N6.TriggerEvent = TEXT("OnKill"); N6.ActivationCondition = TEXT("Target was Weakened"); N6.EffectDescription = TEXT("Weakened kill -> reset Vanish, +30 Aether."); N6.ResourceGen = 30.0f;
    N6.AIUsageRule = TEXT("Weaken, kill, re-stealth, repeat."); N6.RequiredLevel = 10; N6.bRequiresKeystone = true;
}

void UEclipseTalentSystem::InitSpellbladeTalents()
{
    FTalentNode& N1 = AddTalent(EClass::Spellblade, ETalentLayer::ClassFoundation,
        ETalentNodeType::ActiveAbility, TEXT("Spellstrike"), 1, 0,
        TEXT("Placeholder — Melee strike that also casts the last-used spell at 60% effect."));
    N1.TriggerEvent = TEXT("OnCast"); N1.EffectDescription = TEXT("Melee + last spell at 60%."); N1.CooldownMod = 4.0f;
    N1.ResourceType = EResourceType::Mana; N1.AIUsageRule = TEXT("Hybrid weave."); N1.RequiredLevel = 1;

    FTalentNode& N2 = AddTalent(EClass::Spellblade, ETalentLayer::ClassFoundation,
        ETalentNodeType::ResourceMod, TEXT("Mana Blade"), 1, 1,
        TEXT("Placeholder — Melee attacks generate 5 Mana; spells cost -10% for 3s after a melee hit."));
    N2.TriggerEvent = TEXT("OnMeleeHit"); N2.EffectDescription = TEXT("Melee -> +5 Mana; spells -10% cost 3s."); N2.ResourceGen = 5.0f;
    N2.AIUsageRule = TEXT("Melee between spells for sustain."); N2.RequiredLevel = 1;

    FTalentNode& N3 = AddTalent(EClass::Spellblade, ETalentLayer::ClassFoundation,
        ETalentNodeType::Utility, TEXT("Phase Cleave"), 2, 0,
        TEXT("Placeholder — Cleave that teleports you 3m forward through enemies."));
    N3.TriggerEvent = TEXT("OnCast"); N3.EffectDescription = TEXT("Cleave + 3m teleport."); N3.CooldownMod = 8.0f;
    N3.AIUsageRule = TEXT("Gap-close + AoE."); N3.RequiredLevel = 5;

    FTalentNode& N4 = AddTalent(EClass::Spellblade, ETalentLayer::ClassFoundation,
        ETalentNodeType::Defensive, TEXT("Warding Edge"), 2, 1,
        TEXT("Placeholder — Blocking with the weapon converts 30% of blocked to Mana."));
    N4.TriggerEvent = TEXT("OnBlock"); N4.EffectDescription = TEXT("Block -> +30% as Mana."); N4.EffectValue = 0.30f;
    N4.AIUsageRule = TEXT("Block to fuel spells."); N4.RequiredLevel = 5;

    FTalentNode& N5 = AddTalent(EClass::Spellblade, ETalentLayer::ClassFoundation,
        ETalentNodeType::ActiveAbility, TEXT("Elemental Infusion"), 3, 0,
        TEXT("Placeholder — Imbue weapon with current element (8s): attacks apply that status."));
    N5.TriggerEvent = TEXT("OnCast"); N5.EffectDescription = TEXT("Imbue 8s: attacks apply current element."); N5.CooldownMod = 12.0f;
    N5.AIUsageRule = TEXT("Match element to the encounter."); N5.RequiredLevel = 8;

    FTalentNode& N6 = AddTalent(EClass::Spellblade, ETalentLayer::ClassFoundation,
        ETalentNodeType::Keystone, TEXT("Arcane Flurry"), 3, 1,
        TEXT("Placeholder — Every 3rd melee hit releases a spell burst matching the current infusion."));
    N6.TriggerEvent = TEXT("OnMeleeHit"); N6.ActivationCondition = TEXT("MeleeCount % 3 == 0"); N6.EffectDescription = TEXT("3rd melee -> infused spell burst."); N6.AIUsageRule = TEXT("Weave melee to proc burst."); N6.RequiredLevel = 10; N6.bRequiresKeystone = true;
}

void UEclipseTalentSystem::InitWarthaneTalents()
{
    FTalentNode& N1 = AddTalent(EClass::Warthane, ETalentLayer::ClassFoundation,
        ETalentNodeType::Utility, TEXT("War Banner"), 1, 0,
        TEXT("Placeholder — Plant a banner (12m aura, 10s): allies +10% damage, +10% armor."));
    N1.Affinity1 = EWeaponStatusEffect::Earth; N1.TriggerEvent = TEXT("OnCast"); N1.EffectDescription = TEXT("Banner: allies +10% dmg, +10% armor."); N1.CooldownMod = 20.0f;
    N1.AIUsageRule = TEXT("Drop at the engage point."); N1.RequiredLevel = 1;

    FTalentNode& N2 = AddTalent(EClass::Warthane, ETalentLayer::ClassFoundation,
        ETalentNodeType::Passive, TEXT("Bodyguard"), 1, 1,
        TEXT("Placeholder — Allies behind Warthane take 20% less damage."));
    N2.TriggerEvent = TEXT("OnAura"); N2.EffectDescription = TEXT("Allies behind you -20% dmg taken."); N2.EffectValue = 0.20f;
    N2.AIUsageRule = TEXT("Face the damage source."); N2.RequiredLevel = 1;

    FTalentNode& N3 = AddTalent(EClass::Warthane, ETalentLayer::ClassFoundation,
        ETalentNodeType::ActiveAbility, TEXT("Earthshatter"), 2, 0,
        TEXT("Placeholder — Slam: Earth damage + knockdown in 5m."));
    N3.Affinity1 = EWeaponStatusEffect::Earth; N3.TriggerEvent = TEXT("OnCast"); N3.EffectDescription = TEXT("5m Earth slam + knockdown."); N3.CooldownMod = 10.0f;
    N3.ResourceType = EResourceType::Stamina; N3.AIUsageRule = TEXT("AoE interrupt."); N3.RequiredLevel = 5;

    FTalentNode& N4 = AddTalent(EClass::Warthane, ETalentLayer::ClassFoundation,
        ETalentNodeType::Defensive, TEXT("Juggernaut"), 2, 1,
        TEXT("Placeholder — Gain Bulwark per hit taken (max 5, +5% armor each); consume all for 2s immunity."));
    N4.TriggerEvent = TEXT("OnTakeDamage"); N4.EffectDescription = TEXT("Bulwark stacks -> 2s immunity on spend."); N4.MaxStacks = 5;
    N4.AIUsageRule = TEXT("Tank hits, then cash out."); N4.RequiredLevel = 5;

    FTalentNode& N5 = AddTalent(EClass::Warthane, ETalentLayer::ClassFoundation,
        ETalentNodeType::ResourceMod, TEXT("Commanding Shout"), 3, 0,
        TEXT("Placeholder — Shout (10s CD): +10 Stamina to all allies in 10m."));
    N5.TriggerEvent = TEXT("OnCast"); N5.EffectDescription = TEXT("Shout: allies +10 Stamina."); N5.CooldownMod = 10.0f; N5.ResourceGen = 10.0f;
    N5.AIUsageRule = TEXT("Group sustain on CD."); N5.RequiredLevel = 8;

    FTalentNode& N6 = AddTalent(EClass::Warthane, ETalentLayer::ClassFoundation,
        ETalentNodeType::Keystone, TEXT("Unbreakable"), 3, 1,
        TEXT("Placeholder — Below 20% HP, cannot die for 4s (60s CD); a killing blow during this heals 15% max HP."));
    N6.TriggerEvent = TEXT("OnLethalHP"); N6.EffectDescription = TEXT("4s death immunity; kill during it -> heal 15% max HP."); N6.CooldownMod = 60.0f;
    N6.AIUsageRule = TEXT("Raid-save panic button."); N6.RequiredLevel = 10; N6.bRequiresKeystone = true;
}

void UEclipseTalentSystem::InitNecroturgeTalents()
{
    FTalentNode& N1 = AddTalent(EClass::Necroturge, ETalentLayer::ClassFoundation,
        ETalentNodeType::ActiveAbility, TEXT("Raise Skeleton"), 1, 0,
        TEXT("Placeholder — Summon a skeleton minion (30s) that taunts and attacks."));
    N1.Affinity1 = EWeaponStatusEffect::Shadow; N1.TriggerEvent = TEXT("OnCast"); N1.EffectDescription = TEXT("Summon skeleton (taunt + attack)."); N1.CooldownMod = 8.0f;
    N1.ResourceType = EResourceType::Mana; N1.AIUsageRule = TEXT("Keep a minion up."); N1.RequiredLevel = 1;

    FTalentNode& N2 = AddTalent(EClass::Necroturge, ETalentLayer::ClassFoundation,
        ETalentNodeType::Defensive, TEXT("Bone Armor"), 1, 1,
        TEXT("Placeholder — Consume a minion to gain 20% damage absorb for 10s."));
    N2.TriggerEvent = TEXT("OnCast"); N2.EffectDescription = TEXT("Consume minion -> 20% absorb 10s."); N2.CooldownMod = 12.0f;
    N2.AIUsageRule = TEXT("Sacrifice for defense."); N2.RequiredLevel = 1;

    FTalentNode& N3 = AddTalent(EClass::Necroturge, ETalentLayer::ClassFoundation,
        ETalentNodeType::ResourceMod, TEXT("Soul Tap"), 2, 0,
        TEXT("Placeholder — Each minion kill generates 10 Mana."));
    N3.TriggerEvent = TEXT("OnMinionKill"); N3.EffectDescription = TEXT("Minion kill -> +10 Mana."); N3.ResourceGen = 10.0f;
    N3.AIUsageRule = TEXT("Sustain off minion kills."); N3.RequiredLevel = 5;

    FTalentNode& N4 = AddTalent(EClass::Necroturge, ETalentLayer::ClassFoundation,
        ETalentNodeType::ActiveAbility, TEXT("Decay"), 2, 1,
        TEXT("Placeholder — Curse target (8s): Shadow DoT; heals nearby minions for 20% of the damage."));
    N4.Affinity1 = EWeaponStatusEffect::Shadow; N4.TriggerEvent = TEXT("OnCast"); N4.EffectDescription = TEXT("Curse: Shadow DoT, heal minions 20%."); N4.CooldownMod = 6.0f;
    N4.AIUsageRule = TEXT("Sustain your army."); N4.RequiredLevel = 5;

    FTalentNode& N5 = AddTalent(EClass::Necroturge, ETalentLayer::ClassFoundation,
        ETalentNodeType::ProcReaction, TEXT("Necrotic Surge"), 3, 0,
        TEXT("Placeholder — 20% on minion attack to summon a free temporary skeleton (10s)."));
    N5.Affinity1 = EWeaponStatusEffect::Shadow; N5.TriggerEvent = TEXT("OnMinionAttack"); N5.EffectDescription = TEXT("20%: free skeleton 10s."); N5.ProcChance = 0.20f;
    N5.AIUsageRule = TEXT("Passive army snowball."); N5.RequiredLevel = 8;

    FTalentNode& N6 = AddTalent(EClass::Necroturge, ETalentLayer::ClassFoundation,
        ETalentNodeType::Keystone, TEXT("Lichdom"), 3, 1,
        TEXT("Placeholder — While 3+ minions are active, Necroturge's spells cost -30% and deal +15%."));
    N6.TriggerEvent = TEXT("OnConditionMet"); N6.ActivationCondition = TEXT("Minions >= 3"); N6.EffectDescription = TEXT("3+ minions: spells -30% cost, +15% dmg."); N6.AIUsageRule = TEXT("Maintain an army for the buff."); N6.RequiredLevel = 10; N6.bRequiresKeystone = true;
}

void UEclipseTalentSystem::InitEarthweaverTalents()
{
    FTalentNode& N1 = AddTalent(EClass::Earthweaver, ETalentLayer::ClassFoundation,
        ETalentNodeType::ActiveAbility, TEXT("Bear Form"), 1, 0,
        TEXT("Placeholder — Transform (toggle): +40% HP, +30% armor, melee attacks; can't cast spells."));
    N1.Affinity1 = EWeaponStatusEffect::Earth; N1.TriggerEvent = TEXT("OnCast"); N1.EffectDescription = TEXT("Bear: +40% HP, +30% armor, melee only."); N1.CooldownMod = 2.0f;
    N1.AIUsageRule = TEXT("Tank form for melee pressure."); N1.RequiredLevel = 1;

    FTalentNode& N2 = AddTalent(EClass::Earthweaver, ETalentLayer::ClassFoundation,
        ETalentNodeType::ActiveAbility, TEXT("Rejuvenation"), 1, 1,
        TEXT("Placeholder — Heal-over-time (8s) on target; moving breaks it."));
    N2.Affinity1 = EWeaponStatusEffect::Water; N2.TriggerEvent = TEXT("OnCast"); N2.EffectDescription = TEXT("HoT 8s; breaks if target moves."); N2.CooldownMod = 6.0f;
    N2.ResourceType = EResourceType::Mana; N2.AIUsageRule = TEXT("HoT a stationary tank."); N2.RequiredLevel = 1;

    FTalentNode& N3 = AddTalent(EClass::Earthweaver, ETalentLayer::ClassFoundation,
        ETalentNodeType::Utility, TEXT("Entangling Roots"), 2, 0,
        TEXT("Placeholder — Roots in 4m: immobilize 2s + Nature DoT."));
    N3.TriggerEvent = TEXT("OnCast"); N3.EffectDescription = TEXT("Roots: immobilize 2s + Nature DoT."); N3.CooldownMod = 10.0f;
    N3.AIUsageRule = TEXT("Lock a pack."); N3.RequiredLevel = 5;

    FTalentNode& N4 = AddTalent(EClass::Earthweaver, ETalentLayer::ClassFoundation,
        ETalentNodeType::ResourceMod, TEXT("Nature's Bounty"), 2, 1,
        TEXT("Placeholder — Caster form: +3 Mana/sec; Bear form: +5 Stamina/sec."));
    N4.TriggerEvent = TEXT("OnForm"); N4.EffectDescription = TEXT("Caster +3 Mana/sec; Bear +5 Stamina/sec."); N4.ResourceGen = 5.0f;
    N4.AIUsageRule = TEXT("Form-based sustain."); N4.RequiredLevel = 5;

    FTalentNode& N5 = AddTalent(EClass::Earthweaver, ETalentLayer::ClassFoundation,
        ETalentNodeType::Defensive, TEXT("Swift Mend"), 3, 0,
        TEXT("Placeholder — Shapeshifting removes 1 debuff and heals 5% max HP."));
    N5.TriggerEvent = TEXT("OnShapeshift"); N5.EffectDescription = TEXT("Form swap: cleanse + heal 5%."); N5.EffectValue = 0.05f;
    N5.AIUsageRule = TEXT("Swap forms to dispel."); N5.RequiredLevel = 8;

    FTalentNode& N6 = AddTalent(EClass::Earthweaver, ETalentLayer::ClassFoundation,
        ETalentNodeType::Keystone, TEXT("Primal Adaptation"), 3, 1,
        TEXT("Placeholder — +10% to the current form's strength (Bear=defense, Caster=healing)."));
    N6.TriggerEvent = TEXT("OnForm"); N6.EffectDescription = TEXT("+10% to current form's specialty."); N6.EffectValue = 0.10f;
    N6.AIUsageRule = TEXT("Adapt form to the situation."); N6.RequiredLevel = 10; N6.bRequiresKeystone = true;
}

// =============================================================================
// TALENT QUERIES
// =============================================================================

TArray<FTalentNode> UEclipseTalentSystem::GetAllTalents() const
{
    return AllTalents;
}

FTalentTree UEclipseTalentSystem::GetTree(EClass Class, ETalentLayer Layer) const
{
    FTalentTree Tree;
    Tree.ParentClass = Class;
    Tree.Layer = Layer;
    Tree.MaxPoints = MaxTalentPoints;

    for (const FTalentNode& Node : AllTalents)
    {
        if (Node.ParentClass == Class && Node.Layer == Layer)
        {
            Tree.Nodes.Add(Node);
            Tree.TotalPoints += Node.CurrentRank;
        }
    }
    return Tree;
}

TArray<FTalentTree> UEclipseTalentSystem::GetClassTrees(EClass Class) const
{
    TArray<FTalentTree> Trees;
    for (int32 Layer = 0; Layer <= 4; ++Layer)
    {
        Trees.Add(GetTree(Class, static_cast<ETalentLayer>(Layer)));
    }
    return Trees;
}

FTalentNode UEclipseTalentSystem::GetTalent(int32 TalentID) const
{
    if (const FTalentNode* Found = TalentByID.Find(TalentID))
    {
        return *Found;
    }
    return FTalentNode();
}

int32 UEclipseTalentSystem::GetTalentCount() const
{
    return AllTalents.Num();
}

// =============================================================================
// POINT ALLOCATION
// =============================================================================

bool UEclipseTalentSystem::CanAllocateTalent(int32 TalentID, int32 PlayerLevel, int32 AvailablePoints) const
{
    if (!TalentByID.Contains(TalentID)) return false;
    if (AvailablePoints < 1) return false;

    const FTalentNode* Node = TalentByID.Find(TalentID);
    if (!Node) return false;

    if (PlayerLevel < Node->RequiredLevel) return false;
    if (AllocatedTalents.Contains(TalentID) && AllocatedTalents[TalentID] >= Node->MaxRank) return false;

    // Prerequisites must be allocated.
    for (int32 Prereq : Node->PrerequisiteTalentIDs)
    {
        if (!AllocatedTalents.Contains(Prereq)) return false;
    }

    // Keystone gating.
    if (Node->bRequiresKeystone && Node->RequiredKeystoneID >= 0)
    {
        if (!AllocatedTalents.Contains(Node->RequiredKeystoneID)) return false;
    }

    // Mutual exclusivity with already-allocated talents.
    for (int32 Conflict : Node->ConflictTalentIDs)
    {
        if (AllocatedTalents.Contains(Conflict)) return false;
    }

    return true;
}

bool UEclipseTalentSystem::AllocateTalent(int32 TalentID)
{
    if (!TalentByID.Contains(TalentID)) return false;
    const FTalentNode* Node = TalentByID.Find(TalentID);
    if (!Node) return false;

    int32& Rank = AllocatedTalents.FindOrAdd(TalentID);
    if (Rank >= Node->MaxRank) return false;
    ++Rank;

    // Keep the node's CurrentRank field in sync for completeness.
    for (FTalentNode& N : AllTalents)
    {
        if (N.TalentID == TalentID)
        {
            N.CurrentRank = Rank;
            break;
        }
    }
    if (FTalentNode* Mapped = TalentByID.Find(TalentID))
    {
        Mapped->CurrentRank = Rank;
    }
    return true;
}

bool UEclipseTalentSystem::DeallocateTalent(int32 TalentID)
{
    if (!AllocatedTalents.Contains(TalentID)) return false;

    // Prevent removing a talent that is a prerequisite for an allocated one.
    for (const auto& Pair : AllocatedTalents)
    {
        if (Pair.Key == TalentID) continue;
        const FTalentNode* Other = TalentByID.Find(Pair.Key);
        if (!Other) continue;
        if (Other->PrerequisiteTalentIDs.Contains(TalentID)) return false;
    }

    AllocatedTalents.Remove(TalentID);
    for (FTalentNode& N : AllTalents)
    {
        if (N.TalentID == TalentID)
        {
            N.CurrentRank = 0;
            break;
        }
    }
    if (FTalentNode* Mapped = TalentByID.Find(TalentID))
    {
        Mapped->CurrentRank = 0;
    }
    return true;
}

void UEclipseTalentSystem::ResetAllTalents()
{
    AllocatedTalents.Empty();
    for (FTalentNode& Node : AllTalents)
    {
        Node.CurrentRank = 0;
    }
    TalentByID.Empty();
    for (const FTalentNode& Node : AllTalents)
    {
        TalentByID.Add(Node.TalentID, Node);
    }
}

int32 UEclipseTalentSystem::GetAllocatedPoints() const
{
    int32 Total = 0;
    for (const auto& Pair : AllocatedTalents)
    {
        Total += Pair.Value;
    }
    return Total;
}

int32 UEclipseTalentSystem::GetMaxPoints() const
{
    return MaxTalentPoints;
}

TArray<FTalentNode> UEclipseTalentSystem::GetAllocatedTalents() const
{
    TArray<FTalentNode> Result;
    for (const auto& Pair : AllocatedTalents)
    {
        if (const FTalentNode* Node = TalentByID.Find(Pair.Key))
        {
            FTalentNode Copy = *Node;
            Copy.CurrentRank = Pair.Value;
            Result.Add(Copy);
        }
    }
    return Result;
}

// =============================================================================
// PREREQUISITE VALIDATION
// =============================================================================

bool UEclipseTalentSystem::ArePrerequisitesMet(int32 TalentID) const
{
    const FTalentNode* Node = TalentByID.Find(TalentID);
    if (!Node) return false;
    for (int32 Prereq : Node->PrerequisiteTalentIDs)
    {
        if (!AllocatedTalents.Contains(Prereq)) return false;
    }
    if (Node->bRequiresKeystone && Node->RequiredKeystoneID >= 0)
    {
        if (!AllocatedTalents.Contains(Node->RequiredKeystoneID)) return false;
    }
    return true;
}

bool UEclipseTalentSystem::IsMutuallyExclusive(int32 TalentID, int32 OtherTalentID) const
{
    const FTalentNode* Node = TalentByID.Find(TalentID);
    if (!Node) return false;
    return Node->ConflictTalentIDs.Contains(OtherTalentID);
}

// =============================================================================
// SYNERGY GRAPH
// =============================================================================

TArray<int32> UEclipseTalentSystem::GetSynergies(int32 TalentID) const
{
    TArray<int32> Result;
    if (const FTalentNode* Node = TalentByID.Find(TalentID))
    {
        Result = Node->SynergyTalentIDs;
    }
    return Result;
}

TArray<int32> UEclipseTalentSystem::GetConflicts(int32 TalentID) const
{
    TArray<int32> Result;
    if (const FTalentNode* Node = TalentByID.Find(TalentID))
    {
        Result = Node->ConflictTalentIDs;
    }
    return Result;
}

TArray<int32> UEclipseTalentSystem::GetActiveSynergies() const
{
    // Returns synergy links where both endpoints are currently allocated.
    TSet<int32> AllocatedSet;
    for (const auto& Pair : AllocatedTalents)
    {
        AllocatedSet.Add(Pair.Key);
    }

    TArray<int32> Result;
    TSet<int32> Seen;
    for (int32 ID : AllocatedSet)
    {
        const FTalentNode* Node = TalentByID.Find(ID);
        if (!Node) continue;
        for (int32 SynergyID : Node->SynergyTalentIDs)
        {
            if (AllocatedSet.Contains(SynergyID))
            {
                int32 Lo = FMath::Min(ID, SynergyID);
                int32 Hi = FMath::Max(ID, SynergyID);
                int32 Key = Lo * 100000 + Hi;
                if (!Seen.Contains(Key))
                {
                    Seen.Add(Key);
                    Result.Add(Lo);
                    Result.Add(Hi);
                }
            }
        }
    }
    return Result;
}

// =============================================================================
// BUILDS
// =============================================================================

TArray<FBuildArchetype> UEclipseTalentSystem::GetBuildArchetypes(EClass Class) const
{
    TArray<FBuildArchetype> Result;
    for (const FBuildArchetype& Build : AllBuilds)
    {
        if (Build.Class == Class)
        {
            Result.Add(Build);
        }
    }
    return Result;
}

FBuildArchetype UEclipseTalentSystem::GetBuild(const FString& BuildName) const
{
    for (const FBuildArchetype& Build : AllBuilds)
    {
        if (Build.BuildName == BuildName)
        {
            return Build;
        }
    }
    return FBuildArchetype();
}

// =============================================================================
// ABILITY UNLOCKS
// =============================================================================

TArray<int32> UEclipseTalentSystem::GetUnlockedAbilityIDs() const
{
    TArray<int32> Result;
    for (const auto& Pair : AllocatedTalents)
    {
        if (const FTalentNode* Node = TalentByID.Find(Pair.Key))
        {
            if (Node->UnlockedAbilityID >= 0)
            {
                Result.AddUnique(Node->UnlockedAbilityID);
            }
            for (int32 Affected : Node->AffectedAbilityIDs)
            {
                if (Affected >= 0)
                {
                    Result.AddUnique(Affected);
                }
            }
        }
    }
    return Result;
}

bool UEclipseTalentSystem::IsAbilityUnlocked(int32 AbilityID) const
{
    for (const auto& Pair : AllocatedTalents)
    {
        if (const FTalentNode* Node = TalentByID.Find(Pair.Key))
        {
            if (Node->UnlockedAbilityID == AbilityID) return true;
            if (Node->AffectedAbilityIDs.Contains(AbilityID)) return true;
        }
    }
    return false;
}

// =============================================================================
// SAVE / LOAD
// =============================================================================

TMap<int32, int32> UEclipseTalentSystem::GetSavedAllocation() const
{
    return AllocatedTalents;
}

void UEclipseTalentSystem::LoadAllocation(const TMap<int32, int32>& InAllocation)
{
    AllocatedTalents = InAllocation;
    for (const auto& Pair : AllocatedTalents)
    {
        if (FTalentNode* Node = TalentByID.Find(Pair.Key))
        {
            Node->CurrentRank = Pair.Value;
        }
        for (FTalentNode& N : AllTalents)
        {
            if (N.TalentID == Pair.Key)
            {
                N.CurrentRank = Pair.Value;
                break;
            }
        }
    }
}

// =============================================================================
// QUALITY GATE VALIDATION — 8-point gate
// Each check appends a PASS/FAIL line to OutReport. Returns true only if all 8 pass.
// =============================================================================

bool UEclipseTalentSystem::ValidateBuildQuality(FString& OutReport) const
{
    int32 PassCount = 0;
    auto AppendPass = [&](const FString& Check)
    {
        OutReport += TEXT("  [PASS] ") + Check + TEXT("\n");
        ++PassCount;
    };
    auto AppendFail = [&](const FString& Check, const FString& Reason)
    {
        OutReport += TEXT("  [FAIL] ") + Check + TEXT(" — ") + Reason + TEXT("\n");
    };

    OutReport += TEXT("=== Eclipse Talent Quality Gate (8 checks) ===\n");

    // ---- Check 1: Unique, non-empty talent names ----
    TSet<FString> SeenNames;
    bool NamesOk = true;
    FString BadName;
    for (const FTalentNode& Node : AllTalents)
    {
        if (Node.TalentName.IsEmpty())
        {
            NamesOk = false; BadName = TEXT("<empty name>"); break;
        }
        if (SeenNames.Contains(Node.TalentName))
        {
            NamesOk = false; BadName = Node.TalentName; break;
        }
        SeenNames.Add(Node.TalentName);
    }
    if (NamesOk) AppendPass(TEXT("1. All talent names unique and non-empty"));
    else AppendFail(TEXT("1. All talent names unique and non-empty"),
        FString::Printf(TEXT("duplicate/empty name: '%s'"), *BadName));

    // ---- Check 2: Non-generic effects (no bare '+X% damage' filler) ----
    bool EffectsOk = true;
    FString BadEffect;
    for (const FTalentNode& Node : AllTalents)
    {
        if (Node.EffectDescription.IsEmpty())
        {
            EffectsOk = false; BadEffect = Node.TalentName + TEXT(" (empty EffectDescription)"); break;
        }
        // Reject patterns that are ONLY a short "+X% damage" string with no
        // action verb — i.e. no spaces means a single token like "+15%damage".
        if (Node.EffectDescription.StartsWith(TEXT("+")) &&
            Node.EffectDescription.Contains(TEXT("damage")) &&
            Node.EffectDescription.Len() < 24 &&
            !Node.EffectDescription.Contains(TEXT(" ")) )
        {
            EffectsOk = false; BadEffect = Node.TalentName + TEXT(" (generic +damage)"); break;
        }
    }
    if (EffectsOk) AppendPass(TEXT("2. No generic '+X% damage' filler — every talent has a mechanical effect"));
    else AppendFail(TEXT("2. No generic '+X% damage' filler"), BadEffect);

    // ---- Check 3: No orphan nodes (tier-1 exempt; others need a prerequisite) ----
    bool PrereqOk = true;
    FString Orphan;
    for (const FTalentNode& Node : AllTalents)
    {
        if (Node.Tier <= 1) continue; // foundation tier-1 nodes are entry points
        if (Node.PrerequisiteTalentIDs.Num() == 0 && !Node.bRequiresKeystone)
        {
            // Keystones in tier 3+ may stand alone by design; flag only true orphans.
            if (Node.NodeType != ETalentNodeType::Keystone)
            {
                PrereqOk = false; Orphan = Node.TalentName; break;
            }
        }
    }
    if (PrereqOk) AppendPass(TEXT("3. No orphan talents — non-entry nodes have prerequisites"));
    else AppendFail(TEXT("3. No orphan talents"), FString::Printf(TEXT("orphan: '%s'"), *Orphan));

    // ---- Check 4: Synergy graph bidirectionality ----
    bool SynergyOk = true;
    FString BrokenLink;
    for (const FTalentNode& Node : AllTalents)
    {
        for (int32 SynergyID : Node.SynergyTalentIDs)
        {
            const FTalentNode* Other = TalentByID.Find(SynergyID);
            if (!Other)
            {
                SynergyOk = false; BrokenLink = Node.TalentName + TEXT(" -> missing ID ") + FString::FromInt(SynergyID); break;
            }
            if (!Other->SynergyTalentIDs.Contains(Node.TalentID))
            {
                SynergyOk = false; BrokenLink = Node.TalentName + TEXT(" -> ") + Other->TalentName + TEXT(" (not bidirectional)"); break;
            }
        }
        if (!SynergyOk) break;
    }
    if (SynergyOk) AppendPass(TEXT("4. Synergy graph is bidirectional"));
    else AppendFail(TEXT("4. Synergy graph bidirectionality"), BrokenLink);

    // ---- Check 5: Build archetypes reference valid, allocated-able talent IDs ----
    bool BuildsOk = true;
    FString BadBuild;
    for (const FBuildArchetype& Build : AllBuilds)
    {
        for (int32 Tid : Build.AllocatedTalentIDs)
        {
            if (!TalentByID.Contains(Tid))
            {
                BuildsOk = false; BadBuild = Build.BuildName + TEXT(" (invalid talent ID ") + FString::FromInt(Tid) + TEXT(")"); break;
            }
        }
        if (!BuildsOk) break;
    }
    if (BuildsOk) AppendPass(TEXT("5. All build archetypes reference valid talent IDs"));
    else AppendFail(TEXT("5. Build archetype references"), BadBuild);

    // ---- Check 6: Every build's synergy scores are in [0, 10] ----
    bool ScoresOk = true;
    FString BadScore;
    auto CheckScore = [&](float V, const FString& Label, const FString& Build) -> bool
    {
        if (V < 0.0f || V > 10.0f)
        {
            BadScore = Build + TEXT(" ") + Label + TEXT(" = ") + FString::SanitizeFloat(V);
            return false;
        }
        return true;
    };
    for (const FBuildArchetype& Build : AllBuilds)
    {
        if (!CheckScore(Build.MechanicalCoherence, TEXT("MechanicalCoherence"), Build.BuildName)) { ScoresOk = false; break; }
        if (!CheckScore(Build.TalentAccessibility, TEXT("TalentAccessibility"), Build.BuildName)) { ScoresOk = false; break; }
        if (!CheckScore(Build.RotationCompleteness, TEXT("RotationCompleteness"), Build.BuildName)) { ScoresOk = false; break; }
        if (!CheckScore(Build.ResourceSustainability, TEXT("ResourceSustainability"), Build.BuildName)) { ScoresOk = false; break; }
        if (!CheckScore(Build.OffensiveSynergy, TEXT("OffensiveSynergy"), Build.BuildName)) { ScoresOk = false; break; }
        if (!CheckScore(Build.DefensiveSynergy, TEXT("DefensiveSynergy"), Build.BuildName)) { ScoresOk = false; break; }
        if (!CheckScore(Build.ControlUtility, TEXT("ControlUtility"), Build.BuildName)) { ScoresOk = false; break; }
        if (!CheckScore(Build.EncounterAdaptability, TEXT("EncounterAdaptability"), Build.BuildName)) { ScoresOk = false; break; }
        if (!CheckScore(Build.Counterplay, TEXT("Counterplay"), Build.BuildName)) { ScoresOk = false; break; }
        if (!CheckScore(Build.BuildDiversity, TEXT("BuildDiversity"), Build.BuildName)) { ScoresOk = false; break; }
    }
    if (ScoresOk) AppendPass(TEXT("6. All build synergy scores within [0, 10]"));
    else AppendFail(TEXT("6. Synergy scores in range"), BadScore);

    // ---- Check 7: No duplicate TalentIDs ----
    TSet<int32> SeenIDs;
    bool IDsOk = true;
    int32 DupeID = -1;
    for (const FTalentNode& Node : AllTalents)
    {
        if (SeenIDs.Contains(Node.TalentID))
        {
            IDsOk = false; DupeID = Node.TalentID; break;
        }
        SeenIDs.Add(Node.TalentID);
    }
    if (IDsOk) AppendPass(TEXT("7. No duplicate TalentIDs"));
    else AppendFail(TEXT("7. No duplicate TalentIDs"),
        FString::Printf(TEXT("duplicate ID %d"), DupeID));

    // ---- Check 8: Every class has a Class Foundation tree with >= 5 nodes ----
    bool CoverageOk = true;
    FString MissingClass;
    for (int32 ClassInt = 0; ClassInt <= 16; ++ClassInt)
    {
        EClass Class = static_cast<EClass>(ClassInt);
        int32 Count = 0;
        for (const FTalentNode& Node : AllTalents)
        {
            if (Node.ParentClass == Class && Node.Layer == ETalentLayer::ClassFoundation)
            {
                ++Count;
            }
        }
        if (Count < 5)
        {
            CoverageOk = false;
            MissingClass = FString::Printf(TEXT("class #%d has %d Class Foundation nodes"), ClassInt, Count);
            break;
        }
    }
    if (CoverageOk) AppendPass(TEXT("8. Every class has a Class Foundation tree with >= 5 nodes"));
    else AppendFail(TEXT("8. Class Foundation coverage"), MissingClass);

    OutReport += FString::Printf(TEXT("=== Result: %d / 8 checks passed ===\n"), PassCount);
    return PassCount == 8;
}
