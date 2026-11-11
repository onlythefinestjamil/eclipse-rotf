// Port of Saved/.Aura/Slices/aetherborn-combo/src/arena.js
//
// Slice authored in metres with three.js axes; this port multiplies by 100 for
// centimetres and maps three.js (x, y, z) onto Unreal (X = x, Y = z, Z = y).
// That mapping is a mirror (det = -1), so a three.js rotation of +theta about
// axis A becomes a rotation about -A in Unreal; every rotation below is
// converted with that sign flip and from radians to the degrees FRotator wants.

#include "AetherbornComboArena.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogAetherbornComboArena, Log, All);

namespace
{
	/** Metres -> centimetres. Every authored length in arena.js goes through this. */
	constexpr float MetresToCm = 100.0f;

	/** Camera rig constants from arena.js, already mapped to Unreal axes and cm. */
	const FVector CameraBase(-4.2f * MetresToCm, 3.4f * MetresToCm, 2.4f * MetresToCm);
	const FVector CameraTarget(0.4f * MetresToCm, 0.0f * MetresToCm, 1.05f * MetresToCm);

	constexpr float ShakeDecay = 0.35f;              // seconds for full energy to bleed off
	constexpr float ShakeMaxCm = 0.12f * MetresToCm; // 12 cm max camera offset from shake
	constexpr float ShakeFloor = 0.0015f;            // energy^2 below which shake is ignored
	constexpr float MaxDt = 0.05f;

	constexpr float GroundSizeCm = 2600.0f; // 26 m plane
	constexpr int32 PillarCount = 7;

	/** xorshift32, matching makeRng() from arena.js so the dressing lands identically. */
	struct FArenaRng
	{
		uint32 State;

		explicit FArenaRng(uint32 Seed)
			: State(Seed != 0u ? Seed : 1u)
		{
		}

		float Next()
		{
			State ^= State << 13;
			State ^= State >> 17;
			State ^= State << 5;
			return static_cast<float>(State) / 4294967296.0f;
		}
	};

	/** Weak so GC may collect them; resolved again with LoadObject if it does. */
	TWeakObjectPtr<UStaticMesh> GArenaCubeMesh;
	TWeakObjectPtr<UStaticMesh> GArenaPlaneMesh;

	UStaticMesh* ResolveArenaCubeMesh()
	{
		UStaticMesh* Mesh = GArenaCubeMesh.Get();
		if (!Mesh)
		{
			Mesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
			GArenaCubeMesh = Mesh;
		}
		return Mesh;
	}

	UStaticMesh* ResolveArenaPlaneMesh()
	{
		UStaticMesh* Mesh = GArenaPlaneMesh.Get();
		if (!Mesh)
		{
			Mesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
			GArenaPlaneMesh = Mesh;
		}
		return Mesh;
	}

	/** three.js Euler (x, y, z) radians -> Unreal FRotator degrees, mirror-corrected. */
	FRotator EulerThreeToUnreal(float RotX, float RotY, float RotZ)
	{
		// three z -> Unreal -Y (Pitch), three y -> Unreal -Z (Yaw), three x -> Unreal -X (Roll).
		return FRotator(
			FMath::RadiansToDegrees(-RotZ),
			FMath::RadiansToDegrees(-RotY),
			FMath::RadiansToDegrees(-RotX));
	}
}

AAetherbornComboArena::AAetherbornComboArena()
{
	// The rig is advanced once per frame by AAetherbornComboPawn::Tick, which owns
	// the single camera. The arena therefore does not need to tick itself.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// Engine primitives the courtyard is built from (100 cm cube / 100 cm plane).
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeFinder.Succeeded())
	{
		GArenaCubeMesh = CubeFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneFinder(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneFinder.Succeeded())
	{
		GArenaPlaneMesh = PlaneFinder.Object;
	}
}

void AAetherbornComboArena::BeginPlay()
{
	Super::BeginPlay();

	// ---------------------------------------------------------------- ground
	if (!Ground)
	{
		Ground = NewObject<UStaticMeshComponent>(this, TEXT("Ground"));
		Ground->SetupAttachment(Root);
		Ground->SetMobility(EComponentMobility::Movable);
		Ground->RegisterComponent();
	}

	Ground->SetStaticMesh(ResolveArenaPlaneMesh());
	// /Engine/BasicShapes/Plane is a 100x100 cm quad in the XY plane, so a 26x
	// scale is the 26 m courtyard floor.
	Ground->SetRelativeScale3D(FVector(GroundSizeCm / 100.0f, GroundSizeCm / 100.0f, 1.0f));
	Ground->SetRelativeLocation(FVector::ZeroVector);
	Ground->SetRelativeRotation(FRotator::ZeroRotator);
	Ground->SetCastShadow(false);
	if (GroundMat)
	{
		Ground->SetMaterial(0, GroundMat);
	}

	// ------------------------------------------- rim dressing: pillars + rubble
	FArenaRng Rng(0xbadc0de);

	for (int32 i = 0; i < PillarCount; ++i)
	{
		const float Angle = (static_cast<float>(i) / static_cast<float>(PillarCount)) * 2.0f * PI + 0.4f + Rng.Next() * 0.16f;
		const float RadiusCm = (10.4f + Rng.Next() * 2.1f) * MetresToCm;
		const float WidthCm = (0.65f + Rng.Next() * 0.5f) * MetresToCm;
		const float HeightCm = (0.8f + Rng.Next() * 1.7f) * MetresToCm;
		const float DepthCm = (0.65f + Rng.Next() * 0.5f) * MetresToCm;

		const float RotX = (Rng.Next() - 0.5f) * 0.1f;
		const float RotY = Rng.Next() * PI;
		const float RotZ = (Rng.Next() - 0.5f) * 0.13f;

		// Box size authored as (width, height, depth) -> Unreal (X = depth, Y = width, Z = height).
		const FVector SizeCm(DepthCm, WidthCm, HeightCm);
		const FVector LocCm(
			FMath::Cos(Angle) * RadiusCm,
			FMath::Sin(Angle) * RadiusCm,
			HeightCm * 0.5f - 6.0f);

		UMaterialInterface* PillarMat = ((i % 3) == 0) ? StoneAltMat.Get() : StoneMat.Get();
		MakePart(
			Root,
			FString::Printf(TEXT("Pillar_%d"), i),
			SizeCm,
			LocCm,
			EulerThreeToUnreal(RotX, RotY, RotZ),
			PillarMat);

		// 1-2 broken chunks scattered at the foot of each pillar.
		const int32 Chunks = 1 + static_cast<int32>(Rng.Next() * 2.0f);
		for (int32 b = 0; b < Chunks; ++b)
		{
			const float ChunkCm = (0.18f + Rng.Next() * 0.3f) * MetresToCm;
			const float OffsetX = (Rng.Next() - 0.5f) * 1.8f * MetresToCm;
			const float OffsetY = (Rng.Next() - 0.5f) * 1.8f * MetresToCm;
			const float ChunkRotX = (Rng.Next() - 0.5f) * 0.3f;
			const float ChunkRotY = Rng.Next() * PI;
			const float ChunkRotZ = (Rng.Next() - 0.5f) * 0.25f;

			// BoxGeometry(s, s * 0.7, s * 1.1) -> Unreal (X = s * 1.1, Y = s, Z = s * 0.7).
			const FVector ChunkSizeCm(ChunkCm * 1.1f, ChunkCm, ChunkCm * 0.7f);
			const FVector ChunkLocCm(
				LocCm.X + OffsetX,
				LocCm.Y + OffsetY,
				ChunkCm * 0.35f);

			MakePart(
				Root,
				FString::Printf(TEXT("Rubble_%d_%d"), i, b),
				ChunkSizeCm,
				ChunkLocCm,
				EulerThreeToUnreal(ChunkRotX, ChunkRotY, ChunkRotZ),
				StoneMat);
		}
	}

	// --------------------------------------------------------------- lights
	// Key: warm ember directional from behind the fighter's left shoulder.
	{
		UDirectionalLightComponent* KeyLight = NewObject<UDirectionalLightComponent>(this, TEXT("KeyLight"));
		KeyLight->SetupAttachment(Root);
		KeyLight->SetMobility(EComponentMobility::Movable);
		KeyLight->RegisterComponent();
		KeyLight->SetIntensity(1.7f);
		KeyLight->SetLightColor(FLinearColor(FColor(255, 122, 51))); // PALETTE.ember

		const FVector KeyFrom(-5.6f * MetresToCm, 4.8f * MetresToCm, 7.2f * MetresToCm);
		const FVector KeyTo(0.3f * MetresToCm, 0.1f * MetresToCm, 0.9f * MetresToCm);
		KeyLight->SetWorldRotation((KeyTo - KeyFrom).Rotation());
	}

	// Fill: cool bounce from the opposite corner, aimed back at the duel.
	{
		UDirectionalLightComponent* FillLight = NewObject<UDirectionalLightComponent>(this, TEXT("FillLight"));
		FillLight->SetupAttachment(Root);
		FillLight->SetMobility(EComponentMobility::Movable);
		FillLight->RegisterComponent();
		FillLight->SetIntensity(0.35f);
		FillLight->SetLightColor(FLinearColor(FColor(95, 127, 168))); // PALETTE.cool

		const FVector FillFrom(4.6f * MetresToCm, -6.6f * MetresToCm, 3.6f * MetresToCm);
		const FVector FillTo(-0.2f * MetresToCm, 0.2f * MetresToCm, 0.9f * MetresToCm);
		FillLight->SetWorldRotation((FillTo - FillFrom).Rotation());
	}

	// Dim ember source on the dummy side for dark-fantasy warmth.
	{
		UPointLightComponent* EmberLight = NewObject<UPointLightComponent>(this, TEXT("EmberLight"));
		EmberLight->SetupAttachment(Root);
		EmberLight->SetMobility(EComponentMobility::Movable);
		EmberLight->RegisterComponent();
		EmberLight->SetIntensity(9.0f);
		EmberLight->SetAttenuationRadius(14.0f * MetresToCm);
		EmberLight->SetLightColor(FLinearColor(FColor(200, 73, 27))); // PALETTE.emberDeep
		EmberLight->SetWorldLocation(FVector(3.6f * MetresToCm, 2.0f * MetresToCm, 1.2f * MetresToCm));
	}
}

void AAetherbornComboArena::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Intentionally empty: AAetherbornComboPawn::Tick calls UpdateRig() exactly once per
	// frame. Advancing the rig here as well would double the drift and the shake decay.
}

UStaticMeshComponent* AAetherbornComboArena::MakePart(USceneComponent* InParent, const FString& InName, const FVector& SizeCm, const FVector& LocCm, const FRotator& Rot, UMaterialInterface* Mat)
{
	UStaticMeshComponent* Part = NewObject<UStaticMeshComponent>(this, *InName);
	if (!Part)
	{
		return nullptr;
	}

	Part->SetupAttachment(InParent ? InParent : Root.Get());
	Part->SetMobility(EComponentMobility::Movable);
	Part->SetStaticMesh(ResolveArenaCubeMesh());
	// The engine cube measures 100 cm on each side, so scale = size / 100.
	Part->SetRelativeScale3D(SizeCm / 100.0f);
	Part->SetRelativeLocation(LocCm);
	Part->SetRelativeRotation(Rot);
	Part->SetCastShadow(true);
	if (Mat)
	{
		Part->SetMaterial(0, Mat);
	}
	Part->RegisterComponent();

	Parts.Add(Part);
	return Part;
}

void AAetherbornComboArena::Punch(float Amount)
{
	// arena.js: punch(amount) with a 0.6 default for anything that is not finite.
	const float Add = FMath::IsFinite(Amount) ? Amount : 0.6f;
	ShakeEnergy = FMath::Min(1.0f, ShakeEnergy + Add);

	UE_LOG(LogAetherbornComboArena, Log, TEXT("Punch %.2f -> shake energy %.2f"), Add, ShakeEnergy);
}

void AAetherbornComboArena::UpdateRig(float DeltaSeconds)
{
	const float D = FMath::Clamp(DeltaSeconds, 0.0f, MaxDt);
	RigTime += D;
	ShakeEnergy = FMath::Max(0.0f, ShakeEnergy - D / ShakeDecay);

	// Snappier tail than a linear falloff.
	const float Amp = ShakeEnergy * ShakeEnergy;

	// Breathing drift around the base position (three x -> X, three z -> Y, three y -> Z).
	FVector Offset(
		FMath::Sin(RigTime * 0.55f) * 6.0f + FMath::Sin(RigTime * 0.21f + 1.7f) * 3.0f,
		FMath::Sin(RigTime * 0.37f + 0.8f) * 4.5f,
		FMath::Cos(RigTime * 0.43f) * 3.5f);

	if (Amp > ShakeFloor)
	{
		Offset.X += (FMath::FRand() - 0.5f) * 2.0f * ShakeMaxCm * Amp;
		Offset.Y += (FMath::FRand() - 0.5f) * 2.0f * ShakeMaxCm * Amp * 0.6f;
		Offset.Z += (FMath::FRand() - 0.5f) * 2.0f * ShakeMaxCm * Amp;
	}

	if (Offset.SizeSquared() > ShakeMaxCm * ShakeMaxCm)
	{
		Offset = Offset.GetSafeNormal() * ShakeMaxCm;
	}

	RigOffset = Offset;
}

FVector AAetherbornComboArena::GetCameraLocation() const
{
	return CameraBase + RigOffset;
}

FVector AAetherbornComboArena::GetCameraTarget() const
{
	return CameraTarget;
}
