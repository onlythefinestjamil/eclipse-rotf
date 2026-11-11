#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "InputCoreTypes.h"
#include "GameFramework/SaveGame.h"
#include "EclipseKeybindSubsystem.generated.h"

UCLASS()
class ECLIPSEROTF_API UEclipseKeybindSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UPROPERTY()
    TMap<FName, FKey> Bindings;
};

/** Action -> key map with duplicate rejection and save/load, applied to IMC_Default at runtime. */
UCLASS()
class ECLIPSEROTF_API UEclipseKeybindSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Keybinds") bool Rebind(FName Action, FKey NewKey);
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Keybinds") FKey GetBinding(FName Action) const;
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Keybinds") FName GetActionForKey(FKey Key) const;
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Keybinds") TMap<FName, FKey> GetAllBindings() const { return Bindings; }
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Keybinds") FString GetBindingDisplayName(FName Action) const;
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Keybinds") void SaveBindings();
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Keybinds") void LoadBindings();
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Keybinds") void ApplyToMappingContext();

private:
    UPROPERTY()
    TMap<FName, FKey> Defaults;

    UPROPERTY()
    TMap<FName, FKey> Bindings;

    void BuildDefaults();
    static FString SaveSlotName() { return TEXT("EclipseKeybinds"); }
};
