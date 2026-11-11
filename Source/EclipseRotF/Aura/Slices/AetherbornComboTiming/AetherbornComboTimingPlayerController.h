// Copyright Eclipse RotF. Aetherborn combo timing slice (Eclipse timings).
//
// The player controller creates the slice's two widgets and is the only thing the
// widgets talk to: they stay presentation-only and every tuning / input call lands here
// and is forwarded to the pawn that owns the combo machine.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "AetherbornComboTimingHUDWidget.h"
#include "AetherbornComboTimingTuningWidget.h"

#include "AetherbornComboTimingPlayerController.generated.h"

class AAetherbornComboTimingPawn;

UCLASS()
class ECLIPSEROTF_API AAetherbornComboTimingPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AAetherbornComboTimingPlayerController();

	//~ Begin AActor interface
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	//~ End AActor interface

	/** Combo window width in seconds (the HUD/tuning slider drives this). */
	UFUNCTION(BlueprintCallable, Category = "Aetherborn|Combo")
	void SetComboWindow(float InWindow);

	/** Playback speed multiplier (the tuning slider drives this). */
	UFUNCTION(BlueprintCallable, Category = "Aetherborn|Combo")
	void SetSpeed(float InSpeed);

	/** Input buffering toggle (the tuning checkbox drives this). */
	UFUNCTION(BlueprintCallable, Category = "Aetherborn|Combo")
	void SetBuffer(bool bInBuffer);

	/** Drops the chain, the pose and the arena. */
	UFUNCTION(BlueprintCallable, Category = "Aetherborn|Combo")
	void ResetCombo();

	/** Light attack request coming from the HUD button or a key. */
	UFUNCTION(BlueprintCallable, Category = "Aetherborn|Combo")
	void RequestLight();

	/** Heavy attack request coming from the HUD button or a key. */
	UFUNCTION(BlueprintCallable, Category = "Aetherborn|Combo")
	void RequestHeavy();

	UFUNCTION(BlueprintPure, Category = "Aetherborn|UI")
	UAetherbornComboTimingHUDWidget* GetHUDWidget() const;

	UFUNCTION(BlueprintPure, Category = "Aetherborn|UI")
	UAetherbornComboTimingTuningWidget* GetTuningWidget() const;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherborn|UI")
	TSubclassOf<UAetherbornComboTimingHUDWidget> HUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherborn|UI")
	TSubclassOf<UAetherbornComboTimingTuningWidget> TuningWidgetClass;

protected:
	/** Creates and adds both widgets if the classes are set and they are not up yet. */
	void EnsureWidgets();

private:
	/** The pawn that owns the combo machine, or null while nothing is possessed. */
	AAetherbornComboTimingPawn* GetComboPawn() const;

	UPROPERTY(Transient)
	TObjectPtr<UAetherbornComboTimingHUDWidget> HUDWidget;

	UPROPERTY(Transient)
	TObjectPtr<UAetherbornComboTimingTuningWidget> TuningWidget;
};
