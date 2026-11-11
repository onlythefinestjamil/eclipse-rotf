// Copyright EclipseRotF. All Rights Reserved.

#include "AetherbornComboPlayerController.h"

#include "AetherbornComboArena.h"
#include "AetherbornComboComboMachine.h"
#include "AetherbornComboDummy.h"
#include "AetherbornComboFighter.h"
#include "AetherbornComboHUDWidget.h"

#include "Blueprint/UserWidget.h"
#include "Components/InputComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"

AAetherbornComboPlayerController::AAetherbornComboPlayerController()
{
	// The HUD is pointer driven as well as keyboard driven.
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void AAetherbornComboPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// ------------------------------------------------------------------ parts
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AAetherbornComboArena> It(World); It; ++It)
		{
			Arena = *It;
			break;
		}
		for (TActorIterator<AAetherbornComboDummy> It(World); It; ++It)
		{
			Dummy = *It;
			break;
		}
		for (TActorIterator<AAetherbornComboFighter> It(World); It; ++It)
		{
			Fighter = *It;
			break;
		}
	}

	if (!Arena)
	{
		UE_LOG(LogAetherbornCombo, Warning, TEXT("AetherbornCombo: no AAetherbornComboArena in the level; camera rig and punch impulses are disabled."));
	}
	if (!Dummy)
	{
		UE_LOG(LogAetherbornCombo, Warning, TEXT("AetherbornCombo: no AAetherbornComboDummy in the level; hits will not register visually."));
	}
	if (!Fighter)
	{
		UE_LOG(LogAetherbornCombo, Warning, TEXT("AetherbornCombo: no AAetherbornComboFighter in the level; poses will not update."));
	}

	// ------------------------------------------------------------ combo machine
	Machine = NewObject<UComboMachine>(this);
	if (Machine)
	{
		Machine->OnSwing.AddDynamic(this, &AAetherbornComboPlayerController::HandleSwing);
		Machine->OnHit.AddDynamic(this, &AAetherbornComboPlayerController::HandleHit);
		Machine->OnChain.AddDynamic(this, &AAetherbornComboPlayerController::HandleChain);
		Machine->OnDrop.AddDynamic(this, &AAetherbornComboPlayerController::HandleDrop);
		Machine->Reset(true);
	}
	else
	{
		UE_LOG(LogAetherbornCombo, Error, TEXT("AetherbornCombo: failed to create the combo machine."));
	}

	// -------------------------------------------------------------------- HUD
	if (HUDClass)
	{
		HUDWidget = CreateWidget<UAetherbornComboHUDWidget>(this, HUDClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
			HUDWidget->OnAttackPressed.BindUObject(this, &AAetherbornComboPlayerController::AttackPressed);
			HUDWidget->OnResetPressed.BindUObject(this, &AAetherbornComboPlayerController::ResetPressed);
		}
		else
		{
			UE_LOG(LogAetherbornCombo, Warning, TEXT("AetherbornCombo: CreateWidget returned null for HUDClass %s."), *GetNameSafe(HUDClass.Get()));
		}
	}
	else
	{
		UE_LOG(LogAetherbornCombo, Warning, TEXT("AetherbornCombo: HUDClass is not set; the timing bar and the tuning panel will not appear."));
	}

	// ------------------------------------------------------------------ input
	SetInputMode(FInputModeGameAndUI().SetHideCursorDuringCapture(false));
	SetShowMouseCursor(true);

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (DefaultIMC)
			{
				Subsystem->AddMappingContext(DefaultIMC, 1);
			}
			else
			{
				UE_LOG(LogAetherbornCombo, Warning, TEXT("AetherbornCombo: DefaultIMC is not set; keyboard attack input is unavailable."));
			}
		}
	}

	UE_LOG(LogAetherbornCombo, Log, TEXT("Aetherborn combo director ready (arena=%s dummy=%s fighter=%s)"),
		*GetNameSafe(Arena), *GetNameSafe(Dummy), *GetNameSafe(Fighter));
}

void AAetherbornComboPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (AttackAction)
		{
			EnhancedInput->BindAction(AttackAction, ETriggerEvent::Started, this, &AAetherbornComboPlayerController::OnAttackAction);
		}
		else
		{
			UE_LOG(LogAetherbornCombo, Warning, TEXT("AetherbornCombo: AttackAction is not set on %s."), *GetNameSafe(this));
		}

		if (ResetAction)
		{
			EnhancedInput->BindAction(ResetAction, ETriggerEvent::Started, this, &AAetherbornComboPlayerController::OnResetAction);
		}
	}
	else
	{
		UE_LOG(LogAetherbornCombo, Warning, TEXT("AetherbornCombo: the input component is not a UEnhancedInputComponent; attack input is unavailable."));
	}
}

void AAetherbornComboPlayerController::PlayerTick(float DeltaTime)
{
	// The single driver loop: panel settings -> combo clock -> pose -> HUD.
	FComboSettings Settings;
	if (HUDWidget)
	{
		Settings = HUDWidget->ReadSettings();
	}

	if (Machine)
	{
		Machine->Update(DeltaTime, Settings);
		LastSnapshot = Machine->GetSnapshot();
	}

	if (Fighter)
	{
		Fighter->UpdatePose(DeltaTime, LastSnapshot);
	}

	if (HUDWidget)
	{
		HUDWidget->UpdateFromSnapshot(LastSnapshot, Settings);
	}

	Super::PlayerTick(DeltaTime);
}

void AAetherbornComboPlayerController::OnAttackAction(const FInputActionValue& Value)
{
	AttackPressed();
}

void AAetherbornComboPlayerController::OnResetAction(const FInputActionValue& Value)
{
	ResetPressed();
}

void AAetherbornComboPlayerController::AttackPressed()
{
	UE_LOG(LogAetherbornCombo, Log, TEXT("AetherbornCombo: attack pressed (step %d, phase %d, buffered %d)"),
		Machine ? Machine->GetStep() : 0,
		static_cast<int32>(LastSnapshot.Phase),
		LastSnapshot.bBuffered ? 1 : 0);

	if (Machine)
	{
		Machine->Press();
	}
}

void AAetherbornComboPlayerController::ResetPressed()
{
	if (Machine)
	{
		Machine->Reset(true);
	}
	if (Fighter)
	{
		Fighter->SetStep(0);
	}

	UE_LOG(LogAetherbornCombo, Log, TEXT("AetherbornCombo: chain reset (longest %d)"), LastSnapshot.Longest);
}

void AAetherbornComboPlayerController::HandleSwing(int32 Step)
{
	if (Fighter)
	{
		Fighter->SetStep(Step);
	}
}

void AAetherbornComboPlayerController::HandleHit(int32 Step, float Damage)
{
	if (Dummy)
	{
		Dummy->Hit(Damage);
	}
	if (Arena)
	{
		Arena->Punch(0.3f);
	}
	if (HUDWidget)
	{
		// The floating number belongs where the blow landed - on the dummy's chest,
		// not at the fighter's sword tip (which is where it was popping before).
		const FVector PopAt = Dummy ? Dummy->GetHitPoint()
			: (Fighter ? Fighter->GetHitPoint() : FVector::ZeroVector);
		HUDWidget->PopDamage(PopAt, Damage);
	}
}

void AAetherbornComboPlayerController::HandleChain(int32 Step)
{
	if (Arena)
	{
		Arena->Punch(0.75f);
	}
	if (HUDWidget)
	{
		HUDWidget->Pulse(0.95f);
	}
}

void AAetherbornComboPlayerController::HandleDrop(FString Reason)
{
	if (Arena)
	{
		Arena->Punch(0.1f);
	}

	// Only a genuine timeout earns the stronger miss feedback.
	if (Reason.Equals(TEXT("timeout"), ESearchCase::IgnoreCase) && HUDWidget)
	{
		HUDWidget->Pulse(0.42f);
	}
}

FComboSnapshot AAetherbornComboPlayerController::GetLastSnapshot() const
{
	return LastSnapshot;
}
