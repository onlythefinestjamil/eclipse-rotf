// Eclipse Solara Level — First Playable Compact Scene
// 300m playable hero path with vista, village, ruins, checkpoint
// Per V4 directive: build small polished Solara scene, not empty giant open world

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EclipseSolaraLevel.generated.h"

UCLASS(Blueprintable)
class ECLIPSEROTF_API AEclipseSolaraLevel : public AActor
{
    GENERATED_BODY()

public:
    AEclipseSolaraLevel();

    // Level zones
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solara")
    FVector VistaPoint = FVector(0, 0, 500); // High overlook

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solara")
    FVector VillageCenter = FVector(500, 0, 100); // Trade avenue

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solara")
    FVector RuinsEntry = FVector(1000, 0, 50); // Ancient ruins entrance

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solara")
    FVector CheckpointLocation = FVector(1500, 0, 50); // Grace Site checkpoint

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solara")
    FVector BossArena = FVector(2000, 0, 0); // Khorez boss arena

    // Level dimensions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solara")
    float LevelLength = 2500.0f; // ~300m playable path

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solara")
    float LevelWidth = 800.0f;

    // Environment
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solara|Environment")
    bool bIsDayTime = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solara|Environment")
    float FogDensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solara|Environment")
    FLinearColor AmbientColor = FLinearColor(0.05f, 0.05f, 0.08f, 1.0f); // Dark blue-black

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Solara|Environment")
    FLinearColor SunColor = FLinearColor(0.9f, 0.7f, 0.3f, 1.0f); // Warm amber

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Solara")
    void InitializeLevel();

    UFUNCTION(BlueprintCallable, Category = "Solara")
    void SpawnEnemy(FVector Location, bool bIsRanged);

    UFUNCTION(BlueprintCallable, Category = "Solara")
    void SpawnBoss();

    UFUNCTION(BlueprintCallable, Category = "Solara")
    void SaveCheckpoint();

protected:
    virtual void BeginPlay() override;
};
