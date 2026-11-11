// AetherbornComboTiming — the combat / timing simulation.
//
// Port of the Slice's src/combo.js createCombo(), reduced to the single-player
// attack model: damage only on the Eclipse Hit Window notify, then a short
// hit-stop; the Eclipse Combo Window opens at the start of the swing's recovery;
// a press before it buffers (EARLY when buffering is off), inside it chains, after
// it is late, at CHAIN_MAX it is max; the heavy is a separate 1.35 s commitment
// that refuses everything. Deterministic and allocation-free per frame.
#pragma once

#include "CoreMinimal.h"

#include "UObject/Object.h"

#include "AetherbornComboTimingTypes.h"

#include "AetherbornComboTimingComboMachine.generated.h"

/**
 * One attack at a time, driven by Update().
 * GetSnapshot() returns the view the HUD and the arena read; GetEvents() the
 * events of the last Update() only.
 */
UCLASS(BlueprintType)
class ECLIPSEROTF_API UAetherbornComboTimingComboMachine : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Advance the sim. Hit-stop burns DeltaSeconds first and returns early while the
	 * sim is still frozen (presses are stashed for the next live frame).
	 */
	void Update(float DeltaSeconds, bool bLightPress, bool bHeavyPress, const FAetherbornComboTimingTuning& Tuning);

	/** Back to neutral, everything zeroed (the session best included). */
	void Reset();

	const FAetherbornComboTimingSnapshot& GetSnapshot() const;

	const TArray<FAetherbornComboTimingEvent>& GetEvents() const;

	/** Damage of light chain step Step (1..5): 40, 44, 48, 52, 56. */
	static float LightDamageForStep(int32 Step);

	/** The snapshot of an idle machine; safe before the first Update(). */
	static FAetherbornComboTimingSnapshot MakeIdlePreview(const FAetherbornComboTimingTuning& Tuning);

private:
	void SetInput(EAetherbornComboTimingVerdict Verdict, EAetherbornComboTimingRefuse Reason = EAetherbornComboTimingRefuse::None);
	void StartChain();
	void StartLight(int32 NextStep);
	void StartHeavy();
	/** Deal the running attack's damage and start its hit-stop. */
	void Land();
	/** End the running attack, emitting ChainEnd (or Dropped when nothing landed). */
	void EndAttack();
	void RebuildSnapshot(const FAetherbornComboTimingTuning& Tuning) const;

	EAetherbornComboTimingKind Kind = EAetherbornComboTimingKind::Idle;
	int32 Step = 1;
	float Elapsed = 0.f;
	int32 Combo = 0;
	int32 Best = 0;
	bool bQueued = false;

	/** True when the buffered press is a heavy, so it fires as the finisher. */
	bool bQueuedHeavy = false;
	bool bLanded = false;
	float FreezeT = 0.f;
	float FreezeDur = 0.f;
	EAetherbornComboTimingVerdict LastInput = EAetherbornComboTimingVerdict::None;
	int32 LastInputSeq = 0;
	EAetherbornComboTimingRefuse RefuseReason = EAetherbornComboTimingRefuse::None;
	float Clock = 0.f;
	float LastHitClock = 0.f;
	FAetherbornComboTimingTuning LastTuning;
	/** Presses made while the sim was fully frozen, kept for the next live frame. */
	bool bStashedLight = false;
	bool bStashedHeavy = false;
	TArray<FAetherbornComboTimingEvent> Events;
	mutable FAetherbornComboTimingSnapshot CachedSnapshot;
};
