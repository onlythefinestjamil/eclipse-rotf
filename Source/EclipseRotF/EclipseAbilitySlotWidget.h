// One action bar cell. WBP_AbilitySlot supplies the look; this class drives it from live ability state.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EclipseAbilitySlotWidget.generated.h"

class UBorder;
class UImage;
class UProgressBar;
class UTextBlock;
struct FEclipseAbilityRuntime;

UCLASS(Abstract)
class ECLIPSEROTF_API UEclipseAbilitySlotWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Push one ability's live state into the bound widgets. Safe with every binding missing. */
    void RefreshSlot(const FEclipseAbilityRuntime& InAbility, bool bReady, bool bSuggested, bool bOnGCD);

protected:
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> Text_Name;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> Text_Key;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> Text_Cooldown;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> Text_Cost;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UProgressBar> Bar_Cooldown;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UBorder> Border_Frame;
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UImage> Image_Icon;

    /** Frame tint when the ability can fire right now. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Bar")
    FLinearColor ReadyColor = FLinearColor(0.129f, 0.153f, 0.192f, 1.0f);

    /** Frame tint when a cooldown, resource or the commitment gate blocks it. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Bar")
    FLinearColor BlockedColor = FLinearColor(0.055f, 0.063f, 0.086f, 1.0f);

    /** Frame tint for the ability the rotation layer suggests next. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Bar")
    FLinearColor SuggestedColor = FLinearColor(1.0f, 0.478f, 0.2f, 1.0f);

    /** Frame tint while the global cooldown is running and nothing may fire. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Bar")
    FLinearColor GCDColor = FLinearColor(0.196f, 0.216f, 0.271f, 1.0f);
};
