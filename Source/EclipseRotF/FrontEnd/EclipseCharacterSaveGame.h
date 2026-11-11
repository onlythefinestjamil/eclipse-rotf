#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "FrontEnd/EclipseFrontEndTypes.h"
#include "EclipseCharacterSaveGame.generated.h"

UCLASS(BlueprintType)
class ECLIPSEROTF_API UEclipseCharacterSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Eclipse|FrontEnd")
    TArray<FEclipseCharacterSlot> Slots;

    UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Eclipse|FrontEnd")
    int32 LastSelectedIndex = INDEX_NONE;
};
