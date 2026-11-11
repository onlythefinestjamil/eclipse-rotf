#include "EclipseAbilitySlotWidget.h"

#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

#include "EclipseAbilityComponent.h" // FEclipseAbilityRuntime

namespace
{
    FString ResourceLetter(EResourceType Type)
    {
        switch (Type)
        {
        case EResourceType::Mana:     return TEXT("M");
        case EResourceType::Stamina:  return TEXT("S");
        case EResourceType::Rage:     return TEXT("R");
        case EResourceType::Reagents: return TEXT("Rg");
        case EResourceType::Aether:   return TEXT("A");
        default:                      return FString();
        }
    }
}

void UEclipseAbilitySlotWidget::RefreshSlot(const FEclipseAbilityRuntime& InAbility, bool bReady, bool bSuggested, bool bOnGCD)
{
    if (Text_Name)
    {
        Text_Name->SetText(FText::FromString(InAbility.DisplayName));
    }

    if (Text_Key)
    {
        Text_Key->SetText(FText::FromString(InAbility.KeyLabel));
    }

    if (Text_Cost)
    {
        if (InAbility.Cost > 0.0f)
        {
            Text_Cost->SetText(FText::FromString(FString::Printf(TEXT("%.0f %s"), InAbility.Cost, *ResourceLetter(InAbility.CostType))));
        }
        else
        {
            Text_Cost->SetText(FText::GetEmpty());
        }
    }

    if (Bar_Cooldown)
    {
        // Fills from empty back to full as the cooldown runs down.
        const float Percent = InAbility.Cooldown > 0.0f
            ? FMath::Clamp(1.0f - InAbility.CooldownRemaining / InAbility.Cooldown, 0.0f, 1.0f)
            : 1.0f;
        Bar_Cooldown->SetPercent(Percent);
    }

    if (Text_Cooldown)
    {
        Text_Cooldown->SetText(InAbility.CooldownRemaining > 0.05f
            ? FText::FromString(FString::Printf(TEXT("%.1f"), InAbility.CooldownRemaining))
            : FText::GetEmpty());
    }

    if (Border_Frame)
    {
        FLinearColor Tint = bReady ? ReadyColor : (bOnGCD ? GCDColor : BlockedColor);
        if (bReady && bSuggested)
        {
            Tint = SuggestedColor;
        }
        Border_Frame->SetBrushColor(Tint);
    }

    if (Image_Icon)
    {
        if (InAbility.Icon)
        {
            Image_Icon->SetBrushFromTexture(InAbility.Icon.Get(), false);
            Image_Icon->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
        else
        {
            Image_Icon->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}
