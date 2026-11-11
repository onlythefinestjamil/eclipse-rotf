// Copyright (c) Eclipse Rot. All Rights Reserved.
//
// Aetherborn combo timing - game mode.
//
// The Slice is a single-actor playground: no waves, no scoring, no spawning of its
// own. The game mode exists only so a level can be pointed at this slice and come
// up with the right pawn and the right player controller, which is what owns the
// HUD and tuning widgets.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Aura/Slices/AetherbornComboTiming/AetherbornComboTimingTypes.h"

#include "AetherbornComboTimingGameMode.generated.h"

/** Sets DefaultPawnClass / PlayerControllerClass in its constructor; nothing else. */
UCLASS()
class ECLIPSEROTF_API AAetherbornComboTimingGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AAetherbornComboTimingGameMode();
};
