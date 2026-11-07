// Eclipse HUD System Implementation
// Health, Stamina, Mana, Rage, Aether, Fracture Flow, Cooldowns, Debuffs, Cast Bar, Combat Log

#include "EclipseHUDSystem.h"
#include "Engine/Engine.h"

UEclipseHUDSystem::UEclipseHUDSystem()
{
    InitializeResources();
}

void UEclipseHUDSystem::InitializeResources()
{
    // Health
    FResourceBar Health;
    Health.Type = EResourceType::None;
    Health.Current = 100.0f;
    Health.Max = 100.0f;
    Health.RegenRate = 0.0f; // No passive health regen
    Health.RegenDelay = 0.0f;
    Health.BarColor = FLinearColor(0.8f, 0.1f, 0.1f, 1.0f); // Red
    Resources.Add(EResourceType::None, Health);

    // Stamina
    FResourceBar Stamina;
    Stamina.Type = EResourceType::Stamina;
    Stamina.Current = 100.0f;
    Stamina.Max = 100.0f;
    Stamina.RegenRate = 25.0f; // Fast regen
    Stamina.RegenDelay = 1.0f;
    Stamina.BarColor = FLinearColor(0.2f, 0.8f, 0.3f, 1.0f); // Green
    Resources.Add(EResourceType::Stamina, Stamina);

    // Mana
    FResourceBar Mana;
    Mana.Type = EResourceType::Mana;
    Mana.Current = 50.0f;
    Mana.Max = 50.0f;
    Mana.RegenRate = 8.0f;
    Mana.RegenDelay = 1.5f;
    Mana.BarColor = FLinearColor(0.2f, 0.4f, 0.9f, 1.0f); // Blue
    Resources.Add(EResourceType::Mana, Mana);

    // Rage
    FResourceBar Rage;
    Rage.Type = EResourceType::Rage;
    Rage.Current = 0.0f;
    Rage.Max = 100.0f;
    Rage.RegenRate = 0.0f; // Builds from taking damage, not passive
    Rage.RegenDelay = 0.0f;
    Rage.BarColor = FLinearColor(0.9f, 0.2f, 0.1f, 1.0f); // Blood red
    Resources.Add(EResourceType::Rage, Rage);

    // Aether
    FResourceBar Aether;
    Aether.Type = EResourceType::Aether;
    Aether.Current = 0.0f;
    Aether.Max = 100.0f;
    Aether.RegenRate = 3.0f;
    Aether.RegenDelay = 2.0f;
    Aether.BarColor = FLinearColor(0.6f, 0.3f, 0.9f, 1.0f); // Violet
    Resources.Add(EResourceType::Aether, Aether);

    // Flow (Fracture Flow)
    FlowCurrent = 0.0f;
    FlowMax = 100.0f;

    // Cooldowns
    Cooldowns.SetNum(8); // 8 ability slots

    // Combat log
    CombatLog.Reserve(50);
}

void UEclipseHUDSystem::UpdateResource(EResourceType Type, float Current, float Max)
{
    if (FResourceBar* Bar = Resources.Find(Type))
    {
        Bar->Current = FMath::Clamp(Current, 0.0f, Max);
        Bar->Max = Max;
    }
}

FResourceBar UEclipseHUDSystem::GetResource(EResourceType Type) const
{
    if (const FResourceBar* Bar = Resources.Find(Type))
    {
        return *Bar;
    }
    return FResourceBar();
}

void UEclipseHUDSystem::SetResourceRegen(EResourceType Type, float RegenRate, float RegenDelay)
{
    if (FResourceBar* Bar = Resources.Find(Type))
    {
        Bar->RegenRate = RegenRate;
        Bar->RegenDelay = RegenDelay;
    }
}

void UEclipseHUDSystem::UpdateFlow(float Current, float Max)
{
    FlowCurrent = FMath::Clamp(Current, 0.0f, Max);
    FlowMax = Max;
}

bool UEclipseHUDSystem::IsFlowFull() const
{
    return FlowCurrent >= FlowMax;
}

void UEclipseHUDSystem::TriggerFlowFinisher()
{
    if (IsFlowFull())
    {
        UE_LOG(LogTemp, Log, TEXT("[HUD] FRACTURE FLOW FINISHER TRIGGERED!"));
        FlowCurrent = 0.0f;
    }
}

void UEclipseHUDSystem::StartCooldown(int32 Slot, const FString& AbilityName, float Duration)
{
    if (Cooldowns.IsValidIndex(Slot))
    {
        Cooldowns[Slot].SlotIndex = Slot;
        Cooldowns[Slot].AbilityName = AbilityName;
        Cooldowns[Slot].CooldownRemaining = Duration;
        Cooldowns[Slot].CooldownMax = Duration;
        UE_LOG(LogTemp, Log, TEXT("[HUD] Cooldown started: %s (Slot %d, %.1fs)"), *AbilityName, Slot, Duration);
    }
}

float UEclipseHUDSystem::GetCooldownRemaining(int32 Slot) const
{
    if (Cooldowns.IsValidIndex(Slot))
    {
        return Cooldowns[Slot].CooldownRemaining;
    }
    return 0.0f;
}

bool UEclipseHUDSystem::IsOnCooldown(int32 Slot) const
{
    if (Cooldowns.IsValidIndex(Slot))
    {
        return Cooldowns[Slot].CooldownRemaining > 0.0f;
    }
    return false;
}

void UEclipseHUDSystem::AddDebuff(const FString& Name, float Duration, EWeaponStatusEffect Element, float Stacks)
{
    FDebuffEntry Debuff;
    Debuff.DebuffName = Name;
    Debuff.DurationRemaining = Duration;
    Debuff.ElementType = Element;
    Debuff.Stacks = Stacks;

    switch (Element)
    {
    case EWeaponStatusEffect::Fire:
        Debuff.IconColor = FLinearColor(0.9f, 0.3f, 0.1f, 1.0f);
        break;
    case EWeaponStatusEffect::Ice:
        Debuff.IconColor = FLinearColor(0.3f, 0.7f, 0.9f, 1.0f);
        break;
    case EWeaponStatusEffect::Lightning:
        Debuff.IconColor = FLinearColor(0.6f, 0.4f, 0.9f, 1.0f);
        break;
    case EWeaponStatusEffect::Shadow:
        Debuff.IconColor = FLinearColor(0.3f, 0.1f, 0.4f, 1.0f);
        break;
    case EWeaponStatusEffect::Holy:
        Debuff.IconColor = FLinearColor(0.9f, 0.8f, 0.3f, 1.0f);
        break;
    case EWeaponStatusEffect::Poison:
        Debuff.IconColor = FLinearColor(0.4f, 0.8f, 0.2f, 1.0f);
        break;
    default:
        Debuff.IconColor = FLinearColor::Red;
        break;
    }

    ActiveDebuffs.Add(Debuff);
    UE_LOG(LogTemp, Log, TEXT("[HUD] Debuff added: %s (%.1fs, %d stacks)"), *Name, Duration, (int32)Stacks);
}

void UEclipseHUDSystem::RemoveDebuff(const FString& Name)
{
    ActiveDebuffs.RemoveAll([&Name](const FDebuffEntry& D) { return D.DebuffName == Name; });
}

TArray<FDebuffEntry> UEclipseHUDSystem::GetActiveDebuffs() const
{
    return ActiveDebuffs;
}

void UEclipseHUDSystem::StartCastBar(const FString& SpellName, float CastTime)
{
    bCastInProgress = true;
    CurrentCastSpell = SpellName;
    CastProgress = 0.0f;
    CastMaxTime = CastTime;
    UE_LOG(LogTemp, Log, TEXT("[HUD] Cast bar started: %s (%.1fs)"), *SpellName, CastTime);
}

void UEclipseHUDSystem::UpdateCastBar(float CurrentTime)
{
    if (bCastInProgress)
    {
        CastProgress = FMath::Clamp(CurrentTime, 0.0f, CastMaxTime);
        if (CastProgress >= CastMaxTime)
        {
            bCastInProgress = false;
            UE_LOG(LogTemp, Log, TEXT("[HUD] Cast complete: %s"), *CurrentCastSpell);
        }
    }
}

void UEclipseHUDSystem::CancelCastBar()
{
    if (bCastInProgress)
    {
        bCastInProgress = false;
        UE_LOG(LogTemp, Log, TEXT("[HUD] Cast cancelled: %s"), *CurrentCastSpell);
    }
}

bool UEclipseHUDSystem::IsCasting() const
{
    return bCastInProgress;
}

void UEclipseHUDSystem::AddCombatLogEntry(const FString& Entry)
{
    CombatLog.Add(Entry);
    if (CombatLog.Num() > 50)
    {
        CombatLog.RemoveAt(0);
    }
}

TArray<FString> UEclipseHUDSystem::GetCombatLog(int32 MaxEntries) const
{
    TArray<FString> Result;
    int32 Start = FMath::Max(0, CombatLog.Num() - MaxEntries);
    for (int32 i = Start; i < CombatLog.Num(); i++)
    {
        Result.Add(CombatLog[i]);
    }
    return Result;
}

void UEclipseHUDSystem::SetTargetInfo(const FString& Name, float Health, float MaxHealth, EWeakPointType WeakPoint, bool bIsBoss)
{
    UE_LOG(LogTemp, Log, TEXT("[HUD] Target: %s (HP: %.0f/%.0f, WeakPoint: %d, Boss: %s)"),
        *Name, Health, MaxHealth, (int32)WeakPoint, bIsBoss ? TEXT("Yes") : TEXT("No"));
}

void UEclipseHUDSystem::ClearTargetInfo()
{
    UE_LOG(LogTemp, Log, TEXT("[HUD] Target cleared"));
}

void UEclipseHUDSystem::SetPartySize(int32 Size)
{
    UE_LOG(LogTemp, Log, TEXT("[HUD] Party size set to %d"), Size);
}

void UEclipseHUDSystem::UpdatePartyMember(int32 Index, const FString& Name, float Health, float MaxHealth, int32 Role)
{
    UE_LOG(LogTemp, Log, TEXT("[HUD] Party[%d]: %s (HP: %.0f/%.0f, Role: %d)"), Index, *Name, Health, MaxHealth, Role);
}

void UEclipseHUDSystem::SetBossHealth(const FString& Name, float Current, float Max)
{
    BossName = Name;
    BossHealthCurrent = Current;
    BossHealthMax = Max;
    bBossHealthVisible = true;
    UE_LOG(LogTemp, Log, TEXT("[HUD] Boss: %s (HP: %.0f/%.0f)"), *Name, Current, Max);
}

void UEclipseHUDSystem::ShowBossHealth(bool bShow)
{
    bBossHealthVisible = bShow;
}

void UEclipseHUDSystem::SetHUDVisible(bool bVisible)
{
    bHUDVisible = bVisible;
}

void UEclipseHUDSystem::SetWidgetVisible(EHUDWidget Widget, bool bVisible)
{
    UE_LOG(LogTemp, Log, TEXT("[HUD] Widget %d visibility: %s"), (int32)Widget, bVisible ? TEXT("On") : TEXT("Off"));
}

void UEclipseHUDSystem::ShowNotification(const FString& Text, float Duration)
{
    UE_LOG(LogTemp, Log, TEXT("[HUD] Notification: %s (%.1fs)"), *Text, Duration);
}
