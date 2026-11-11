// Copyright Eclipse RotF. Aetherborn combo timing slice (Eclipse timings).

#include "AetherbornComboTimingPlayerController.h"

#include "AetherbornComboTimingHUDWidget.h"
#include "AetherbornComboTimingPawn.h"
#include "AetherbornComboTimingTuningWidget.h"

#include "Blueprint/UserWidget.h"
#include "Engine/World.h"

AAetherbornComboTimingPlayerController::AAetherbornComboTimingPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = false;
}

void AAetherbornComboTimingPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// UMG only ticks a widget that asks for it, so the HUD is refreshed from here as
	// well; its update is frame-stamped so this never doubles the per-frame push.
	if (HUDWidget)
	{
		HUDWidget->Refresh(DeltaSeconds);
	}
}

void AAetherbornComboTimingPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}

	EnsureWidgets();

	// Game and UI: the HUD buttons and the tuning sliders take the mouse, the cursor is
	// always visible, and Enhanced Input keeps driving the pawn while a widget is up.
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);

	bShowMouseCursor = true;
}

void AAetherbornComboTimingPlayerController::EnsureWidgets()
{
	if (!HUDWidget && HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UAetherbornComboTimingHUDWidget>(this, HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport(0);
		}
		else
		{
			UE_LOG(LogAetherbornComboTiming, Warning, TEXT("Could not create the combo HUD widget from HUDWidgetClass."));
		}
	}

	if (!TuningWidget && TuningWidgetClass)
	{
		TuningWidget = CreateWidget<UAetherbornComboTimingTuningWidget>(this, TuningWidgetClass);
		if (TuningWidget)
		{
			TuningWidget->AddToViewport(1);
		}
		else
		{
			UE_LOG(LogAetherbornComboTiming, Warning, TEXT("Could not create the feel tuning widget from TuningWidgetClass."));
		}
	}
}

AAetherbornComboTimingPawn* AAetherbornComboTimingPlayerController::GetComboPawn() const
{
	return Cast<AAetherbornComboTimingPawn>(GetPawn());
}

void AAetherbornComboTimingPlayerController::SetComboWindow(float InWindow)
{
	if (AAetherbornComboTimingPawn* ComboPawn = GetComboPawn())
	{
		ComboPawn->SetComboWindow(InWindow);
	}
}

void AAetherbornComboTimingPlayerController::SetSpeed(float InSpeed)
{
	if (AAetherbornComboTimingPawn* ComboPawn = GetComboPawn())
	{
		ComboPawn->SetSpeed(InSpeed);
	}
}

void AAetherbornComboTimingPlayerController::SetBuffer(bool bInBuffer)
{
	if (AAetherbornComboTimingPawn* ComboPawn = GetComboPawn())
	{
		ComboPawn->SetBuffer(bInBuffer);
	}
}

void AAetherbornComboTimingPlayerController::ResetCombo()
{
	if (AAetherbornComboTimingPawn* ComboPawn = GetComboPawn())
	{
		ComboPawn->ResetCombo();
	}
}

void AAetherbornComboTimingPlayerController::RequestLight()
{
	if (AAetherbornComboTimingPawn* ComboPawn = GetComboPawn())
	{
		ComboPawn->RequestLight();
	}
}

void AAetherbornComboTimingPlayerController::RequestHeavy()
{
	if (AAetherbornComboTimingPawn* ComboPawn = GetComboPawn())
	{
		ComboPawn->RequestHeavy();
	}
}

UAetherbornComboTimingHUDWidget* AAetherbornComboTimingPlayerController::GetHUDWidget() const
{
	return HUDWidget;
}

UAetherbornComboTimingTuningWidget* AAetherbornComboTimingPlayerController::GetTuningWidget() const
{
	return TuningWidget;
}
