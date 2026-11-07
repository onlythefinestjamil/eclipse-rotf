// Eclipse Damage Rotation Implementation
// No auto-attack. Manual attacks, manual casts, manual abilities. High skill ceiling.

#include "EclipseDamageRotation.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"

UEclipseDamageRotation::UEclipseDamageRotation()
{
    // Initialize cooldowns for all actions
    CooldownDuration.Add(EInputAction::LightAttack, 0.0f);      // No cooldown — chainable
    CooldownDuration.Add(EInputAction::HeavyAttack, 0.5f);      // Short recovery
    CooldownDuration.Add(EInputAction::ChargedAttack, 1.5f);     // Longer recovery
    CooldownDuration.Add(EInputAction::WeaponArt, 8.0f);        // Weapon art cooldown
    CooldownDuration.Add(EInputAction::Spell1, 3.0f);
    CooldownDuration.Add(EInputAction::Spell2, 5.0f);
    CooldownDuration.Add(EInputAction::Spell3, 8.0f);
    CooldownDuration.Add(EInputAction::Spell4, 12.0f);
    CooldownDuration.Add(EInputAction::Ability1, 15.0f);
    CooldownDuration.Add(EInputAction::Ability2, 20.0f);
    CooldownDuration.Add(EInputAction::Ability3, 30.0f);
    CooldownDuration.Add(EInputAction::Ultimate, 180.0f);       // 3 min ultimate
    CooldownDuration.Add(EInputAction::Dodge, 1.0f);
    CooldownDuration.Add(EInputAction::Parry, 2.0f);

    // Copy to remaining
    for (auto& Pair : CooldownDuration)
    {
        CooldownRemaining.Add(Pair.Key, 0.0f);
    }

    // Spell resource costs
    SpellResourceCost.Add(1, 20.0f);
    SpellResourceCost.Add(2, 30.0f);
    SpellResourceCost.Add(3, 40.0f);
    SpellResourceCost.Add(4, 60.0f);

    // Spell cooldowns
    SpellCooldownMax.Add(1, 3.0f);
    SpellCooldownMax.Add(2, 5.0f);
    SpellCooldownMax.Add(3, 8.0f);
    SpellCooldownMax.Add(4, 12.0f);
    for (auto& Pair : SpellCooldownMax)
    {
        SpellCooldowns.Add(Pair.Key, 0.0f);
    }
}

void UEclipseDamageRotation::InitializeClassRotation(EClass Class)
{
    // Each class has different damage values and rotation style
    switch (Class)
    {
    case EClass::Vanguard:
        LightDamage = 90.0f;
        HeavyDamage = 180.0f;
        ChargedDamage = 300.0f;
        WeaponArtDamage = 350.0f;
        UltimateDamage = 600.0f;
        // Tank rotation: build momentum with blocks, spend on heavy hits
        break;

    case EClass::UmbralAssassin:
        LightDamage = 60.0f;  // Fast but lower per-hit
        HeavyDamage = 120.0f;
        ChargedDamage = 200.0f;
        WeaponArtDamage = 280.0f;
        UltimateDamage = 450.0f;
        // Assassin rotation: build Flow through perfect dodges, burst with finishers
        break;

    case EClass::Arcanist:
        LightDamage = 50.0f;  // Weak melee
        HeavyDamage = 80.0f;
        SpellDamage = 200.0f;  // Strong spells
        AbilityDamage = 250.0f;
        UltimateDamage = 700.0f;
        // Caster rotation: spell priority, weave melee between casts
        break;

    case EClass::Berserker:
        LightDamage = 100.0f;
        HeavyDamage = 200.0f;
        ChargedDamage = 350.0f;
        WeaponArtDamage = 400.0f;
        UltimateDamage = 800.0f;
        // Berserker: spam heavy attacks, rage increases damage
        break;

    case EClass::Templar:
        LightDamage = 70.0f;
        HeavyDamage = 140.0f;
        SpellDamage = 100.0f;
        AbilityDamage = 150.0f;
        UltimateDamage = 500.0f;
        // Templar: hybrid tank/healer, balance damage and healing
        break;

    default:
        // Standard values already set in constructor
        break;
    }
}

float UEclipseDamageRotation::ProcessAction(EInputAction Action, bool bHitWeakPoint, float TimingAccuracy)
{
    // TimingAccuracy: 0.0 = perfect, 1.0 = worst
    // Perfect timing = full damage + Flow build
    // Poor timing = reduced damage, no Flow

    float DamageMultiplier = 1.0f - (TimingAccuracy * 0.3f); // Up to 30% damage reduction for bad timing
    float FlowGain = 0.0f;
    float Damage = 0.0f;

    switch (Action)
    {
    case EInputAction::LightAttack:
        Damage = LightDamage * DamageMultiplier;
        FlowGain = 5.0f;
        // AdvanceCombo (was a label, removed)
        if (CanChainLight())
        {
            ComboStep++;
            ComboTimer = ComboWindow;
        }
        break;

    case EInputAction::HeavyAttack:
        Damage = HeavyDamage * DamageMultiplier;
        FlowGain = 8.0f;
        if (CanChainHeavy())
        {
            ComboStep++;
            ComboTimer = ComboWindow;
        }
        break;

    case EInputAction::ChargedAttack:
        Damage = ChargedDamage * DamageMultiplier;
        FlowGain = 12.0f;
        CooldownRemaining[EInputAction::ChargedAttack] = CooldownDuration[EInputAction::ChargedAttack];
        break;

    case EInputAction::WeaponArt:
        if (IsActionAvailable(EInputAction::WeaponArt, 0.0f))
        {
            Damage = WeaponArtDamage * DamageMultiplier;
            FlowGain = 20.0f;
            CooldownRemaining[EInputAction::WeaponArt] = CooldownDuration[EInputAction::WeaponArt];
        }
        break;

    case EInputAction::Spell1:
    case EInputAction::Spell2:
    case EInputAction::Spell3:
    case EInputAction::Spell4:
        {
            int32 Slot = (int32)Action - (int32)EInputAction::Spell1 + 1;
            if (CanCastSpell(Slot, 100.0f)) // Would pass actual resource
            {
                Damage = SpellDamage * DamageMultiplier;
                FlowGain = 10.0f;
                SpellCooldowns[Slot] = SpellCooldownMax[Slot];
            }
        }
        break;

    case EInputAction::Ability1:
    case EInputAction::Ability2:
    case EInputAction::Ability3:
        if (IsActionAvailable(Action, 100.0f))
        {
            Damage = AbilityDamage * DamageMultiplier;
            FlowGain = 15.0f;
            CooldownRemaining[Action] = CooldownDuration[Action];
        }
        break;

    case EInputAction::Ultimate:
        if (IsActionAvailable(EInputAction::Ultimate, 100.0f))
        {
            Damage = UltimateDamage * DamageMultiplier;
            FlowGain = 50.0f;
            CooldownRemaining[EInputAction::Ultimate] = CooldownDuration[EInputAction::Ultimate];
        }
        break;

    case EInputAction::Dodge:
        // Dodge doesn't deal damage but builds Flow on perfect timing
        if (TimingAccuracy < 0.2f) // Perfect dodge
        {
            FlowGain = 15.0f;
        }
        CooldownRemaining[EInputAction::Dodge] = CooldownDuration[EInputAction::Dodge];
        break;

    case EInputAction::Parry:
        // Parry doesn't deal damage but opens riposte
        if (TimingAccuracy < 0.15f) // Perfect parry
        {
            FlowGain = 20.0f;
            Damage = HeavyDamage * 2.0f; // Riposte damage after parry
        }
        CooldownRemaining[EInputAction::Parry] = CooldownDuration[EInputAction::Parry];
        break;

    case EInputAction::AerialAttack:
        Damage = LightDamage * 1.2f * DamageMultiplier; // Aerial bonus
        FlowGain = 10.0f;
        AerialComboStep++;
        break;
    }

    // Weak point = guaranteed crit (2x damage)
    if (bHitWeakPoint)
    {
        Damage *= 2.0f;
        FlowGain *= 1.5f; // More Flow for precision hits
        UE_LOG(LogTemp, Log, TEXT("WEAK POINT CRIT! Damage: %.1f"), Damage);
    }

    UE_LOG(LogTemp, Log, TEXT("[Rotation] Action: %d, Damage: %.1f, Flow: +%.1f, Timing: %.2f"),
        (int32)Action, Damage, FlowGain, TimingAccuracy);

    return Damage;
}

EInputAction UEclipseDamageRotation::GetRecommendedAction(float CurrentResource, float CurrentFlow)
{
    // Priority system — returns the best available action
    // This is what the rotation suggests (player can ignore it — it's optional)

    // If ultimate is available and Flow is full → ultimate
    if (IsActionAvailable(EInputAction::Ultimate, CurrentResource) && CurrentFlow >= 100.0f)
    {
        return EInputAction::Ultimate;
    }

    // If weapon art is available → weapon art
    if (IsActionAvailable(EInputAction::WeaponArt, CurrentResource))
    {
        return EInputAction::WeaponArt;
    }

    // If abilities are available → use highest priority ability
    if (IsActionAvailable(EInputAction::Ability1, CurrentResource))
    {
        return EInputAction::Ability1;
    }

    // If spells are available and have resources → cast
    if (CanCastSpell(1, CurrentResource))
    {
        return EInputAction::Spell1;
    }

    // If charged attack is available → charged
    if (IsActionAvailable(EInputAction::ChargedAttack, CurrentResource))
    {
        return EInputAction::ChargedAttack;
    }

    // If in combo chain → continue
    if (CanChainLight())
    {
        return EInputAction::LightAttack;
    }

    // Default to heavy attack (highest sustained damage)
    if (IsActionAvailable(EInputAction::HeavyAttack, CurrentResource))
    {
        return EInputAction::HeavyAttack;
    }

    // Fallback to light attack (always available)
    return EInputAction::LightAttack;
}

bool UEclipseDamageRotation::IsActionAvailable(EInputAction Action, float CurrentResource)
{
    // Check cooldown
    if (CooldownRemaining.Contains(Action) && CooldownRemaining[Action] > 0.0f)
    {
        return false;
    }

    // Check resource (would check against actual resource cost)
    // For now, abilities and spells need resources
    if (Action >= EInputAction::Spell1 && Action <= EInputAction::Spell4)
    {
        int32 Slot = (int32)Action - (int32)EInputAction::Spell1 + 1;
        if (SpellResourceCost.Contains(Slot) && CurrentResource < SpellResourceCost[Slot])
        {
            return false;
        }
    }

    return true;
}

float UEclipseDamageRotation::GetCooldownRemaining(EInputAction Action)
{
    if (CooldownRemaining.Contains(Action))
    {
        return CooldownRemaining[Action];
    }
    return 0.0f;
}

void UEclipseDamageRotation::SetClassRotation(EClass CharacterClass)
{
    InitializeClassRotation(CharacterClass);
    UE_LOG(LogTemp, Log, TEXT("Rotation set for class: %d"), (int32)CharacterClass);
}

int32 UEclipseDamageRotation::GetComboStep() const
{
    return ComboStep;
}

bool UEclipseDamageRotation::CanChainLight() const
{
    return ComboStep < MaxComboSteps && ComboTimer > 0.0f;
}

bool UEclipseDamageRotation::CanChainHeavy() const
{
    return ComboStep < MaxComboSteps && ComboTimer > 0.0f;
}

bool UEclipseDamageRotation::CanTransitionToAerial() const
{
    // Can go aerial if in a combo and not already airborne
    return ComboStep > 0 && !bIsAirborne;
}

bool UEclipseDamageRotation::CanCastSpell(int32 SpellSlot, float CurrentResource)
{
    if (SpellSlot < 1 || SpellSlot > 4) return false;

    if (SpellCooldowns.Contains(SpellSlot) && SpellCooldowns[SpellSlot] > 0.0f)
    {
        return false;
    }

    if (SpellResourceCost.Contains(SpellSlot) && CurrentResource < SpellResourceCost[SpellSlot])
    {
        return false;
    }

    return true;
}

float UEclipseDamageRotation::CastSpell(int32 SpellSlot, float CurrentResource)
{
    if (!CanCastSpell(SpellSlot, CurrentResource))
    {
        return 0.0f;
    }

    float Damage = SpellDamage;
    SpellCooldowns[SpellSlot] = SpellCooldownMax[SpellSlot];

    UE_LOG(LogTemp, Log, TEXT("[Rotation] Spell %d cast — Damage: %.1f, Resource cost: %.1f"),
        SpellSlot, Damage, SpellResourceCost[SpellSlot]);

    return Damage;
}

void UEclipseDamageRotation::StartAimingSpell(int32 SpellSlot)
{
    if (CanCastSpell(SpellSlot, 100.0f))
    {
        AimedSpellSlot = SpellSlot;
        bIsAiming = true;
        UE_LOG(LogTemp, Log, TEXT("[Rotation] Aiming spell %d..."), SpellSlot);
    }
}

float UEclipseDamageRotation::ReleaseAimedSpell()
{
    if (!bIsAiming || AimedSpellSlot < 0)
    {
        return 0.0f;
    }

    bIsAiming = false;
    float Damage = CastSpell(AimedSpellSlot, 100.0f);
    AimedSpellSlot = -1;

    UE_LOG(LogTemp, Log, TEXT("[Rotation] Aimed spell released — Damage: %.1f"), Damage);
    return Damage;
}
