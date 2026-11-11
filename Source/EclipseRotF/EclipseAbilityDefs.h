// Static ability definitions for the EclipseRotF combat loop.
// One row per ability in a DataTable; UEclipseAbilityComponent falls back to an identical built-in
// table when no table is assigned, so the combat loop never depends on an asset being wired up.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "EclipseCharacter.h" // EResourceType
#include "EclipseAbilityDefs.generated.h"

class UTexture2D;

USTRUCT(BlueprintType)
struct FEclipseAbilityDef : public FTableRowBase
{
    GENERATED_BODY()

    /** Row name is the ability Id used by UEclipseAbilityComponent::ActivateAbility. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Ability") FString DisplayName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Ability") FString Description;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Ability") float Cooldown = 5.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Ability") EResourceType CostType = EResourceType::Mana;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Ability") float Cost = 0.0f;
    /** When true the ability is refused while the combat state machine owns the body (attacking, dodging, casting, staggered). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Ability") bool bRequiresFreeAction = true;
    /** Higher wins when the rotation layer picks the next button. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Ability") int32 RotationPriority = 0;
    /** Left-to-right order on the action bar. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Ability") int32 Slot = 0;
    /** Display-only key cap, mirrored from IMC_Default. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Ability") FString KeyLabel;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Ability") TObjectPtr<UTexture2D> Icon = nullptr;
};
