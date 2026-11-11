#include "Aura/Slices/AetherbornComboTiming/AetherbornComboTimingFx.h"

#include "Camera/PlayerCameraManager.h"
#include "Components/TextRenderComponent.h"
#include "Engine/Font.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Math/RotationMatrix.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

UAetherbornComboTimingFx::UAetherbornComboTimingFx()
{
	Numbers.Reserve(NumberPoolSize);
}

void UAetherbornComboTimingFx::Initialize(AActor* InOwner, UNiagaraSystem* InImpactNiagara, UFont* InDamageFont)
{
	const bool bOwnerChanged = (OwnerActor.Get() != InOwner);

	OwnerActor = InOwner;
	ImpactSystem = InImpactNiagara;
	NumberFont = InDamageFont;

	if (bOwnerChanged || NumberComponents.Num() != NumberPoolSize)
	{
		// The pool belongs to one actor; when the owner changes the old components stay
		// with the old actor and we simply forget them.
		NumberComponents.Reset();
		Numbers.Reset();
		BuildPool();
	}

	Reset();
}

void UAetherbornComboTimingFx::BuildPool()
{
	AActor* Owner = OwnerActor.Get();
	if (!Owner)
	{
		WarnOnce(TEXT("Owner"), TEXT("no owning actor - the damage numbers cannot be created"));
		return;
	}

	USceneComponent* AttachParent = Owner->GetRootComponent();

	NumberComponents.Reserve(NumberPoolSize);
	Numbers.Reserve(NumberPoolSize);

	for (int32 Index = 0; Index < NumberPoolSize; ++Index)
	{
		const FName ComponentName = *FString::Printf(TEXT("DamageNumber_%d"), Index);
		UTextRenderComponent* Comp = NewObject<UTextRenderComponent>(Owner, ComponentName);
		if (!Comp)
		{
			continue;
		}

		Comp->SetMobility(EComponentMobility::Movable);
		if (AttachParent)
		{
			Comp->SetupAttachment(AttachParent);
		}
		Comp->RegisterComponent();
		Owner->AddInstanceComponent(Comp);

		Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Comp->SetGenerateOverlapEvents(false);
		Comp->SetCastShadow(false);
		Comp->SetHorizontalAlignment(EHTA_Center);
		Comp->SetVerticalAlignment(EVRTA_TextCenter);
		Comp->SetWorldSize(32.f);
		Comp->SetText(FText::GetEmpty());
		if (NumberFont)
		{
			Comp->SetFont(NumberFont);
		}
		Comp->SetVisibility(false);

		NumberComponents.Add(Comp);
		Numbers.Add(FAetherbornComboTimingNumberSlot());
	}
}

float UAetherbornComboTimingFx::Rnd()
{
	// Deterministic LCG from fx.js: repeated hits vary, but a replay looks the same.
	Seed = Seed * 1664525u + 1013904223u;
	return static_cast<float>(static_cast<double>(Seed) / 4294967296.0);
}

int32 UAetherbornComboTimingFx::TakeNumberSlot() const
{
	int32 OldestIndex = 0;
	float OldestTime = -1.f;

	for (int32 Index = 0; Index < Numbers.Num(); ++Index)
	{
		if (!Numbers[Index].bActive)
		{
			return Index;
		}
		if (Numbers[Index].Time > OldestTime)
		{
			OldestTime = Numbers[Index].Time;
			OldestIndex = Index;
		}
	}

	return OldestIndex;
}

void UAetherbornComboTimingFx::BillboardToCamera(UTextRenderComponent* Component, const FVector& WorldLocation) const
{
	if (!Component)
	{
		return;
	}

	const AActor* Owner = OwnerActor.Get();
	const UWorld* World = Owner ? Owner->GetWorld() : nullptr;
	if (!World)
	{
		return;
	}

	const APlayerController* PC = World->GetFirstPlayerController();
	const APlayerCameraManager* CameraManager = PC ? PC->PlayerCameraManager : nullptr;
	if (!CameraManager)
	{
		return;
	}

	// World-space text is drawn on the component's local XY plane facing +X, so we point
	// +X away from the camera - the digit then faces the viewer.
	const FVector Away = (WorldLocation - CameraManager->GetCameraLocation()).GetSafeNormal();
	if (!Away.IsNearlyZero())
	{
		Component->SetWorldRotation(FRotationMatrix::MakeFromX(Away).Rotator());
	}
}

void UAetherbornComboTimingFx::SpawnHit(const FVector& Location, float Damage, bool bHeavy, int32 Combo)
{
	const int32 Chain = FMath::Clamp(Combo, 1, 99);
	const int32 DamageRounded = FMath::Clamp(FMath::RoundToInt(FMath::IsFinite(Damage) ? Damage : 0.f), 0, 9999);

	// ---- 1. floating damage number (cool + small for light, hot + big for heavy, growing
	//         with the chain length exactly like fx.js)
	const int32 SlotIndex = TakeNumberSlot();
	if (NumberComponents.IsValidIndex(SlotIndex) && Numbers.IsValidIndex(SlotIndex))
	{
		UTextRenderComponent* Comp = NumberComponents[SlotIndex];
		if (!Comp)
		{
			WarnOnce(TEXT("NumberComponent"), TEXT("a pooled damage number went missing - that hit shows no number"));
		}
		else
		{
			FAetherbornComboTimingNumberSlot& Slot = Numbers[SlotIndex];
			Slot.bActive = true;
			Slot.Time = 0.f;
			Slot.WorldSize = (bHeavy ? 46.f : 32.f) * (1.f + 0.09f * static_cast<float>(Chain - 1));
			Slot.Tint = bHeavy ? AetherbornComboTiming::HeavyNumber : AetherbornComboTiming::HitNumber;
			Slot.Origin = FVector(
				Location.X - 12.f + Rnd() * 26.f,
				Location.Y + (Rnd() - 0.5f) * 12.f,
				Location.Z + (bHeavy ? 142.f : 122.f) + Rnd() * 10.f);
			Slot.Drift = FVector((Rnd() - 0.5f) * 34.f, (Rnd() - 0.5f) * 18.f, 0.f);

			Comp->SetText(FText::FromString(FString::FromInt(DamageRounded)));
			Comp->SetWorldSize(Slot.WorldSize);
			Comp->SetTextRenderColor(Slot.Tint.ToFColor(true));
			Comp->SetWorldLocation(Slot.Origin);
			Comp->SetVisibility(true);
			BillboardToCamera(Comp, Slot.Origin);
		}
	}

	// ---- 2. the authored impact burst, scaled up for heavy swings
	if (!ImpactSystem)
	{
		WarnOnce(TEXT("ImpactNiagara"), TEXT("no ImpactNiagara assigned - hits show no impact burst"));
		return;
	}

	AActor* Owner = OwnerActor.Get();
	UWorld* World = Owner ? Owner->GetWorld() : nullptr;
	if (!World)
	{
		return;
	}

	const float ImpactScale = bHeavy ? 1.6f : 1.0f;
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, ImpactSystem, Location, FRotator::ZeroRotator,
	                                              FVector(ImpactScale), /*bAutoDestroy=*/true,
	                                              /*bAutoActivate=*/true, ENCPoolMethod::None,
	                                              /*bPreCullCheck=*/true);
}

void UAetherbornComboTimingFx::Tick(float DeltaSeconds)
{
	const float Dt = (FMath::IsFinite(DeltaSeconds) && DeltaSeconds > 0.f) ? FMath::Min(0.1f, DeltaSeconds) : 0.f;
	if (Dt <= 0.f)
	{
		return;
	}

	// One camera lookup per frame, shared by every live number.
	FVector CameraLocation = FVector::ZeroVector;
	bool bHaveCamera = false;
	if (const AActor* Owner = OwnerActor.Get())
	{
		if (const UWorld* World = Owner->GetWorld())
		{
			if (const APlayerController* PC = World->GetFirstPlayerController())
			{
				if (const APlayerCameraManager* CameraManager = PC->PlayerCameraManager)
				{
					CameraLocation = CameraManager->GetCameraLocation();
					bHaveCamera = true;
				}
			}
		}
	}

	for (int32 Index = 0; Index < Numbers.Num(); ++Index)
	{
		FAetherbornComboTimingNumberSlot& Slot = Numbers[Index];
		if (!Slot.bActive)
		{
			continue;
		}

		UTextRenderComponent* Comp = NumberComponents.IsValidIndex(Index) ? NumberComponents[Index].Get() : nullptr;
		if (!Comp)
		{
			Slot.bActive = false;
			continue;
		}

		Slot.Time += Dt;
		const float U = Slot.Time / NumberLife;
		if (U >= 1.f)
		{
			Slot.bActive = false;
			Comp->SetVisibility(false);
			continue;
		}

		// Rise on an ease-out cubic, drifting a little sideways as it goes.
		const float Ease = 1.f - FMath::Pow(1.f - U, 3.f);
		const FVector Position = Slot.Origin + FVector(Slot.Drift.X * U, Slot.Drift.Y * U, NumberRise * Ease);
		Comp->SetWorldLocation(Position);

		// Pop-in: the number starts a third larger and settles within the first 15 %.
		const float Grow = 1.f + 0.34f * FMath::Max(0.f, 1.f - U / 0.15f);
		Comp->SetWorldSize(Slot.WorldSize * Grow);

		// Fade in over 50 ms, then fade out over the tail of the life. World text is drawn
		// with the opaque text material, so the fade darkens the tint instead of blending it.
		const float FadeIn = FMath::Min(1.f, Slot.Time / 0.05f);
		const float FadeOut = 1.f - FMath::Min(1.f, FMath::Max(0.f, (U - 0.42f) / 0.58f));
		const float Alpha = FMath::Min(FadeIn, FadeOut);
		Comp->SetTextRenderColor((Slot.Tint * Alpha).ToFColor(true));

		if (bHaveCamera)
		{
			const FVector Away = (Position - CameraLocation).GetSafeNormal();
			if (!Away.IsNearlyZero())
			{
				Comp->SetWorldRotation(FRotationMatrix::MakeFromX(Away).Rotator());
			}
		}
	}
}

void UAetherbornComboTimingFx::Reset()
{
	for (int32 Index = 0; Index < Numbers.Num(); ++Index)
	{
		Numbers[Index].bActive = false;
		Numbers[Index].Time = 0.f;

		if (NumberComponents.IsValidIndex(Index) && NumberComponents[Index])
		{
			NumberComponents[Index]->SetVisibility(false);
		}
	}

	Seed = 0x9e3779b9u;
}

void UAetherbornComboTimingFx::WarnOnce(FName Key, const FString& Message)
{
	if (WarnedKeys.Contains(Key))
	{
		return;
	}
	WarnedKeys.Add(Key);
	UE_LOG(LogAetherbornComboTiming, Warning, TEXT("[Fx] %s"), *Message);
}
