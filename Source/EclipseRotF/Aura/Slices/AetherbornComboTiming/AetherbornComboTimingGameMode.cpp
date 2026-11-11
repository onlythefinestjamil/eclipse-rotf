// Copyright (c) Eclipse Rot. All Rights Reserved.
//
// Aetherborn combo timing - game mode.

#include "Aura/Slices/AetherbornComboTiming/AetherbornComboTimingGameMode.h"

#include "Aura/Slices/AetherbornComboTiming/AetherbornComboTimingPawn.h"
#include "Aura/Slices/AetherbornComboTiming/AetherbornComboTimingPlayerController.h"

AAetherbornComboTimingGameMode::AAetherbornComboTimingGameMode()
{
	// The whole slice is one duel in one level: spawn the combo-timing pawn and the
	// controller that owns the HUD and the tuning panel. Everything else is authored
	// in the level (fighter, arena, camera framing), so nothing is overridden here.
	DefaultPawnClass = AAetherbornComboTimingPawn::StaticClass();
	PlayerControllerClass = AAetherbornComboTimingPlayerController::StaticClass();
}
