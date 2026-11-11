#include "FrontEnd/EclipseFrontEndSubsystem.h"

#include "FrontEnd/EclipseCharacterSaveGame.h"
#include "FrontEnd/EclipseFrontEndSettings.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"
#include "MoviePlayer.h"
#include "Misc/App.h"
#include "HAL/PlatformTime.h"

DEFINE_LOG_CATEGORY(LogEclipseFrontEnd);

const FString UEclipseFrontEndSubsystem::SaveSlotName = TEXT("EclipseCharacters");

void UEclipseFrontEndSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    LoadOrCreateSave();
    PreLoadMapHandle = FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UEclipseFrontEndSubsystem::HandlePreLoadMap);
    PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UEclipseFrontEndSubsystem::HandlePostLoadMap);
    UE_LOG(LogEclipseFrontEnd, Log, TEXT("[FrontEnd] Subsystem initialized, %d saved character(s)"), SaveData ? SaveData->Slots.Num() : 0);
}

void UEclipseFrontEndSubsystem::Deinitialize()
{
    if (LoadTickHandle.IsValid())
    {
        FTSTicker::GetCoreTicker().RemoveTicker(LoadTickHandle);
        LoadTickHandle.Reset();
    }
    FCoreUObjectDelegates::PreLoadMap.Remove(PreLoadMapHandle);
    FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
    Super::Deinitialize();
}

// ---------------------------------------------------------------- Ancestries

TArray<FEclipseAncestryInfo> UEclipseFrontEndSubsystem::GetAllAncestries() const
{
    TArray<FEclipseAncestryInfo> Result;
    const UEclipseFrontEndSettings* Settings = GetDefault<UEclipseFrontEndSettings>();
    UDataTable* Table = Settings->AncestryTable.LoadSynchronous();
    if (!Table)
    {
        UE_LOG(LogEclipseFrontEnd, Warning, TEXT("[FrontEnd] AncestryTable is not set (Project Settings > Game > Eclipse Front End)"));
        return Result;
    }
    const UEnum* Enum = StaticEnum<EAncestry>();
    for (int32 i = 0; i < Enum->NumEnums() - 1; ++i)
    {
        const FName RowName(*Enum->GetNameStringByIndex(i));
        if (const FEclipseAncestryInfo* Row = Table->FindRow<FEclipseAncestryInfo>(RowName, TEXT("GetAllAncestries"), false))
        {
            Result.Add(*Row);
        }
        else
        {
            UE_LOG(LogEclipseFrontEnd, Warning, TEXT("[FrontEnd] Missing ancestry row %s"), *RowName.ToString());
        }
    }
    return Result;
}

bool UEclipseFrontEndSubsystem::GetAncestryInfo(EAncestry Ancestry, FEclipseAncestryInfo& OutInfo) const
{
    const UDataTable* Table = GetDefault<UEclipseFrontEndSettings>()->AncestryTable.LoadSynchronous();
    if (!Table)
    {
        return false;
    }
    const FName RowName(*StaticEnum<EAncestry>()->GetNameStringByValue(static_cast<int64>(Ancestry)));
    if (const FEclipseAncestryInfo* Row = Table->FindRow<FEclipseAncestryInfo>(RowName, TEXT("GetAncestryInfo"), false))
    {
        OutInfo = *Row;
        if (!Row->bAttributesFinal)
        {
            UE_LOG(LogEclipseFrontEnd, Verbose, TEXT("[FrontEnd] %s attributes TBD (placeholder values)"), *RowName.ToString());
        }
        return true;
    }
    return false;
}

// ---------------------------------------------------------------- Slots

void UEclipseFrontEndSubsystem::LoadOrCreateSave()
{
    if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
    {
        SaveData = Cast<UEclipseCharacterSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
    }
    if (!SaveData)
    {
        SaveData = Cast<UEclipseCharacterSaveGame>(UGameplayStatics::CreateSaveGameObject(UEclipseCharacterSaveGame::StaticClass()));
        UE_LOG(LogEclipseFrontEnd, Log, TEXT("[FrontEnd] No save found, created empty character save"));
    }
}

bool UEclipseFrontEndSubsystem::WriteSave()
{
    const bool bOk = SaveData && UGameplayStatics::SaveGameToSlot(SaveData, SaveSlotName, 0);
    UE_LOG(LogEclipseFrontEnd, Log, TEXT("[FrontEnd] Save '%s' %s (%d slots)"), *SaveSlotName, bOk ? TEXT("written") : TEXT("FAILED"), SaveData ? SaveData->Slots.Num() : 0);
    return bOk;
}

TArray<FEclipseCharacterSlot> UEclipseFrontEndSubsystem::GetSlots() const
{
    return SaveData ? SaveData->Slots : TArray<FEclipseCharacterSlot>();
}

int32 UEclipseFrontEndSubsystem::GetMaxSlots() const
{
    return GetDefault<UEclipseFrontEndSettings>()->MaxCharacterSlots;
}

bool UEclipseFrontEndSubsystem::CreateCharacter(const FString& CharacterName, EAncestry Ancestry, EEclipseRoleTendency Role, int32& OutSlotIndex)
{
    OutSlotIndex = INDEX_NONE;
    const FString Name = CharacterName.TrimStartAndEnd();
    if (!SaveData || Name.IsEmpty())
    {
        UE_LOG(LogEclipseFrontEnd, Warning, TEXT("[FrontEnd] CreateCharacter rejected: empty name"));
        return false;
    }
    if (SaveData->Slots.Num() >= GetMaxSlots())
    {
        UE_LOG(LogEclipseFrontEnd, Warning, TEXT("[FrontEnd] CreateCharacter rejected: all %d slots used"), GetMaxSlots());
        return false;
    }

    FEclipseCharacterSlot NewSlot;
    NewSlot.CharacterName = Name;
    NewSlot.Ancestry = Ancestry;
    NewSlot.Role = Role;
    NewSlot.Level = 1;
    NewSlot.CreatedAt = FDateTime::UtcNow();
    OutSlotIndex = SaveData->Slots.Add(NewSlot);
    SaveData->LastSelectedIndex = OutSlotIndex;

    UE_LOG(LogEclipseFrontEnd, Log, TEXT("[FrontEnd] Created '%s' (%s, %s) in slot %d"), *Name,
        *StaticEnum<EAncestry>()->GetNameStringByValue(static_cast<int64>(Ancestry)),
        *StaticEnum<EEclipseRoleTendency>()->GetNameStringByValue(static_cast<int64>(Role)), OutSlotIndex);
    WriteSave();
    OnSlotsChanged.Broadcast();
    return true;
}

bool UEclipseFrontEndSubsystem::SelectSlot(int32 SlotIndex)
{
    if (!SaveData || !SaveData->Slots.IsValidIndex(SlotIndex))
    {
        return false;
    }
    SaveData->LastSelectedIndex = SlotIndex;
    UE_LOG(LogEclipseFrontEnd, Log, TEXT("[FrontEnd] Selected slot %d '%s'"), SlotIndex, *SaveData->Slots[SlotIndex].CharacterName);
    WriteSave();
    OnSlotsChanged.Broadcast();
    return true;
}

bool UEclipseFrontEndSubsystem::DeleteSlot(int32 SlotIndex)
{
    if (!SaveData || !SaveData->Slots.IsValidIndex(SlotIndex))
    {
        return false;
    }
    UE_LOG(LogEclipseFrontEnd, Log, TEXT("[FrontEnd] Deleted slot %d '%s'"), SlotIndex, *SaveData->Slots[SlotIndex].CharacterName);
    SaveData->Slots.RemoveAt(SlotIndex);
    if (SaveData->LastSelectedIndex >= SaveData->Slots.Num())
    {
        SaveData->LastSelectedIndex = SaveData->Slots.Num() - 1;
    }
    WriteSave();
    OnSlotsChanged.Broadcast();
    return true;
}

int32 UEclipseFrontEndSubsystem::GetSelectedSlotIndex() const
{
    return (SaveData && SaveData->Slots.IsValidIndex(SaveData->LastSelectedIndex)) ? SaveData->LastSelectedIndex : INDEX_NONE;
}

bool UEclipseFrontEndSubsystem::GetActiveCharacter(FEclipseCharacterSlot& OutSlot) const
{
    const int32 Index = GetSelectedSlotIndex();
    if (Index == INDEX_NONE)
    {
        return false;
    }
    OutSlot = SaveData->Slots[Index];
    return true;
}

// ---------------------------------------------------------------- Travel

void UEclipseFrontEndSubsystem::SetProgress(float NewProgress)
{
    LoadProgress = FMath::Clamp(FMath::Max(LoadProgress, NewProgress), 0.0f, 1.0f);
    if (LoadProgress < 0.35f)      { LoadStatus = NSLOCTEXT("EclipseFrontEnd", "LoadAssets", "Loading Assets..."); }
    else if (LoadProgress < 0.75f) { LoadStatus = NSLOCTEXT("EclipseFrontEnd", "LoadSync", "Synchronizing Realm..."); }
    else                           { LoadStatus = NSLOCTEXT("EclipseFrontEnd", "LoadWorld", "Preparing World State..."); }
    OnLoadProgress.Broadcast(LoadProgress, LoadStatus);
}

bool UEclipseFrontEndSubsystem::EnterWorld()
{
    if (bTravelling)
    {
        return false;
    }
    if (GetSelectedSlotIndex() == INDEX_NONE)
    {
        UE_LOG(LogEclipseFrontEnd, Warning, TEXT("[FrontEnd] EnterWorld rejected: no character selected"));
        return false;
    }
    const UEclipseFrontEndSettings* Settings = GetDefault<UEclipseFrontEndSettings>();
    if (Settings->WorldLevel.IsNull())
    {
        UE_LOG(LogEclipseFrontEnd, Error, TEXT("[FrontEnd] EnterWorld: WorldLevel not set in Eclipse Front End settings"));
        return false;
    }

    bTravelling = true;
    bPackageLoaded = false;
    bOpenRequested = false;
    LoadProgress = 0.0f;
    TravelStartTime = FPlatformTime::Seconds();
    PendingPackageName = FName(*Settings->WorldLevel.GetLongPackageName());

    if (UClass* WidgetClass = Settings->LoadingScreenClass.LoadSynchronous())
    {
        LoadingWidget = CreateWidget<UUserWidget>(GetGameInstance(), WidgetClass);
        if (LoadingWidget)
        {
            LoadingWidget->AddToViewport(1000);
        }
    }
    else
    {
        UE_LOG(LogEclipseFrontEnd, Warning, TEXT("[FrontEnd] LoadingScreenClass not set, travelling without loading screen"));
    }

    SetProgress(0.0f);
    UE_LOG(LogEclipseFrontEnd, Log, TEXT("[FrontEnd] EnterWorld: async loading %s"), *PendingPackageName.ToString());

    if (FindPackage(nullptr, *PendingPackageName.ToString()) != nullptr)
    {
        // Already resident (e.g. editor has it open): nothing to stream, progress is driven by time.
        bPackageLoaded = true;
    }
    else
    {
        LoadPackageAsync(PendingPackageName.ToString(),
            FLoadPackageAsyncDelegate::CreateUObject(this, &UEclipseFrontEndSubsystem::HandlePackageLoaded));
    }

    LoadTickHandle = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateUObject(this, &UEclipseFrontEndSubsystem::TickLoad), 0.0f);
    return true;
}

void UEclipseFrontEndSubsystem::HandlePackageLoaded(const FName& PackageName, UPackage* LoadedPackage, EAsyncLoadingResult::Type Result)
{
    PreloadedPackage = LoadedPackage;
    bPackageLoaded = true;
    UE_LOG(LogEclipseFrontEnd, Log, TEXT("[FrontEnd] Async load of %s finished (%s)"), *PackageName.ToString(),
        Result == EAsyncLoadingResult::Succeeded ? TEXT("ok") : TEXT("failed, OpenLevel will load synchronously"));
}

bool UEclipseFrontEndSubsystem::TickLoad(float DeltaTime)
{
    const UEclipseFrontEndSettings* Settings = GetDefault<UEclipseFrontEndSettings>();
    const float MinTime = FMath::Max(0.1f, Settings->MinimumLoadingScreenTime);
    const float Elapsed = static_cast<float>(FPlatformTime::Seconds() - TravelStartTime);
    const float TimeAlpha = FMath::Clamp(Elapsed / MinTime, 0.0f, 1.0f);

    if (bOpenRequested)
    {
        // One frame at 100% has been drawn: open the level now.
        LoadTickHandle.Reset();
        UE_LOG(LogEclipseFrontEnd, Log, TEXT("[FrontEnd] Opening %s"), *PendingPackageName.ToString());
        UGameplayStatics::OpenLevelBySoftObjectPtr(GetGameInstance(), Settings->WorldLevel);
        return false;
    }

    float LoadAlpha = 1.0f;
    if (!bPackageLoaded)
    {
        const float Percent = GetAsyncLoadPercentage(PendingPackageName);
        LoadAlpha = Percent >= 0.0f ? Percent / 100.0f : 0.0f;
    }
    // Display = the slower of real streaming and the minimum display time, capped at 99% until both finish.
    SetProgress(FMath::Min(FMath::Min(LoadAlpha, TimeAlpha), 0.99f));

    if (bPackageLoaded && TimeAlpha >= 1.0f)
    {
        SetProgress(1.0f);
        bOpenRequested = true;
    }
    return true;
}

void UEclipseFrontEndSubsystem::HandlePreLoadMap(const FString& MapName)
{
    if (!bTravelling || GIsEditor || !IsMoviePlayerEnabled() || !LoadingWidget)
    {
        return;
    }
    // Packaged builds: keep the loading screen on screen during the blocking map load.
    FLoadingScreenAttributes Attributes;
    Attributes.bAutoCompleteWhenLoadingCompletes = true;
    Attributes.MinimumLoadingScreenDisplayTime = 0.0f;
    Attributes.WidgetLoadingScreen = LoadingWidget->TakeWidget();
    GetMoviePlayer()->SetupLoadingScreen(Attributes);
    UE_LOG(LogEclipseFrontEnd, Log, TEXT("[FrontEnd] MoviePlayer loading screen up for %s"), *MapName);
}

void UEclipseFrontEndSubsystem::HandlePostLoadMap(UWorld* LoadedWorld)
{
    if (!bTravelling)
    {
        return;
    }
    bTravelling = false;
    bPackageLoaded = false;
    bOpenRequested = false;
    if (LoadingWidget)
    {
        LoadingWidget->RemoveFromParent();
        LoadingWidget = nullptr;
    }
    PreloadedPackage = nullptr;
    UE_LOG(LogEclipseFrontEnd, Log, TEXT("[FrontEnd] Arrived in %s"), LoadedWorld ? *LoadedWorld->GetMapName() : TEXT("?"));
}
