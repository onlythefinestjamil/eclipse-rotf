// Copyright Eclipse RotF. Aetherborn combo timing slice (Eclipse timings).
//
// Presentation only: the HUD never simulates anything. NativeTick pulls the pawn's
// snapshot and pushes it into the text blocks, the pips and the bar overlays, and the
// buttons forward to the player controller.
//
// Bar overlays (playhead, hit tick, combo band, parry band, heavy band) are plain
// UImages sitting in a canvas over PB_AttackProgress: they are placed by anchor
// fractions so the bar keeps its proportions at any widget size.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "AetherbornComboTimingTypes.h"

#include "AetherbornComboTimingHUDWidget.generated.h"

class AAetherbornComboTimingPawn;
class AAetherbornComboTimingPlayerController;
class UBorder;
class UButton;
class UImage;
class UProgressBar;
class UTextBlock;
class UWidget;

UCLASS()
class ECLIPSEROTF_API UAetherbornComboTimingHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	//~ Begin UUserWidget interface
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	//~ End UUserWidget interface

	/** What the HUD is currently displaying, readable from a Blueprint graph. */
	UFUNCTION(BlueprintPure, Category = "Aetherborn|Combo")
	FAetherbornComboTimingSnapshot GetComboSnapshot() const;

	UFUNCTION(BlueprintPure, Category = "Aetherborn|Combo")
	FAetherbornComboTimingTuning GetComboTuning() const;

	/** Current chain length (0 when idle). */
	UFUNCTION(BlueprintPure, Category = "Aetherborn|Combo")
	int32 GetComboCount() const;

	/** Best chain reached since the last reset. */
	UFUNCTION(BlueprintPure, Category = "Aetherborn|Combo")
	int32 GetBestChain() const;

	/**
	 * Pushes the pawn's current snapshot into every bound widget.
	 *
	 * Called once per frame: from NativeTick when UMG ticks the widget, and from the
	 * owning controller as the guarantee. Idempotent per frame, so both may call it.
	 */
	void Refresh(float DeltaSeconds);

protected:
	// ---- required: the HUD cannot run without these -------------------------
	UPROPERTY(meta = (BindWidget))
	UProgressBar* PB_AttackProgress;

	UPROPERTY(meta = (BindWidget))
	UImage* Img_Playhead;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Txt_Flash;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Txt_Combo;

	// ---- optional: everything below degrades gracefully ---------------------
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* Img_HitTick;

	UPROPERTY(meta = (BindWidgetOptional))
	UImage* Img_ComboBand;

	UPROPERTY(meta = (BindWidgetOptional))
	UImage* Img_ParryBand;

	UPROPERTY(meta = (BindWidgetOptional))
	UImage* Img_HeavyBand;

	UPROPERTY(meta = (BindWidgetOptional))
	UImage* Img_Swing;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Txt_HitLabel;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Txt_ComboLabel;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Txt_HeavyLabel;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Txt_ParryLabel;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Txt_Need;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Txt_Best;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Txt_SinceHit;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Txt_History;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Txt_Chip;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Txt_Timings;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* Txt_Live;

	UPROPERTY(meta = (BindWidgetOptional))
	UBorder* Pip_1;

	UPROPERTY(meta = (BindWidgetOptional))
	UBorder* Pip_2;

	UPROPERTY(meta = (BindWidgetOptional))
	UBorder* Pip_3;

	UPROPERTY(meta = (BindWidgetOptional))
	UBorder* Pip_4;

	UPROPERTY(meta = (BindWidgetOptional))
	UBorder* Pip_5;

	UPROPERTY(meta = (BindWidgetOptional))
	UBorder* Pip_Heavy;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_Light;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_Heavy;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_Reset;

private:
	/** The window chip states the top-left card can show. */
	enum class EChipState : uint8
	{
		Closed,
		Buffer,
		Heavy,
		Parry,
		Freeze
	};

	void BindButtons();
	void UnbindButtons();

	UFUNCTION()
	void HandleLightClicked();

	UFUNCTION()
	void HandleHeavyClicked();

	UFUNCTION()
	void HandleResetClicked();

	AAetherbornComboTimingPawn* GetComboPawn() const;
	AAetherbornComboTimingPlayerController* GetComboController() const;

	/** Big centred feedback line (BUFFERED / TOO EARLY / DROPPED / ...). */
	void ShowFlash(const FString& Message, const FLinearColor& Color, float HoldSeconds);

	void UpdateFlash(float DeltaSeconds);

	void PushHistory(const FString& Entry);
	void RenderHistory();

	/** Pushes one snapshot into every bound widget. */
	void UpdateFromSnapshot(const FAetherbornComboTimingSnapshot& Snapshot, const FAetherbornComboTimingTuning& Tuning);

	/** The project timings card and the live x-speed line. */
	void UpdateTimingsCard(const FAetherbornComboTimingTuning& Tuning);

	/** Places a band as an anchor range; falls back to a translation off-canvas. */
	void PlaceRange(UWidget* Target, float From, float To, float& CachedFrom, float& CachedTo, float TrackWidth);

	/** Places a thin tick centred on one fraction of the bar. */
	void PlaceTick(UWidget* Target, float At, float HalfWidth, float& CachedAt, float TrackWidth);

	void UpdatePips(const FAetherbornComboTimingSnapshot& Snapshot, bool bLightLive, bool bHeavyLive);

	/** Frame the last Refresh() ran on, so a frame is never pushed twice. */
	uint64 LastRefreshFrame = 0;

	/** Flash time left, in seconds. */
	float FlashTimer = 0.f;

	/** Last chain results: "x3" for a light chain, "H" for a heavy finisher. */
	TArray<FString> History;

	bool bHasPrevSeq = false;
	int32 PrevSeq = 0;
	int32 PrevCombo = 0;
	int32 PrevBest = 0;
	EAetherbornComboTimingKind PrevKind = EAetherbornComboTimingKind::Idle;

	float LastSwingFrom = -1.f;
	float LastSwingTo = -1.f;
	float LastHitTickAt = -1.f;
	float LastPlayheadAt = -1.f;
	float LastComboFrom = -1.f;
	float LastComboTo = -1.f;
	float LastParryFrom = -1.f;
	float LastParryTo = -1.f;
	float LastHeavyFrom = -1.f;
	float LastHeavyTo = -1.f;
};
