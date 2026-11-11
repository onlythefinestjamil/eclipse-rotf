// Copyright EclipseRotF. All Rights Reserved.

#include "AetherbornComboHUDWidget.h"

#include "Blueprint/SlateBlueprintLibrary.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/CheckBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/SlateWrapperTypes.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/SlateColor.h"

namespace
{
	// hud.js constants.
	constexpr int32 MaxDamageNumbers = 12;
	constexpr float DamageNumberLife = 0.95f;
	constexpr float RingLife = 0.45f;
	constexpr float MissFlashTime = 0.52f;

	/** History signature token for one attempt kind (mirrors the js `kind` strings). */
	FString KindKey(EComboAttemptKind Kind)
	{
		switch (Kind)
		{
		case EComboAttemptKind::Hit:      return TEXT("hit");
		case EComboAttemptKind::Buffered: return TEXT("buffered");
		case EComboAttemptKind::Early:    return TEXT("early");
		case EComboAttemptKind::Late:     return TEXT("late");
		case EComboAttemptKind::Timeout:  return TEXT("timeout");
		default:                          return TEXT("unknown");
		}
	}

	bool IsHitKind(EComboAttemptKind Kind)
	{
		return Kind == EComboAttemptKind::Hit || Kind == EComboAttemptKind::Buffered;
	}

	bool IsMissKind(EComboAttemptKind Kind)
	{
		return Kind == EComboAttemptKind::Early
			|| Kind == EComboAttemptKind::Late
			|| Kind == EComboAttemptKind::Timeout;
	}

	FLinearColor EmberColor()      { return FLinearColor(1.0f, 0.478f, 0.2f, 1.0f); }
	FLinearColor WindowColor()     { return FLinearColor(1.0f, 0.69f, 0.4f, 1.0f); }
	FLinearColor WindupColor()     { return FLinearColor(0.373f, 0.498f, 0.659f, 1.0f); }
	FLinearColor RecoveryColor()   { return FLinearColor(0.42f, 0.455f, 0.533f, 1.0f); }
	FLinearColor IdleColor()       { return FLinearColor(0.575f, 0.633f, 0.729f, 1.0f); }
	FLinearColor MissColor()       { return FLinearColor(0.576f, 0.63f, 0.71f, 1.0f); }
	FLinearColor CueHitColor()     { return FLinearColor(1.0f, 0.69f, 0.4f, 1.0f); }
	FLinearColor CueWindowColor()  { return FLinearColor(1.0f, 0.82f, 0.66f, 1.0f); }
	FLinearColor PipOffColor()     { return FLinearColor(0.24f, 0.26f, 0.31f, 1.0f); }
	FLinearColor ChipMissColor()   { return FLinearColor(0.58f, 0.62f, 0.70f, 1.0f); }
	FLinearColor DamageTextColor() { return FLinearColor(1.0f, 0.86f, 0.62f, 1.0f); }
}

void UAetherbornComboHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// --------------------------------------------------------------- listeners
	if (Button_Attack)
	{
		Button_Attack->OnClicked.AddDynamic(this, &UAetherbornComboHUDWidget::OnAttackButtonClicked);
	}
	if (Button_Reset)
	{
		Button_Reset->OnClicked.AddDynamic(this, &UAetherbornComboHUDWidget::OnResetButtonClicked);
	}

	// ------------------------------------------------------------------ tuning
	if (Slider_WindowLen)
	{
		Slider_WindowLen->MinValue = 0.08f;
		Slider_WindowLen->MaxValue = 0.60f;
		Slider_WindowLen->StepSize = 0.01f;
		Slider_WindowLen->SetValue(0.25f);
	}
	if (Slider_SpeedMult)
	{
		Slider_SpeedMult->MinValue = 0.50f;
		Slider_SpeedMult->MaxValue = 2.00f;
		Slider_SpeedMult->StepSize = 0.05f;
		Slider_SpeedMult->SetValue(1.00f);
	}
	if (CheckBox_EarlyBuffers)
	{
		CheckBox_EarlyBuffers->SetIsChecked(false);
	}
	RefreshValueLabels();

	// -------------------------------------------------------------- rest state
	CachedCueText.Empty();
	CachedCueKind.Empty();
	LastHistorySignature.Empty();
	MissHideTime = 0.0f;
	RingHideTime = 0.0f;
	VignetteAlpha = 0.0f;
	LastPhaseStep = -1;
	DamageNumbers.Reset();

	if (Border_MissFlash)
	{
		Border_MissFlash->SetVisibility(ESlateVisibility::Hidden);
	}
	if (Image_Vignette)
	{
		Image_Vignette->SetRenderOpacity(0.0f);
	}
	if (Image_Ring)
	{
		Image_Ring->SetRenderOpacity(0.0f);
		Image_Ring->SetVisibility(ESlateVisibility::Hidden);
	}
	if (Text_Cue)
	{
		Text_Cue->SetText(FText::GetEmpty());
	}
}

// ---------------------------------------------------------------------- input

void UAetherbornComboHUDWidget::OnAttackButtonClicked()
{
	OnAttackPressed.ExecuteIfBound();
}

void UAetherbornComboHUDWidget::OnResetButtonClicked()
{
	OnResetPressed.ExecuteIfBound();
}

// -------------------------------------------------------------------- tuning

FComboSettings UAetherbornComboHUDWidget::ReadSettings() const
{
	FComboSettings Out;

	Out.WindowLen = Slider_WindowLen
		? FMath::Clamp(static_cast<float>(Slider_WindowLen->GetValue()), 0.08f, 0.60f)
		: AetherbornComboTiming::Window;

	Out.SpeedMult = Slider_SpeedMult
		? FMath::Clamp(static_cast<float>(Slider_SpeedMult->GetValue()), 0.50f, 8.0f)
		: 1.0f;

	Out.bEarlyBuffers = CheckBox_EarlyBuffers ? CheckBox_EarlyBuffers->IsChecked() : false;

	// The labels sit next to the sliders, so refresh them whenever the panel is read.
	const_cast<UAetherbornComboHUDWidget*>(this)->RefreshValueLabels();
	return Out;
}

float UAetherbornComboHUDWidget::GetComboWindowValue() const
{
	if (!Slider_WindowLen)
	{
		return AetherbornComboTiming::Window;
	}
	return FMath::Clamp(static_cast<float>(Slider_WindowLen->GetValue()), 0.08f, 0.60f);
}

void UAetherbornComboHUDWidget::RefreshValueLabels()
{
	if (Text_WindowLenValue && Slider_WindowLen)
	{
		Text_WindowLenValue->SetText(FText::FromString(FString::Printf(TEXT("%.2f"), Slider_WindowLen->GetValue())));
	}
	if (Text_SpeedMultValue && Slider_SpeedMult)
	{
		Text_SpeedMultValue->SetText(FText::FromString(FString::Printf(TEXT("%.2f"), Slider_SpeedMult->GetValue())));
	}
}

// ----------------------------------------------------------------------- cues

void UAetherbornComboHUDWidget::SetCueText(const FString& Text, const FString& Kind)
{
	CachedCueText = Text;
	CachedCueKind = Kind;

	if (!Text_Cue)
	{
		return;
	}

	if (Text.IsEmpty())
	{
		Text_Cue->SetText(FText::GetEmpty());
		return;
	}

	FLinearColor Color = CueHitColor();
	if (Kind.Equals(TEXT("miss"), ESearchCase::IgnoreCase))
	{
		Color = MissColor();
	}
	else if (Kind.Equals(TEXT("window"), ESearchCase::IgnoreCase))
	{
		Color = CueWindowColor();
	}

	Text_Cue->SetText(FText::FromString(Text));
	Text_Cue->SetColorAndOpacity(FSlateColor(Color));
}

// --------------------------------------------------------------------- chips

void UAetherbornComboHUDWidget::RebuildChips(const TArray<FComboAttempt>& History)
{
	if (!Box_Chips)
	{
		return;
	}

	// The "no swings yet" note lives in the chip box; remember that so it can be re-added.
	const bool bEmptyNoteWasInBox = Text_ChipsEmpty
		&& Text_ChipsEmpty->GetParent() == static_cast<UPanelWidget*>(Box_Chips.Get());
	Box_Chips->ClearChildren();

	if (History.Num() == 0)
	{
		if (Text_ChipsEmpty)
		{
			Text_ChipsEmpty->SetVisibility(ESlateVisibility::HitTestInvisible);
			if (bEmptyNoteWasInBox)
			{
				Box_Chips->AddChildToHorizontalBox(Text_ChipsEmpty);
			}
		}
		return;
	}

	if (Text_ChipsEmpty)
	{
		Text_ChipsEmpty->SetVisibility(ESlateVisibility::Collapsed);
	}

	const int32 First = FMath::Max(0, History.Num() - 10);
	for (int32 Index = First; Index < History.Num(); ++Index)
	{
		const FComboAttempt& Entry = History[Index];
		const int32 ChipStep = FMath::Clamp(Entry.Step, 1, 3);
		const bool bHit = IsHitKind(Entry.Kind);

		UTextBlock* Chip = NewObject<UTextBlock>(this);
		if (!Chip)
		{
			continue;
		}

		Chip->SetText(FText::FromString(FString::FromInt(ChipStep)));
		Chip->SetColorAndOpacity(FSlateColor(bHit ? WindowColor() : ChipMissColor()));

		FSlateFontInfo Font = Chip->GetFont();
		Font.Size = 14;
		Chip->SetFont(Font);

		if (UHorizontalBoxSlot* ChipSlot = Box_Chips->AddChildToHorizontalBox(Chip))
		{
			ChipSlot->SetPadding(FMargin(3.0f, 0.0f, 3.0f, 0.0f));
			ChipSlot->SetVerticalAlignment(VAlign_Center);
		}
	}
}

// ---------------------------------------------------------------- timing bar

void UAetherbornComboHUDWidget::ApplyBarGeometry(float WindupSeconds, float ActiveSeconds, float WindowSeconds, float RecoverySeconds)
{
	const float Total = FMath::Max(WindupSeconds + ActiveSeconds + WindowSeconds + RecoverySeconds, 1e-4f);

	auto SetFill = [Total](UBorder* Segment, float Seconds)
	{
		if (!Segment)
		{
			return;
		}
		if (UHorizontalBoxSlot* BoxSlot = Cast<UHorizontalBoxSlot>(Segment->Slot))
		{
			FSlateChildSize Size(ESlateSizeRule::Fill);
			Size.Value = FMath::Max(Seconds / Total, 0.0001f);
			BoxSlot->SetSize(Size);
		}
	};

	SetFill(Border_SegWindup, WindupSeconds);
	SetFill(Border_SegActive, ActiveSeconds);
	SetFill(Border_SegWindow, WindowSeconds);
	SetFill(Border_SegRecovery, RecoverySeconds);

	// The playhead is a thin 3 px sliver riding the track.
	if (Image_Playhead)
	{
		if (UCanvasPanelSlot* PlayheadSlot = Cast<UCanvasPanelSlot>(Image_Playhead->Slot))
		{
			FVector2D Size = PlayheadSlot->GetSize();
			Size.X = 3.0f;
			if (Size.Y <= 0.0f)
			{
				Size.Y = 18.0f;
			}
			PlayheadSlot->SetSize(Size);
		}
	}
}

// ------------------------------------------------------------------ per frame

void UAetherbornComboHUDWidget::UpdateFromSnapshot(const FComboSnapshot& Snapshot, const FComboSettings& Settings)
{
	UWorld* World = GetWorld();
	const float Now = World ? World->GetTimeSeconds() : 0.0f;
	const float FrameStep = World ? FMath::Clamp(World->GetDeltaSeconds(), 0.0f, 0.1f) : 0.0f;

	// ---- bar geometry (same maths as hud.js) ------------------------------
	const float Speed = FMath::Clamp(Settings.SpeedMult > 0.0f ? Settings.SpeedMult : 1.0f, 0.2f, 8.0f);
	const float WindowLen = FMath::Clamp(Settings.WindowLen > 0.0f ? Settings.WindowLen : AetherbornComboTiming::Window, 0.01f, 5.0f);
	const float BaseW = AetherbornComboTiming::Windup / Speed;
	const float BaseA = AetherbornComboTiming::Active / Speed;
	const float BaseR = AetherbornComboTiming::Recovery / Speed;
	const float WindowSec = FMath::Min(WindowLen, BaseR);
	const float RemainSec = FMath::Max(0.0f, BaseR - WindowSec);
	const float Total = FMath::Max(BaseW + BaseA + WindowSec + RemainSec, 1e-4f);
	const float FW = BaseW / Total;
	const float FA = BaseA / Total;
	const float FWin = WindowSec / Total;
	const float FRem = RemainSec / Total;

	ApplyBarGeometry(BaseW, BaseA, WindowSec, RemainSec);

	// ---- phase pill ------------------------------------------------------
	FString PhaseKey(TEXT("idle"));
	switch (Snapshot.Phase)
	{
	case EComboPhase::Windup:   PhaseKey = TEXT("windup");   break;
	case EComboPhase::Active:   PhaseKey = TEXT("active");   break;
	case EComboPhase::Recovery: PhaseKey = TEXT("recovery"); break;
	default:                    PhaseKey = TEXT("idle");     break;
	}
	if (Snapshot.bWindowOpen)
	{
		PhaseKey = TEXT("window");
	}

	FString PhaseLabel(TEXT("Idle"));
	FLinearColor PillColor = IdleColor();
	if (PhaseKey == TEXT("windup"))
	{
		PhaseLabel = TEXT("Windup");
		PillColor = WindupColor();
	}
	else if (PhaseKey == TEXT("active"))
	{
		PhaseLabel = TEXT("Active");
		PillColor = EmberColor();
	}
	else if (PhaseKey == TEXT("window"))
	{
		PhaseLabel = TEXT("Window");
		PillColor = WindowColor();
	}
	else if (PhaseKey == TEXT("recovery"))
	{
		PhaseLabel = TEXT("Recovery");
		PillColor = RecoveryColor();
	}

	if (Text_PhaseLabel)
	{
		Text_PhaseLabel->SetText(FText::FromString(PhaseLabel));
	}
	if (Border_PhasePill)
	{
		Border_PhasePill->SetBrushColor(PillColor);
	}

	// ---- chain pips + longest --------------------------------------------
	const int32 Step = FMath::Clamp(Snapshot.Step, 0, 3);
	if (Step != LastPhaseStep)
	{
		LastPhaseStep = Step;
		UBorder* Pips[3] = { Border_Pip1, Border_Pip2, Border_Pip3 };
		for (int32 Index = 0; Index < 3; ++Index)
		{
			if (!Pips[Index])
			{
				continue;
			}
			Pips[Index]->SetBrushColor(Index < Step ? EmberColor() : PipOffColor());
		}
	}
	if (Text_Longest)
	{
		Text_Longest->SetText(FText::AsNumber(FMath::Max(0, Snapshot.Longest)));
	}

	// ---- playhead --------------------------------------------------------
	const float PhaseProgress = FMath::Clamp(Snapshot.PhaseProgress, 0.0f, 1.0f);
	float Head = 0.0f;
	bool bHeadOn = true;

	if (PhaseKey == TEXT("windup"))
	{
		Head = FW * PhaseProgress;
	}
	else if (PhaseKey == TEXT("active"))
	{
		Head = FW + FA * PhaseProgress;
	}
	else if (PhaseKey == TEXT("window"))
	{
		const float WindowProgress = Snapshot.WindowProgress > 0.0f ? Snapshot.WindowProgress : 0.0f;
		Head = FW + FA + FWin * FMath::Clamp(WindowProgress, 0.0f, 1.0f);
	}
	else if (PhaseKey == TEXT("recovery"))
	{
		const float Elapsed = PhaseProgress * BaseR;
		const float InWindow = WindowSec > 0.0f ? FMath::Min(Elapsed, WindowSec) / WindowSec : 0.0f;
		const float AfterWindow = RemainSec > 0.0f
			? FMath::Max(0.0f, Elapsed - WindowSec) / RemainSec
			: (Elapsed > WindowSec ? 1.0f : 0.0f);
		Head = FW + FA + FWin * InWindow + FRem * AfterWindow;
	}
	else
	{
		bHeadOn = false;
	}

	if (Image_Playhead)
	{
		float TrackWidth = 0.0f;
		if (Box_Track)
		{
			TrackWidth = Box_Track->GetCachedGeometry().GetLocalSize().X;
			if (TrackWidth <= 0.0f)
			{
				TrackWidth = Box_Track->GetDesiredSize().X;
			}
		}

		const float HeadX = FMath::Clamp(Head, 0.0f, 1.0f) * FMath::Max(TrackWidth, 0.0f);
		if (UCanvasPanelSlot* PlayheadSlot = Cast<UCanvasPanelSlot>(Image_Playhead->Slot))
		{
			FVector2D Position = PlayheadSlot->GetPosition();
			Position.X = HeadX;
			PlayheadSlot->SetPosition(Position);
		}
		else
		{
			Image_Playhead->SetRenderTranslation(FVector2D(HeadX, 0.0f));
		}

		Image_Playhead->SetVisibility(bHeadOn ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}

	// ---- cue straight from the machine -----------------------------------
	if (Snapshot.Cue != CachedCueText || Snapshot.CueKind != CachedCueKind)
	{
		SetCueText(Snapshot.Cue, Snapshot.CueKind);
	}

	// ---- attempt history -------------------------------------------------
	FString Signature;
	for (const FComboAttempt& Entry : Snapshot.History)
	{
		Signature += FString::Printf(TEXT("%d:%s,"), Entry.Step, *KindKey(Entry.Kind));
	}

	if (Signature != LastHistorySignature)
	{
		const FString PreviousSignature = LastHistorySignature;
		LastHistorySignature = Signature;
		RebuildChips(Snapshot.History);

		if (Snapshot.History.Num() > 0)
		{
			const FComboAttempt& Latest = Snapshot.History.Last();
			const FString LatestKey = FString::Printf(TEXT("%d:%s,"), Latest.Step, *KindKey(Latest.Kind));
			const bool bAlreadySeen = !PreviousSignature.IsEmpty() && PreviousSignature.EndsWith(LatestKey, ESearchCase::CaseSensitive);
			if (IsMissKind(Latest.Kind) && !bAlreadySeen)
			{
				MissHideTime = Now + MissFlashTime;
				if (Border_MissFlash)
				{
					Border_MissFlash->SetVisibility(ESlateVisibility::HitTestInvisible);
				}
			}
		}
	}

	if (MissHideTime > 0.0f && Now >= MissHideTime)
	{
		MissHideTime = 0.0f;
		if (Border_MissFlash)
		{
			Border_MissFlash->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	// ---- ember vignette --------------------------------------------------
	if (VignetteAlpha > 0.0f)
	{
		VignetteAlpha = FMath::Max(0.0f, VignetteAlpha - FrameStep * 2.4f);
	}
	if (Image_Vignette)
	{
		const float Drawn = FMath::RoundToInt(VignetteAlpha * 100.0f) / 100.0f;
		Image_Vignette->SetRenderOpacity(Drawn * 0.92f);
	}

	// ---- chain ring ------------------------------------------------------
	if (Image_Ring)
	{
		if (RingHideTime > 0.0f && Now < RingHideTime)
		{
			const float Remaining = FMath::Max(0.0f, RingHideTime - Now);
			const float K = FMath::Clamp(1.0f - Remaining / RingLife, 0.0f, 1.0f);
			const float Scale = 0.55f + K * 1.75f;
			Image_Ring->SetVisibility(ESlateVisibility::HitTestInvisible);
			Image_Ring->SetRenderScale(FVector2D(Scale, Scale));
			Image_Ring->SetRenderOpacity(0.55f * (1.0f - K));
		}
		else if (RingHideTime > 0.0f)
		{
			RingHideTime = 0.0f;
			Image_Ring->SetRenderOpacity(0.0f);
			Image_Ring->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	TickDamageNumbers();
}

// ------------------------------------------------------------------ feedback

void UAetherbornComboHUDWidget::Pulse(float Strength)
{
	const float Amount = FMath::Clamp(Strength, 0.0f, 1.0f);
	if (Amount <= 0.0f)
	{
		return;
	}

	VignetteAlpha = FMath::Min(1.0f, VignetteAlpha + Amount * 0.85f);

	if (Amount >= 0.4f)
	{
		const UWorld* World = GetWorld();
		const float Now = World ? World->GetTimeSeconds() : 0.0f;
		RingHideTime = Now + RingLife;
		if (Image_Ring)
		{
			Image_Ring->SetVisibility(ESlateVisibility::HitTestInvisible);
			Image_Ring->SetRenderScale(FVector2D(0.55f, 0.55f));
			Image_Ring->SetRenderOpacity(0.55f);
		}
	}
}

void UAetherbornComboHUDWidget::PopDamage(const FVector& WorldLocation, float Damage)
{
	if (!Canvas_Fx)
	{
		return;
	}

	// ---- screen position --------------------------------------------------
	FVector2D ScreenPosition = FVector2D::ZeroVector;
	bool bProjected = false;

	APlayerController* PlayerController = GetOwningPlayer();
	if (PlayerController)
	{
		bProjected = UGameplayStatics::ProjectWorldToScreen(PlayerController, WorldLocation, ScreenPosition, false);
		if (!bProjected)
		{
			int32 ViewportX = 0;
			int32 ViewportY = 0;
			PlayerController->GetViewportSize(ViewportX, ViewportY);
			ScreenPosition = FVector2D(ViewportX * 0.5f, ViewportY * 0.5f);
		}
	}

	// ---- the number itself ------------------------------------------------
	UTextBlock* Number = NewObject<UTextBlock>(this);
	if (!Number)
	{
		return;
	}

	Number->SetText(FText::FromString(FString::FromInt(FMath::RoundToInt(Damage))));
	Number->SetColorAndOpacity(FSlateColor(DamageTextColor()));

	FSlateFontInfo Font = Number->GetFont();
	Font.Size = 22;
	Number->SetFont(Font);
	Number->SetRenderOpacity(0.0f);

	UCanvasPanelSlot* NumberSlot = Canvas_Fx->AddChildToCanvas(Number);
	if (NumberSlot)
	{
		NumberSlot->SetAutoSize(true);
		NumberSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		NumberSlot->SetPosition(ScreenPosition);
		NumberSlot->SetZOrder(10);
	}

	// ---- keep the pool bounded -------------------------------------------
	while (DamageNumbers.Num() >= MaxDamageNumbers)
	{
		if (DamageNumbers[0].Text)
		{
			DamageNumbers[0].Text->RemoveFromParent();
		}
		DamageNumbers.RemoveAt(0);
	}

	FComboDamageNumber& Entry = DamageNumbers.AddDefaulted_GetRef();
	Entry.Text = Number;
	Entry.StartPos = ScreenPosition;
	Entry.Drift = FMath::FRandRange(-23.0f, 23.0f);
	Entry.Rise = FMath::FRandRange(58.0f, 86.0f);
	Entry.Life = DamageNumberLife;
	Entry.MaxLife = DamageNumberLife;
}

void UAetherbornComboHUDWidget::TickDamageNumbers()
{
	if (DamageNumbers.Num() == 0)
	{
		return;
	}

	const UWorld* World = GetWorld();
	const float Step = World ? FMath::Clamp(World->GetDeltaSeconds(), 0.0f, 0.1f) : 0.0f;

	for (int32 Index = DamageNumbers.Num() - 1; Index >= 0; --Index)
	{
		FComboDamageNumber& Number = DamageNumbers[Index];
		Number.Life -= Step;

		if (Number.Life <= 0.0f)
		{
			if (Number.Text)
			{
				Number.Text->RemoveFromParent();
			}
			DamageNumbers.RemoveAt(Index);
			continue;
		}

		if (!Number.Text)
		{
			continue;
		}

		const float T = 1.0f - Number.Life / FMath::Max(Number.MaxLife, 1e-4f);
		const float Opacity = FMath::Max(0.0f, 1.0f - T * T * T);
		const float Scale = 1.0f + 0.4f * (1.0f - T) - 0.08f * T;
		const FVector2D Position(Number.StartPos.X + Number.Drift * T, Number.StartPos.Y - Number.Rise * T);

		if (UCanvasPanelSlot* NumberSlot = Cast<UCanvasPanelSlot>(Number.Text->Slot))
		{
			NumberSlot->SetPosition(Position);
		}
		else
		{
			Number.Text->SetRenderTranslation(FVector2D(Number.Drift * T, -Number.Rise * T));
		}

		Number.Text->SetRenderScale(FVector2D(Scale, Scale));
		Number.Text->SetRenderOpacity(Opacity);
	}
}
