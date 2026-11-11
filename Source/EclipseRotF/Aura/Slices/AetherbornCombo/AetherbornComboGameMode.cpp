// Copyright EclipseRotF. All Rights Reserved.

#include "AetherbornComboGameMode.h"

#include "AetherbornComboHUDWidget.h"
#include "AetherbornComboPawn.h"
#include "AetherbornComboPlayerController.h"

AAetherbornComboGameMode::AAetherbornComboGameMode()
{
	DefaultPawnClass = AAetherbornComboPawn::StaticClass();
	PlayerControllerClass = AAetherbornComboPlayerController::StaticClass();
}
