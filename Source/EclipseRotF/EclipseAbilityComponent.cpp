#include "EclipseAbilityComponent.h"

#include "EclipseAbilityDefs.h"
#include "EclipseCombatComponent.h"
#include "Engine/DataTable.h"
#include "Engine/OverlapResult.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogEclipseAbilities, Log, All);

UEclipseAbilityComponent::UEclipseAbilityComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UEclipseAbilityComponent::BeginPlay()
{
    Super::BeginPlay();
    SetupAbilityTable();
    UE_LOG(LogEclipseAbilities, Log, TEXT("[ABILITY] Ability component ready with %d abilities (GCD %.1fs)"), Abilities.Num(), GCDDuration);
}

void UEclipseAbilityComponent::SetupAbilityTable()
{
    Abilities.Reset();

    // Built-in definition set, kept in sync with /Game/Data/DT_EclipseAbilities so the combat loop
    // works with no asset assigned. The data table wins as soon as one is loaded.
    auto AddFallback = [this](FName Id, const TCHAR* Name, float Cooldown, EResourceType CostType, float Cost,
        int32 Slot, const TCHAR* KeyLabel, bool bRequiresFreeAction, int32 RotationPriority)
    {
        FEclipseAbilityRuntime Entry;
        Entry.Id = Id;
        Entry.DisplayName = Name;
        Entry.Cooldown = Cooldown;
        Entry.CooldownRemaining = 0.0f;
        Entry.CostType = CostType;
        Entry.Cost = Cost;
        Entry.Slot = Slot;
        Entry.KeyLabel = KeyLabel;
        Entry.bRequiresFreeAction = bRequiresFreeAction;
        Entry.RotationPriority = RotationPriority;
        Abilities.Add(Entry);
    };

    if (!AbilityDefs)
    {
        AbilityDefs = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/DT_EclipseAbilities.DT_EclipseAbilities"));
    }

    if (AbilityDefs)
    {
        AbilityDefs->ForeachRow<FEclipseAbilityDef>(TEXT("EclipseSetupAbilityTable"),
            [this](const FName& RowName, const FEclipseAbilityDef& Def)
            {
                FEclipseAbilityRuntime Entry;
                Entry.Id = RowName;
                Entry.DisplayName = Def.DisplayName;
                Entry.Cooldown = Def.Cooldown;
                Entry.CooldownRemaining = 0.0f;
                Entry.CostType = Def.CostType;
                Entry.Cost = Def.Cost;
                Entry.Slot = Def.Slot;
                Entry.KeyLabel = Def.KeyLabel;
                Entry.bRequiresFreeAction = Def.bRequiresFreeAction;
                Entry.RotationPriority = Def.RotationPriority;
                Entry.Icon = Def.Icon;
                Abilities.Add(Entry);
            });
    }

    const bool bFromTable = Abilities.Num() > 0;

    if (!bFromTable)
    {
        AddFallback(TEXT("AetherBolt"), TEXT("Aether Bolt"), 3.0f, EResourceType::Mana, 15.0f, 0, TEXT("Q"), false, 60);
        AddFallback(TEXT("FractureStrike"), TEXT("Fracture Strike"), 5.0f, EResourceType::Aether, 20.0f, 1, TEXT("E"), true, 80);
        AddFallback(TEXT("WingShield"), TEXT("Wing Shield"), 12.0f, EResourceType::Stamina, 30.0f, 2, TEXT("1"), true, 40);
        AddFallback(TEXT("PhaseStep"), TEXT("Phase Step"), 8.0f, EResourceType::Mana, 25.0f, 3, TEXT("2"), false, 20);
        AddFallback(TEXT("AetherStorm"), TEXT("Aether Storm"), 60.0f, EResourceType::Mana, 50.0f, 4, TEXT("R"), true, 100);
        AddFallback(TEXT("FractureBlade"), TEXT("Fracture Blade"), 30.0f, EResourceType::Stamina, 40.0f, 5, TEXT("F"), true, 70);
    }

    // The bar walks slots left to right, so order the array the same way.
    Abilities.Sort([](const FEclipseAbilityRuntime& A, const FEclipseAbilityRuntime& B)
    {
        return A.Slot < B.Slot;
    });

    UE_LOG(LogEclipseAbilities, Log, TEXT("[ABILITY] %d abilities loaded from %s"),
        Abilities.Num(), bFromTable ? TEXT("data table") : TEXT("built-in fallback"));
}

void UEclipseAbilityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (GCDRemaining > 0.0f)
    {
        GCDRemaining = FMath::Max(0.0f, GCDRemaining - DeltaTime);
    }

    for (FEclipseAbilityRuntime& Ability : Abilities)
    {
        if (Ability.CooldownRemaining > 0.0f)
        {
            Ability.CooldownRemaining = FMath::Max(0.0f, Ability.CooldownRemaining - DeltaTime);
        }
    }
}

int32 UEclipseAbilityComponent::FindAbilityIndex(FName AbilityId) const
{
    for (int32 Index = 0; Index < Abilities.Num(); ++Index)
    {
        if (Abilities[Index].Id == AbilityId)
        {
            return Index;
        }
    }
    return INDEX_NONE;
}

FEclipseAbilityRuntime UEclipseAbilityComponent::GetAbility(FName AbilityId) const
{
    const int32 Index = FindAbilityIndex(AbilityId);
    return Index == INDEX_NONE ? FEclipseAbilityRuntime() : Abilities[Index];
}

float UEclipseAbilityComponent::GetCooldownRemaining(FName AbilityId) const
{
    const int32 Index = FindAbilityIndex(AbilityId);
    return Index == INDEX_NONE ? 0.0f : Abilities[Index].CooldownRemaining;
}

FEclipseAbilityRuntime UEclipseAbilityComponent::GetAbilityBySlot(int32 Slot) const
{
    for (const FEclipseAbilityRuntime& Ability : Abilities)
    {
        if (Ability.Slot == Slot)
        {
            return Ability;
        }
    }
    return FEclipseAbilityRuntime();
}

bool UEclipseAbilityComponent::CanActivateAbility(FName AbilityId, FString& OutReason) const
{
    OutReason.Reset();

    const int32 Index = FindAbilityIndex(AbilityId);
    if (Index == INDEX_NONE)
    {
        OutReason = TEXT("unknown ability");
        return false;
    }

    const FEclipseAbilityRuntime& Ability = Abilities[Index];
    AEclipseCharacter* OwnerChar = Cast<AEclipseCharacter>(GetOwner());
    if (!OwnerChar)
    {
        OutReason = TEXT("no owner");
        return false;
    }

    if (GCDRemaining > 0.0f)
    {
        OutReason = FString::Printf(TEXT("GCD %.2fs"), GCDRemaining);
        return false;
    }
    if (Ability.CooldownRemaining > 0.0f)
    {
        OutReason = FString::Printf(TEXT("%.1fs cooldown"), Ability.CooldownRemaining);
        return false;
    }
    if (!OwnerChar->HasResource(Ability.CostType, Ability.Cost))
    {
        const FString ResourceName = StaticEnum<EResourceType>()->GetNameStringByValue((int64)Ability.CostType);
        OutReason = FString::Printf(TEXT("needs %.0f %s"), Ability.Cost, *ResourceName);
        return false;
    }

    // The ultimate additionally requires a full Aether bar.
    if (AbilityId == TEXT("AetherStorm") && !OwnerChar->HasResource(EResourceType::Aether, 100.0f))
    {
        OutReason = TEXT("Aether not full");
        return false;
    }

    // Commitment gate. An ability that needs a free action cannot be used while the combat state
    // machine owns the body; the escape abilities are still refused while staggered.
    if (const UEclipseCombatComponent* Combat = OwnerChar->GetCombat())
    {
        const bool bBlocked = Ability.bRequiresFreeAction
            ? Combat->IsActing()
            : (Combat->State == ECombatState::Stagger);
        if (bBlocked)
        {
            OutReason = TEXT("committed");
            return false;
        }
    }

    return true;
}

TArray<FName> UEclipseAbilityComponent::GetReadyAbilities() const
{
    TArray<FName> Ready;
    for (const FEclipseAbilityRuntime& Ability : Abilities)
    {
        FString Blocker;
        if (CanActivateAbility(Ability.Id, Blocker))
        {
            Ready.Add(Ability.Id);
        }
    }
    return Ready;
}

FName UEclipseAbilityComponent::GetRotationSuggestion() const
{
    FName Best = NAME_None;
    int32 BestPriority = MIN_int32;

    for (const FEclipseAbilityRuntime& Ability : Abilities)
    {
        if (Ability.RotationPriority <= BestPriority)
        {
            continue;
        }

        FString Blocker;
        if (CanActivateAbility(Ability.Id, Blocker))
        {
            BestPriority = Ability.RotationPriority;
            Best = Ability.Id;
        }
    }

    return Best;
}

bool UEclipseAbilityComponent::TryActivateRotation()
{
    const FName Next = GetRotationSuggestion();
    if (Next.IsNone())
    {
        UE_LOG(LogEclipseAbilities, Log, TEXT("[ROTATION] nothing ready"));
        return false;
    }

    UE_LOG(LogEclipseAbilities, Log, TEXT("[ROTATION] activating %s"), *Next.ToString());
    return ActivateAbility(Next);
}

bool UEclipseAbilityComponent::SpendCost(const FEclipseAbilityRuntime& Ability)
{
    AEclipseCharacter* OwnerChar = Cast<AEclipseCharacter>(GetOwner());
    if (!OwnerChar)
    {
        return false;
    }
    return OwnerChar->TrySpendResource(Ability.CostType, Ability.Cost);
}

bool UEclipseAbilityComponent::ActivateAbility(FName AbilityId)
{
    const int32 Index = FindAbilityIndex(AbilityId);
    if (Index == INDEX_NONE)
    {
        return false;
    }

    FEclipseAbilityRuntime& Ability = Abilities[Index];
    AEclipseCharacter* OwnerChar = Cast<AEclipseCharacter>(GetOwner());
    if (!OwnerChar)
    {
        return false;
    }

    FString Blocker;
    if (!CanActivateAbility(AbilityId, Blocker))
    {
        UE_LOG(LogEclipseAbilities, Log, TEXT("[ABILITY] %s refused - %s"), *Ability.DisplayName, *Blocker);
        return false;
    }

    if (!SpendCost(Ability))
    {
        return false;
    }
    if (AbilityId == TEXT("AetherStorm"))
    {
        OwnerChar->TrySpendResource(EResourceType::Aether, 100.0f);
    }

    Ability.CooldownRemaining = Ability.Cooldown;
    GCDRemaining = GCDDuration;
    OwnerChar->AddResourceValue(EResourceType::Aether, 15.0f);

    const FVector Forward = OwnerChar->GetActorForwardVector();
    const FVector Origin = OwnerChar->GetActorLocation();
    UEclipseCombatComponent* Combat = OwnerChar->GetCombat();

    UE_LOG(LogEclipseAbilities, Log, TEXT("[ABILITY] %s activated (cost %.0f, cooldown %.1fs)"), *Ability.DisplayName, Ability.Cost, Ability.Cooldown);

    if (AbilityId == TEXT("AetherBolt"))
    {
        const FVector Start = Origin + FVector(0, 0, 50.0f);
        const FVector End = Start + Forward * 2000.0f;
        FHitResult Hit;
        FCollisionQueryParams Params(SCENE_QUERY_STAT(AetherBolt), false, OwnerChar);
        DrawDebugLine(GetWorld(), Start, End, FColor(145, 70, 255), false, 0.6f, 0, 4.0f);
        if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Pawn, Params))
        {
            const float Damage = BaseDamage + 0.5f * BaseDamage * 2.0f;
            const float Applied = UGameplayStatics::ApplyDamage(Hit.GetActor(), Damage, OwnerChar->GetInstigatorController(), OwnerChar, UDamageType::StaticClass());
            DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 90.0f, 12, FColor(255, 215, 0), false, 0.6f);
            UE_LOG(LogEclipseAbilities, Log, TEXT("[DAMAGE] attacker=%s target=%s amount=%.1f type=ability"), *OwnerChar->GetName(), *Hit.GetActor()->GetName(), Applied);
        }
    }
    else if (AbilityId == TEXT("FractureStrike"))
    {
        const float ComboBonus = (Combat && Combat->ComboWindowRemaining > 0.0f) ? 1.5f : 1.0f;
        const float Damage = (50.0f + WeaponDamage) * ComboBonus;
        ApplyAreaDamage(Origin + Forward * 250.0f, 500.0f, Damage, TEXT("ability"));
        DrawDebugCone(GetWorld(), Origin, Forward, 500.0f, FMath::DegreesToRadians(45.0f), FMath::DegreesToRadians(45.0f), 12, FColor(255, 215, 0), false, 0.5f);
    }
    else if (AbilityId == TEXT("WingShield"))
    {
        bWingShieldActive = true;
        if (UCharacterMovementComponent* Move = OwnerChar->GetCharacterMovement())
        {
            Move->DisableMovement();
        }
        DrawDebugSphere(GetWorld(), Origin + Forward * 120.0f, 160.0f, 16, FColor(145, 70, 255), false, 3.0f);
        GetWorld()->GetTimerManager().SetTimer(WingShieldHandle, this, &UEclipseAbilityComponent::EndWingShield, 3.0f, false);
    }
    else if (AbilityId == TEXT("PhaseStep"))
    {
        const FVector Destination = Origin + Forward * 800.0f;
        OwnerChar->SetActorLocation(Destination, false, nullptr, ETeleportType::TeleportPhysics);
        DrawDebugSphere(GetWorld(), Origin, 80.0f, 10, FColor(145, 70, 255), false, 0.5f);
        DrawDebugSphere(GetWorld(), Destination, 80.0f, 10, FColor(255, 215, 0), false, 0.5f);
        if (Combat)
        {
            Combat->bDodgeIFrameActive = true;
        }
        FTimerHandle PhaseHandle;
        FTimerDelegate PhaseDelegate = FTimerDelegate::CreateWeakLambda(this, [Combat]()
        {
            if (Combat)
            {
                Combat->bDodgeIFrameActive = false;
            }
        });
        GetWorld()->GetTimerManager().SetTimer(PhaseHandle, PhaseDelegate, 0.2f, false);
    }
    else if (AbilityId == TEXT("AetherStorm"))
    {
        bChannelingAetherStorm = true;
        if (UCharacterMovementComponent* Move = OwnerChar->GetCharacterMovement())
        {
            Move->DisableMovement();
        }
        DrawDebugSphere(GetWorld(), Origin, 1500.0f, 24, FColor(145, 70, 255), false, 2.0f);
        GetWorld()->GetTimerManager().SetTimer(StormChannelHandle, this, &UEclipseAbilityComponent::FinishAetherStorm, 2.0f, false);
    }
    else if (AbilityId == TEXT("FractureBlade"))
    {
        if (Combat)
        {
            Combat->SetDamageBuff(1.5f, 10.0f);
        }
    }

    return true;
}

void UEclipseAbilityComponent::ApplyAreaDamage(const FVector& Origin, float Radius, float Damage, const FString& Label)
{
    UWorld* World = GetWorld();
    AActor* Owner = GetOwner();
    if (!World || !Owner)
    {
        return;
    }

    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(EclipseAbilityArea), false, Owner);
    World->OverlapMultiByChannel(Overlaps, Origin, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(Radius), Params);
    DrawDebugSphere(World, Origin, Radius, 16, FColor(145, 70, 255), false, 0.5f);

    TSet<AActor*> AlreadyHit;
    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* Target = Overlap.GetActor();
        if (!Target || Target == Owner || Target->IsA(AEclipseCharacter::StaticClass()) || AlreadyHit.Contains(Target))
        {
            continue;
        }
        AlreadyHit.Add(Target);
        const float Applied = UGameplayStatics::ApplyDamage(Target, Damage, Owner->GetInstigatorController(), Owner, UDamageType::StaticClass());
        UE_LOG(LogEclipseAbilities, Log, TEXT("[DAMAGE] attacker=%s target=%s amount=%.1f type=%s"), *Owner->GetName(), *Target->GetName(), Applied, *Label);
    }
}

void UEclipseAbilityComponent::FinishAetherStorm()
{
    AEclipseCharacter* OwnerChar = Cast<AEclipseCharacter>(GetOwner());
    if (!OwnerChar)
    {
        return;
    }

    bChannelingAetherStorm = false;
    if (UCharacterMovementComponent* Move = OwnerChar->GetCharacterMovement())
    {
        Move->SetMovementMode(MOVE_Walking);
    }

    const float Damage = 200.0f + 2.0f * WeaponDamage;
    ApplyAreaDamage(OwnerChar->GetActorLocation(), 1500.0f, Damage, TEXT("ability"));
    UE_LOG(LogEclipseAbilities, Log, TEXT("[ABILITY] Aether Storm released for %.1f damage"), Damage);
}

void UEclipseAbilityComponent::EndWingShield()
{
    bWingShieldActive = false;
    if (AEclipseCharacter* OwnerChar = Cast<AEclipseCharacter>(GetOwner()))
    {
        if (UCharacterMovementComponent* Move = OwnerChar->GetCharacterMovement())
        {
            Move->SetMovementMode(MOVE_Walking);
        }
    }
    UE_LOG(LogEclipseAbilities, Log, TEXT("[ABILITY] Wing Shield expired"));
}
