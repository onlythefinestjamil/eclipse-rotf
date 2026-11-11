#include "Aura/Slices/AetherbornComboTiming/AetherbornComboTimingArena.h"

#include "Aura/Slices/AetherbornComboTiming/AetherbornComboTimingFx.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Font.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Math/RotationMatrix.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "UObject/ConstructorHelpers.h"

namespace AetherbornComboTimingArenaConstants
{
	/** The pillar stands this far in front of the arena origin (3 m from world.js, in cm). */
	constexpr float PillarBaseOffsetX = 240.f;

	/** Local height of the pillar's glowing core band above the pivot (1.62 m). */
	constexpr float PillarCoreHeightZ = 162.f;

	/** UE light intensities are physical (lux / candela), the Slice's are unitless; this
	 *  keeps the relative balance of key / rim / fill / core-light exactly as authored. */
	constexpr float CoreLightUnit = 30.f;
}

AAetherbornComboTimingArena::AAetherbornComboTimingArena()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneFinder(TEXT("/Engine/BasicShapes/Plane.Plane"));

	// ------------------------------------------------------------------ backdrop
	// Inverted sphere standing in for the Slice's sky shader. The negative XY scale
	// mirrors the mesh outwards so we see its inside (the renderer flips winding for
	// mirrored transforms), which is the usual engine sky-dome trick.
	Dome = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Dome"));
	Dome->SetupAttachment(SceneRoot);
	Dome->SetMobility(EComponentMobility::Movable);
	Dome->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Dome->SetCastShadow(false);
	Dome->SetGenerateOverlapEvents(false);
	if (UStaticMesh* SphereMesh = SphereFinder.Object)
	{
		Dome->SetStaticMesh(SphereMesh);
	}
	Dome->SetRelativeScale3D(FVector(-80.f, -80.f, 80.f)); // radius 4000

	// ------------------------------------------------------------------ ground
	// Dark base plate, tinted from the palette at BeginPlay; the grid plane is drawn
	// just above it with GridMaterial and the Slice's transparent grid shader.
	Ground = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Ground"));
	Ground->SetupAttachment(SceneRoot);
	Ground->SetMobility(EComponentMobility::Movable);
	Ground->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Ground->SetCastShadow(false);
	if (UStaticMesh* PlaneMesh = PlaneFinder.Object)
	{
		Ground->SetStaticMesh(PlaneMesh);
	}
	Ground->SetRelativeLocation(FVector(0.f, 0.f, -2.f));
	Ground->SetRelativeScale3D(FVector(62.f, 62.f, 1.f)); // 6200 x 6200

	GridPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GridPlane"));
	GridPlane->SetupAttachment(SceneRoot);
	GridPlane->SetMobility(EComponentMobility::Movable);
	GridPlane->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GridPlane->SetCastShadow(false);
	if (UStaticMesh* PlaneMesh = PlaneFinder.Object)
	{
		GridPlane->SetStaticMesh(PlaneMesh);
	}
	GridPlane->SetRelativeLocation(FVector(0.f, 0.f, 0.4f));
	GridPlane->SetRelativeScale3D(FVector(60.f, 60.f, 1.f)); // 6000 x 6000

	// ------------------------------------------------------------------ pillar
	PillarPivot = CreateDefaultSubobject<USceneComponent>(TEXT("PillarPivot"));
	PillarPivot->SetupAttachment(SceneRoot);
	PillarPivot->SetMobility(EComponentMobility::Movable);
	PillarPivot->SetRelativeLocation(FVector(AetherbornComboTimingArenaConstants::PillarBaseOffsetX, 0.f, 0.f));

	BuildPillar(CylinderFinder.Object);

	CoreLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("CoreLight"));
	CoreLight->SetupAttachment(PillarPivot);
	CoreLight->SetMobility(EComponentMobility::Movable);
	CoreLight->SetRelativeLocation(FVector(0.f, 0.f, AetherbornComboTimingArenaConstants::PillarCoreHeightZ));
	CoreLight->SetAttenuationRadius(700.f);
	CoreLight->SetIntensity(AetherbornComboTimingArenaConstants::CoreLightUnit * 4.5f);
	CoreLight->SetCastShadows(false);

	// ------------------------------------------------------------------ arena lights
	auto MakeDirectional = [this](const TCHAR* Name, const FVector& FromPosition, float Intensity, bool bCastShadows)
	{
		UDirectionalLightComponent* Light = CreateDefaultSubobject<UDirectionalLightComponent>(Name);
		Light->SetupAttachment(SceneRoot);
		Light->SetMobility(EComponentMobility::Movable);
		Light->SetRelativeLocation(FromPosition);
		// Point the light from its Slice position back at the arena origin.
		Light->SetRelativeRotation(FRotationMatrix::MakeFromX((-FromPosition).GetSafeNormal()).Rotator());
		Light->SetIntensity(Intensity);
		Light->SetCastShadows(bCastShadows);
		return Light;
	};

	// three.js position (x, y-up, z) -> engine position (x, z, y); intensities as authored.
	KeyLight = MakeDirectional(TEXT("KeyLight"), FVector(-340.f, -320.f, 300.f), 2.0f, true);
	RimLight = MakeDirectional(TEXT("RimLight"), FVector(260.f, 320.f, 340.f), 3.4f, false);
	FillLight = MakeDirectional(TEXT("FillLight"), FVector(260.f, -240.f, 120.f), 0.5f, false);

	// ------------------------------------------------------------------ timing ring
	auto MakeRingMesh = [this](const TCHAR* Name, float WidthCm)
	{
		UStaticMeshComponent* Mesh = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Mesh->SetupAttachment(SceneRoot);
		Mesh->SetMobility(EComponentMobility::Movable);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Mesh->SetCastShadow(false);
		Mesh->SetGenerateOverlapEvents(false);
		Mesh->SetRelativeLocation(FVector(0.f, 0.f, 3.f));
		Mesh->SetRelativeScale3D(FVector(WidthCm / 100.f, WidthCm / 100.f, 1.f));
		return Mesh;
	};

	RingMesh = MakeRingMesh(TEXT("RingMesh"), 220.f);
	HaloMesh = MakeRingMesh(TEXT("HaloMesh"), 270.f);
	RingMesh->SetStaticMesh(PlaneFinder.Object);
	HaloMesh->SetStaticMesh(PlaneFinder.Object);
	HaloMesh->SetTranslucentSortPriority(4);
	RingMesh->SetTranslucentSortPriority(3);

	// ------------------------------------------------------------------ dust
	DustComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Dust"));
	DustComponent->SetupAttachment(SceneRoot);
	DustComponent->SetMobility(EComponentMobility::Movable);
	DustComponent->bAutoActivate = false;

	// ------------------------------------------------------------------ vignette
	Vignette = CreateDefaultSubobject<UPostProcessComponent>(TEXT("Vignette"));
	Vignette->SetupAttachment(SceneRoot);
	Vignette->bUnbound = true;
	Vignette->Priority = 0.f;
	Vignette->Settings.bOverride_VignetteIntensity = true;
	Vignette->Settings.VignetteIntensity = 0.55f;
}

void AAetherbornComboTimingArena::BuildPillar(UStaticMesh* CylinderMesh)
{
	// Stacked cylinders, straight out of world.js. Radii are in metres; the engine
	// cylinder mesh is 1 m across and 1 m tall, so scale XY = rTop + rBottom and
	// scale Z = height.
	if (!CylinderMesh)
	{
		WarnOnce(TEXT("PillarMesh"), TEXT("basic shape cylinder missing - the pillar cannot be built"));
		return;
	}

	// base plate, plinth, shaft
	AddPillarPart(CylinderMesh, 0.62f, 0.72f, 0.10f, 0.05f, PillarTrim);
	AddPillarPart(CylinderMesh, 0.46f, 0.54f, 0.14f, 0.16f, PillarStone);
	AddPillarPart(CylinderMesh, 0.30f, 0.36f, 2.10f, 1.21f, PillarStone);

	// trim bands
	AddPillarPart(CylinderMesh, 0.365f, 0.36f, 0.07f, 0.55f, PillarTrim);
	AddPillarPart(CylinderMesh, 0.345f, 0.35f, 0.07f, 1.05f, PillarTrim);
	AddPillarPart(CylinderMesh, 0.325f, 0.33f, 0.09f, 2.10f, PillarTrim);

	// glowing core band
	AddPillarPart(CylinderMesh, 0.322f, 0.322f, 0.26f, 1.62f, PillarCore);

	// cap + crown
	AddPillarPart(CylinderMesh, 0.34f, 0.30f, 0.14f, 2.33f, PillarTrim);
	AddPillarPart(CylinderMesh, 0.10f, 0.22f, 0.20f, 2.49f, PillarStone);
}

void AAetherbornComboTimingArena::AddPillarPart(UStaticMesh* Mesh, float RadiusTop, float RadiusBottom, float HeightM,
                                               float CenterZM, TArray<TObjectPtr<UStaticMeshComponent>>& Group,
                                               bool bBottomAligned)
{
	if (!Mesh || !PillarPivot)
	{
		return;
	}

	// Deterministic subobject names: the build order is fixed, so the running count is too.
	const int32 Index = PillarStone.Num() + PillarTrim.Num() + PillarCore.Num();
	const FName PartName = *FString::Printf(TEXT("PillarPart_%d"), Index);

	UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(PartName);
	Part->SetMobility(EComponentMobility::Movable);
	Part->SetupAttachment(PillarPivot);
	Part->SetStaticMesh(Mesh);
	Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Part->SetGenerateOverlapEvents(false);

	const float CenterZ = bBottomAligned ? (CenterZM + HeightM * 0.5f) : CenterZM;
	Part->SetRelativeLocation(FVector(0.f, 0.f, CenterZ * 100.f));
	Part->SetRelativeScale3D(FVector(RadiusTop + RadiusBottom, RadiusTop + RadiusBottom, HeightM));

	Group.Add(Part);
}

void AAetherbornComboTimingArena::BeginPlay()
{
	Super::BeginPlay();

	CreateDynamicMaterials();

	if (DustComponent)
	{
		if (DustNiagara)
		{
			DustComponent->SetAsset(DustNiagara);
			DustComponent->Activate(true);
		}
		else
		{
			WarnOnce(TEXT("DustNiagara"), TEXT("no DustNiagara assigned - the arena dust is skipped"));
		}
	}

	if (!Fx)
	{
		Fx = NewObject<UAetherbornComboTimingFx>(this);
	}
	if (Fx)
	{
		Fx->Initialize(this, ImpactNiagara, DamageFont);
	}

	ResetArena();
}

UMaterialInstanceDynamic* AAetherbornComboTimingArena::MakeTintedMID(UMaterialInterface* Base, const FLinearColor& Color,
                                                                    const TCHAR* Purpose)
{
	if (!Base)
	{
		WarnOnce(FName(*FString::Printf(TEXT("MID_%s"), Purpose)),
		         FString::Printf(TEXT("no base material for the %s - it keeps the default grey look"), Purpose));
		return nullptr;
	}

	UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Base, this);
	if (MID)
	{
		// The engine basic-shape material carries a "Color" vector parameter; setting the
		// common alternates as well is harmless when a parameter does not exist.
		MID->SetVectorParameterValue(TEXT("Color"), Color);
		MID->SetVectorParameterValue(TEXT("BaseColor"), Color);
	}
	return MID;
}

void AAetherbornComboTimingArena::CreateDynamicMaterials()
{
	// Authored materials first: they carry the Slice's shaders.
	if (Dome && DomeMaterial)
	{
		Dome->SetMaterial(0, DomeMaterial);
	}
	else if (!DomeMaterial)
	{
		WarnOnce(TEXT("DomeMaterial"), TEXT("no DomeMaterial assigned - the backdrop keeps the default material"));
	}

	if (GridPlane)
	{
		if (GridMaterial)
		{
			GridMID = UMaterialInstanceDynamic::Create(GridMaterial, this);
			UMaterialInterface* GridApplied = GridMID ? GridMID.Get() : GridMaterial.Get();
			GridPlane->SetMaterial(0, GridApplied);
		}
		else
		{
			WarnOnce(TEXT("GridMaterial"), TEXT("no GridMaterial assigned - the floor grid is skipped"));
		}
	}

	if (RingMesh)
	{
		if (RingMaterial)
		{
			RingMID = UMaterialInstanceDynamic::Create(RingMaterial, this);
			UMaterialInterface* RingApplied = RingMID ? RingMID.Get() : RingMaterial.Get();
			RingMesh->SetMaterial(0, RingApplied);
		}
		else
		{
			WarnOnce(TEXT("RingMaterial"), TEXT("no RingMaterial assigned - the timing ring is skipped"));
		}
	}

	if (HaloMesh)
	{
		if (HaloMaterial)
		{
			HaloMID = UMaterialInstanceDynamic::Create(HaloMaterial, this);
			UMaterialInterface* HaloApplied = HaloMID ? HaloMID.Get() : HaloMaterial.Get();
			HaloMesh->SetMaterial(0, HaloApplied);
		}
		else
		{
			WarnOnce(TEXT("HaloMaterial"), TEXT("no HaloMaterial assigned - the ring halo is skipped"));
		}
	}

	// Pillar: stone / trim / core are tinted instances of the basic-shape material.
	UMaterialInterface* PillarBase = nullptr;
	if (PillarStone.Num() > 0 && PillarStone[0])
	{
		PillarBase = PillarStone[0]->GetMaterial(0);
	}
	if (!PillarBase && PillarTrim.Num() > 0 && PillarTrim[0])
	{
		PillarBase = PillarTrim[0]->GetMaterial(0);
	}
	if (!PillarBase && PillarCore.Num() > 0 && PillarCore[0])
	{
		PillarBase = PillarCore[0]->GetMaterial(0);
	}

	StoneMID = MakeTintedMID(PillarBase, AetherbornComboTiming::DummyColor, TEXT("pillar stone"));
	TrimMID = MakeTintedMID(PillarBase, AetherbornComboTiming::HeroTrim, TEXT("pillar trim"));
	CoreMID = MakeTintedMID(PillarBase, AetherbornComboTiming::DummyCore, TEXT("pillar core"));

	auto TintGroup = [](TArray<TObjectPtr<UStaticMeshComponent>>& Group, UMaterialInstanceDynamic* MID)
	{
		if (!MID)
		{
			return;
		}
		for (const TObjectPtr<UStaticMeshComponent>& Part : Group)
		{
			if (Part)
			{
				Part->SetMaterial(0, MID);
			}
		}
	};

	TintGroup(PillarStone, StoneMID);
	TintGroup(PillarTrim, TrimMID);
	TintGroup(PillarCore, CoreMID);

	if (Ground)
	{
		GroundMID = MakeTintedMID(Ground->GetMaterial(0), AetherbornComboTiming::Ground, TEXT("ground"));
		if (GroundMID)
		{
			Ground->SetMaterial(0, GroundMID);
		}
	}

	// Light colours come straight from the Slice's palette.
	if (KeyLight)
	{
		KeyLight->SetLightColor(AetherbornComboTiming::KeyColor);
	}
	if (RimLight)
	{
		RimLight->SetLightColor(AetherbornComboTiming::Rim);
	}
	if (FillLight)
	{
		FillLight->SetLightColor(AetherbornComboTiming::Grid);
	}
	if (CoreLight)
	{
		CoreLight->SetLightColor(AetherbornComboTiming::DummyCore);
	}
}

void AAetherbornComboTimingArena::ApplyRingScalars(UMaterialInstanceDynamic* MID, float Fill, float WinStart,
                                                  float WinCancel, float WinEnd, float Open, float Pulse) const
{
	if (!MID)
	{
		return;
	}

	MID->SetScalarParameterValue(TEXT("uFill"), Fill);
	MID->SetScalarParameterValue(TEXT("uWinStart"), WinStart);
	MID->SetScalarParameterValue(TEXT("uWinCancel"), WinCancel);
	MID->SetScalarParameterValue(TEXT("uWinEnd"), WinEnd);
	MID->SetScalarParameterValue(TEXT("uOpen"), Open);
	MID->SetScalarParameterValue(TEXT("uPulse"), Pulse);
}

void AAetherbornComboTimingArena::SetSnapshot(const FAetherbornComboTimingSnapshot& InSnapshot)
{
	Snapshot = InSnapshot;

	// Mirrors world.js updateSnapshot(): everything is normalised by the attack length
	// and only drawn while a swing is actually live.
	const float Duration = FMath::Max(0.0001f, Snapshot.AttackDuration);
	const bool bLive = Snapshot.Step > 0;

	const float Fill = bLive ? FMath::Clamp(Snapshot.AttackElapsed / Duration, 0.f, 1.f) : 0.f;
	const float WinStart = bLive ? FMath::Clamp(Snapshot.WindowStart / Duration, 0.f, 1.f) : 0.f;
	const float WinEnd = bLive ? FMath::Clamp(Snapshot.WindowEnd / Duration, 0.f, 1.f) : 0.f;
	const float Open = Snapshot.bWindowOpen ? 1.f : 0.f;
	const float Pulse = 0.5f + 0.5f * FMath::Sin(ElapsedTime * 15.f);

	ApplyRingScalars(RingMID, Fill, WinStart, WinEnd, WinEnd, Open, Pulse);
	ApplyRingScalars(HaloMID, Fill, WinStart, WinEnd, WinEnd, Open, Pulse);
}

void AAetherbornComboTimingArena::SpawnHit(const FVector& Location, float Damage, bool bHeavy)
{
	// world.js kick(): the impulse scales with damage and is heavier on heavy swings.
	const float SafeDamage = FMath::IsFinite(Damage) ? Damage : 10.f;
	const float Mag = (bHeavy ? 1.7f : 1.0f) * (0.65f + FMath::Min(1.2f, SafeDamage / 50.f));

	KickAngleVel += 2.6f * Mag;
	KickOffsetVel += 1.5f * Mag;
	Flash = FMath::Min(1.7f, Flash + 0.85f * Mag);

	HitCombo = FMath::Clamp(HitCombo + 1, 1, FMath::Max(1, AetherbornComboTiming::ChainMax));

	if (Fx)
	{
		Fx->SpawnHit(Location, SafeDamage, bHeavy, HitCombo);
	}
}

FVector AAetherbornComboTimingArena::GetHitPoint() const
{
	const FVector LocalCore(0.f, 0.f, AetherbornComboTimingArenaConstants::PillarCoreHeightZ);
	if (PillarPivot)
	{
		return PillarPivot->GetComponentTransform().TransformPosition(LocalCore);
	}

	return GetActorTransform().TransformPosition(
		FVector(AetherbornComboTimingArenaConstants::PillarBaseOffsetX, 0.f, AetherbornComboTimingArenaConstants::PillarCoreHeightZ));
}

void AAetherbornComboTimingArena::ResetArena()
{
	KickAngle = 0.f;
	KickAngleVel = 0.f;
	KickOffset = 0.f;
	KickOffsetVel = 0.f;
	Flash = 0.f;
	HitCombo = 0;
	Snapshot = FAetherbornComboTimingSnapshot();

	if (PillarPivot)
	{
		PillarPivot->SetRelativeRotation(FRotator::ZeroRotator);
		PillarPivot->SetRelativeLocation(FVector(AetherbornComboTimingArenaConstants::PillarBaseOffsetX, 0.f, 0.f));
	}

	ApplyRingScalars(RingMID, 0.f, 0.f, 0.f, 0.f, 0.f, 0.5f);
	ApplyRingScalars(HaloMID, 0.f, 0.f, 0.f, 0.f, 0.f, 0.5f);

	if (Fx)
	{
		Fx->Reset();
	}
}

void AAetherbornComboTimingArena::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const float Dt = FMath::Clamp(DeltaSeconds, 0.f, 0.05f);
	ElapsedTime += Dt;

	// ---- the two damped springs from world.js (rotation 110 / 11.5, z kick 90 / 9.5)
	KickAngleVel += (-110.f * KickAngle - 11.5f * KickAngleVel) * Dt;
	KickAngle += KickAngleVel * Dt;
	KickOffsetVel += (-90.f * KickOffset - 9.5f * KickOffsetVel) * Dt;
	KickOffset += KickOffsetVel * Dt;

	if (PillarPivot)
	{
		const float LeanDegrees = FMath::RadiansToDegrees(FMath::Clamp(KickAngle, -0.7f, 0.7f));
		PillarPivot->SetRelativeRotation(FRotator(LeanDegrees, 0.f, 0.f));
		PillarPivot->SetRelativeLocation(
			FVector(AetherbornComboTimingArenaConstants::PillarBaseOffsetX + FMath::Clamp(KickOffset * 100.f, -30.f, 45.f),
			        0.f, 0.f));
	}

	// ---- emissive flash decay + idle breathing of the pillar core
	Flash *= FMath::Exp(-Dt * 5.5f);
	const float IdlePulse = 0.85f + 0.22f * FMath::Sin(ElapsedTime * 2.1f);

	if (CoreLight)
	{
		const float Intensity = AetherbornComboTimingArenaConstants::CoreLightUnit *
		                        (4.5f + 1.0f * IdlePulse + Flash * 20.f);
		CoreLight->SetIntensity(Intensity);
	}

	if (CoreMID)
	{
		// BasicShapeMaterial has no emissive input, so the flash brightens the tint instead;
		// a dimmer instance quickly returns to the palette colour as Flash decays.
		const FLinearColor Base = AetherbornComboTiming::DummyCore;
		CoreMID->SetVectorParameterValue(TEXT("Color"), Base * FMath::Clamp(IdlePulse + Flash * 2.4f, 0.2f, 4.f));
	}

	if (GridMID)
	{
		GridMID->SetScalarParameterValue(TEXT("uTime"), ElapsedTime);
	}

	if (Fx)
	{
		Fx->Tick(Dt);
	}
}

void AAetherbornComboTimingArena::WarnOnce(FName Key, const FString& Message)
{
	if (WarnedKeys.Contains(Key))
	{
		return;
	}
	WarnedKeys.Add(Key);
	UE_LOG(LogAetherbornComboTiming, Warning, TEXT("[Arena] %s"), *Message);
}

void AAetherbornComboTimingArena::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (DustComponent)
	{
		DustComponent->Deactivate();
	}
	if (Fx)
	{
		Fx->Reset();
	}

	Super::EndPlay(EndPlayReason);
}
