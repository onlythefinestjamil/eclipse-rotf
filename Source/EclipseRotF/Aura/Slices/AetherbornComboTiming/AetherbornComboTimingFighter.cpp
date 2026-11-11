// Copyright (c) Eclipse Rot. All Rights Reserved.
//
// Aetherborn combo timing - the hero body performance.
//
// A faithful port of the *motion* of src/hero.js createHero().update(), retargeted
// from a hand-built box rig to a real rigged biped: instead of damping 16 joints
// toward authored Euler key poses, the whole body is damped toward an authored
// offset (twist / lean / roll / step / crouch) and the idle breathing layer is
// added on top of it. The phase structure and the easing curves are the Slice's:
//
//   windup    settle -> coil      easeInOut(t)      (lean back, small step back)
//   strike    coil   -> strike    pow(t, 0.65)      (whip into the extreme)
//   recovery  strike -> settle    easeOut(t/0.55)
//             settle -> idle      easeInOut(rest)
//
// Everything is damped with the same exponential decay hero.js used (1 - e^-lt),
// so the result is frame-rate independent: no dt-dependent lerp factors.

#include "Aura/Slices/AetherbornComboTiming/AetherbornComboTimingFighter.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"

namespace
{
	/** Torso damping, 1/seconds. hero.js damped its joints at 12..27; the torso is the heavy end. */
	constexpr float TorsoDamping = 14.f;

	/** Root-step damping. hero.js damped its root step at exactly 9. */
	constexpr float LungeDamping = 9.f;

	/** Crouch damping: the knees give faster than the torso settles. */
	constexpr float CrouchDamping = 12.f;

	/** Frame-rate independent exponential damp, the same curve as THREE.MathUtils.damp. */
	FORCEINLINE float DampChannel(float Value, float Target, float Lambda, float DeltaSeconds)
	{
		const float Alpha = 1.f - FMath::Exp(-FMath::Max(0.f, Lambda) * DeltaSeconds);
		return FMath::Lerp(Value, Target, Alpha);
	}

	FORCEINLINE float EaseInOut(float T) { return T * T * (3.f - 2.f * T); }
	FORCEINLINE float EaseOut(float T) { return 1.f - (1.f - T) * (1.f - T); }

	/** The idle guard, which is the origin every offset is measured from. */
	const FAetherbornFighterOffset GIdleOffset;

	FAetherbornFighterOffset LerpOffset(const FAetherbornFighterOffset& A, const FAetherbornFighterOffset& B, float T)
	{
		const float C = FMath::Clamp(T, 0.f, 1.f);
		FAetherbornFighterOffset Out;
		Out.Yaw = FMath::Lerp(A.Yaw, B.Yaw, C);
		Out.Roll = FMath::Lerp(A.Roll, B.Roll, C);
		Out.Lean = FMath::Lerp(A.Lean, B.Lean, C);
		Out.Lunge = FMath::Lerp(A.Lunge, B.Lunge, C);
		Out.Crouch = FMath::Lerp(A.Crouch, B.Crouch, C);
		return Out;
	}

	void AddOffset(FAetherbornFighterOffset& Dst, const FAetherbornFighterOffset& Src)
	{
		Dst.Yaw += Src.Yaw;
		Dst.Roll += Src.Roll;
		Dst.Lean += Src.Lean;
		Dst.Lunge += Src.Lunge;
		Dst.Crouch += Src.Crouch;
	}

	/**
	 * The three attack silhouettes, in Unreal units (cm / degrees).
	 *
	 * Slot 1 and 2 are the two light slashes of the chain: same weight, opposite
	 * handedness, so the chain reads as an alternating left/right combo. Slot 3 is
	 * the finisher, ported from hero.js index 2: a much shorter, heavier cycle that
	 * rocks back further in the windup and drives through with a big crouch.
	 */
	const FAetherbornFighterStyle GAttackStyles[] =
	{
		// ---- slot 1: fast diagonal slash, right to left
		{
			/* Coil   */ {  22.f,   4.f,   7.f,  -5.f, 2.2f },
			/* Strike */ { -30.f,  -6.f,  -9.f,  14.f, 2.2f },
			/* Settle */ { -10.f,  -2.f,  -3.f,   6.f, 2.2f },
		},
		// ---- slot 2: reverse horizontal, left to right
		{
			/* Coil   */ { -22.f,  -4.f,   7.f,  -5.f, 2.2f },
			/* Strike */ {  30.f,   6.f,  -9.f,  14.f, 2.2f },
			/* Settle */ {  10.f,   2.f,  -3.f,   6.f, 2.2f },
		},
		// ---- slot 3: overhead heavy cleave
		{
			/* Coil   */ {   0.f,   0.f,  13.f,  -8.f, 3.5f },
			/* Strike */ {   0.f,   0.f, -16.f,  26.f, 5.5f },
			/* Settle */ {   0.f,   0.f,  -5.f,  18.f, 2.0f },
		},
	};

	/** Authored attack slots (UE_ARRAY_COUNT is unsigned; keep every clamp typed int32). */
	constexpr int32 GAttackStyleCount = static_cast<int32>(UE_ARRAY_COUNT(GAttackStyles));

	/** Writes the combined transform in one go so the mesh never sees a half-applied pose. */
	void WriteBodyTransform(USkeletalMeshComponent* Component, const FVector& Location, const FRotator& Rotation)
	{
		if (!Component)
		{
			return;
		}
		Component->SetRelativeLocation(Location);
		Component->SetRelativeRotation(Rotation);
	}
}

AAetherbornComboTimingFighter::AAetherbornComboTimingFighter()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// The body mesh is driven purely by the performance below plus the optional
	// montage, so it never needs collision or overlap events of its own.
	BodyMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BodyMesh"));
	BodyMeshComponent->SetupAttachment(Root);
	BodyMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BodyMeshComponent->SetGenerateOverlapEvents(false);
	BodyMeshComponent->SetMobility(EComponentMobility::Movable);

	// Three optional light slots (slash, reverse slash, finisher lead-in) plus the
	// dedicated heavy montage, so the Blueprint child can fill in whatever it has.
	LightAnimations.SetNum(3);
}

void AAetherbornComboTimingFighter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Resolves the skeletal mesh the Blueprint child assigned. The C++ never names a
	// content path, so the hero can be swapped from the editor at any time.
	SyncBodyMesh();
}

void AAetherbornComboTimingFighter::BeginPlay()
{
	Super::BeginPlay();

	SyncBodyMesh();

	if (!BodyMesh)
	{
		UE_LOG(LogAetherbornComboTiming, Warning,
			TEXT("%s: no BodyMesh assigned. Assign the hero skeletal mesh on the Blueprint child of "
				 "AAetherbornComboTimingFighter; the performance still runs without it."),
			*GetName());
	}

	// Start from the authored idle guard, whatever the editor left behind.
	ResetPose();
}

void AAetherbornComboTimingFighter::SyncBodyMesh()
{
	if (!BodyMeshComponent)
	{
		return;
	}

	if (BodyMesh && BodyMeshComponent->GetSkeletalMeshAsset() != BodyMesh.Get())
	{
		BodyMeshComponent->SetSkeletalMeshAsset(BodyMesh.Get());
	}
}

void AAetherbornComboTimingFighter::ApplyAnim(int32 PoseStep, EAetherbornComboTimingPhase Phase, float PhaseT, float AttackT)
{
	// A windup that was not running on the previous frame is the start of an attack:
	// that is the only moment a montage (if one is assigned and compatible) starts.
	const bool bAttackStarted = (Phase == EAetherbornComboTimingPhase::Windup)
		&& (LastPhase != EAetherbornComboTimingPhase::Windup);

	if (bAttackStarted)
	{
		PlayAttackMontage(PoseStep);
	}

	AnimStep = FMath::Clamp(PoseStep, 0, GAttackStyleCount);
	AnimPhase = Phase;
	AnimPhaseT = FMath::Clamp(PhaseT, 0.f, 1.f);
	AnimAttackT = FMath::Max(0.f, AttackT);
	LastPhase = Phase;

	// Back in the guard: the current slot is over, so the next attack of the same
	// slot is allowed to restart its montage.
	if (AnimStep == 0 || Phase == EAetherbornComboTimingPhase::Neutral)
	{
		MontageStep = 0;
	}

	UE_LOG(LogAetherbornComboTiming, Verbose,
		TEXT("%s: apply pose step %d, phase %d, t=%.2f, attack=%.2fs"),
		*GetName(), AnimStep, static_cast<int32>(AnimPhase), AnimPhaseT, AnimAttackT);
}

void AAetherbornComboTimingFighter::OnHitLanded(float Damage, bool bHeavy)
{
	// The recoil impulse: a heavy hit is a full stagger that lingers, a light hit
	// barely rocks the guard. Damage nudges the strength so a finisher late in the
	// chain rocks the hero harder than the first swing.
	const float BaseStrength = bHeavy ? 1.f : 0.45f;
	const float DamageScale = FMath::Clamp(Damage / FMath::Max(1.f, AetherbornComboTiming::HeavyDamage), 0.f, 1.f);

	RecoilAmount = FMath::Clamp(RecoilAmount + BaseStrength * (0.75f + 0.25f * DamageScale), 0.f, 1.35f);
	RecoilDamping = bHeavy ? 3.6f : 6.5f;

	UE_LOG(LogAetherbornComboTiming, Verbose, TEXT("%s: hit recoil %.2f (%.0f damage, heavy=%d)"),
		*GetName(), RecoilAmount, Damage, bHeavy ? 1 : 0);
}

void AAetherbornComboTimingFighter::ResetPose()
{
	AnimStep = 0;
	AnimPhase = EAetherbornComboTimingPhase::Neutral;
	AnimPhaseT = 0.f;
	AnimAttackT = 0.f;
	LastPhase = EAetherbornComboTimingPhase::Neutral;

	Current = FAetherbornFighterOffset();
	RecoilAmount = 0.f;
	RecoilDamping = 6.f;
	PoseTime = 0.f;

	// Stop whatever the optional montage was doing; the code performance takes over
	// from the guard pose immediately.
	if (MontageStep != 0 && BodyMeshComponent)
	{
		if (UAnimInstance* AnimInstance = BodyMeshComponent->GetAnimInstance())
		{
			if (UAnimMontage* ActiveMontage = AnimInstance->GetCurrentActiveMontage())
			{
				AnimInstance->Montage_Stop(0.12f, ActiveMontage);
			}
		}
	}
	MontageStep = 0;

	WriteBodyTransform(BodyMeshComponent, FVector::ZeroVector, FRotator(0.f, BodyYawOffset, 0.f));
}

void AAetherbornComboTimingFighter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// hero.js clamps its own dt, so a hitch cannot fling the pose across the room.
	const float Step = FMath::Clamp(DeltaSeconds, 0.f, 0.1f);
	PoseTime += Step;

	// ---- recoil bleeds off exponentially; a heavy stagger lingers longer
	if (RecoilAmount > 0.f)
	{
		RecoilAmount *= FMath::Exp(-RecoilDamping * Step);
		if (RecoilAmount < 0.01f)
		{
			RecoilAmount = 0.f;
		}
	}

	FAetherbornFighterOffset RecoilOffset;
	if (RecoilAmount > 0.f)
	{
		RecoilOffset.Lean = RecoilAmount * 9.f;    // rocked back onto the heels
		RecoilOffset.Crouch = RecoilAmount * 3.f;  // knees absorb it
		RecoilOffset.Lunge = -RecoilAmount * 5.f;  // pushed off the target
		RecoilOffset.Roll = RecoilAmount * 2.5f;   // and slightly off balance
	}

	// ---- damp every channel toward this frame's target offset
	const FAetherbornFighterOffset Target = ComputeTargetOffset(RecoilOffset);

	Current.Yaw = DampChannel(Current.Yaw, Target.Yaw, TorsoDamping, Step);
	Current.Roll = DampChannel(Current.Roll, Target.Roll, TorsoDamping, Step);
	Current.Lean = DampChannel(Current.Lean, Target.Lean, TorsoDamping, Step);
	Current.Lunge = DampChannel(Current.Lunge, Target.Lunge, LungeDamping, Step);
	Current.Crouch = DampChannel(Current.Crouch, Target.Crouch, CrouchDamping, Step);

	// ---- idle layer (hero.js idleW): full strength in the guard, a fifth of it
	//      while an attack is running so the body still feels alive mid-swing.
	const bool bAttacking = (AnimStep > 0) && (AnimPhase != EAetherbornComboTimingPhase::Neutral);
	const float IdleWeight = bAttacking ? 0.22f : 1.f;

	const float Bob = FMath::Sin(PoseTime * 1.7f) * 1.0f * IdleWeight;                        // cm, breathing rise
	const float BreathPitch = FMath::Sin(PoseTime * 1.7f + 0.4f) * 0.8f * IdleWeight;         // degrees
	const float SwayRoll = FMath::Sin(PoseTime * 0.52f + 0.7f) * 1.4f * IdleWeight;           // degrees, weight shift
	const float SwayYaw = FMath::Sin(PoseTime * 0.62f + 1.4f) * 1.8f * IdleWeight;            // degrees, slow chest turn

	if (!BodyMeshComponent)
	{
		return;
	}

	// Local +Y is forward, +Z up: the lunge steps forward and the crouch drops the body.
	const FVector NewLocation(0.f, Current.Lunge, -Current.Crouch + Bob);
	const FRotator NewRotation(
		Current.Lean + BreathPitch,
		BodyYawOffset + Current.Yaw + SwayYaw,
		Current.Roll + SwayRoll);

	WriteBodyTransform(BodyMeshComponent, NewLocation, NewRotation);
}

FAetherbornFighterOffset AAetherbornComboTimingFighter::ComputeTargetOffset(const FAetherbornFighterOffset& RecoilOffset) const
{
	FAetherbornFighterOffset Target;

	const int32 Step = FMath::Clamp(AnimStep, 0, GAttackStyleCount);
	const float T = FMath::Clamp(AnimPhaseT, 0.f, 1.f);

	if (Step > 0)
	{
		const FAetherbornFighterStyle& Style = StyleForStep(Step);

		switch (AnimPhase)
		{
		case EAetherbornComboTimingPhase::Windup:
			// Raise out of the guard into the coil: lean back, coil the torso away
			// from the coming strike and take a small step back.
			Target = LerpOffset(GIdleOffset, Style.Coil, EaseInOut(T));
			break;

		case EAetherbornComboTimingPhase::Strike:
			// The blade whips: accelerate into the extreme, exactly the Slice's curve.
			Target = LerpOffset(Style.Coil, Style.Strike, FMath::Pow(T, 0.65f));
			break;

		case EAetherbornComboTimingPhase::Recovery:
			// Bleed the follow-through into the settle pose, then settle to the guard.
			if (T < 0.55f)
			{
				Target = LerpOffset(Style.Strike, Style.Settle, EaseOut(T / 0.55f));
			}
			else
			{
				Target = LerpOffset(Style.Settle, GIdleOffset, EaseInOut((T - 0.55f) / 0.45f));
			}
			break;

		case EAetherbornComboTimingPhase::Neutral:
		default:
			Target = GIdleOffset;
			break;
		}
	}

	// The recoil is an impulse on top of the performance, never a pose of its own.
	AddOffset(Target, RecoilOffset);

	return Target;
}

const FAetherbornFighterStyle& AAetherbornComboTimingFighter::StyleForStep(int32 PoseStep)
{
	const int32 Index = FMath::Clamp(PoseStep - 1, 0, GAttackStyleCount - 1);
	return GAttackStyles[Index];
}

void AAetherbornComboTimingFighter::PlayAttackMontage(int32 PoseStep)
{
	if (!BodyMeshComponent)
	{
		return;
	}

	const int32 Slot = FMath::Clamp(PoseStep, 1, GAttackStyleCount);
	UAnimSequence* Sequence = (Slot >= 3)
		? HeavyAnimation.Get()
		: (LightAnimations.IsValidIndex(Slot - 1) ? LightAnimations[Slot - 1].Get() : nullptr);

	// Nothing assigned is the normal case on a body with no clips: the code-driven
	// performance below is still the motion.
	if (!Sequence)
	{
		MontageStep = 0;
		return;
	}

	// Single-node playback: this character has no AnimBlueprint, so there is no slot
	// node for a montage to play through. PlayAnimation drives the retargeted clip's
	// bones directly and returns 0 when the clip does not fit the body's skeleton.
	BodyMeshComponent->PlayAnimation(Sequence, false);

	const float Length = Sequence->GetPlayLength();
	if (Length <= 0.f)
	{
		if (!bMontageSkipLogged)
		{
			bMontageSkipLogged = true;
			UE_LOG(LogAetherbornComboTiming, Warning,
				TEXT("%s: sequence '%s' is not compatible with the hero body's skeleton. "
					 "Using the code-driven performance instead."),
				*GetName(), *Sequence->GetName());
		}
		MontageStep = 0;
		return;
	}

	MontageStep = Slot;
}
