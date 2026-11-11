// Owns character slots, the active character, and Enter World travel with an async-loaded loading screen.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Containers/Ticker.h"
#include "FrontEnd/EclipseFrontEndTypes.h"
#include "EclipseFrontEndSubsystem.generated.h"

class UEclipseCharacterSaveGame;
class UUserWidget;
class UPackage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEclipseLoadProgressSignature, float, Percent, FText, Status);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEclipseSlotsChangedSignature);

UCLASS()
class ECLIPSEROTF_API UEclipseFrontEndSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    static const FString SaveSlotName;

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // --- Ancestries ---

    /** All ancestry rows, ordered by EAncestry value. */
    UFUNCTION(BlueprintCallable, Category = "Eclipse|FrontEnd")
    TArray<FEclipseAncestryInfo> GetAllAncestries() const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|FrontEnd")
    bool GetAncestryInfo(EAncestry Ancestry, FEclipseAncestryInfo& OutInfo) const;

    // --- Slots ---

    UFUNCTION(BlueprintPure, Category = "Eclipse|FrontEnd")
    TArray<FEclipseCharacterSlot> GetSlots() const;

    UFUNCTION(BlueprintPure, Category = "Eclipse|FrontEnd")
    int32 GetMaxSlots() const;

    /** Creates, saves and selects a new character. Fails on empty name or full slots. */
    UFUNCTION(BlueprintCallable, Category = "Eclipse|FrontEnd")
    bool CreateCharacter(const FString& CharacterName, EAncestry Ancestry, EEclipseRoleTendency Role, int32& OutSlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|FrontEnd")
    bool SelectSlot(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|FrontEnd")
    bool DeleteSlot(int32 SlotIndex);

    UFUNCTION(BlueprintPure, Category = "Eclipse|FrontEnd")
    int32 GetSelectedSlotIndex() const;

    UFUNCTION(BlueprintPure, Category = "Eclipse|FrontEnd")
    bool GetActiveCharacter(FEclipseCharacterSlot& OutSlot) const;

    // --- Travel ---

    /** Shows the loading screen, async-loads the world level, then opens it. Needs a selected slot. */
    UFUNCTION(BlueprintCallable, Category = "Eclipse|FrontEnd")
    bool EnterWorld();

    /** 0..1 */
    UFUNCTION(BlueprintPure, Category = "Eclipse|FrontEnd")
    float GetLoadProgress() const { return LoadProgress; }

    UFUNCTION(BlueprintPure, Category = "Eclipse|FrontEnd")
    FText GetLoadStatus() const { return LoadStatus; }

    UFUNCTION(BlueprintPure, Category = "Eclipse|FrontEnd")
    bool IsTravelling() const { return bTravelling; }

    UPROPERTY(BlueprintAssignable, Category = "Eclipse|FrontEnd")
    FEclipseLoadProgressSignature OnLoadProgress;

    UPROPERTY(BlueprintAssignable, Category = "Eclipse|FrontEnd")
    FEclipseSlotsChangedSignature OnSlotsChanged;

private:
    void LoadOrCreateSave();
    bool WriteSave();
    void SetProgress(float NewProgress);
    bool TickLoad(float DeltaTime);
    void HandlePackageLoaded(const FName& PackageName, UPackage* LoadedPackage, EAsyncLoadingResult::Type Result);
    void HandlePreLoadMap(const FString& MapName);
    void HandlePostLoadMap(UWorld* LoadedWorld);

    UPROPERTY(Transient)
    TObjectPtr<UEclipseCharacterSaveGame> SaveData = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UUserWidget> LoadingWidget = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UPackage> PreloadedPackage = nullptr;

    FTSTicker::FDelegateHandle LoadTickHandle;
    FDelegateHandle PreLoadMapHandle;
    FDelegateHandle PostLoadMapHandle;
    FName PendingPackageName;
    float LoadProgress = 0.0f;
    FText LoadStatus;
    double TravelStartTime = 0.0;
    bool bTravelling = false;
    bool bPackageLoaded = false;
    bool bOpenRequested = false;
};
