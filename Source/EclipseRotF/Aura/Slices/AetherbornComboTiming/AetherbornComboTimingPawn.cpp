// Copyright Eclipse RotF. Aetherborn combo timing slice (Eclipse timings).

#include "AetherbornComboTimingPawn.h"

#include "AetherbornComboTimingArena.h"
#include "AetherbornComboTimingComboMachine.h"
#include "AetherbornComboTimingFighter.h"

#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputMappingContext.h"

// The retuned timings live in namespace AetherbornComboTiming (Types.h).
using namespace AetherbornComboTiming;

namespace AetherbornComboTimingPawnConstants
{
	// The slice camera never moves: it frames the hero and the dummy like the DOM prototype.
	const FVector CameraLocation(-435.f, 160.f, 205.f);
	const FVector CameraTarget(10.f, 105.f, 115.f);
	constexpr float CameraFieldOfView = 66.f;

	/** Enhanced Input mapping context priority; the slice only has one context. */
	constexpr int32 DefaultMappingPriority = 0;
}

AAetherbornComboTimingPawn::AAetherbornComboTimingPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(SceneRoot);

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(SceneRoot);
	ViewCamera->SetRelativeLocation(FVector::ZeroVector);
	ViewCamera->SetRelativeRotation(FRotator::ZeroRotator);
	ViewCamera->FieldOfView = AetherbornComboTimingPawnConstants::CameraFieldOfView;
	ViewCamera->bUsePawnControlRotation = false;

	// The machine is a plain UObject owned by the pawn; it holds the whole simulation.
	Machine = CreateDefaultSubobject<UAetherbornComboTimingComboMachine>(TEXT("ComboMachine"));

	// The camera is fixed, so the controller never rotates the pawn.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	SetActorLocation(AetherbornComboTimingPawnConstants::CameraLocation);
	SetActorRotation((AetherbornComboTimingPawnConstants::CameraTarget - AetherbornComboTimingPawnConstants::CameraLocation).Rotation());

	// Default feel tuning: 0.28 s combo window, 1x speed, buffering on.
	Tuning.ComboWindow = 0.28f;
	Tuning.SpeedMul = 1.f;
	Tuning.bBuffer = true;
}

void AAetherbornComboTimingPawn::BeginPlay()
{
	Super::BeginPlay();

	// The game mode spawns us at a player start; the slice camera is static, so pin it here.
	SetActorLocation(AetherbornComboTimingPawnConstants::CameraLocation);
	SetActorRotation((AetherbornComboTimingPawnConstants::CameraTarget - AetherbornComboTimingPawnConstants::CameraLocation).Rotation());

	if (Machine)
	{
		Machine->Reset();
		IdleSnapshot = UAetherbornComboTimingComboMachine::MakeIdlePreview(Tuning);
	}

	CacheSceneActors();
}

void AAetherbornComboTimingPawn::CacheSceneActors()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (!Fighter.IsValid())
	{
		for (TActorIterator<AAetherbornComboTimingFighter> It(World); It; ++It)
		{
			Fighter = *It;
			break;
		}
	}

	if (!Arena.IsValid())
	{
		for (TActorIterator<AAetherbornComboTimingArena> It(World); It; ++It)
		{
			Arena = *It;
			break;
		}
	}
}

void AAetherbornComboTimingPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	APlayerController* PlayerController = Cast<APlayerController>(NewController);
	if (!PlayerController)
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		if (DefaultMappingContext)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, AetherbornComboTimingPawnConstants::DefaultMappingPriority);
		}
		else
		{
			UE_LOG(LogAetherbornComboTiming, Warning, TEXT("Pawn has no DefaultMappingContext assigned; keyboard input will not reach the combo machine."));
		}
	}
}

void AAetherbornComboTimingPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInput)
	{
		UE_LOG(LogAetherbornComboTiming, Warning, TEXT("Pawn expects an EnhancedInputComponent; no keys were bound."));
		return;
	}

	if (LightAction)
	{
		EnhancedInput->BindAction(LightAction, ETriggerEvent::Started, this, &ThisClass::HandleLightPressed);
	}

	if (HeavyAction)
	{
		EnhancedInput->BindAction(HeavyAction, ETriggerEvent::Started, this, &ThisClass::HandleHeavyPressed);
	}

	if (ResetAction)
	{
		EnhancedInput->BindAction(ResetAction, ETriggerEvent::Started, this, &ThisClass::HandleResetPressed);
	}
}

void AAetherbornComboTimingPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!Machine)
	{
		return;
	}

	// 1. One-shot presses collected from keys or from the HUD since the last tick.
	const bool bLightPress = bPendingLight;
	const bool bHeavyPress = bPendingHeavy;
	bPendingLight = false;
	bPendingHeavy = false;

	// 2. Advance the combo machine once.
	Machine->Update(DeltaSeconds, bLightPress, bHeavyPress, Tuning);

	// 3. Read what it produced.
	const FAetherbornComboTimingSnapshot& Snapshot = Machine->GetSnapshot();

	// 4. Drive the pose.
	AAetherbornComboTimingFighter* LiveFighter = Fighter.Get();
	if (LiveFighter)
	{
		// AttackT is elapsed seconds inside the whole attack (it keys the montage slot).
		LiveFighter->ApplyAnim(Snapshot.PoseStep, Snapshot.Phase, Snapshot.PhaseT, Snapshot.AttackElapsed);
	}

	// 5. Drive the arena, then cash in this tick's hit events at the dummy.
	AAetherbornComboTimingArena* LiveArena = Arena.Get();
	if (LiveArena)
	{
		LiveArena->SetSnapshot(Snapshot);

		for (const FAetherbornComboTimingEvent& Event : Machine->GetEvents())
		{
			if (Event.Type != EAetherbornComboTimingEventType::Hit)
			{
				continue;
			}

			const FVector HitPoint = LiveArena->GetHitPoint();
			LiveArena->SpawnHit(HitPoint, Event.Damage, Event.bHeavy);

			if (LiveFighter)
			{
				LiveFighter->OnHitLanded(Event.Damage, Event.bHeavy);
			}

			OnHitLanded.Broadcast(HitPoint, Event.Damage, Event.bHeavy);
		}
	}
}

const FAetherbornComboTimingSnapshot& AAetherbornComboTimingPawn::GetSnapshot() const
{
	if (Machine)
	{
		return Machine->GetSnapshot();
	}

	return IdleSnapshot;
}

const FAetherbornComboTimingTuning& AAetherbornComboTimingPawn::GetTuning() const
{
	return Tuning;
}

void AAetherbornComboTimingPawn::SetTuning(const FAetherbornComboTimingTuning& InTuning)
{
	Tuning.ComboWindow = FMath::Clamp(InTuning.ComboWindow, ComboWindowMin, ComboWindowMax);
	Tuning.SpeedMul = FMath::Clamp(InTuning.SpeedMul, SpeedMin, SpeedMax);
	Tuning.bBuffer = InTuning.bBuffer;
}

void AAetherbornComboTimingPawn::SetComboWindow(float InWindow)
{
	Tuning.ComboWindow = FMath::Clamp(InWindow, ComboWindowMin, ComboWindowMax);
}

void AAetherbornComboTimingPawn::SetSpeed(float InSpeed)
{
	Tuning.SpeedMul = FMath::Clamp(InSpeed, SpeedMin, SpeedMax);
}

void AAetherbornComboTimingPawn::SetBuffer(bool bInBuffer)
{
	Tuning.bBuffer = bInBuffer;
}

void AAetherbornComboTimingPawn::ResetCombo()
{
	bPendingLight = false;
	bPendingHeavy = false;

	if (Machine)
	{
		Machine->Reset();
	}

	if (AAetherbornComboTimingFighter* LiveFighter = Fighter.Get())
	{
		LiveFighter->ResetPose();
	}

	if (AAetherbornComboTimingArena* LiveArena = Arena.Get())
	{
		LiveArena->ResetArena();
	}
}

void AAetherbornComboTimingPawn::RequestLight()
{
	bPendingLight = true;
}

void AAetherbornComboTimingPawn::RequestHeavy()
{
	bPendingHeavy = true;
}

void AAetherbornComboTimingPawn::HandleLightPressed()
{
	RequestLight();
}

void AAetherbornComboTimingPawn::HandleHeavyPressed()
{
	RequestHeavy();
}

void AAetherbornComboTimingPawn::HandleResetPressed()
{
	ResetCombo();
}
