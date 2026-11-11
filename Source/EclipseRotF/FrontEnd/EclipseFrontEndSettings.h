#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "EclipseFrontEndSettings.generated.h"

class UDataTable;
class UUserWidget;
class UWorld;

/** Project Settings > Game > Eclipse Front End. */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Eclipse Front End"))
class ECLIPSEROTF_API UEclipseFrontEndSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    /** DataTable with FEclipseAncestryInfo rows, one per EAncestry. */
    UPROPERTY(Config, EditAnywhere, Category = "FrontEnd", meta = (RequiredAssetDataTags = "RowStructure=/Script/EclipseRotF.EclipseAncestryInfo"))
    TSoftObjectPtr<UDataTable> AncestryTable;

    /** Widget shown while the world level loads. */
    UPROPERTY(Config, EditAnywhere, Category = "FrontEnd")
    TSoftClassPtr<UUserWidget> LoadingScreenClass;

    /** Level opened by Enter World. */
    UPROPERTY(Config, EditAnywhere, Category = "FrontEnd")
    TSoftObjectPtr<UWorld> WorldLevel;

    /** Loading screen stays at least this long so it is readable. */
    UPROPERTY(Config, EditAnywhere, Category = "FrontEnd", meta = (ClampMin = 0.0))
    float MinimumLoadingScreenTime = 2.0f;

    UPROPERTY(Config, EditAnywhere, Category = "FrontEnd", meta = (ClampMin = 1))
    int32 MaxCharacterSlots = 8;

    virtual FName GetCategoryName() const override { return TEXT("Game"); }
};
