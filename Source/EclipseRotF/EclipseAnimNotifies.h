#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "EclipseAnimNotifies.generated.h"

/**
 * Place on a light/heavy attack montage on the frame the blade is out.
 * Drives UEclipseCombatComponent::OpenHitWindow() -> the swing's single damage sweep.
 */
UCLASS(meta = (DisplayName = "Eclipse Hit Window"))
class ECLIPSEROTF_API UEclipseAnimNotify_HitWindow : public UAnimNotify
{
    GENERATED_BODY()

public:
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
    virtual FString GetNotifyName_Implementation() const override;
};

/**
 * Place on a light attack montage in its recovery section.
 * Drives UEclipseCombatComponent::OpenComboWindow() -> the chain window for the next swing.
 */
UCLASS(meta = (DisplayName = "Eclipse Combo Window"))
class ECLIPSEROTF_API UEclipseAnimNotify_ComboWindow : public UAnimNotify
{
    GENERATED_BODY()

public:
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
    virtual FString GetNotifyName_Implementation() const override;
};
