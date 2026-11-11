// Copyright (c) Eclipse Rot. All Rights Reserved.
//
// Aetherborn combo timing - the feel-tuning panel.
//
// Port of src/controls.js createPanelSpec(). The Slice built its panel from a
// spec; this is the same three controls, with the ranges coming from the tuning
// constants rather than being typed into the widget:
//
//   comboWindow  range  0.05 .. 0.50  step 0.01  default 0.28
//   speed        range  0.40 .. 2.00  step 0.05  default 1.00
//   buffer       toggle on by default
//
// Edits are pushed through AAetherbornComboTimingPlayerController, which owns the
// hop to the possessed pawn's combo machine.

#include "Aura/Slices/AetherbornComboTiming/AetherbornComboTimingTuningWidget.h"

#include "Aura/Slices/AetherbornComboTiming/AetherbornComboTimingPawn.h"
#include "Aura/Slices/AetherbornComboTiming/AetherbornComboTimingPlayerController.h"

#include "Components/CheckBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "GameFramework/Pawn.h"

namespace
{
	/** Panel step sizes, from the plan's Interface (src/controls.js COMBO_WINDOW_RANGE / SPEED_RANGE). */
	constexpr float ComboWindowStep = 0.01f;
	constexpr float SpeedStep = 0.05f;
}

void UAetherbornComboTimingTuningWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// The ranges live in code so the panel and the machine can never disagree about
	// what is a legal combo window.
	if (Sl_ComboWindow)
	{
		Sl_ComboWindow->SetMinValue(AetherbornComboTiming::ComboWindowMin);
		Sl_ComboWindow->SetMaxValue(AetherbornComboTiming::ComboWindowMax);
		Sl_ComboWindow->SetStepSize(ComboWindowStep);
		Sl_ComboWindow->OnValueChanged.RemoveDynamic(this, &UAetherbornComboTimingTuningWidget::HandleComboWindowChanged);
		Sl_ComboWindow->OnValueChanged.AddDynamic(this, &UAetherbornComboTimingTuningWidget::HandleComboWindowChanged);
	}

	if (Sl_Speed)
	{
		Sl_Speed->SetMinValue(AetherbornComboTiming::SpeedMin);
		Sl_Speed->SetMaxValue(AetherbornComboTiming::SpeedMax);
		Sl_Speed->SetStepSize(SpeedStep);
		Sl_Speed->OnValueChanged.RemoveDynamic(this, &UAetherbornComboTimingTuningWidget::HandleSpeedChanged);
		Sl_Speed->OnValueChanged.AddDynamic(this, &UAetherbornComboTimingTuningWidget::HandleSpeedChanged);
	}

	if (Cb_Buffer)
	{
		Cb_Buffer->OnCheckStateChanged.RemoveDynamic(this, &UAetherbornComboTimingTuningWidget::HandleBufferChanged);
		Cb_Buffer->OnCheckStateChanged.AddDynamic(this, &UAetherbornComboTimingTuningWidget::HandleBufferChanged);
	}

	RefreshControlsFromTuning();
}

void UAetherbornComboTimingTuningWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// A widget created before the pawn is possessed cannot read the live tuning in
	// Construct; retry until it can, then leave the controls entirely to the user.
	if (bRefreshedOnce)
	{
		return;
	}

	if (const AAetherbornComboTimingPlayerController* Controller = GetAetherController())
	{
		if (Controller->GetPawn() != nullptr)
		{
			RefreshControlsFromTuning();
		}
	}
}

AAetherbornComboTimingPlayerController* UAetherbornComboTimingTuningWidget::GetAetherController() const
{
	return Cast<AAetherbornComboTimingPlayerController>(GetOwningPlayer());
}

void UAetherbornComboTimingTuningWidget::RefreshControlsFromTuning()
{
	// The plan's defaults are the fallback whenever the pawn is not reachable yet.
	const FAetherbornComboTimingTuning Defaults;
	FAetherbornComboTimingTuning Live = Defaults;
	bool bHaveLive = false;

	if (const AAetherbornComboTimingPlayerController* Controller = GetAetherController())
	{
		if (const AAetherbornComboTimingPawn* Pawn = Cast<AAetherbornComboTimingPawn>(Controller->GetPawn()))
		{
			Live = Pawn->GetTuning();
			bHaveLive = true;
		}
	}

	// Writing the controls re-enters the change callbacks; bSyncing keeps them from
	// pushing the values we just read straight back into the machine.
	bSyncing = true;

	if (Sl_ComboWindow)
	{
		Sl_ComboWindow->SetValue(FMath::Clamp(Live.ComboWindow,
			AetherbornComboTiming::ComboWindowMin, AetherbornComboTiming::ComboWindowMax));
	}
	if (Sl_Speed)
	{
		Sl_Speed->SetValue(FMath::Clamp(Live.SpeedMul,
			AetherbornComboTiming::SpeedMin, AetherbornComboTiming::SpeedMax));
	}
	if (Cb_Buffer)
	{
		Cb_Buffer->SetIsChecked(Live.bBuffer);
	}

	bSyncing = false;

	// Only stop re-reading once a real pawn supplied the values, so the panel keeps
	// following the machine until then.
	bRefreshedOnce = bRefreshedOnce || bHaveLive;

	RefreshValueLabels();
}

void UAetherbornComboTimingTuningWidget::RefreshValueLabels()
{
	const FAetherbornComboTimingTuning Defaults;

	if (Txt_ComboWindowValue)
	{
		const float Value = Sl_ComboWindow ? Sl_ComboWindow->GetValue() : Defaults.ComboWindow;
		Txt_ComboWindowValue->SetText(FText::FromString(FString::Printf(TEXT("%.2f s"), Value)));
	}

	if (Txt_SpeedValue)
	{
		const float Value = Sl_Speed ? Sl_Speed->GetValue() : Defaults.SpeedMul;
		Txt_SpeedValue->SetText(FText::FromString(FString::Printf(TEXT("%.2fx"), Value)));
	}
}

void UAetherbornComboTimingTuningWidget::PushToController()
{
	if (bSyncing)
	{
		return;
	}

	if (AAetherbornComboTimingPlayerController* Controller = GetAetherController())
	{
		if (Sl_ComboWindow)
		{
			Controller->SetComboWindow(Sl_ComboWindow->GetValue());
		}
		if (Sl_Speed)
		{
			Controller->SetSpeed(Sl_Speed->GetValue());
		}
		if (Cb_Buffer)
		{
			Controller->SetBuffer(Cb_Buffer->IsChecked());
		}
	}
	else
	{
		// Expected while previewing the widget in the designer, so never a warning.
		UE_LOG(LogAetherbornComboTiming, Verbose,
			TEXT("%s: no Aetherborn player controller (unpossessed) - the tuning change was not applied."),
			*GetName());
	}

	RefreshValueLabels();
}

void UAetherbornComboTimingTuningWidget::HandleComboWindowChanged(float Value)
{
	if (bSyncing)
	{
		return;
	}

	if (Sl_ComboWindow && !FMath::IsNearlyEqual(Sl_ComboWindow->GetValue(), Value))
	{
		Sl_ComboWindow->SetValue(Value);
	}

	PushToController();
}

void UAetherbornComboTimingTuningWidget::HandleSpeedChanged(float Value)
{
	if (bSyncing)
	{
		return;
	}

	if (Sl_Speed && !FMath::IsNearlyEqual(Sl_Speed->GetValue(), Value))
	{
		Sl_Speed->SetValue(Value);
	}

	PushToController();
}

void UAetherbornComboTimingTuningWidget::HandleBufferChanged(bool bIsChecked)
{
	if (bSyncing)
	{
		return;
	}

	if (Cb_Buffer && Cb_Buffer->IsChecked() != bIsChecked)
	{
		Cb_Buffer->SetIsChecked(bIsChecked);
	}

	PushToController();
}
