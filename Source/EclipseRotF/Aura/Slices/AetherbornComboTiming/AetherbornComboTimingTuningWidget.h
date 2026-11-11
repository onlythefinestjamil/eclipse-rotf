// Copyright (c) Eclipse Rot. All Rights Reserved.
//
// Aetherborn combo timing - the feel-tuning panel.
//
// Port of src/controls.js createPanelSpec(): the Slice asked for a panel with a
// combo-window range, an attack-speed range and an early-input buffering toggle.
// The first two get a slider, the toggle a check box, and each echoes its value
// in a text block next to it so the panel reads the same as the Slice's.
//
// Every binding is optional on purpose: the widget is expected to be built up in
// the editor over time, and a half-built tree must still construct and tick.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Aura/Slices/AetherbornComboTiming/AetherbornComboTimingTypes.h"

#include "AetherbornComboTimingTuningWidget.generated.h"

class UCheckBox;
class USlider;
class UTextBlock;
class AAetherbornComboTimingPlayerController;

/**
 * The tuning panel: combo window duration, attack speed and the buffering toggle.
 *
 * Edits are pushed straight through AAetherbornComboTimingPlayerController, which
 * forwards them to the possessed pawn's combo machine, so the panel never owns a
 * copy of the tuning itself.
 */
UCLASS()
class ECLIPSEROTF_API UAetherbornComboTimingTuningWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Combo window duration in seconds (AetherbornComboTiming::ComboWindowMin..ComboWindowMax). */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USlider> Sl_ComboWindow;

	/** Attack speed multiplier (AetherbornComboTiming::SpeedMin..SpeedMax). */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USlider> Sl_Speed;

	/** Early-input buffering on/off. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCheckBox> Cb_Buffer;

	/** Echoes Sl_ComboWindow as "0.28 s". */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Txt_ComboWindowValue;

	/** Echoes Sl_Speed as "1.00x". */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Txt_SpeedValue;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	/** Reads the live tuning (pawn if we can reach it, otherwise the plan defaults) into the controls. */
	void RefreshControlsFromTuning();

	/** Writes one slider/checkbox change through to the controller. */
	void PushToController();

	/** Rebuilds the two value labels from the current control positions. */
	void RefreshValueLabels();

	/** The owning Aetherborn player controller, or null. */
	AAetherbornComboTimingPlayerController* GetAetherController() const;

	/** Slider / check box callbacks. */
	UFUNCTION()
	void HandleComboWindowChanged(float Value);

	UFUNCTION()
	void HandleSpeedChanged(float Value);

	UFUNCTION()
	void HandleBufferChanged(bool bIsChecked);

	/** True while the controls are being written programmatically, so the change
	 *  callbacks do not echo back into the controller (and the tuning) again. */
	bool bSyncing = false;

	/** True once the initial refresh has run, so a late-native pawn still syncs. */
	bool bRefreshedOnce = false;
};
