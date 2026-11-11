#include "EclipseEquipmentComponent.h"

#include "EclipseCharacter.h"
#include "EclipseCombatComponent.h"
#include "EclipseEquipmentTypes.h"
#include "EclipseEquipmentTags.h"
#include "EclipseAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

DEFINE_LOG_CATEGORY_STATIC(LogEclipseEquip, Log, All);

// ---------------------------------------------------------------------------------------------
// UEclipseItemDefinition helpers
// ---------------------------------------------------------------------------------------------

float UEclipseItemDefinition::GetRarityMultiplier(ERarityTier InRarity)
{
    switch (InRarity)
    {
    case ERarityTier::Common: return 1.00f;
    case ERarityTier::Uncommon: return 1.08f;
    case ERarityTier::Rare: return 1.18f;
    case ERarityTier::Epic: return 1.30f;
    case ERarityTier::Legendary: return 1.45f;
    case ERarityTier::Mythic: return 1.60f;
    case ERarityTier::Eclipse: return 1.75f;
    default: return 1.0f;
    }
}

float UEclipseItemDefinition::ComputeWeaponPower() const
{
    return LightDamage * (1.0f + 0.04f * FMath::Max(0, ItemLevel - 1)) * GetRarityMultiplier(Rarity);
}

// ---------------------------------------------------------------------------------------------
// UEclipseEquipmentComponent
// ---------------------------------------------------------------------------------------------

UEclipseEquipmentComponent::UEclipseEquipmentComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

AEclipseCharacter* UEclipseEquipmentComponent::GetOwnerChar() const
{
    return Cast<AEclipseCharacter>(GetOwner());
}

UAbilitySystemComponent* UEclipseEquipmentComponent::GetOwnerASC() const
{
    const AEclipseCharacter* Char = GetOwnerChar();
    return Char ? Char->GetAbilitySystemComponent() : nullptr;
}

void UEclipseEquipmentComponent::BeginPlay()
{
    Super::BeginPlay();

    AEclipseCharacter* Char = GetOwnerChar();
    if (!Char)
    {
        UE_LOG(LogEclipseEquip, Warning, TEXT("[EQUIP] owner is not an AEclipseCharacter - component idle"));
        return;
    }

    CacheBaseline();

    if (Char->DefaultLoadout.Num() == 0)
    {
        UE_LOG(LogEclipseEquip, Warning, TEXT("[EQUIP] DefaultLoadout is empty on %s - nothing equipped"), *Char->GetName());
        return;
    }

    LoadoutIndex = 0;
    EquipItem(Char->DefaultLoadout[0]);
}

void UEclipseEquipmentComponent::CacheBaseline()
{
    if (bBaselineCached)
    {
        return;
    }
    const AEclipseCharacter* Char = GetOwnerChar();
    if (!Char)
    {
        return;
    }
    if (const UEclipseCombatComponent* Combat = Char->GetCombat())
    {
        BaseLightDamage = Combat->LightDamage;
        BaseHeavyDamage = Combat->HeavyDamage;
        BaseAttackRange = Combat->AttackRange;
    }
    BaseWeaponScale = Char->WeaponAttachScale;
    bBaselineCached = true;
    UE_LOG(LogEclipseEquip, Log, TEXT("[EQUIP] baseline cached light=%.1f heavy=%.1f reach=%.0f"), BaseLightDamage, BaseHeavyDamage, BaseAttackRange);
}

bool UEclipseEquipmentComponent::EquipItem(UEclipseItemDefinition* Item)
{
    AEclipseCharacter* Char = GetOwnerChar();
    if (!Item || !Char)
    {
        UE_LOG(LogEclipseEquip, Warning, TEXT("[EQUIP] equip refused - %s"), Item ? TEXT("no owner") : TEXT("null item"));
        return false;
    }

    CacheBaseline();

    if (EquippedWeapon)
    {
        UnequipWeapon();
    }

    EquippedWeapon = Item;

    // Combat stats: the combat component's floats are not GAS attributes, so they are pushed directly.
    if (UEclipseCombatComponent* Combat = Char->GetCombat())
    {
        Combat->LightDamage = Item->LightDamage;
        Combat->HeavyDamage = Item->HeavyDamage;
        Combat->AttackRange = Item->Reach;
    }

    // Visual: soft mesh loaded on equip (placeholder until weapon meshes pass the spend gate).
    UStaticMesh* Mesh = Item->Mesh.LoadSynchronous();
    Char->SetWeaponMesh(Mesh);
    if (Char->WeaponMesh && Mesh)
    {
        Char->WeaponMesh->SetRelativeScale3D(Item->MeshScale);
        Char->WeaponMesh->SetRelativeLocation(Char->WeaponAttachOffset + Item->MeshOffset);
        Char->WeaponMesh->SetRelativeRotation((FQuat(Char->WeaponAttachRotation) * FQuat(Item->MeshRotation)).Rotator());
    }
    if (!Mesh)
    {
        UE_LOG(LogEclipseEquip, Warning, TEXT("[EQUIP] %s has no mesh assigned"), *Item->GetName());
    }

    // GAS: equip attribute modifiers on the wearer.
    if (UAbilitySystemComponent* ASC = GetOwnerASC())
    {
        FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
        Context.AddSourceObject(Item);
        FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(UEclipseGE_EquipStats::StaticClass(), 1.0f, Context);
        if (Spec.IsValid())
        {
            Spec.Data->SetSetByCallerMagnitude(EclipseEquipNames::WeaponPower, Item->ComputeWeaponPower());
            Spec.Data->SetSetByCallerMagnitude(EclipseEquipNames::CritChance, Item->CritChance);
            Spec.Data->SetSetByCallerMagnitude(EclipseEquipNames::CritDamage, Item->CritDamage);
            Spec.Data->SetSetByCallerMagnitude(EclipseEquipNames::PoiseDamage, Item->PoiseDamageMultiplier);
            StatsHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
        }
        for (const TSubclassOf<UGameplayEffect>& EffectClass : Item->EquipEffects)
        {
            if (!EffectClass)
            {
                continue;
            }
            FGameplayEffectSpecHandle Extra = ASC->MakeOutgoingSpec(EffectClass, 1.0f, Context);
            if (Extra.IsValid())
            {
                ExtraHandles.Add(ASC->ApplyGameplayEffectSpecToSelf(*Extra.Data.Get()));
            }
        }
        UE_LOG(LogEclipseEquip, Log, TEXT("[EQUIP] GAS stats applied: WeaponPower=%.1f Crit=%.2f PoiseX=%.2f (%d extra effects)"),
            ASC->GetNumericAttribute(UEclipseAttributeSet::GetWeaponPowerAttribute()),
            ASC->GetNumericAttribute(UEclipseAttributeSet::GetCritChanceAttribute()),
            ASC->GetNumericAttribute(UEclipseAttributeSet::GetPoiseDamageAttribute()),
            ExtraHandles.Num());
    }
    else
    {
        UE_LOG(LogEclipseEquip, Warning, TEXT("[EQUIP] no AbilitySystemComponent on owner - GAS stats skipped"));
    }

    UE_LOG(LogEclipseEquip, Log, TEXT("[EQUIP] equipped %s (%s %s %s, status %.0f/hit) light=%.1f heavy=%.1f reach=%.0f"),
        *Item->DisplayName.ToString(),
        *StaticEnum<ERarityTier>()->GetNameStringByValue(static_cast<int64>(Item->Rarity)),
        *StaticEnum<EWeaponType>()->GetNameStringByValue(static_cast<int64>(Item->WeaponFamily)),
        *EclipseEquipmentTags::AffinityName(Item->PrimaryAffinity),
        Item->StatusBuildupPerHit, Item->LightDamage, Item->HeavyDamage, Item->Reach);

    OnWeaponEquipped.Broadcast(Item);
    return true;
}

void UEclipseEquipmentComponent::UnequipWeapon()
{
    if (!EquippedWeapon)
    {
        return;
    }

    const FString OldName = EquippedWeapon->DisplayName.ToString();

    if (UAbilitySystemComponent* ASC = GetOwnerASC())
    {
        if (StatsHandle.IsValid())
        {
            ASC->RemoveActiveGameplayEffect(StatsHandle);
        }
        for (const FActiveGameplayEffectHandle& Handle : ExtraHandles)
        {
            if (Handle.IsValid())
            {
                ASC->RemoveActiveGameplayEffect(Handle);
            }
        }
    }
    StatsHandle.Invalidate();
    ExtraHandles.Reset();

    if (AEclipseCharacter* Char = GetOwnerChar())
    {
        if (UEclipseCombatComponent* Combat = Char->GetCombat())
        {
            Combat->LightDamage = BaseLightDamage;
            Combat->HeavyDamage = BaseHeavyDamage;
            Combat->AttackRange = BaseAttackRange;
        }
        Char->SetWeaponMesh(nullptr);
        if (Char->WeaponMesh)
        {
            Char->WeaponMesh->SetRelativeScale3D(BaseWeaponScale);
        }
    }

    EquippedWeapon = nullptr;
    UE_LOG(LogEclipseEquip, Log, TEXT("[EQUIP] unequipped %s (combat restored to light=%.1f heavy=%.1f reach=%.0f)"),
        *OldName, BaseLightDamage, BaseHeavyDamage, BaseAttackRange);
}

void UEclipseEquipmentComponent::CycleWeapon()
{
    const AEclipseCharacter* Char = GetOwnerChar();
    if (!Char || Char->DefaultLoadout.Num() == 0)
    {
        UE_LOG(LogEclipseEquip, Warning, TEXT("[EQUIP] cycle refused - empty loadout"));
        return;
    }
    if (const UEclipseCombatComponent* Combat = Char->GetCombat())
    {
        if (Combat->IsActing())
        {
            UE_LOG(LogEclipseEquip, Log, TEXT("[EQUIP] cycle refused - combat busy"));
            return;
        }
    }

    const int32 Num = Char->DefaultLoadout.Num();
    LoadoutIndex = (LoadoutIndex + 1 + Num) % Num;
    UE_LOG(LogEclipseEquip, Log, TEXT("[EQUIP] cycle -> slot %d/%d"), LoadoutIndex + 1, Num);
    EquipItem(Char->DefaultLoadout[LoadoutIndex]);
}

void UEclipseEquipmentComponent::HandleHit(AActor* Target)
{
    if (!Target || !EquippedWeapon || EquippedWeapon->StatusBuildupPerHit <= 0.0f)
    {
        return;
    }

    UAbilitySystemComponent* TargetASC = Target->FindComponentByClass<UAbilitySystemComponent>();
    if (!TargetASC)
    {
        return; // Floors, props: no status.
    }

    const EElementalAffinity Affinity = EquippedWeapon->PrimaryAffinity;
    const FString AffName = EclipseEquipmentTags::AffinityName(Affinity);
    const FGameplayTag StatusTag = EclipseEquipmentTags::StatusTagFor(Affinity);
    const FGameplayTag ImmuneTag = EclipseEquipmentTags::ImmunityTagFor(Affinity);

    if (TargetASC->HasMatchingGameplayTag(StatusTag))
    {
        UE_LOG(LogEclipseEquip, Log, TEXT("[STATUS] %s already active on %s - buildup ignored"), *AffName, *Target->GetName());
        return;
    }
    if (TargetASC->HasMatchingGameplayTag(ImmuneTag))
    {
        UE_LOG(LogEclipseEquip, Log, TEXT("[STATUS] %s immune to %s - buildup ignored"), *Target->GetName(), *AffName);
        return;
    }

    float Threshold = TargetASC->GetNumericAttribute(UEclipseAttributeSet::GetStatusThresholdAttribute());
    if (Threshold <= 0.0f)
    {
        Threshold = 100.0f;
    }
    const float NewBuildup = TargetASC->GetNumericAttribute(UEclipseAttributeSet::GetStatusBuildupAttribute()) + EquippedWeapon->StatusBuildupPerHit;

    if (NewBuildup < Threshold)
    {
        TargetASC->SetNumericAttributeBase(UEclipseAttributeSet::GetStatusBuildupAttribute(), NewBuildup);
        UE_LOG(LogEclipseEquip, Log, TEXT("[STATUS] buildup %s %.0f/%.0f on %s"), *AffName, NewBuildup, Threshold, *Target->GetName());
        return;
    }

    TargetASC->SetNumericAttributeBase(UEclipseAttributeSet::GetStatusBuildupAttribute(), 0.0f);

    UAbilitySystemComponent* SourceASC = GetOwnerASC();
    UAbilitySystemComponent* SpecMaker = SourceASC ? SourceASC : TargetASC;
    FGameplayEffectContextHandle Context = SpecMaker->MakeEffectContext();
    Context.AddSourceObject(EquippedWeapon);

    const float Duration = FMath::Max(0.1f, EquippedWeapon->StatusDuration);
    const float ImmuneDuration = Duration + FMath::Max(0.0f, EquippedWeapon->StatusImmunity);

    auto ApplyTimedTag = [&](const FGameplayTag& Tag, float Seconds)
    {
        FGameplayEffectSpecHandle Spec = SpecMaker->MakeOutgoingSpec(UEclipseGE_TimedTag::StaticClass(), 1.0f, Context);
        if (!Spec.IsValid())
        {
            return;
        }
        Spec.Data->SetDuration(Seconds, true);
        Spec.Data->DynamicGrantedTags.AddTag(Tag);
        if (SourceASC)
        {
            SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
        }
        else
        {
            TargetASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
        }
    };

    ApplyTimedTag(StatusTag, Duration);
    ApplyTimedTag(ImmuneTag, ImmuneDuration);

    UE_LOG(LogEclipseEquip, Log, TEXT("[STATUS] %s applied to %s for %.1fs (immune until %.1fs) tag=%d"),
        *AffName, *Target->GetName(), Duration, ImmuneDuration, TargetASC->HasMatchingGameplayTag(StatusTag) ? 1 : 0);
}
