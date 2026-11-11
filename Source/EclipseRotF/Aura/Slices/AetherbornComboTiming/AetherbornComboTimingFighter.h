// Copyright (c) Eclipse Rot. All Rights Reserved.
//
// Aetherborn combo timing - the hero body.
//
// Ported from src/hero.js createHero(), but only the *performance*: the idle
// breathing/bob layer, the windup coil, the strike lunge and torso twist, the
// recovery settle and the contact-shadow squash. The procedural box rig and its
// 16 authored joint pose tables are gone - the hero is a real rigged biped now,
// so the motion is applied as a damped whole-body offset on the mesh component
// instead of per-joint Euler angles.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Aura/Slices/AetherbornComboTiming/AetherbornComboTimingTypes.h"

#include "AetherbornComboTimingFighter.generated.h"

class UAnimMontage;
class UAnimSequence;
class USceneComponent;
class USkeletalMesh;
class USkeletalMeshComponent;

/**
 * One key moment of an attack, expressed as whole-body offsets.
 *
 * Authored in Unreal units: centimetres for the translations, degrees for the
 * rotations (hero.js authored the same performance as radians on a 1m-tall rig).
 * The body faces local +Y, so a positive Lunge steps toward +Y and a positive
 * Crouch drops the body toward -Z.
 */
struct FAetherbornFighterOffset
{
	/** Torso twist around Z. + = twist to the hero's right. */
	float Yaw = 0.f;
	/** Side lean around Y. */
	float Roll = 0.f;
	/** Pitch: + = lean back (coil), - = lean into the strike. */
	float Lean = 0.f;
	/** Forward step along local +Y, centimetres. Negative steps back. */
	float Lunge = 0.f;
	/** Downward drop, centimetres. */
	float Crouch = 0.f;
};

/** The coil / strike / settle offsets that give one attack slot its silhouette. */
struct FAetherbornFighterStyle
{
	FAetherbornFighterOffset Coil;
	FAetherbornFighterOffset Strike;
	FAetherbornFighterOffset Settle;
};

/**
 * The armoured hero body.
 *
 * Owns a single USkeletalMeshComponent (BodyMeshComponent) driven by the
 * Blueprint-assigned BodyMesh slot. ApplyAnim() records the pose the combo
 * machine wants; the actor ticks, builds this frame's target offset from the
 * authored styles, damps the live offsets toward it and layers the idle
 * breathing on top (hero.js update()).
 *
 * Motion is authored for a body facing local +Y, so the actor's Yaw stays a
 * plain Blueprint-editable default; BodyYawOffset corrects the mesh if the
 * imported character faces a different axis.
 */
UCLASS(Blueprintable)
class ECLIPSEROTF_API AAetherbornComboTimingFighter : public AActor
{
	GENERATED_BODY()

public:
	AAetherbornComboTimingFighter();

	/**
	 * Records this frame's performance request.
	 *
	 * @param PoseStep  1..3 selects the attack slot (1 right-to-left slash, 2 left-to-right,
	 *                  3 overhead heavy cleave); 0 is the idle guard.
	 * @param Phase     Which beat of the attack the machine is in.
	 * @param PhaseT    Normalised progress through that beat, 0..1.
	 * @param AttackT   Elapsed seconds inside the whole attack, used to key the montage slot.
	 */
	void ApplyAnim(int32 PoseStep, EAetherbornComboTimingPhase Phase, float PhaseT, float AttackT);

	/** Adds a short recoil impulse on top of the current pose (the dummy hit back). */
	void OnHitLanded(float Damage, bool bHeavy);

	/** Snaps the body back to the idle guard and drops the attack, recoil and montage state. */
	void ResetPose();

	/** Rigged biped the Blueprint child assigns. Never a hard-coded content path. */
	UPROPERTY(EditDefaultsOnly, Category = "Aetherborn|Body")
	TObjectPtr<USkeletalMesh> BodyMesh;

	/**
	 * AnimSequences for attack slots 1..3, retargeted onto this body's skeleton.
	 * Played as a single-node animation, so no AnimBlueprint slot is required.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Aetherborn|Body")
	TArray<TObjectPtr<UAnimSequence>> LightAnimations;

	/** Sequence for the slot-3 overhead cleave. */
	UPROPERTY(EditDefaultsOnly, Category = "Aetherborn|Body")
	TObjectPtr<UAnimSequence> HeavyAnimation;

	/** Yaw added to the mesh, for characters whose authored forward is not +Y. */
	UPROPERTY(EditDefaultsOnly, Category = "Aetherborn|Body")
	float BodyYawOffset = 0.f;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	/** Pushes the BodyMesh slot onto the component when it differs. */
	void SyncBodyMesh();

	/** Builds this frame's target offset from AnimStep / AnimPhase / AnimPhaseT. */
	FAetherbornFighterOffset ComputeTargetOffset(const FAetherbornFighterOffset& RecoilOffset) const;

	/** Plays (or skips, logging once) the montage for an attack slot. */
	void PlayAttackMontage(int32 PoseStep);

	/** The style table entry for a pose step, clamped to the authored slots. */
	static const FAetherbornFighterStyle& StyleForStep(int32 PoseStep);

	/** Root scene component; the mesh hangs off it so gameplay transforms stay clean. */
	UPROPERTY(VisibleAnywhere, Category = "Aetherborn|Body")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category = "Aetherborn|Body")
	TObjectPtr<USkeletalMeshComponent> BodyMeshComponent;

	/** Live damped offset actually written to the mesh component. */
	FAetherbornFighterOffset Current;

	/** Idle breathing clock, seconds (hero.js time). */
	float PoseTime = 0.f;

	/** Recoil intensity, 0..1, decaying after every landed hit. */
	float RecoilAmount = 0.f;
	/** How fast the recoil bleeds off, 1/seconds; a heavy hit snaps back slower. */
	float RecoilDamping = 6.f;

	/** Last request received from ApplyAnim(). */
	int32 AnimStep = 0;
	EAetherbornComboTimingPhase AnimPhase = EAetherbornComboTimingPhase::Neutral;
	float AnimPhaseT = 0.f;
	float AnimAttackT = 0.f;

	/** Phase seen on the previous ApplyAnim(), so an attack start can be detected. */
	EAetherbornComboTimingPhase LastPhase = EAetherbornComboTimingPhase::Neutral;
	/** Pose step of the montage currently playing, 0 when none. */
	int32 MontageStep = 0;
	/** One-shot log guard so a missing/incompatible montage only warns once. */
	bool bMontageSkipLogged = false;
};
