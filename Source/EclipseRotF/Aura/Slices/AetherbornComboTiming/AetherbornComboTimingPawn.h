// Copyright Eclipse RotF. Aetherborn combo timing slice (Eclipse timings).
//
// The pawn is the single owner of the combo simulation for the slice: it drives the
// machine's Update() once per tick, hands the resulting snapshot to the fighter and the
// arena, and fires the hit events at the arena's dummy hit point. It also carries the
// fixed slice camera and the Enhanced Input bindings that feed the machine.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#include "AetherbornComboTimingTypes.h"

#include "AetherbornComboTimingPawn.generated.h"

class AAetherbornComboTimingArena;
class AAetherbornComboTimingFighter;
class UAetherbornComboTimingComboMachine;
class UCameraComponent;
class UInputAction;
class UInputComponent;
class UInputMappingContext;

UCLASS()
class ECLIPSEROTF_API AAetherbornComboTimingPawn : public APawn
{
	GENERATED_BODY()

public:
	AAetherbornComboTimingPawn();

	//~ Begin AActor interface
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	//~ End AActor interface

	//~ Begin APawn interface
	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	//~ End APawn interface

	/** Latest machine snapshot. Returns an idle preview before the first update. */
	const FAetherbornComboTimingSnapshot& GetSnapshot() const;

	/** Current feel tuning: combo window width, speed multiplier, input buffering. */
	const FAetherbornComboTimingTuning& GetTuning() const;

	/** Replaces the whole tuning block, clamping every value to its legal range. */
	UFUNCTION(BlueprintCallable, Category = "Aetherborn|Combo")
	void SetTuning(const FAetherbornComboTimingTuning& InTuning);

	/** Combo window width in seconds, clamped to [ComboWindowMin, ComboWindowMax]. */
	UFUNCTION(BlueprintCallable, Category = "Aetherborn|Combo")
	void SetComboWindow(float InWindow);

	/** Playback speed multiplier, clamped to [SpeedMin, SpeedMax]. */
	UFUNCTION(BlueprintCallable, Category = "Aetherborn|Combo")
	void SetSpeed(float InSpeed);

	/** Enables or disables press buffering inside the combo window. */
	UFUNCTION(BlueprintCallable, Category = "Aetherborn|Combo")
	void SetBuffer(bool bInBuffer);

	/** Drops the chain, the pose and the arena back to their idle state. */
	UFUNCTION(BlueprintCallable, Category = "Aetherborn|Combo")
	void ResetCombo();

	/** One-shot light press, queued for the next tick's Update(). */
	UFUNCTION(BlueprintCallable, Category = "Aetherborn|Combo")
	void RequestLight();

	/** One-shot heavy press, queued for the next tick's Update(). */
	UFUNCTION(BlueprintCallable, Category = "Aetherborn|Combo")
	void RequestHeavy();

	/** Fired for every hit event, with the arena's hit point. */
	UPROPERTY(BlueprintAssignable, Category = "Aetherborn|Combo")
	FOnAetherbornComboTimingHit OnHitLanded;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherborn|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherborn|Input")
	TObjectPtr<UInputAction> LightAction;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherborn|Input")
	TObjectPtr<UInputAction> HeavyAction;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherborn|Input")
	TObjectPtr<UInputAction> ResetAction;

	/** Live feel tuning. ComboWindow 0.05-0.50 (default 0.28), SpeedMul 0.40-2.00. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherborn|Combo")
	FAetherbornComboTimingTuning Tuning;

private:
	void HandleLightPressed();
	void HandleHeavyPressed();
	void HandleResetPressed();

	/** Grabs the slice's fighter and arena from the level (they are level actors). */
	void CacheSceneActors();

	UPROPERTY(Transient)
	TObjectPtr<UAetherbornComboTimingComboMachine> Machine;

	UPROPERTY(Transient)
	TObjectPtr<UCameraComponent> ViewCamera;

	UPROPERTY(Transient)
	TWeakObjectPtr<AAetherbornComboTimingFighter> Fighter;

	UPROPERTY(Transient)
	TWeakObjectPtr<AAetherbornComboTimingArena> Arena;

	/** Snapshot handed out before the machine has ticked once. */
	FAetherbornComboTimingSnapshot IdleSnapshot;

	/** One-shot press flags, set by keys or by the HUD and consumed on the next tick. */
	bool bPendingLight = false;
	bool bPendingHeavy = false;
};
