// Eclipse front end: ancestry table row + character slot types.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "EclipseCharacter.h"
#include "EclipseFrontEndTypes.generated.h"

ECLIPSEROTF_API DECLARE_LOG_CATEGORY_EXTERN(LogEclipseFrontEnd, Log, All);

UENUM(BlueprintType)
enum class EEclipseRoleTendency : uint8
{
    Tank    UMETA(DisplayName = "Tank"),
    DPS     UMETA(DisplayName = "DPS"),
    Bruiser UMETA(DisplayName = "Bruiser"),
};

/** One playable ancestry, shown in the character creation screen. Row name = EAncestry value name. */
USTRUCT(BlueprintType)
struct ECLIPSEROTF_API FEclipseAncestryInfo : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ancestry")
    EAncestry Ancestry = EAncestry::Aetherborn;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ancestry")
    FText DisplayName;

    /** Short line under the name in the grid (e.g. "Lunarian Descendants"). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ancestry")
    FText Subtitle;

    /** Sub-type shown under the title (e.g. "Muscalar"). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ancestry")
    FText TypeName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ancestry", meta = (MultiLine = true))
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ancestry")
    TObjectPtr<UTexture2D> Portrait = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ancestry")
    EEclipseRoleTendency DefaultRole = EEclipseRoleTendency::DPS;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ancestry")
    TArray<FText> CoreTraits;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes", meta = (ClampMin = 0, ClampMax = 100))
    int32 Strength = 60;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes", meta = (ClampMin = 0, ClampMax = 100))
    int32 Endurance = 60;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes", meta = (ClampMin = 0, ClampMax = 100))
    int32 Aether = 60;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes", meta = (ClampMin = 0, ClampMax = 100))
    int32 Agility = 60;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes", meta = (ClampMin = 0, ClampMax = 100))
    int32 Focus = 60;

    /** False = attribute values are display placeholders, not canon balance numbers. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
    bool bAttributesFinal = false;
};

/** One saved character. */
USTRUCT(BlueprintType)
struct ECLIPSEROTF_API FEclipseCharacterSlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Slot")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Slot")
    EAncestry Ancestry = EAncestry::Aetherborn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Slot")
    EEclipseRoleTendency Role = EEclipseRoleTendency::DPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Slot")
    int32 Level = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Slot")
    FDateTime CreatedAt;
};
