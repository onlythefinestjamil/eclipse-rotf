// Copyright Eclipse RotF. Aetherborn combo timing slice (Eclipse timings).
//
// Port of src/hud.js createHud(): the DOM was rebuilt as a Widget Blueprint, so this is
// only the per-frame push of the snapshot into the bound widgets plus the button wiring.

#include "AetherbornComboTimingHUDWidget.h"

#include "AetherbornComboTimingPawn.h"
#include "AetherbornComboTimingPlayerController.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Layout/Geometry.h"

// The retuned timings and the palette live in namespace AetherbornComboTiming (Types.h).
using namespace AetherbornComboTiming;

namespace AetherbornComboTimingHud
{
	/** How long a feedback line stays up, in seconds (hud.js FLASH_TIME / REFUSED_FLASH). */
	constexpr float FlashHold = 0.75f;
	constexpr float RefusedFlashHold = 1.1f;

	constexpr int32 HistoryMax = 6;
	constexpr int32 PipCount = 5;

	/** The card's default combo window, used to flag a moved slider. */
	constexpr float DefaultComboWindow = 0.28f;

	/** Width used for the off-canvas fallback when the bar has not been laid out yet. */
	constexpr float FallbackTrackWidth = 320.f;

	FString Seconds(float Value)
	{
		return FString::Printf(TEXT("%.2f s"), Value);
	}

	/** Write text only when it actually changed (cheap per-frame updates). */
	void SetText(UTextBlock* Block, const FString& Value)
	{
		if (Block && !Block->GetText().ToString().Equals(Value, ESearchCase::CaseSensitive))
		{
			Block->SetText(FText::FromString(Value));
		}
	}

	void SetShown(UWidget* Target, bool bShown)
	{
		if (!Target)
		{
			return;
		}

		const ESlateVisibility Wanted = bShown ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden;
		if (Target->GetVisibility() != Wanted)
		{
			Target->SetVisibility(Wanted);
		}
	}
}

void UAetherbornComboTimingHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindButtons();
	RenderHistory();

	// Nothing is live yet: the heavy/parry rows and the flash line start hidden.
	AetherbornComboTimingHud::SetShown(Txt_Flash, false);
	AetherbornComboTimingHud::SetShown(Txt_HeavyLabel, false);
	AetherbornComboTimingHud::SetShown(Txt_ParryLabel, false);
	AetherbornComboTimingHud::SetShown(Img_HeavyBand, false);
	AetherbornComboTimingHud::SetShown(Img_ParryBand, false);
}

void UAetherbornComboTimingHUDWidget::NativeDestruct()
{
	UnbindButtons();
	Super::NativeDestruct();
}

void UAetherbornComboTimingHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	Refresh(InDeltaTime);
}

void UAetherbornComboTimingHUDWidget::Refresh(float DeltaSeconds)
{
	// The controller also drives this, so guard against two pushes in one frame.
	if (LastRefreshFrame == GFrameCounter)
	{
		return;
	}
	LastRefreshFrame = GFrameCounter;

	UpdateFlash(DeltaSeconds);

	const AAetherbornComboTimingPawn* ComboPawn = GetComboPawn();
	if (!ComboPawn)
	{
		return;
	}

	UpdateFromSnapshot(ComboPawn->GetSnapshot(), ComboPawn->GetTuning());
}

// --------------------------------------------------------------------------- wiring

void UAetherbornComboTimingHUDWidget::BindButtons()
{
	if (Btn_Light)
	{
		Btn_Light->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleLightClicked);
	}

	if (Btn_Heavy)
	{
		Btn_Heavy->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleHeavyClicked);
	}

	if (Btn_Reset)
	{
		Btn_Reset->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleResetClicked);
	}
}

void UAetherbornComboTimingHUDWidget::UnbindButtons()
{
	if (Btn_Light)
	{
		Btn_Light->OnClicked.RemoveDynamic(this, &ThisClass::HandleLightClicked);
	}

	if (Btn_Heavy)
	{
		Btn_Heavy->OnClicked.RemoveDynamic(this, &ThisClass::HandleHeavyClicked);
	}

	if (Btn_Reset)
	{
		Btn_Reset->OnClicked.RemoveDynamic(this, &ThisClass::HandleResetClicked);
	}
}

void UAetherbornComboTimingHUDWidget::HandleLightClicked()
{
	if (AAetherbornComboTimingPlayerController* Controller = GetComboController())
	{
		Controller->RequestLight();
	}
}

void UAetherbornComboTimingHUDWidget::HandleHeavyClicked()
{
	if (AAetherbornComboTimingPlayerController* Controller = GetComboController())
	{
		Controller->RequestHeavy();
	}
}

void UAetherbornComboTimingHUDWidget::HandleResetClicked()
{
	if (AAetherbornComboTimingPlayerController* Controller = GetComboController())
	{
		Controller->ResetCombo();
	}
}

AAetherbornComboTimingPawn* UAetherbornComboTimingHUDWidget::GetComboPawn() const
{
	return Cast<AAetherbornComboTimingPawn>(GetOwningPlayerPawn());
}

AAetherbornComboTimingPlayerController* UAetherbornComboTimingHUDWidget::GetComboController() const
{
	return Cast<AAetherbornComboTimingPlayerController>(GetOwningPlayer());
}

// ------------------------------------------------------------- blueprint read access

FAetherbornComboTimingSnapshot UAetherbornComboTimingHUDWidget::GetComboSnapshot() const
{
	if (const AAetherbornComboTimingPawn* ComboPawn = GetComboPawn())
	{
		return ComboPawn->GetSnapshot();
	}

	return FAetherbornComboTimingSnapshot();
}

FAetherbornComboTimingTuning UAetherbornComboTimingHUDWidget::GetComboTuning() const
{
	if (const AAetherbornComboTimingPawn* ComboPawn = GetComboPawn())
	{
		return ComboPawn->GetTuning();
	}

	return FAetherbornComboTimingTuning();
}

int32 UAetherbornComboTimingHUDWidget::GetComboCount() const
{
	return FMath::Max(0, GetComboSnapshot().Combo);
}

int32 UAetherbornComboTimingHUDWidget::GetBestChain() const
{
	return FMath::Max(0, GetComboSnapshot().Best);
}

// ------------------------------------------------------------------- flash + history

void UAetherbornComboTimingHUDWidget::ShowFlash(const FString& Message, const FLinearColor& Color, float HoldSeconds)
{
	FlashTimer = FMath::Max(0.05f, HoldSeconds);

	if (!Txt_Flash)
	{
		return;
	}

	Txt_Flash->SetText(FText::FromString(Message));
	Txt_Flash->SetColorAndOpacity(FSlateColor(Color));
	AetherbornComboTimingHud::SetShown(Txt_Flash, true);
}

void UAetherbornComboTimingHUDWidget::UpdateFlash(float DeltaSeconds)
{
	if (FlashTimer <= 0.f)
	{
		return;
	}

	FlashTimer = FMath::Max(0.f, FlashTimer - FMath::Max(0.f, DeltaSeconds));
	if (FlashTimer <= 0.f)
	{
		AetherbornComboTimingHud::SetShown(Txt_Flash, false);
	}
}

void UAetherbornComboTimingHUDWidget::PushHistory(const FString& Entry)
{
	History.Add(Entry);
	while (History.Num() > AetherbornComboTimingHud::HistoryMax)
	{
		History.RemoveAt(0);
	}

	RenderHistory();
}

void UAetherbornComboTimingHUDWidget::RenderHistory()
{
	FString Joined;
	for (int32 Index = 0; Index < History.Num(); ++Index)
	{
		if (Index > 0)
		{
			Joined += TEXT("  ");
		}
		Joined += History[Index];
	}

	AetherbornComboTimingHud::SetText(Txt_History, History.Num() > 0 ? Joined : FString(TEXT("- -")));
}

// ------------------------------------------------------------------ bar overlay maths

void UAetherbornComboTimingHUDWidget::PlaceRange(UWidget* Target, float From, float To, float& CachedFrom, float& CachedTo, float TrackWidth)
{
	if (!Target)
	{
		return;
	}

	From = FMath::Clamp(From, 0.f, 1.f);
	To = FMath::Clamp(To, From, 1.f);

	if (FMath::IsNearlyEqual(CachedFrom, From, 0.0005f) && FMath::IsNearlyEqual(CachedTo, To, 0.0005f))
	{
		return;
	}

	CachedFrom = From;
	CachedTo = To;

	// A canvas slot can express the range as anchors, so the band keeps its proportions
	// at any widget size (this is what the DOM's percentage left/width did).
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Target->Slot))
	{
		CanvasSlot->SetAnchors(FAnchors(From, 0.f, To, 1.f));
		CanvasSlot->SetOffsets(FMargin(0.f));
		CanvasSlot->SetAlignment(FVector2D(0.f, 0.f));
		return;
	}

	// Anything else (overlay/box slot) can only be nudged along the bar.
	Target->SetRenderTranslation(FVector2D(From * TrackWidth, 0.f));
}

void UAetherbornComboTimingHUDWidget::PlaceTick(UWidget* Target, float At, float HalfWidth, float& CachedAt, float TrackWidth)
{
	if (!Target)
	{
		return;
	}

	At = FMath::Clamp(At, 0.f, 1.f);

	if (FMath::IsNearlyEqual(CachedAt, At, 0.0005f))
	{
		return;
	}

	CachedAt = At;

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Target->Slot))
	{
		// Zero-width anchor range expanded by the tick's half width: a tick centred on At.
		CanvasSlot->SetAnchors(FAnchors(At, 0.f, At, 1.f));
		CanvasSlot->SetOffsets(FMargin(-HalfWidth, 0.f, HalfWidth, 0.f));
		CanvasSlot->SetAlignment(FVector2D(0.f, 0.f));
		return;
	}

	Target->SetRenderTranslation(FVector2D(At * TrackWidth - HalfWidth, 0.f));
}

void UAetherbornComboTimingHUDWidget::UpdatePips(const FAetherbornComboTimingSnapshot& Snapshot, bool bLightLive, bool bHeavyLive)
{
	UBorder* const Pips[AetherbornComboTimingHud::PipCount] = { Pip_1, Pip_2, Pip_3, Pip_4, Pip_5 };
	const int32 Step = FMath::Clamp(Snapshot.Step, 0, static_cast<int32>(ChainMax));

	for (int32 Index = 0; Index < AetherbornComboTimingHud::PipCount; ++Index)
	{
		UBorder* Pip = Pips[Index];
		if (!Pip)
		{
			continue;
		}

		const int32 Link = Index + 1;
		if (bLightLive && Link == Step)
		{
			Pip->SetBrushColor(LightWindow);
		}
		else if (bLightLive && Link < Step)
		{
			Pip->SetBrushColor(LightWindow * 0.35f);
		}
		else
		{
			Pip->SetBrushColor(TextDim * 0.4f);
		}
	}

	if (Pip_Heavy)
	{
		Pip_Heavy->SetBrushColor(bHeavyLive ? HeavyRecoveryTint : TextDim * 0.4f);
	}
}

// ---------------------------------------------------------------- main per-frame push

void UAetherbornComboTimingHUDWidget::UpdateFromSnapshot(const FAetherbornComboTimingSnapshot& Snapshot, const FAetherbornComboTimingTuning& Tuning)
{
	const bool bLightLive = Snapshot.Kind == EAetherbornComboTimingKind::Light;
	const bool bHeavyLive = Snapshot.Kind == EAetherbornComboTimingKind::Heavy;
	const bool bRecovery = Snapshot.Phase == EAetherbornComboTimingPhase::Recovery;

	const int32 MaxChain = static_cast<int32>(ChainMax);
	const int32 ChainStep = FMath::Clamp(Snapshot.Step, 0, MaxChain);
	const int32 Combo = FMath::Max(0, Snapshot.Combo);
	const int32 Best = FMath::Max(0, Snapshot.Best);

	const float Speed = FMath::Clamp(Tuning.SpeedMul, SpeedMin, SpeedMax);
	const float InvSpeed = 1.f / FMath::Max(KINDA_SMALL_NUMBER, Speed);

	// ---- chain pips ---------------------------------------------------------
	UpdatePips(Snapshot, bLightLive, bHeavyLive);

	// ---- combo, best, time since hit ----------------------------------------
	AetherbornComboTimingHud::SetText(Txt_Combo, FString::Printf(TEXT("x%d"), Combo));
	AetherbornComboTimingHud::SetText(Txt_Best, FString::Printf(TEXT("BEST %d"), Best));
	AetherbornComboTimingHud::SetText(Txt_SinceHit, FString::Printf(TEXT("%d ms"), FMath::Max(0, FMath::RoundToInt(Snapshot.SinceHitMs))));

	// ---- window chip --------------------------------------------------------
	FString ChipLabel = TEXT("WINDOW CLOSED");
	EChipState ChipState = EChipState::Closed;
	if (Snapshot.bParryOpen)
	{
		ChipLabel = TEXT("PARRY WINDOW");
		ChipState = EChipState::Parry;
	}
	else if (Snapshot.bFrozen)
	{
		ChipLabel = TEXT("HIT-STOP");
		ChipState = EChipState::Freeze;
	}
	else if (bHeavyLive)
	{
		ChipLabel = bRecovery ? TEXT("HEAVY RECOVERY - LOCKED") : TEXT("HEAVY SWING");
		ChipState = EChipState::Heavy;
	}
	else if (Snapshot.bWindowOpen)
	{
		ChipLabel = TEXT("COMBO WINDOW OPEN");
		ChipState = EChipState::Buffer;
	}

	AetherbornComboTimingHud::SetText(Txt_Chip, ChipLabel);
	if (Txt_Chip)
	{
		FLinearColor ChipTint = TextDim;
		switch (ChipState)
		{
		case EChipState::Buffer:
			ChipTint = LightWindow;
			break;
		case EChipState::Heavy:
			ChipTint = HeavyRecoveryTint;
			break;
		case EChipState::Parry:
			ChipTint = Parry;
			break;
		case EChipState::Freeze:
			ChipTint = FLinearColor::White;
			break;
		case EChipState::Closed:
		default:
			break;
		}
		Txt_Chip->SetColorAndOpacity(FSlateColor(ChipTint));
	}

	// ---- timing bar: the anim-notify geometry -------------------------------
	// The snapshot carries the machine's own numbers; before the first attack we preview
	// the light swing at the current speed so the bar is never empty.
	const bool bHasAttack = Snapshot.AttackDuration > KINDA_SMALL_NUMBER;
	const float Duration = FMath::Max(0.01f, bHasAttack ? Snapshot.AttackDuration : (LightTotal * InvSpeed));
	const float Elapsed = bHasAttack ? Snapshot.AttackElapsed : 0.f;
	const float HitAt = bHasAttack ? Snapshot.HitAt : (LightHitAt * InvSpeed);
	const float RecoveryStart = bHasAttack ? Snapshot.RecoveryStart : (LightHitAt * InvSpeed);
	const float RecoveryEnd = bHasAttack ? Snapshot.RecoveryEnd : Duration;
	const float WindowDur = FMath::Clamp(Tuning.ComboWindow, ComboWindowMin, ComboWindowMax);
	const float WindowStart = bHasAttack ? Snapshot.WindowStart : RecoveryStart;
	const float WindowEnd = bHasAttack ? Snapshot.WindowEnd : (WindowStart + WindowDur);
	const float ParryEnd = bHasAttack ? Snapshot.ParryEnd : 0.f;

	float TrackWidth = AetherbornComboTimingHud::FallbackTrackWidth;
	if (PB_AttackProgress)
	{
		const FVector2D BarSize = PB_AttackProgress->GetCachedGeometry().GetLocalSize();
		if (BarSize.X > 1.0)
		{
			TrackWidth = static_cast<float>(BarSize.X);
		}
	}

	PlaceRange(Img_Swing, 0.f, RecoveryStart / Duration, LastSwingFrom, LastSwingTo, TrackWidth);
	PlaceTick(Img_HitTick, HitAt / Duration, 1.5f, LastHitTickAt, TrackWidth);
	PlaceTick(Img_Playhead, Elapsed / Duration, 1.f, LastPlayheadAt, TrackWidth);

	if (bHeavyLive)
	{
		PlaceRange(Img_HeavyBand, RecoveryStart / Duration, RecoveryEnd / Duration, LastHeavyFrom, LastHeavyTo, TrackWidth);
		PlaceRange(Img_ParryBand, 0.f, ParryEnd / Duration, LastParryFrom, LastParryTo, TrackWidth);
	}
	else
	{
		PlaceRange(Img_ComboBand, WindowStart / Duration, WindowEnd / Duration, LastComboFrom, LastComboTo, TrackWidth);
	}

	AetherbornComboTimingHud::SetShown(Img_Swing, true);
	AetherbornComboTimingHud::SetShown(Img_HitTick, true);
	AetherbornComboTimingHud::SetShown(Img_Playhead, true);
	AetherbornComboTimingHud::SetShown(Img_HeavyBand, bHeavyLive);
	AetherbornComboTimingHud::SetShown(Img_ParryBand, bHeavyLive);
	AetherbornComboTimingHud::SetShown(Img_ComboBand, !bHeavyLive);

	if (PB_AttackProgress)
	{
		PB_AttackProgress->SetPercent(FMath::Clamp(Elapsed / Duration, 0.f, 1.f));
	}

	// ---- notify labels ------------------------------------------------------
	const float HeavyRecoveryLen = bHeavyLive ? FMath::Max(0.f, RecoveryEnd - RecoveryStart) : (HeavyRecovery * InvSpeed);
	const float ParryLen = (bHeavyLive && ParryEnd > 0.f) ? ParryEnd : (ParryWindow * InvSpeed);

	AetherbornComboTimingHud::SetText(Txt_HitLabel, FString::Printf(TEXT("%s %s"), *HitNotifyLabel, *AetherbornComboTimingHud::Seconds(HitAt)));
	AetherbornComboTimingHud::SetText(Txt_ComboLabel, FString::Printf(TEXT("%s %s%s"), *ComboNotifyLabel, *AetherbornComboTimingHud::Seconds(WindowDur), Snapshot.bWindowClamped ? TEXT(" (recovery extended)") : TEXT("")));
	AetherbornComboTimingHud::SetText(Txt_HeavyLabel, FString::Printf(TEXT("HEAVY RECOVERY %s LOCKED"), *AetherbornComboTimingHud::Seconds(HeavyRecoveryLen)));
	AetherbornComboTimingHud::SetText(Txt_ParryLabel, FString::Printf(TEXT("PARRY %s"), *AetherbornComboTimingHud::Seconds(ParryLen)));

	AetherbornComboTimingHud::SetShown(Txt_HitLabel, true);
	AetherbornComboTimingHud::SetShown(Txt_ComboLabel, !bHeavyLive);
	AetherbornComboTimingHud::SetShown(Txt_HeavyLabel, bHeavyLive);
	AetherbornComboTimingHud::SetShown(Txt_ParryLabel, bHeavyLive);

	// ---- what the player needs to press next --------------------------------
	FString NeedLabel = TEXT("PRESS J");
	FLinearColor NeedTint = TextDim;
	if (bHeavyLive)
	{
		NeedLabel = bRecovery
			? FString::Printf(TEXT("COMMITTED %s"), *AetherbornComboTimingHud::Seconds((HeavyHitAt + HeavyRecovery) * InvSpeed))
			: FString(TEXT("HEAVY SWING"));
		NeedTint = HeavyRecoveryTint;
	}
	else if (bLightLive)
	{
		NeedLabel = ChainStep >= MaxChain
			? FString::Printf(TEXT("CHAIN %d/%d"), ChainStep, MaxChain)
			: FString::Printf(TEXT("CHAIN %d/%d - NEXT: LIGHT"), ChainStep, MaxChain);
		NeedTint = Snapshot.bWindowOpen ? LightWindow : TextDim;
	}

	AetherbornComboTimingHud::SetText(Txt_Need, NeedLabel);
	if (Txt_Need)
	{
		Txt_Need->SetColorAndOpacity(FSlateColor(NeedTint));
	}

	// ---- project timings card -----------------------------------------------
	UpdateTimingsCard(Tuning);

	// ---- feedback flashes from lastInput transitions ------------------------
	const int32 Seq = Snapshot.LastInputSeq;
	if (bHasPrevSeq && Seq != PrevSeq)
	{
		switch (Snapshot.LastInput)
		{
		case EAetherbornComboTimingVerdict::Early:
			ShowFlash(TEXT("TOO EARLY"), Danger, AetherbornComboTimingHud::FlashHold);
			break;
		case EAetherbornComboTimingVerdict::Late:
			ShowFlash(TEXT("TOO LATE"), Danger, AetherbornComboTimingHud::FlashHold);
			break;
		case EAetherbornComboTimingVerdict::Buffered:
			ShowFlash(TEXT("BUFFERED"), LightWindow, AetherbornComboTimingHud::FlashHold);
			break;
		case EAetherbornComboTimingVerdict::Max:
			ShowFlash(TEXT("MAX CHAIN"), Ok, AetherbornComboTimingHud::FlashHold);
			break;
		case EAetherbornComboTimingVerdict::Hit:
			ShowFlash(FString::Printf(TEXT("CHAIN %d"), ChainStep), Ok, AetherbornComboTimingHud::FlashHold);
			break;
		case EAetherbornComboTimingVerdict::Refused:
		{
			FString RefusedLabel = TEXT("REFUSED - LIGHT RECOVERY");
			switch (Snapshot.RefuseReason)
			{
			case EAetherbornComboTimingRefuse::HeavyRecovery:
				RefusedLabel = TEXT("REFUSED - HEAVY RECOVERY");
				break;
			case EAetherbornComboTimingRefuse::HeavySwing:
				RefusedLabel = TEXT("REFUSED - HEAVY SWING");
				break;
			case EAetherbornComboTimingRefuse::LightSwing:
				RefusedLabel = TEXT("REFUSED - LIGHT SWING");
				break;
			case EAetherbornComboTimingRefuse::LightRecovery:
			case EAetherbornComboTimingRefuse::None:
			default:
				break;
			}
			ShowFlash(RefusedLabel, HeavyRecoveryTint, AetherbornComboTimingHud::RefusedFlashHold);
			break;
		}
		default:
			break;
		}
	}
	PrevSeq = Seq;
	bHasPrevSeq = true;

	// ---- chain ended: a full chain reads complete, a short one dropped ------ 
	if (PrevCombo > 0 && Combo == 0 && Best > 0)
	{
		if (PrevKind == EAetherbornComboTimingKind::Heavy)
		{
			ShowFlash(FString::Printf(TEXT("CHAIN %d"), PrevCombo), Ok, AetherbornComboTimingHud::FlashHold);
			PushHistory(TEXT("H"));
		}
		else if (PrevCombo >= MaxChain)
		{
			ShowFlash(FString::Printf(TEXT("CHAIN %d"), PrevCombo), Ok, AetherbornComboTimingHud::FlashHold);
			PushHistory(FString::Printf(TEXT("x%d"), PrevCombo));
		}
		else
		{
			ShowFlash(TEXT("DROPPED"), Danger, AetherbornComboTimingHud::FlashHold);
			PushHistory(FString::Printf(TEXT("x%d"), PrevCombo));
		}
	}

	if (Best < PrevBest)
	{
		History.Reset();
		RenderHistory();
	}

	PrevBest = Best;
	PrevCombo = Combo;
	PrevKind = Snapshot.Kind;
}

void UAetherbornComboTimingHUDWidget::UpdateTimingsCard(const FAetherbornComboTimingTuning& Tuning)
{
	const float Speed = FMath::Clamp(Tuning.SpeedMul, SpeedMin, SpeedMax);
	const float Window = FMath::Clamp(Tuning.ComboWindow, ComboWindowMin, ComboWindowMax);

	const int32 MaxChain = static_cast<int32>(ChainMax);
	const int32 BaseDamage = FMath::RoundToInt(LightDamageBase);
	const int32 MaxDamage = FMath::RoundToInt(LightDamageBase * (1.f + LightDamageStepPct * static_cast<float>(MaxChain - 1)));

	// The card is the project's authored timing table; the live line below scales it.
	const FString Card = FString::Printf(
		TEXT("PROJECT TIMINGS\nLIGHT %s\nHEAVY %s = %.2f + %.2f\nCOMBO WINDOW %s\nPARRY %s\nHIT-STOP %.3f / %.2f s\nCHAIN %d / DMG %d-%d / %d"),
		*AetherbornComboTimingHud::Seconds(LightTotal),
		*AetherbornComboTimingHud::Seconds(HeavyTotal),
		HeavyHitAt,
		HeavyRecovery,
		*AetherbornComboTimingHud::Seconds(Window),
		*AetherbornComboTimingHud::Seconds(ParryWindow),
		LightHitStop,
		HeavyHitStop,
		MaxChain,
		BaseDamage,
		MaxDamage,
		FMath::RoundToInt(HeavyDamage));

	AetherbornComboTimingHud::SetText(Txt_Timings, Card);

	// Off the default 0.28 s window the card reads as "changed".
	if (Txt_Timings)
	{
		const bool bWindowMoved = FMath::Abs(Window - AetherbornComboTimingHud::DefaultComboWindow) > 1e-6f;
		Txt_Timings->SetColorAndOpacity(FSlateColor(bWindowMoved ? CancelWindow : Text));
	}

	const bool bOffDefault = FMath::Abs(Speed - 1.f) > 0.001f;
	if (bOffDefault)
	{
		const float InvSpeed = 1.f / FMath::Max(KINDA_SMALL_NUMBER, Speed);
		const FString Live = FString::Printf(
			TEXT("LIVE x%.2f - LIGHT %s - WINDOW %s - HEAVY %s"),
			Speed,
			*AetherbornComboTimingHud::Seconds(LightTotal * InvSpeed),
			*AetherbornComboTimingHud::Seconds(Window),
			*AetherbornComboTimingHud::Seconds(HeavyTotal * InvSpeed));

		AetherbornComboTimingHud::SetText(Txt_Live, Live);
	}

	AetherbornComboTimingHud::SetShown(Txt_Live, bOffDefault);
}
