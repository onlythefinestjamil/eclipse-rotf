#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EclipsePerfectDodgeComponent.generated.h"

class AActor;
class UStaticMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEclipsePerfectDodgeSignature, AActor*, DodgedSource);

/** One live afterimage ghost: the actor, its fading material instances, and its remaining life. */
USTRUCT()
struct FEclipseAfterImageInstance
{
    GENERATED_BODY()

    UPROPERTY()
    TObjectPtr<AActor> Actor = nullptr;

    UPROPERTY()
    TArray<TObjectPtr<UMaterialInstanceDynamic>> Mids;

    UPROPERTY()
    FVector BaseScale = FVector::OneVector;

    float Remaining = 0.0f;
    float Lifetime = 1.0f;
};

/**
 * Turns a perfect dodge (damage taken while the dodge i-frames are active) into a full response:
 * slow motion, an afterimage burst trailing the dodge, and an optional resource reward.
 *
 * The combat component detects the dodge and calls TriggerPerfectDodge; this component owns every
 * consequence of it, so the feel can be retuned on the BP CDO without touching the state machine.
 */
UCLASS(ClassGroup = (Eclipse), meta = (BlueprintSpawnableComponent))
class ECLIPSEROTF_API UEclipsePerfectDodgeComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEclipsePerfectDodgeComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** Fires once per perfect dodge. Hook Niagara / audio / HUD flashes here from Blueprint. */
    UPROPERTY(BlueprintAssignable, Category = "Eclipse|PerfectDodge")
    FEclipsePerfectDodgeSignature OnPerfectDodge;

    // --- Slow motion ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|PerfectDodge|SlowMo")
    bool bEnableSlowMo = true;

    /** World time dilation during the effect. Lower is slower. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|PerfectDodge|SlowMo", meta = (ClampMin = "0.01", ClampMax = "1.0"))
    float SlowMoTimeDilation = 0.3f;

    /** How long the slow motion lasts, measured in REAL seconds (unaffected by the dilation itself). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|PerfectDodge|SlowMo", meta = (ClampMin = "0.0"))
    float SlowMoDuration = 0.5f;

    /** Length of the ease back to normal speed at the end of the effect, in real seconds. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|PerfectDodge|SlowMo", meta = (ClampMin = "0.0"))
    float SlowMoRampOutSeconds = 0.2f;

    // --- Afterimages ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|PerfectDodge|AfterImage")
    bool bEnableAfterImages = true;

    /** Number of ghost copies spawned per perfect dodge. Few and life-size reads better than many and small. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|PerfectDodge|AfterImage", meta = (ClampMin = "0", ClampMax = "24"))
    int32 AfterImageCount = 4;

    /** Distance between consecutive ghosts along the dodge path, in cm. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|PerfectDodge|AfterImage", meta = (ClampMin = "0.0"))
    float AfterImageSpacing = 45.0f;

    /** Life of the closest ghost, in real seconds. Further ghosts fade proportionally faster. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|PerfectDodge|AfterImage", meta = (ClampMin = "0.05"))
    float AfterImageLifetime = 0.55f;

    /** Peak opacity of a fresh ghost. The ghost dissolves from this to 0; 1.0 reads too solid. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|PerfectDodge|AfterImage", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AfterImageStartOpacity = 0.75f;

    /** Upward drift of a dissolving ghost, in cm per second. Gives the smoke-like rise. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|PerfectDodge|AfterImage", meta = (ClampMin = "0.0"))
    float AfterImageRiseSpeed = 18.0f;

    /** Final uniform scale, used only when no translucent material is available and shrinking has to do the fading. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|PerfectDodge|AfterImage", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AfterImageEndScale = 0.7f;

    /**
     * Translucent/unlit material for the ghosts. Left empty, BeginPlay loads
     * /Game/Generated_Materials/M_AfterImage_Ghost. With no material at all the ghosts fall back
     * to the hero materials and fade by shrinking.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|PerfectDodge|AfterImage")
    TObjectPtr<UMaterialInterface> AfterImageMaterial = nullptr;

    /** Scalar parameter driven 1 -> 0 over a ghost's life when AfterImageMaterial is set. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|PerfectDodge|AfterImage")
    FName OpacityParameterName = TEXT("AfterImageOpacity");

    // --- Reward ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|PerfectDodge|Reward")
    bool bGrantAetherOnPerfectDodge = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|PerfectDodge|Reward")
    float AetherReward = 10.0f;

    // --- Read-only state ---
    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|PerfectDodge")
    int32 PerfectDodgeCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|PerfectDodge")
    bool bSlowMoActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|PerfectDodge")
    float SlowMoRemaining = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|PerfectDodge")
    int32 ActiveAfterImageCount = 0;

    /** Called by the combat component when damage is absorbed by the dodge i-frames. */
    UFUNCTION(BlueprintCallable, Category = "Eclipse|PerfectDodge")
    bool TriggerPerfectDodge(AActor* DodgedSource);

    UFUNCTION(BlueprintPure, Category = "Eclipse|PerfectDodge")
    bool IsSlowMoActive() const { return bSlowMoActive; }

    /** The dilation to restore to if something else briefly overrides world time (e.g. hit-stop). */
    UFUNCTION(BlueprintPure, Category = "Eclipse|PerfectDodge")
    float GetSlowMoDilation() const { return SlowMoTimeDilation; }

protected:
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    void SpawnAfterImageBurst(const FVector& TrailDirection);
    AActor* SpawnSingleAfterImage(const FVector& Location, const FRotator& Rotation, float Lifetime, TArray<TObjectPtr<UMaterialInstanceDynamic>>& OutMids);
    void UpdateAfterImages(float RealDeltaSeconds);
    void ClearAfterImages();
    void ApplyDilation(float Value);
    float GetRealDeltaSeconds();

    UPROPERTY()
    TArray<FEclipseAfterImageInstance> AfterImages;

    double LastRealTimeSeconds = 0.0;
    bool bDilationOwned = false;
};