#include "EclipseAbilityBarWidget.h"

#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"

#include "EclipseCharacter.h"
#include "EclipseAbilityComponent.h"
#include "EclipseAbilitySlotWidget.h"
#include "EclipseCombatComponent.h"

void UEclipseAbilityBarWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (!SourceCharacter)
    {
        SourceCharacter = Cast<AEclipseCharacter>(GetOwningPlayerPawn());
    }

    BuildSlots();
}

void UEclipseAbilityBarWidget::SetSourceCharacter(AEclipseCharacter* InCharacter)
{
    SourceCharacter = InCharacter;
}

void UEclipseAbilityBarWidget::BuildSlots()
{
    SlotWidgets.Reset();

    if (!SlotRow)
    {
        UE_LOG(LogTemp, Warning, TEXT("[HUD] Ability bar has no SlotRow bound - the cell row will stay empty"));
        return;
    }

    SlotRow->ClearChildren();

    TSubclassOf<UEclipseAbilitySlotWidget> CellClass = SlotWidgetClass;
    if (!CellClass)
    {
        CellClass = LoadClass<UEclipseAbilitySlotWidget>(nullptr, TEXT("/Game/UI/WBP_AbilitySlot.WBP_AbilitySlot_C"));
    }
    if (!CellClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("[HUD] Ability slot class not found - /Game/UI/WBP_AbilitySlot missing"));
        return;
    }

    for (int32 Index = 0; Index < SlotCount; ++Index)
    {
        UEclipseAbilitySlotWidget* Cell = CreateWidget<UEclipseAbilitySlotWidget>(GetOwningPlayer(), CellClass);
        if (!Cell)
        {
            continue;
        }

        if (UHorizontalBoxSlot* BoxSlot = SlotRow->AddChildToHorizontalBox(Cell))
        {
            BoxSlot->SetPadding(FMargin(4.0f, 0.0f));
        }

        SlotWidgets.Add(Cell);
    }
}

void UEclipseAbilityBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!SourceCharacter)
    {
        SourceCharacter = Cast<AEclipseCharacter>(GetOwningPlayerPawn());
        if (!SourceCharacter && GetOwningPlayer())
        {
            SourceCharacter = Cast<AEclipseCharacter>(GetOwningPlayer()->GetPawn());
        }
    }

    UEclipseAbilityComponent* Abilities = SourceCharacter ? SourceCharacter->GetAbilities() : nullptr;
    if (!Abilities)
    {
        return;
    }

    const FName Suggested = Abilities->GetRotationSuggestion();

    for (int32 Index = 0; Index < SlotWidgets.Num(); ++Index)
    {
        UEclipseAbilitySlotWidget* Cell = SlotWidgets[Index];
        if (!Cell)
        {
            continue;
        }

        const FEclipseAbilityRuntime Ability = Abilities->GetAbilityBySlot(Index);
        if (Ability.Id.IsNone())
        {
            Cell->SetVisibility(ESlateVisibility::Collapsed);
            continue;
        }

        Cell->SetVisibility(ESlateVisibility::HitTestInvisible);

        FString Blocker;
        const bool bReady = Abilities->CanActivateAbility(Ability.Id, Blocker);
        Cell->RefreshSlot(Ability, bReady, Ability.Id == Suggested, Abilities->IsOnGCD());
    }

    if (Bar_GCD)
    {
        const float Percent = Abilities->GCDDuration > 0.0f
            ? FMath::Clamp(1.0f - Abilities->GCDRemaining / Abilities->GCDDuration, 0.0f, 1.0f)
            : 1.0f;
        Bar_GCD->SetPercent(Percent);
    }

    if (Text_GCD)
    {
        Text_GCD->SetText(Abilities->GCDRemaining > 0.01f
            ? FText::FromString(FString::Printf(TEXT("GCD %.2f"), Abilities->GCDRemaining))
            : FText::FromString(TEXT("GCD ready")));
    }

    UEclipseCombatComponent* Combat = SourceCharacter ? SourceCharacter->GetCombat() : nullptr;
    const bool bCasting = Combat && Combat->bCastActive;

    if (Border_Cast)
    {
        Border_Cast->SetVisibility(bCasting ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
    }
    if (Bar_Cast && Combat)
    {
        Bar_Cast->SetPercent(Combat->CastProgress);
    }
    if (Text_Cast)
    {
        Text_Cast->SetText(bCasting && Combat
            ? FText::FromString(FString::Printf(TEXT("CASTING  %.0f%%"), Combat->CastProgress * 100.0f))
            : FText::GetEmpty());
    }

    if (Text_Hint)
    {
        if (Suggested.IsNone())
        {
            Text_Hint->SetText(FText::GetEmpty());
        }
        else
        {
            const FEclipseAbilityRuntime Next = Abilities->GetAbility(Suggested);
            Text_Hint->SetText(FText::FromString(FString::Printf(TEXT("NEXT  %s  [%s]"), *Next.DisplayName, *Next.KeyLabel)));
        }
    }
}
