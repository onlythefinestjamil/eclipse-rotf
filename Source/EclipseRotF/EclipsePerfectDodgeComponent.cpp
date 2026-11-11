#include "EclipsePerfectDodgeComponent.h"

#include "EclipseCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogEclipseDodge, Log, All);

UEclipsePerfectDodgeComponent::UEclipsePerfectDodgeComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    // Real-time bookkeeping, so the slow motion does not slow down its own timer.
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UEclipsePerfectDodgeComponent::BeginPlay()
{
    Super::BeginPlay();

    if (const UWorld* World = GetWorld())
    {
        LastRealTimeSeconds = World->GetRealTimeSeconds();
    }

    // Ghosts need a translucent material to dissolve; without one they can only shrink.
    if (!AfterImageMaterial)
    {
        AfterImageMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Generated_Materials/M_AfterImage_Ghost.M_AfterImage_Ghost"));
        if (!AfterImageMaterial)
        {
            UE_LOG(LogEclipseDodge, Warning, TEXT("[PERFECT DODGE] Afterimage material missing - ghosts will fade by shrinking only"));
        }
    }

    UE_LOG(LogEclipseDodge, Log, TEXT("[PERFECT DODGE] Component ready (slowmo=%.2f for %.2fs, afterimages=%d, material=%s)"),
        SlowMoTimeDilation, SlowMoDuration, AfterImageCount, AfterImageMaterial ? TEXT("translucent") : TEXT("none"));
}

void UEclipsePerfectDodgeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ClearAfterImages();

    // Never leave the world dilated because the pawn went away mid-effect.
    if (bDilationOwned)
    {
        ApplyDilation(1.0f);
    }

    Super::EndPlay(EndPlayReason);
}

float UEclipsePerfectDodgeComponent::GetRealDeltaSeconds()
{
    const UWorld* World = GetWorld();
    if (!World)
    {
        return 0.0f;
    }

    const double Now = World->GetRealTimeSeconds();
    const float Delta = static_cast<float>(Now - LastRealTimeSeconds);
    LastRealTimeSeconds = Now;
    return FMath::Max(Delta, 0.0f);
}

void UEclipsePerfectDodgeComponent::ApplyDilation(float Value)
{
    const float Clamped = FMath::Clamp(Value, 0.01f, 20.0f);
    UGameplayStatics::SetGlobalTimeDilation(this, Clamped);
    bDilationOwned = !FMath::IsNearlyEqual(Clamped, 1.0f);
}

void UEclipsePerfectDodgeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // DeltaTime here is already dilated by our own slow motion, so track wall-clock time instead.
    const float RealDt = GetRealDeltaSeconds();

    if (bSlowMoActive)
    {
        SlowMoRemaining -= RealDt;

        if (SlowMoRemaining <= 0.0f)
        {
            bSlowMoActive = false;
            SlowMoRemaining = 0.0f;
            ApplyDilation(1.0f);
            UE_LOG(LogEclipseDodge, Log, TEXT("[PERFECT DODGE] Slow motion ended"));
        }
        else if (SlowMoRampOutSeconds > 0.0f && SlowMoRemaining < SlowMoRampOutSeconds)
        {
            // Ease back to normal speed over the final stretch instead of snapping.
            const float Alpha = SlowMoRemaining / SlowMoRampOutSeconds;
            ApplyDilation(FMath::Lerp(1.0f, SlowMoTimeDilation, Alpha));
        }
    }

    if (AfterImages.Num() > 0)
    {
        UpdateAfterImages(RealDt);
    }
}

bool UEclipsePerfectDodgeComponent::TriggerPerfectDodge(AActor* DodgedSource)
{
    ++PerfectDodgeCount;

    const FString SourceName = DodgedSource ? DodgedSource->GetName() : TEXT("Unknown");

    // 1) Slow motion.
    if (bEnableSlowMo && SlowMoDuration > 0.0f)
    {
        bSlowMoActive = true;
        SlowMoRemaining = SlowMoDuration;
        ApplyDilation(SlowMoTimeDilation);
    }

    // 2) Afterimage trail, laid out backwards along the dodge.
    if (bEnableAfterImages && AfterImageCount > 0)
    {
        FVector TrailDirection = FVector::ZeroVector;
        if (const AActor* Owner = GetOwner())
        {
            TrailDirection = -Owner->GetVelocity().GetSafeNormal();
            if (TrailDirection.IsNearlyZero())
            {
                TrailDirection = -Owner->GetActorForwardVector();
            }
        }
        SpawnAfterImageBurst(TrailDirection);
    }

    // 3) Reward.
    if (bGrantAetherOnPerfectDodge && AetherReward > 0.0f)
    {
        if (AEclipseCharacter* MutableChar = Cast<AEclipseCharacter>(GetOwner()))
        {
            MutableChar->AddResourceValue(EResourceType::Aether, AetherReward);
        }
    }

    UE_LOG(LogEclipseDodge, Log,
        TEXT("[PERFECT DODGE] dodged=%s count=%d slowmo=%s(%.2fs @ %.2fx) afterimages=%d aether=+%.0f"),
        *SourceName,
        PerfectDodgeCount,
        bSlowMoActive ? TEXT("ON") : TEXT("OFF"),
        SlowMoRemaining,
        SlowMoTimeDilation,
        AfterImages.Num(),
        bGrantAetherOnPerfectDodge ? AetherReward : 0.0f);

    OnPerfectDodge.Broadcast(DodgedSource);

    return true;
}

void UEclipsePerfectDodgeComponent::SpawnAfterImageBurst(const FVector& TrailDirection)
{
    const AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    // Anchor on the hero MESH, not the capsule. The mesh carries the fitted scale and its own offset,
    // so a ghost placed at the actor origin would float off the real silhouette.
    FVector Origin = Owner->GetActorLocation();
    FRotator Rotation = Owner->GetActorRotation();
    if (const AEclipseCharacter* OwnerChar = Cast<AEclipseCharacter>(Owner))
    {
        if (OwnerChar->HeroMesh)
        {
            Origin = OwnerChar->HeroMesh->GetComponentLocation();
            Rotation = OwnerChar->HeroMesh->GetComponentRotation();
        }
    }

    const FVector Dir = TrailDirection.IsNearlyZero() ? -Owner->GetActorForwardVector() : TrailDirection.GetSafeNormal();

    for (int32 Index = 0; Index < AfterImageCount; ++Index)
    {
        // Index 0 sits on the hero and lives longest; each further ghost is older and fades sooner.
        const float SpacingAlpha = (AfterImageCount > 1) ? static_cast<float>(Index) / static_cast<float>(AfterImageCount - 1) : 0.0f;
        const FVector Location = Origin + Dir * (AfterImageSpacing * static_cast<float>(Index));
        const float Lifetime = FMath::Lerp(AfterImageLifetime, AfterImageLifetime * 0.4f, SpacingAlpha);

        TArray<TObjectPtr<UMaterialInstanceDynamic>> Mids;
        if (AActor* Ghost = SpawnSingleAfterImage(Location, Rotation, Lifetime, Mids))
        {
            FEclipseAfterImageInstance& Entry = AfterImages.AddDefaulted_GetRef();
            Entry.Actor = Ghost;
            Entry.Mids = MoveTemp(Mids);
            Entry.Lifetime = Lifetime;
            Entry.Remaining = Lifetime;
            Entry.BaseScale = Ghost->GetActorScale3D();
        }
    }

    ActiveAfterImageCount = AfterImages.Num();
}

AActor* UEclipsePerfectDodgeComponent::SpawnSingleAfterImage(const FVector& Location, const FRotator& Rotation, float Lifetime, TArray<TObjectPtr<UMaterialInstanceDynamic>>& OutMids)
{
    UWorld* World = GetWorld();
    AEclipseCharacter* OwnerChar = Cast<AEclipseCharacter>(GetOwner());
    if (!World || !OwnerChar || !OwnerChar->HeroMesh)
    {
        return nullptr;
    }

    UStaticMesh* SourceMesh = OwnerChar->HeroMesh->GetStaticMesh();
    if (!SourceMesh)
    {
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    SpawnParams.Owner = OwnerChar;
    SpawnParams.ObjectFlags |= RF_Transient;

    AActor* Ghost = World->SpawnActor<AActor>(AActor::StaticClass(), Location, Rotation, SpawnParams);
    if (!Ghost)
    {
        return nullptr;
    }

    UStaticMeshComponent* GhostMesh = NewObject<UStaticMeshComponent>(Ghost, TEXT("AfterImageMesh"));
    if (!GhostMesh)
    {
        Ghost->Destroy();
        return nullptr;
    }

    GhostMesh->SetMobility(EComponentMobility::Movable);
    Ghost->SetRootComponent(GhostMesh);
    GhostMesh->RegisterComponent();

    // A bare AActor has no root component at the moment SpawnActor applies the spawn transform, so the
    // requested Location was silently dropped and every ghost ended up at the world origin. Now that the
    // mesh IS the root, the actor can actually be moved into place.
    Ghost->SetActorLocationAndRotation(Location, Rotation);

    GhostMesh->SetStaticMesh(SourceMesh);
    GhostMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GhostMesh->SetGenerateOverlapEvents(false);
    GhostMesh->SetCastShadow(false);
    GhostMesh->SetCanEverAffectNavigation(false);

    // Match the hero's fitted scale (the mesh is auto-scaled at BeginPlay).
    Ghost->SetActorScale3D(OwnerChar->HeroMesh->GetComponentScale());

    // Materials: the ghost must wear the translucent afterimage material. CreateAndSetMaterialInstanceDynamic
    // would parent the MID to whatever the slot already holds - the hero's OPAQUE material - so the fade
    // scalar would drive a parameter that does not exist and the ghost would stay a solid clone.
    if (AfterImageMaterial)
    {
        const int32 NumSlots = FMath::Max(GhostMesh->GetNumMaterials(), 1);
        for (int32 Slot = 0; Slot < NumSlots; ++Slot)
        {
            if (UMaterialInstanceDynamic* Mid = GhostMesh->CreateDynamicMaterialInstance(Slot, AfterImageMaterial))
            {
                OutMids.Add(Mid);
            }
        }

        // If no slot accepted a MID, at least put the base material on so the ghost is still violet translucent.
        if (OutMids.Num() == 0)
        {
            GhostMesh->SetMaterial(0, AfterImageMaterial);
            UE_LOG(LogEclipseDodge, Warning, TEXT("[PERFECT DODGE] Could not create a ghost MID - afterimage opacity will not fade"));
        }
    }
    else
    {
        const int32 NumSlots = GhostMesh->GetNumMaterials();
        for (int32 Slot = 0; Slot < NumSlots; ++Slot)
        {
            if (UMaterialInterface* SourceMat = OwnerChar->HeroMesh->GetMaterial(Slot))
            {
                GhostMesh->SetMaterial(Slot, SourceMat);
            }
        }
    }

    return Ghost;
}

void UEclipsePerfectDodgeComponent::UpdateAfterImages(float RealDeltaSeconds)
{
    for (int32 Index = AfterImages.Num() - 1; Index >= 0; --Index)
    {
        FEclipseAfterImageInstance& Entry = AfterImages[Index];

        if (!Entry.Actor)
        {
            AfterImages.RemoveAtSwap(Index);
            continue;
        }

        Entry.Remaining -= RealDeltaSeconds;

        if (Entry.Remaining <= 0.0f)
        {
            Entry.Actor->Destroy();
            AfterImages.RemoveAtSwap(Index);
            continue;
        }

        const float Alpha = FMath::Clamp(Entry.Remaining / FMath::Max(Entry.Lifetime, KINDA_SMALL_NUMBER), 0.0f, 1.0f);

        // Drift upward while dissolving, so the trail reads as rising smoke rather than a freeze-frame.
        if (AfterImageRiseSpeed > 0.0f)
        {
            Entry.Actor->AddActorWorldOffset(FVector(0.0f, 0.0f, AfterImageRiseSpeed * RealDeltaSeconds), false);
        }

        if (Entry.Mids.Num() > 0)
        {
            // Translucent ghost: hold full size and dissolve out through opacity.
            const float Opacity = Alpha * AfterImageStartOpacity;
            for (UMaterialInstanceDynamic* Mid : Entry.Mids)
            {
                if (Mid)
                {
                    Mid->SetScalarParameterValue(OpacityParameterName, Opacity);
                }
            }
        }
        else
        {
            // No translucent material available: shrinking is the only way to make it disappear.
            const float ScaleAlpha = FMath::Lerp(AfterImageEndScale, 1.0f, Alpha);
            Entry.Actor->SetActorScale3D(Entry.BaseScale * ScaleAlpha);
        }
    }

    ActiveAfterImageCount = AfterImages.Num();
}

void UEclipsePerfectDodgeComponent::ClearAfterImages()
{
    for (FEclipseAfterImageInstance& Entry : AfterImages)
    {
        if (Entry.Actor)
        {
            Entry.Actor->Destroy();
        }
    }
    AfterImages.Reset();
    ActiveAfterImageCount = 0;
}
