// Eclipse Visual Pipeline — Lumen + Nanite Dark Fantasy Configuration
// This file configures the rendering pipeline for Elden Ring / Wukong quality
// Run in UE5.8 console or integrate into GameMode BeginPlay

#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EclipseVisualPipeline.generated.h"

UENUM(BlueprintType)
enum class EVisualQualityPreset : uint8
{
    Cinematic    UMETA(DisplayName = "Cinematic — Max quality, 30 FPS target"),
    High          UMETA(DisplayName = "High — High quality, 60 FPS target"),
    Balanced      UMETA(DisplayName = "Balanced — Medium quality, 60 FPS target"),
    Performance   UMETA(DisplayName = "Performance — Low quality, 120 FPS target"),
};

UCLASS(Blueprintable)
class ECLIPSEROTF_API UEclipseVisualPipeline : public UObject
{
    GENERATED_BODY()

public:
    // Apply dark fantasy rendering settings
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Visuals")
    static void ApplyDarkFantasyRendering(EVisualQualityPreset Quality = EVisualQualityPreset::High);

    // Apply Lumen configuration for dark fantasy
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Visuals")
    static void ConfigureLumen(bool bHighQuality = true);

    // Apply Nanite configuration
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Visuals")
    static void ConfigureNanite(bool bEnable = true);

    // Apply post-processing for cinematic look
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Visuals")
    static void ApplyCinematicPostProcess(class UPostProcessComponent* PPComponent);

    // Configure volumetric fog for atmospheric depth
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Visuals")
    static void ConfigureVolumetricFog(class AExponentialHeightFog* FogActor);

    // Get recommended settings for RX 6800 16GB
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Visuals")
    static FString GetRX6800Settings();

private:
    // Dark fantasy color palette values
    // Warm gold: #D4A843
    // Cool blue: #3B5998
    // Deep shadow: #050509
    // Ember orange: #8B4513
    // Moonlight blue: #4A6FA5
};
