#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Aura/Slices/AetherbornComboTiming/AetherbornComboTimingTypes.h"
#include "AetherbornComboTimingFx.generated.h"

class AActor;
class UFont;
class UNiagaraSystem;
class UTextRenderComponent;

/**
 * One pooled floating damage number. Plain data - the component itself lives in
 * UAetherbornComboTimingFx::NumberComponents, in the same order as these slots.
 */
struct FAetherbornComboTimingNumberSlot
{
	bool bActive = false;
	float Time = 0.f;
	FVector Origin = FVector::ZeroVector;
	FVector Drift = FVector::ZeroVector;
	float WorldSize = 0.f;
	/** Palette colour the digit was spawned with, faded out as the number dies. */
	FLinearColor Tint = FLinearColor::White;
};

/**
 * Pooled impact effects, ported from src/fx.js (createFx / spawnHit / update).
 *
 * This is not a Slice-facing type: AAetherbornComboTimingArena owns one of these as a
 * private helper and forwards SpawnHit() into it. The pool is built once (16 damage
 * numbers, all pre-spawned and hidden) so a hit never allocates or loads anything at
 * runtime - the same discipline as the JavaScript version.
 *
 * The Slice draws floating digits from canvas textures, spark bursts and expanding
 * shockwave rings. In engine we get the digits from world-space text render components
 * and leave the sparks + shockwave to the authored Niagara impact system, so the
 * runtime work here is: place a number, set its text/colour/size, rise + fade it.
 */
UCLASS()
class ECLIPSEROTF_API UAetherbornComboTimingFx : public UObject
{
	GENERATED_BODY()

public:
	UAetherbornComboTimingFx();

	/**
	 * Builds (or rebuilds) the pool. Owner must be the actor the text components belong
	 * to; both asset pointers may be null, in which case the matching effect is skipped.
	 */
	void Initialize(AActor* InOwner, UNiagaraSystem* InImpactNiagara, UFont* InDamageFont);

	/** Spawns one impact: a floating damage number, plus a Niagara burst scaled by bHeavy. */
	void SpawnHit(const FVector& Location, float Damage, bool bHeavy, int32 Combo);

	/** Advances the floating numbers (rise, fade); call once per frame from the arena tick. */
	void Tick(float DeltaSeconds);

	/** Hides every live number and restarts the deterministic jitter stream. */
	void Reset();

protected:
	/** Creates the pooled text components once; safe to call again. */
	void BuildPool();

	/** Next deterministic 0..1 value from the LCG (seed 0x9e3779b9, *1664525 + 1013904223). */
	float Rnd();

	/** Logs a warning at most once per key. */
	void WarnOnce(FName Key, const FString& Message);

	/** Fails the requested number slot, recycling the oldest one when the pool is full. */
	int32 TakeNumberSlot() const;

	/** Turns an active number to face the local player camera. */
	void BillboardToCamera(UTextRenderComponent* Component, const FVector& WorldLocation) const;

private:
	/** Seconds a floating number lives (fx.js NUM_LIFE). */
	static constexpr int32 NumberPoolSize = 16;
	static constexpr float NumberLife = 0.9f;
	/** Height the number floats up over its life, in cm (fx.js RISE 0.9 m). */
	static constexpr float NumberRise = 90.f;

	UPROPERTY()
	TWeakObjectPtr<AActor> OwnerActor;

	UPROPERTY()
	TObjectPtr<UNiagaraSystem> ImpactSystem;

	UPROPERTY()
	TObjectPtr<UFont> NumberFont;

	UPROPERTY()
	TArray<TObjectPtr<UTextRenderComponent>> NumberComponents;

	/** Parallel to NumberComponents. */
	TArray<FAetherbornComboTimingNumberSlot> Numbers;

	UPROPERTY()
	TSet<FName> WarnedKeys;

	/** Deterministic jitter state, seeded exactly like fx.js. */
	uint32 Seed = 0x9e3779b9u;
};
