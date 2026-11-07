// Eclipse Save System — Checkpoints, Save/Load, Player Progress
// Grace Sites (Elden Ring-style), boss checkpoints, player progression persistence

#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EclipseSaveSystem.generated.h"

UENUM(BlueprintType)
enum class ESaveSlot : uint8
{
    AutoSave         UMETA(DisplayName = "Auto Save"),
    ManualSave        UMETA(DisplayName = "Manual Save"),
    CheckpointSave    UMETA(DisplayName = "Checkpoint Save"),
    BossCheckpoint    UMETA(DisplayName = "Boss Checkpoint"),
};

USTRUCT(BlueprintType)
struct FCheckpointData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) FString CheckpointName = "";
    UPROPERTY(BlueprintReadWrite) FVector Location = FVector::ZeroVector;
    UPROPERTY(BlueprintReadWrite) FString Region = "";
    UPROPERTY(BlueprintReadWrite) FString Zone = "";
    UPROPERTY(BlueprintReadWrite) bool bIsGraceSite = true;
    UPROPERTY(BlueprintReadWrite) bool bIsBossCheckpoint = false;
    UPROPERTY(BlueprintReadWrite) FString ConnectedBossName = "";
};

USTRUCT(BlueprintType)
struct FPlayerSaveData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) int32 PlayerLevel = 1;
    UPROPERTY(BlueprintReadWrite) float CurrentHealth = 100.0f;
    UPROPERTY(BlueprintReadWrite) float MaxHealth = 100.0f;
    UPROPERTY(BlueprintReadWrite) FString CurrentRegion = "Solara";
    UPROPERTY(BlueprintReadWrite) FString CurrentZone = "Crownlands";
    UPROPERTY(BlueprintReadWrite) FVector LastCheckpointLocation = FVector::ZeroVector;
    UPROPERTY(BlueprintReadWrite) FString LastCheckpointName = "";
    UPROPERTY(BlueprintReadWrite) float TotalPlayTime = 0.0f;
    UPROPERTY(BlueprintReadWrite) TArray<int32> CompletedQuestIDs;
    UPROPERTY(BlueprintReadWrite) TArray<int32> DefeatedBossIDs;
    UPROPERTY(BlueprintReadWrite) TMap<FString, bool> WorldFlags;
    UPROPERTY(BlueprintReadWrite) TMap<FString, float> FactionReputation;
};

UCLASS(Blueprintable)
class ECLIPSEROTF_API UEclipseSaveSystem : public UObject
{
    GENERATED_BODY()

public:
    UEclipseSaveSystem();

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Save")
    bool SaveGame(ESaveSlot Slot, const FPlayerSaveData& PlayerData);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Save")
    bool LoadGame(ESaveSlot Slot, FPlayerSaveData& OutPlayerData);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Save")
    bool CreateCheckpoint(const FString& Name, FVector Location, const FString& Region, const FString& Zone);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Save")
    bool LoadToLastCheckpoint(FPlayerSaveData& OutPlayerData);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Save")
    TArray<FCheckpointData> GetAllCheckpoints() const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Save")
    bool DeleteSave(ESaveSlot Slot);

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Save")
    bool DoesSaveExist(ESaveSlot Slot) const;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Save")
    FString GetSaveInfo(ESaveSlot Slot) const;

private:
    void InitializeDefaultCheckpoints();
    FString GetSaveSlotName(ESaveSlot Slot) const;
    FString GetSavePath() const;

    TArray<FCheckpointData> Checkpoints;
    FPlayerSaveData CurrentPlayerData;
};
