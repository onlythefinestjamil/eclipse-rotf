// Aetherborn combo: camera-only pawn. The arena owns the rig math; this follows it.

#include "AetherbornComboPawn.h"

#include "Camera/CameraComponent.h"
#include "EngineUtils.h"

#include "AetherbornComboArena.h"
#include "AetherbornComboTypes.h"

AAetherbornComboPawn::AAetherbornComboPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// The camera is the pawn's root: the arena rig supplies both its location and its rotation.
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	SetRootComponent(Camera);

	Camera->FieldOfView = 79.0f;
	Camera->bUsePawnControlRotation = false;

	// Keep the vertical FOV fixed (config.js camera: fov 79, aspect maintain Y).
	Camera->bOverrideAspectRatioAxisConstraint = true;
	Camera->AspectRatioAxisConstraint = AspectRatio_MaintainYFOV;
}

void AAetherbornComboPawn::BeginPlay()
{
	Super::BeginPlay();

	if (!Arena)
	{
		for (TActorIterator<AAetherbornComboArena> It(GetWorld()); It; ++It)
		{
			Arena = *It;
			break;
		}
	}

	if (!Arena)
	{
		UE_LOG(LogAetherbornCombo, Warning, TEXT("%s: no AAetherbornComboArena found in the level; the camera will not move."), *GetName());
		return;
	}

	UE_LOG(LogAetherbornCombo, Log, TEXT("%s: following arena %s."), *GetName(), *Arena->GetName());

	// Snap to the rig once so the first rendered frame is already framed correctly.
	SetActorLocation(Arena->GetCameraLocation());
	const FVector Target = Arena->GetCameraTarget();
	SetActorRotation((Target - GetActorLocation()).Rotation());
}

void AAetherbornComboPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!Arena)
	{
		return;
	}

	Arena->UpdateRig(DeltaSeconds);

	SetActorLocation(Arena->GetCameraLocation());

	const FVector Target = Arena->GetCameraTarget();
	SetActorRotation((Target - GetActorLocation()).Rotation());
}
