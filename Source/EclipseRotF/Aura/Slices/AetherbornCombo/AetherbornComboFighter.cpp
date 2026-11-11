// Aetherborn duelist: a stylized humanoid built only from boxes, with a box longsword.
// Port of src/fighter.js. The pose is generated procedurally from the combo snapshot; there is no
// skeletal rig and no imported art, so every part is an engine cube parented to a scene joint.

#include "AetherbornComboFighter.h"

#include "AetherbornComboTypes.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	/** One pose exactly as authored in the Slice (radians). Field order matches FComboPose. */
	struct FComboRawPose
	{
		float sx, sy, sz, ex, ey;
		float tx, ty, tz;
		float ry, lean, bob;
		float lx, ly, lz, lex;
	};

	/** FComboPose out of a raw Slice pose. */
	void ApplyRawPose(FComboPose& Pose, const FComboRawPose& Raw)
	{
		Pose.sx = Raw.sx;
		Pose.sy = Raw.sy;
		Pose.sz = Raw.sz;
		Pose.ex = Raw.ex;
		Pose.ey = Raw.ey;
		Pose.tx = Raw.tx;
		Pose.ty = Raw.ty;
		Pose.tz = Raw.tz;
		Pose.ry = Raw.ry;
		Pose.lean = Raw.lean;
		Pose.bob = Raw.bob;
		Pose.lx = Raw.lx;
		Pose.ly = Raw.ly;
		Pose.lz = Raw.lz;
		Pose.lex = Raw.lex;
	}

	// src/fighter.js RAW: wind-back and strike pose per swing, expressed as swings on GUARD.
	// The Slice builds POSES[k] = { ...GUARD, ...RAW[k].wound }; every channel is overridden here.
	const FComboRawPose RawWound[3] =
	{
		// 1 - horizontal slash, right -> left
		{ -0.28f, -1.18f, 0.30f, -1.15f, 0.22f, -0.04f, -0.44f, -0.05f, -0.22f, -0.05f, -0.02f, -0.60f, 0.58f, 0.10f, -0.90f },
		// 2 - back-hand return slash, left -> right
		{ -0.95f, 0.98f, 0.20f, -0.85f, -0.26f, 0.00f, 0.42f, 0.05f, 0.22f, 0.02f, -0.01f, 0.10f, -0.30f, -0.10f, -0.70f },
		// 3 - overhead chop
		{ -2.45f, -0.20f, 0.20f, -1.35f, 0.00f, -0.26f, -0.08f, 0.00f, -0.10f, -0.12f, 0.03f, -1.30f, 0.20f, 0.10f, -1.10f },
	};

	const FComboRawPose RawStrike[3] =
	{
		{ -1.18f, 1.05f, 0.04f, -0.36f, -0.12f, 0.10f, 0.50f, 0.06f, 0.28f, 0.10f, 0.00f, 0.28f, -0.35f, -0.10f, -0.45f },
		{ -0.78f, -1.32f, 0.30f, -0.30f, 0.22f, 0.08f, -0.50f, -0.06f, -0.30f, 0.06f, 0.00f, -0.38f, 0.62f, 0.15f, -0.55f },
		{ -0.22f, 0.10f, 0.02f, -0.20f, 0.00f, 0.34f, 0.06f, 0.00f, 0.05f, 0.16f, -0.05f, 0.30f, 0.15f, 0.00f, -0.35f },
	};

	/** blend(a, b, u): linear blend of every pose channel with u clamped to 0..1. */
	FComboPose BlendPoses(const FComboPose& A, const FComboPose& B, float U)
	{
		const float T = FMath::Clamp(U, 0.0f, 1.0f);

		FComboPose Out;
		Out.sx = A.sx + (B.sx - A.sx) * T;
		Out.sy = A.sy + (B.sy - A.sy) * T;
		Out.sz = A.sz + (B.sz - A.sz) * T;
		Out.ex = A.ex + (B.ex - A.ex) * T;
		Out.ey = A.ey + (B.ey - A.ey) * T;
		Out.tx = A.tx + (B.tx - A.tx) * T;
		Out.ty = A.ty + (B.ty - A.ty) * T;
		Out.tz = A.tz + (B.tz - A.tz) * T;
		Out.ry = A.ry + (B.ry - A.ry) * T;
		Out.lean = A.lean + (B.lean - A.lean) * T;
		Out.bob = A.bob + (B.bob - A.bob) * T;
		Out.lx = A.lx + (B.lx - A.lx) * T;
		Out.ly = A.ly + (B.ly - A.ly) * T;
		Out.lz = A.lz + (B.lz - A.lz) * T;
		Out.lex = A.lex + (B.lex - A.lex) * T;
		return Out;
	}

	float EaseOutCubic(float T)
	{
		const float X = 1.0f - FMath::Clamp(T, 0.0f, 1.0f);
		return 1.0f - X * X * X;
	}

	float EaseInOut(float T)
	{
		const float X = FMath::Clamp(T, 0.0f, 1.0f);
		return X < 0.5f ? 2.0f * X * X : 1.0f - FMath::Square(-2.0f * X + 2.0f) / 2.0f;
	}

	float Deg(float Radians)
	{
		return FMath::RadiansToDegrees(Radians);
	}
}

AAetherbornComboFighter::AAetherbornComboFighter()
{
	PrimaryActorTick.bCanEverTick = false;

	BuildBody();

	// src/fighter.js GUARD: sword held upright in front of the right shoulder.
	GuardPose.sx = -0.55f;
	GuardPose.sy = -0.50f;
	GuardPose.sz = 0.12f;
	GuardPose.ex = -0.95f;
	GuardPose.ey = 0.10f;
	GuardPose.tx = 0.03f;
	GuardPose.ty = 0.14f;
	GuardPose.tz = 0.00f;
	GuardPose.ry = 0.00f;
	GuardPose.lean = 0.00f;
	GuardPose.bob = 0.00f;
	GuardPose.lx = -0.35f;
	GuardPose.ly = 0.30f;
	GuardPose.lz = 0.00f;
	GuardPose.lex = -0.55f;

	// POSES[1..3]: SwingPoses[0] is the Slice's swing 1.
	for (int32 Index = 0; Index < 3; ++Index)
	{
		SwingPoses[Index].Wound = GuardPose;
		ApplyRawPose(SwingPoses[Index].Wound, RawWound[Index]);

		SwingPoses[Index].Strike = GuardPose;
		ApplyRawPose(SwingPoses[Index].Strike, RawStrike[Index]);
	}

	GuardLive = GuardPose;
	Posed = GuardPose;
}

void AAetherbornComboFighter::BeginPlay()
{
	Super::BeginPlay();

	// Warn about empty material slots: those parts keep the engine default material.
	if (!SteelMat)
	{
		UE_LOG(LogAetherbornCombo, Warning, TEXT("AetherbornComboFighter: material slot 'SteelMat' is unset."));
	}
	if (!SteelDarkMat)
	{
		UE_LOG(LogAetherbornCombo, Warning, TEXT("AetherbornComboFighter: material slot 'SteelDarkMat' is unset."));
	}
	if (!ClothMat)
	{
		UE_LOG(LogAetherbornCombo, Warning, TEXT("AetherbornComboFighter: material slot 'ClothMat' is unset."));
	}
	if (!LeatherMat)
	{
		UE_LOG(LogAetherbornCombo, Warning, TEXT("AetherbornComboFighter: material slot 'LeatherMat' is unset."));
	}
	if (!EmberMat)
	{
		UE_LOG(LogAetherbornCombo, Warning, TEXT("AetherbornComboFighter: material slot 'EmberMat' is unset."));
	}
	if (!FullerMat)
	{
		UE_LOG(LogAetherbornCombo, Warning, TEXT("AetherbornComboFighter: material slot 'FullerMat' is unset."));
	}

	if (EmberMat)
	{
		EmberDMI = UMaterialInstanceDynamic::Create(EmberMat, this);
	}
	if (FullerMat)
	{
		FullerDMI = UMaterialInstanceDynamic::Create(FullerMat, this);
	}

	// Re-collect the cube parts from the live actor so the material swap below can never touch the
	// default object's components, then point the ember / fuller parts at their dynamic instances.
	Parts.Reset();
	TArray<UStaticMeshComponent*> FoundParts;
	GetComponents<UStaticMeshComponent>(FoundParts);
	for (UStaticMeshComponent* Part : FoundParts)
	{
		Parts.Add(Part);

		if (!Part)
		{
			continue;
		}

		UMaterialInterface* Current = Part->GetMaterial(0);
		if (EmberDMI && Current && Current == EmberMat.Get())
		{
			Part->SetMaterial(0, EmberDMI);
		}
		else if (FullerDMI && Current && Current == FullerMat.Get())
		{
			Part->SetMaterial(0, FullerDMI);
		}
	}

	// Start in the authored guard pose with the emissive at its resting glow.
	ApplyPose(GuardPose);
	if (FullerDMI)
	{
		FullerDMI->SetScalarParameterValue(TEXT("EmissiveBoost"), 0.55f);
	}
	if (EmberDMI)
	{
		EmberDMI->SetScalarParameterValue(TEXT("EmissiveBoost"), 1.0f);
	}
}

void AAetherbornComboFighter::SetStep(int32 NewStep)
{
	HintedStep = FMath::Clamp(NewStep, 0, 3);
	UE_LOG(LogAetherbornCombo, Verbose, TEXT("AetherbornComboFighter: hinted swing step %d."), HintedStep);
}

void AAetherbornComboFighter::UpdatePose(float DeltaSeconds, const FComboSnapshot& Snapshot)
{
	// src/fighter.js clamps the frame delta to 50 ms; the clock only feeds the idle sway.
	const float Dt = FMath::Clamp(DeltaSeconds, 0.0f, 0.05f);
	Clock += Dt;

	const int32 Step = Snapshot.Step;
	const EComboPhase Phase = Snapshot.Phase;
	const float Progress = FMath::Clamp(Snapshot.PhaseProgress, 0.0f, 1.0f);

	// Idle breathing / guard sway layered on the authored guard pose.
	const float Breath = FMath::Sin(Clock * 1.9f);
	GuardLive = GuardPose;
	GuardLive.sx = GuardPose.sx + Breath * 0.045f;
	GuardLive.lx = GuardPose.lx + FMath::Sin(Clock * 1.9f + 1.0f) * 0.06f;
	GuardLive.ty = GuardPose.ty + FMath::Sin(Clock * 0.8f) * 0.04f;
	GuardLive.bob = Breath * 0.012f;
	GuardLive.ry = FMath::Sin(Clock * 0.55f) * 0.035f;
	GuardLive.lean = FMath::Sin(Clock * 0.7f) * 0.012f;

	// POSES[step] || POSES[hinted]
	const FComboSwingPose* Set = nullptr;
	if (Step >= 1 && Step <= 3)
	{
		Set = &SwingPoses[Step - 1];
	}
	else if (HintedStep >= 1 && HintedStep <= 3)
	{
		Set = &SwingPoses[HintedStep - 1];
	}

	if (Set && Phase == EComboPhase::Windup)
	{
		// Draw back to the wound pose.
		Posed = BlendPoses(GuardLive, Set->Wound, EaseOutCubic(Progress));
	}
	else if (Set && Phase == EComboPhase::Active)
	{
		// Fast, whip-like travel from wound to strike.
		Posed = BlendPoses(Set->Wound, Set->Strike, FMath::Pow(Progress, 0.7f));
	}
	else if (Set && Phase == EComboPhase::Recovery)
	{
		// Hold the follow-through, then settle back to guard.
		const float U = FMath::Clamp((Progress - 0.10f) / 0.9f, 0.0f, 1.0f);
		Posed = BlendPoses(Set->Strike, GuardLive, EaseInOut(U));
	}
	else
	{
		// Idle / window: ease the current pose back to the live guard pose.
		Posed = BlendPoses(Posed, GuardLive, FMath::Clamp(Dt * 12.0f, 0.0f, 1.0f));
	}

	ApplyPose(Posed);

	// Emissive boost: 1.5 while the blade is active, 1.0 inside the combo window, else 0.
	// Driven here rather than in ApplyPose, which only receives the pose (no phase / window state).
	const float Boost = (Phase == EComboPhase::Active) ? 1.5f : (Snapshot.bWindowOpen ? 1.0f : 0.0f);
	if (FullerDMI)
	{
		FullerDMI->SetScalarParameterValue(TEXT("EmissiveBoost"), 0.55f + Boost);
	}
	if (EmberDMI)
	{
		EmberDMI->SetScalarParameterValue(TEXT("EmissiveBoost"), 1.0f + Boost * 0.6f);
	}
}

FVector AAetherbornComboFighter::GetHitPoint() const
{
	if (SwordTip)
	{
		return SwordTip->GetComponentLocation();
	}
	if (Sword)
	{
		return Sword->GetComponentLocation();
	}
	return GetActorLocation();
}

void AAetherbornComboFighter::ApplyPose(const FComboPose& Pose)
{
	if (ShoulderR)
	{
		ShoulderR->SetRelativeRotation(FRotator(Deg(Pose.sx), Deg(Pose.sy), Deg(Pose.sz)));
	}
	if (ElbowR)
	{
		ElbowR->SetRelativeRotation(FRotator(Deg(Pose.ex), Deg(Pose.ey), 0.0f));
	}
	if (ShoulderL)
	{
		ShoulderL->SetRelativeRotation(FRotator(Deg(Pose.lx), Deg(Pose.ly), Deg(Pose.lz)));
	}
	if (ElbowL)
	{
		ElbowL->SetRelativeRotation(FRotator(Deg(Pose.lex), 0.0f, 0.0f));
	}
	if (Torso)
	{
		Torso->SetRelativeRotation(FRotator(Deg(Pose.tx), Deg(Pose.ty), Deg(Pose.tz)));
	}
	if (Rig)
	{
		Rig->SetRelativeRotation(FRotator(Deg(Pose.lean), Deg(Pose.ry), 0.0f));
	}
	if (Hips)
	{
		// Hips sit at 90 cm; bob is authored in Slice metres.
		Hips->SetRelativeLocation(FVector(0.0f, 0.0f, 90.0f + Pose.bob * 100.0f));
	}
}

void AAetherbornComboFighter::BuildBody()
{
	// The actor faces +X. The Slice faces +z with +y up, so a part authored at Slice +x lands on
	// local +Y, one at Slice +z on local +X, and Slice +y becomes local +Z. Slice metres -> cm.

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	if (Root)
	{
		Root->SetMobility(EComponentMobility::Movable);
	}

	Rig = MakeJoint(Root, TEXT("Rig"), FVector::ZeroVector); // animated body; Root stays free
	Hips = MakeJoint(Rig, TEXT("Hips"), FVector(0.0f, 0.0f, 90.0f));
	Torso = MakeJoint(Hips, TEXT("Torso"), FVector(0.0f, 0.0f, 10.0f));
	Head = MakeJoint(Torso, TEXT("Head"), FVector(0.0f, 0.0f, 80.0f));

	// ---------- body ----------
	MakePart(Hips, TEXT("HipBlock"), FVector(24.0f, 34.0f, 22.0f), FVector::ZeroVector, FRotator::ZeroRotator, ClothMat);
	MakePart(Torso, TEXT("TorsoBlock"), FVector(25.0f, 38.0f, 36.0f), FVector(0.0f, 0.0f, 20.0f), FRotator::ZeroRotator, ClothMat);
	MakePart(Torso, TEXT("ChestPlate"), FVector(30.0f, 46.0f, 28.0f), FVector(0.0f, 0.0f, 50.0f), FRotator::ZeroRotator, SteelDarkMat);
	MakePart(Torso, TEXT("Neck"), FVector(12.0f, 12.0f, 10.0f), FVector(0.0f, 0.0f, 68.0f), FRotator::ZeroRotator, SteelDarkMat);
	// Ember sash, rolled 0.22 rad in the Slice.
	MakePart(Torso, TEXT("EmberSash"), FVector(31.0f, 44.0f, 8.5f), FVector(0.0f, 0.0f, 36.0f), FRotator(0.0f, 0.0f, Deg(0.22f)), EmberMat);

	// ---------- head ----------
	MakePart(Head, TEXT("Skull"), FVector(24.0f, 24.0f, 26.0f), FVector(0.0f, 0.0f, 2.0f), FRotator::ZeroRotator, SteelMat);
	MakePart(Head, TEXT("VisorSlit"), FVector(2.0f, 19.0f, 3.5f), FVector(12.5f, 0.0f, 4.0f), FRotator::ZeroRotator, EmberMat);
	MakePart(Head, TEXT("CrownBand"), FVector(27.0f, 27.0f, 6.0f), FVector(0.0f, 0.0f, 16.0f), FRotator::ZeroRotator, SteelDarkMat);

	// ---------- legs ----------
	// Slice leg descriptors { x, z, knee }: a leg joint at Slice (x, -0.02, z).
	struct FLegDef
	{
		float X;      // Slice x, cm -> local Y
		float Z;      // Slice z, cm -> local X
		float Knee;   // knee bend, radians
	};

	const FLegDef Legs[2] =
	{
		{ -13.0f, -2.0f, -0.14f },
		{ 13.0f, 5.0f, -0.06f },
	};
	const TCHAR* LegSuffix[2] = { TEXT("R"), TEXT("L") };

	for (int32 Index = 0; Index < 2; ++Index)
	{
		const FLegDef& LegDef = Legs[Index];
		const FString Suffix(LegSuffix[Index]);

		USceneComponent* Leg = MakeJoint(Hips, FString(TEXT("Leg")) + Suffix, FVector(LegDef.Z, LegDef.X, -2.0f));
		MakePart(Leg, FString(TEXT("Thigh")) + Suffix, FVector(17.0f, 16.0f, 40.0f), FVector(0.0f, 0.0f, -20.0f), FRotator::ZeroRotator, ClothMat);
		MakePart(Leg, FString(TEXT("KneeGuard")) + Suffix, FVector(19.0f, 17.0f, 10.0f), FVector(0.0f, 0.0f, -36.0f), FRotator::ZeroRotator, SteelDarkMat);

		USceneComponent* Knee = MakeJoint(Leg, FString(TEXT("Knee")) + Suffix, FVector(0.0f, 0.0f, -40.0f));
		if (Knee)
		{
			Knee->SetRelativeRotation(FRotator(Deg(LegDef.Knee), 0.0f, 0.0f));
		}
		MakePart(Knee, FString(TEXT("Shin")) + Suffix, FVector(15.0f, 14.0f, 38.0f), FVector(0.0f, 0.0f, -19.0f), FRotator::ZeroRotator, ClothMat);
		MakePart(Knee, FString(TEXT("Boot")) + Suffix, FVector(30.0f, 17.0f, 12.0f), FVector(5.0f, 0.0f, -42.0f), FRotator::ZeroRotator, LeatherMat);
	}

	// ---------- arms ----------
	// one arm: shoulder at Slice (0.28 * side, 0.60, 0), elbow 33 cm down the upper arm.
	auto BuildArm = [this](float Side, const TCHAR* Suffix, TObjectPtr<USceneComponent>& OutShoulder, TObjectPtr<USceneComponent>& OutElbow)
	{
		const FString NameSuffix(Suffix);
		OutShoulder = MakeJoint(Torso, FString(TEXT("Shoulder")) + NameSuffix, FVector(0.0f, 28.0f * Side, 60.0f));
		MakePart(OutShoulder, FString(TEXT("Pauldron")) + NameSuffix, FVector(27.0f, 21.0f, 16.0f), FVector(0.0f, 0.0f, 2.0f), FRotator::ZeroRotator, SteelDarkMat);
		MakePart(OutShoulder, FString(TEXT("UpperArm")) + NameSuffix, FVector(13.0f, 12.0f, 32.0f), FVector(0.0f, 0.0f, -17.0f), FRotator::ZeroRotator, ClothMat);

		OutElbow = MakeJoint(OutShoulder, FString(TEXT("Elbow")) + NameSuffix, FVector(0.0f, 0.0f, -33.0f));
		MakePart(OutElbow, FString(TEXT("Forearm")) + NameSuffix, FVector(12.0f, 11.5f, 30.0f), FVector(0.0f, 0.0f, -15.0f), FRotator::ZeroRotator, ClothMat);
		MakePart(OutElbow, FString(TEXT("Bracer")) + NameSuffix, FVector(15.5f, 14.5f, 11.0f), FVector(0.0f, 0.0f, -29.0f), FRotator::ZeroRotator, SteelDarkMat);
	};

	// The Slice's fighter faces +z, so its right hand sits at -x: side -1 is the right arm.
	BuildArm(-1.0f, TEXT("R"), ShoulderR, ElbowR);
	BuildArm(1.0f, TEXT("L"), ShoulderL, ElbowL);

	// ---------- longsword (built along +y, held from the right hand) ----------
	Sword = MakeJoint(ElbowR, TEXT("Sword"), FVector(0.0f, 0.0f, -36.0f));
	if (Sword)
	{
		// Slice: sword.rotation.x = PI/2 turns the +y-built blade onto +z (the facing axis), i.e.
		// the blade points forward out of the fist on local +X here.
		Sword->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	}

	MakePart(Sword, TEXT("SwordGrip"), FVector(5.0f, 5.0f, 22.0f), FVector::ZeroVector, FRotator::ZeroRotator, LeatherMat);
	MakePart(Sword, TEXT("SwordPommel"), FVector(8.5f, 8.5f, 6.0f), FVector(0.0f, 0.0f, -14.0f), FRotator::ZeroRotator, SteelDarkMat);
	MakePart(Sword, TEXT("SwordCrossguard"), FVector(7.5f, 30.0f, 5.5f), FVector(0.0f, 0.0f, 13.0f), FRotator::ZeroRotator, SteelMat);
	MakePart(Sword, TEXT("SwordBlade"), FVector(3.8f, 11.5f, 102.0f), FVector(0.0f, 0.0f, 65.0f), FRotator::ZeroRotator, SteelMat);
	MakePart(Sword, TEXT("SwordFuller"), FVector(4.8f, 4.4f, 88.0f), FVector(0.0f, 0.0f, 62.0f), FRotator::ZeroRotator, FullerMat);

	SwordTip = MakeJoint(Sword, TEXT("SwordTip"), FVector(0.0f, 0.0f, 118.0f));
}

USceneComponent* AAetherbornComboFighter::MakeJoint(USceneComponent* InParent, const FString& InName, const FVector& LocCm)
{
	USceneComponent* Joint = CreateDefaultSubobject<USceneComponent>(FName(*InName));
	if (!Joint)
	{
		return nullptr;
	}

	Joint->SetMobility(EComponentMobility::Movable);
	Joint->SetupAttachment(InParent);
	Joint->SetRelativeLocation(LocCm);
	return Joint;
}

UStaticMeshComponent* AAetherbornComboFighter::MakePart(USceneComponent* InParent, const FString& InName, const FVector& SizeCm, const FVector& LocCm, const FRotator& Rot, UMaterialInterface* Mat)
{
	// Engine unit cube (100 cm); parts are just scaled copies of it.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));

	UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(FName(*InName));
	if (!Part)
	{
		return nullptr;
	}

	if (CubeFinder.Object)
	{
		Part->SetStaticMesh(CubeFinder.Object);
	}

	Part->SetMobility(EComponentMobility::Movable);
	Part->SetupAttachment(InParent);
	Part->SetRelativeLocation(LocCm);
	Part->SetRelativeRotation(Rot);
	Part->SetRelativeScale3D(SizeCm / 100.0f);
	Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Part->SetGenerateOverlapEvents(false);

	if (Mat)
	{
		Part->SetMaterial(0, Mat);
	}

	Parts.Add(Part);
	return Part;
}
