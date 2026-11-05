#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EclipseDRSystem.generated.h"

UENUM(BlueprintType)
enum class EDRCategory : uint8
{
    Stun, Fear, Incapacitate, Disorient, Root, Silence,
};

USTRUCT(BlueprintType)
struct FDRTracker
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) EDRCategory Category;
    UPROPERTY(BlueprintReadWrite) int32 Stage = 0;
    UPROPERTY(BlueprintReadWrite) float LastAppliedTime = 0.0f;
    UPROPERTY(BlueprintReadWrite) float ResetDuration = 15.0f;
};

UCLASS(Blueprintable)
class ECLIPSEROTF_API UEclipseDRSystem : public UObject
{
    GENERATED_BODY()

public:
    UEclipseDRSystem();

    UFUNCTION(BlueprintCallable, Category = "Eclipse|DR")
    float GetDRMultiplier(EDRCategory Category) const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|DR")
    float ApplyCC(EDRCategory Category, float BaseDuration);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|DR")
    bool IsImmune(EDRCategory Category) const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|DR")
    void ResetCategory(EDRCategory Category);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|DR")
    TArray<FDRTracker> GetActiveTrackers() const;

private:
    TMap<EDRCategory, FDRTracker> Trackers;
    float ResetDuration = 15.0f;
};
