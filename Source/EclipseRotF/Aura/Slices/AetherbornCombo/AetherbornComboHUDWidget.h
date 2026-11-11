#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AetherbornComboTypes.h"
#include "AetherbornComboHUDWidget.generated.h"

class UBorder;
class UTextBlock;
class UHorizontalBox;
class UImage;
class UButton;
class USlider;
class UCheckBox;
class UCanvasPanel;

USTRUCT()
struct FComboDamageNumber
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UTextBlock> Text;

	UPROPERTY()
	FVector2D StartPos = FVector2D::ZeroVector;

	UPROPERTY()
	float Drift = 0.0f;

	UPROPERTY()
	float Rise = 0.0f;

	UPROPERTY()
	float Life = 0.0f;

	UPROPERTY()
	float MaxLife = 0.95f;
};

/** Timing-bar HUD: phase pill, cues, pips, attempt chips, tuning panel and screen-space FX. */
UCLASS()
class UAetherbornComboHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/** Broadcast when the on-screen ATTACK button is pressed (bound in C++ by the director). */
	FSimpleDelegate OnAttackPressed;

	/** Broadcast when the on-screen RESET button is pressed (bound in C++ by the director). */
	FSimpleDelegate OnResetPressed;

	UFUNCTION(BlueprintCallable, Category = "Aetherborn|UI")
	FComboSettings ReadSettings() const;

	UFUNCTION(BlueprintCallable, Category = "Aetherborn|UI")
	void UpdateFromSnapshot(const FComboSnapshot& Snapshot, const FComboSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "Aetherborn|UI")
	void PopDamage(const FVector& WorldLocation, float Damage);

	UFUNCTION(BlueprintCallable, Category = "Aetherborn|UI")
	void Pulse(float Strength);

	/** Exposed so the controller can log what the panel currently reports. */
	UFUNCTION(BlueprintPure, Category = "Aetherborn|UI")
	float GetComboWindowValue() const;

protected:
	UFUNCTION()
	void OnAttackButtonClicked();

	UFUNCTION()
	void OnResetButtonClicked();

	void SetCueText(const FString& Text, const FString& Kind);
	void RebuildChips(const TArray<FComboAttempt>& History);
	void ApplyBarGeometry(float WindupSeconds, float ActiveSeconds, float WindowSeconds, float RecoverySeconds);
	void RefreshValueLabels();
	void TickDamageNumbers();

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UBorder> Border_PhasePill;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Text_PhaseLabel;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Text_Cue;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UBorder> Border_Pip1;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UBorder> Border_Pip2;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UBorder> Border_Pip3;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Text_Longest;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UHorizontalBox> Box_Track;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UBorder> Border_SegWindup;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UBorder> Border_SegActive;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UBorder> Border_SegWindow;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UBorder> Border_SegRecovery;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UImage> Image_Playhead;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UBorder> Border_MissFlash;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> Button_Attack;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> Button_Reset;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UHorizontalBox> Box_Chips;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Text_ChipsEmpty;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<USlider> Slider_WindowLen;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<USlider> Slider_SpeedMult;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UCheckBox> CheckBox_EarlyBuffers;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Text_WindowLenValue;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> Text_SpeedMultValue;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UImage> Image_Vignette;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UImage> Image_Ring;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UCanvasPanel> Canvas_Fx;

	UPROPERTY()
	TArray<FComboDamageNumber> DamageNumbers;

	UPROPERTY()
	FString CachedCueText;

	UPROPERTY()
	FString CachedCueKind;

	UPROPERTY()
	FString LastHistorySignature;

	UPROPERTY()
	float MissHideTime = 0.0f;

	UPROPERTY()
	float RingHideTime = 0.0f;

	UPROPERTY()
	float VignetteAlpha = 0.0f;

	UPROPERTY()
	int32 LastPhaseStep = -1;
};
