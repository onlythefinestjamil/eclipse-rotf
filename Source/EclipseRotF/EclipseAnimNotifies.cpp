#include "EclipseAnimNotifies.h"

#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "EclipseCombatComponent.h"

namespace
{
    /** Resolves the combat component that owns the mesh the montage is playing on. */
    UEclipseCombatComponent* FindCombatComponent(USkeletalMeshComponent* MeshComp, const TCHAR* NotifyLabel)
    {
        AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
        UEclipseCombatComponent* Combat = Owner ? Owner->FindComponentByClass<UEclipseCombatComponent>() : nullptr;
        if (!Combat)
        {
            UE_LOG(LogTemp, Warning, TEXT("[ANIM] %s notify on %s but no EclipseCombatComponent"),
                NotifyLabel, *GetNameSafe(Owner));
        }
        return Combat;
    }
}

void UEclipseAnimNotify_HitWindow::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (UEclipseCombatComponent* Combat = FindCombatComponent(MeshComp, TEXT("HitWindow")))
    {
        UE_LOG(LogTemp, Log, TEXT("[ANIM] notify HitWindow on %s"), *Combat->GetOwner()->GetName());
        Combat->OpenHitWindow();
    }
}

FString UEclipseAnimNotify_HitWindow::GetNotifyName_Implementation() const
{
    return TEXT("Eclipse Hit Window");
}

void UEclipseAnimNotify_ComboWindow::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (UEclipseCombatComponent* Combat = FindCombatComponent(MeshComp, TEXT("ComboWindow")))
    {
        UE_LOG(LogTemp, Log, TEXT("[ANIM] notify ComboWindow on %s"), *Combat->GetOwner()->GetName());
        Combat->OpenComboWindow();
    }
}

FString UEclipseAnimNotify_ComboWindow::GetNotifyName_Implementation() const
{
    return TEXT("Eclipse Combo Window");
}
