// Action bar for the Aetherborn: six ability cells, the GCD sweep, the cast bar and the rotation hint.
// Everything is driven from UEclipseAbilityComponent, which stays the single source of truth for
// cooldowns, the GCD and affordability - the bar holds no timers of its own.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EclipseAbilityBarWidget.generated.h"

class AEclipseCharacter;
class UEclipseAbilitySlotWidget;
class UBorder;
class UHorizontalBox;
class UProgressBar;
class UTextBlock;

UCLASS(Abstract)
class ECLIPSEROTF_API UEclipseAbilityBarWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    /** Explicit source. Left null the bar falls back to the owning player pawn. */
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Bar")
    void SetSourceCharacter(AEclipseCharacter* InCharacter);

    /** How many ability cells the bar builds. Six covers the current ability table. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Eclipse|Bar") int32 SlotCount = 6;

    /** Cell widget class. Falls back to /Game/UI/WBP_AbilitySlot when unset. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Eclipse|Bar")
    TSubclassOf<UEclipseAbilitySlotWidget> SlotWidgetClass;

protected:
    void BuildSlots();

    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UHorizontalBox> SlotRow;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UProgressBar> Bar_GCD;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> Text_GCD;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UBorder> Border_Cast;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UProgressBar> Bar_Cast;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> Text_Cast;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> Text_Hint;

    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Bar") TObjectPtr<AEclipseCharacter> SourceCharacter = nullptr;
    UPROPERTY() TArray<TObjectPtr<UEclipseAbilitySlotWidget>> SlotWidgets;
};
