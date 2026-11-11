// Port of Saved/.Aura/Slices/aetherborn-combo/src/dummy.js
//
// The Slice authored the dummy in metres facing +z inside a group rotated -90
// degrees on Y, so it faces -x in world space. Here that -90 degree yaw is
// baked into the local layout instead: three.js local (x, y, z) becomes Unreal
// local (-z, x, y) * 100, which leaves the chest plate facing -X with an
// unrotated actor. Box sizes follow the same axis swap
// ((w, h, d) -> (d * 100, w * 100, h * 100)), and rotations use the mirror
// correction: three z -> +X (Roll), three x -> -Y (Pitch), three y -> -Z (Yaw).

#include "AetherbornComboDummy.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogAetherbornComboDummy, Log, All);

namespace
{
	/** Metres -> centimetres, as everywhere in this export. */
	constexpr float MetresToCm = 100.0f;

	constexpr float MaxDt = 0.05f;
	constexpr float FlashDecay = 3.4f;
	constexpr float RecoilStiffness = 120.0f;
	constexpr float RecoilDamping = 11.0f;

	const FName EmissiveBoostParam(TEXT("EmissiveBoost"));

	/** Weak so GC may collect it; resolved again with LoadObject if it does. */
	TWeakObjectPtr<UStaticMesh> GDummyCubeMesh;

	UStaticMesh* ResolveDummyCubeMesh()
	{
		UStaticMesh* Mesh = GDummyCubeMesh.Get();
		if (!Mesh)
		{
			Mesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
			GDummyCubeMesh = Mesh;
		}
		return Mesh;
	}

	/** three.js local (x, y, z) in metres -> Unreal local cm for the dummy's baked -90 yaw. */
	FVector PartLoc(float X, float Y, float Z)
	{
		return FVector(-Z * MetresToCm, X * MetresToCm, Y * MetresToCm);
	}

	/** three.js BoxGeometry(w, h, d) -> Unreal box size in cm. */
	FVector PartSize(float W, float H, float D)
	{
		return FVector(D * MetresToCm, W * MetresToCm, H * MetresToCm);
	}
}

AAetherbornComboDummy::AAetherbornComboDummy()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeFinder.Succeeded())
	{
		GDummyCubeMesh = CubeFinder.Object;
	}
}

void AAetherbornComboDummy::BeginPlay()
{
	Super::BeginPlay();

	// Reaction materials first, so the parts can be built straight onto them.
	if (StrawMat)
	{
		StrawDMI = UMaterialInstanceDynamic::Create(StrawMat, this);
	}
	if (RustMat)
	{
		RustDMI = UMaterialInstanceDynamic::Create(RustMat, this);
		if (RustDMI)
		{
			// dummy.js authors the rust plate with a faint standing ember glow.
			RustDMI->SetScalarParameterValue(EmissiveBoostParam, 0.2f);
		}
	}

	UMaterialInterface* StrawSurface = StrawDMI ? static_cast<UMaterialInterface*>(StrawDMI.Get()) : StrawMat.Get();
	UMaterialInterface* RustSurface = RustDMI ? static_cast<UMaterialInterface*>(RustDMI.Get()) : RustMat.Get();

	// ------------------------------------------------- base, pad and wooden post
	MakePart(Root, TEXT("Base"), PartSize(0.92f, 0.12f, 0.92f), PartLoc(0.0f, 0.06f, 0.0f), FRotator::ZeroRotator, StoneMat);
	MakePart(Root, TEXT("Pad"), PartSize(0.46f, 0.10f, 0.46f), PartLoc(0.0f, 0.15f, 0.0f), FRotator::ZeroRotator, WoodMat);
	MakePart(Root, TEXT("Post"), PartSize(0.16f, 0.82f, 0.16f), PartLoc(0.0f, 0.56f, 0.0f), FRotator::ZeroRotator, WoodMat);

	// ------------------------------------------------------------------- body
	// The packed body recoils about its own base, 95 cm up the post.
	Body = NewObject<USceneComponent>(this, TEXT("Body"));
	if (Body)
	{
		Body->SetupAttachment(Root);
		Body->SetMobility(EComponentMobility::Movable);
		Body->SetRelativeLocation(FVector(0.0f, 0.0f, 0.95f * MetresToCm));
		Body->RegisterComponent();
	}

	USceneComponent* BodyParent = Body ? Body.Get() : Root.Get();

	// Three stacked straw blocks.
	MakePart(BodyParent, TEXT("StrawLower"), PartSize(0.54f, 0.24f, 0.42f), PartLoc(0.0f, 0.10f, 0.0f), FRotator::ZeroRotator, StrawSurface);
	MakePart(BodyParent, TEXT("StrawMid"), PartSize(0.60f, 0.28f, 0.48f), PartLoc(0.0f, 0.38f, 0.0f), FRotator::ZeroRotator, StrawSurface);
	MakePart(BodyParent, TEXT("StrawUpper"), PartSize(0.50f, 0.24f, 0.40f), PartLoc(0.0f, 0.62f, 0.0f), FRotator::ZeroRotator, StrawSurface);

	// Rope bands holding the straw in.
	MakePart(BodyParent, TEXT("RopeLower"), PartSize(0.63f, 0.05f, 0.51f), PartLoc(0.0f, 0.25f, 0.0f), FRotator::ZeroRotator, RopeMat);
	MakePart(BodyParent, TEXT("RopeUpper"), PartSize(0.62f, 0.05f, 0.50f), PartLoc(0.0f, 0.50f, 0.0f), FRotator::ZeroRotator, RopeMat);

	// Straw cross-arms, slightly canted (rotation.z in the Slice -> Unreal Roll).
	MakePart(
		BodyParent,
		TEXT("ArmA"),
		PartSize(0.98f, 0.12f, 0.15f),
		PartLoc(0.0f, 0.56f, 0.02f),
		FRotator(0.0f, 0.0f, FMath::RadiansToDegrees(0.10f)),
		StrawSurface);
	MakePart(
		BodyParent,
		TEXT("ArmB"),
		PartSize(0.84f, 0.10f, 0.13f),
		PartLoc(0.0f, 0.62f, -0.03f),
		FRotator(0.0f, 0.0f, FMath::RadiansToDegrees(-0.14f)),
		StrawSurface);

	// Rear brace keeping the arms from tipping forward.
	MakePart(BodyParent, TEXT("Brace"), PartSize(0.12f, 0.12f, 0.34f), PartLoc(0.0f, 0.58f, -0.16f), FRotator::ZeroRotator, WoodMat);

	// Head with its rope collar.
	MakePart(BodyParent, TEXT("Head"), PartSize(0.30f, 0.30f, 0.30f), PartLoc(0.0f, 0.92f, 0.0f), FRotator::ZeroRotator, StrawSurface);
	MakePart(BodyParent, TEXT("Collar"), PartSize(0.31f, 0.06f, 0.31f), PartLoc(0.0f, 0.87f, 0.0f), FRotator::ZeroRotator, RopeMat);
	MakePart(BodyParent, TEXT("HeadStrap"), PartSize(0.16f, 0.05f, 0.31f), PartLoc(0.0f, 1.03f, 0.0f), FRotator::ZeroRotator, RopeMat);

	// Rust target plate on the chest, facing -X towards the fighter.
	MakePart(BodyParent, TEXT("TargetPlate"), PartSize(0.17f, 0.17f, 0.02f), PartLoc(0.0f, 0.40f, 0.245f), FRotator::ZeroRotator, RustSurface);

	// -------------------------------------------------------------- hit anchor
	Chest = NewObject<USceneComponent>(this, TEXT("Chest"));
	if (Chest)
	{
		Chest->SetupAttachment(BodyParent);
		Chest->SetMobility(EComponentMobility::Movable);
		Chest->SetRelativeLocation(PartLoc(0.0f, 0.42f, 0.26f));
		Chest->RegisterComponent();
	}

	UE_LOG(LogAetherbornComboDummy, Log, TEXT("Dummy built with %d parts"), Parts.Num());
}

UStaticMeshComponent* AAetherbornComboDummy::MakePart(USceneComponent* InParent, const FString& InName, const FVector& SizeCm, const FVector& LocCm, const FRotator& Rot, UMaterialInterface* Mat)
{
	UStaticMeshComponent* Part = NewObject<UStaticMeshComponent>(this, *InName);
	if (!Part)
	{
		return nullptr;
	}

	Part->SetupAttachment(InParent ? InParent : Root.Get());
	Part->SetMobility(EComponentMobility::Movable);
	Part->SetStaticMesh(ResolveDummyCubeMesh());
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

void AAetherbornComboDummy::Hit(float Damage)
{
	// dummy.js: hit(damage) -> flash = 1, recoilVel += 1.15 + min(2.4, damage * 0.09)
	const float Dmg = FMath::IsFinite(Damage) ? Damage : 0.0f;

	Flash = 1.0f;
	RecoilVel += 1.15f + FMath::Min(2.4f, Dmg * 0.09f);

	UE_LOG(LogAetherbornComboDummy, Log, TEXT("Dummy hit for %.1f (recoil vel %.2f)"), Dmg, RecoilVel);
}

void AAetherbornComboDummy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const float D = FMath::Clamp(DeltaSeconds, 0.0f, MaxDt);

	// Damped spring back to rest.
	RecoilVel += (-RecoilStiffness * Recoil - RecoilDamping * RecoilVel) * D;
	Recoil += RecoilVel * D;
	if (FMath::Abs(Recoil) < 0.0004f && FMath::Abs(RecoilVel) < 0.001f)
	{
		Recoil = 0.0f;
		RecoilVel = 0.0f;
	}

	if (Body)
	{
		// Recoil is authored in radians; FRotator takes degrees.
		const float RecoilDeg = FMath::RadiansToDegrees(Recoil);
		Body->SetRelativeRotation(FRotator(RecoilDeg, 0.0f, RecoilDeg * 0.32f));
	}

	// Flash pulse on the straw body and the rust plate.
	Flash = FMath::Max(0.0f, Flash - D * FlashDecay);
	const float F = Flash * Flash;

	if (StrawDMI)
	{
		StrawDMI->SetScalarParameterValue(EmissiveBoostParam, F * 1.55f);
	}
	if (RustDMI)
	{
		RustDMI->SetScalarParameterValue(EmissiveBoostParam, 0.2f + F * 2.2f);
	}
}

FVector AAetherbornComboDummy::GetHitPoint() const
{
	if (Chest)
	{
		return Chest->GetComponentLocation();
	}
	return GetActorLocation();
}
