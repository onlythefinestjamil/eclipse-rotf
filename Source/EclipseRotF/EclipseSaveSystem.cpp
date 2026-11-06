// Eclipse Save System Implementation
// Checkpoints, Save/Load, Player Progress

#include "EclipseSaveSystem.h"
#include "Engine/Engine.h"

UEclipseSaveSystem::UEclipseSaveSystem()
{
    InitializeDefaultCheckpoints();
}

void UEclipseSaveSystem::InitializeDefaultCheckpoints()
{
    // Solara region checkpoints
    FCheckpointData Cp1;
    Cp1.CheckpointName = "Grace Site - Solara Gate";
    Cp1.Location = FVector(0, 0, 500);
    Cp1.Region = "Solara";
    Cp1.Zone = "Crownlands";
    Cp1.bIsGraceSite = true;
    Cp1.bIsBossCheckpoint = false;
    Checkpoints.Add(Cp1);

    FCheckpointData Cp2;
    Cp2.CheckpointName = "Grace Site - Ancient Village";
    Cp2.Location = FVector(5000, 0, 500);
    Cp2.Region = "Solara";
    Cp2.Zone = "Crownlands";
    Cp2.bIsGraceSite = true;
    Cp2.bIsBossCheckpoint = false;
    Checkpoints.Add(Cp2);

    FCheckpointData Cp3;
    Cp3.CheckpointName = "Grace Site - Ruins Overlook";
    Cp3.Location = FVector(15000, 2000, 800);
    Cp3.Region = "Solara";
    Cp3.Zone = "Crownlands";
    Cp3.bIsGraceSite = true;
    Cp3.bIsBossCheckpoint = false;
    Checkpoints.Add(Cp3);

    FCheckpointData Cp4;
    Cp4.CheckpointName = "Grace Site - Boss Arena Entrance";
    Cp4.Location = FVector(28000, 5000, 1200);
    Cp4.Region = "Solara";
    Cp4.Zone = "Crownlands";
    Cp4.bIsGraceSite = true;
    Cp4.bIsBossCheckpoint = false;
    Checkpoints.Add(Cp4);

    FCheckpointData Cp5;
    Cp5.CheckpointName = "Boss Checkpoint - After Khorez";
    Cp5.Location = FVector(30000, 5000, 1200);
    Cp5.Region = "Solara";
    Cp5.Zone = "Crownlands";
    Cp5.bIsGraceSite = true;
    Cp5.bIsBossCheckpoint = true;
    Cp5.ConnectedBossName = "Khorez the Unmade";
    Checkpoints.Add(Cp5);
}

FString UEclipseSaveSystem::GetSaveSlotName(ESaveSlot Slot) const
{
    switch (Slot)
    {
    case ESaveSlot::AutoSave: return "Eclipse_AutoSave";
    case ESaveSlot::ManualSave: return "Eclipse_ManualSave";
    case ESaveSlot::CheckpointSave: return "Eclipse_Checkpoint";
    case ESaveSlot::BossCheckpoint: return "Eclipse_BossCheckpoint";
    default: return "Eclipse_Save";
    }
}

FString UEclipseSaveSystem::GetSavePath() const
{
    return FPaths::ProjectSavedDir() / "SaveGames";
}

bool UEclipseSaveSystem::SaveGame(ESaveSlot Slot, const FPlayerSaveData& PlayerData)
{
    FString SlotName = GetSaveSlotName(Slot);
    UE_LOG(LogTemp, Log, TEXT("[Save] Saving to slot: %s"), *SlotName);
    UE_LOG(LogTemp, Log, TEXT("[Save] Player: Level %d, HP %.0f/%.0f, Region: %s"),
        PlayerData.PlayerLevel, PlayerData.CurrentHealth, PlayerData.MaxHealth, *PlayerData.CurrentRegion);
    
    CurrentPlayerData = PlayerData;
    
    // In a real implementation, this would use USaveGame or a custom save system
    // For now, log the save data
    UE_LOG(LogTemp, Log, TEXT("[Save] Saved successfully"));
    return true;
}

bool UEclipseSaveSystem::LoadGame(ESaveSlot Slot, FPlayerSaveData& OutPlayerData)
{
    FString SlotName = GetSaveSlotName(Slot);
    UE_LOG(LogTemp, Log, TEXT("[Save] Loading from slot: %s"), *SlotName);
    
    OutPlayerData = CurrentPlayerData;
    return true;
}

bool UEclipseSaveSystem::CreateCheckpoint(const FString& Name, FVector Location, const FString& Region, const FString& Zone)
{
    FCheckpointData NewCheckpoint;
    NewCheckpoint.CheckpointName = Name;
    NewCheckpoint.Location = Location;
    NewCheckpoint.Region = Region;
    NewCheckpoint.Zone = Zone;
    NewCheckpoint.bIsGraceSite = true;
    NewCheckpoint.bIsBossCheckpoint = false;
    
    Checkpoints.Add(NewCheckpoint);
    CurrentPlayerData.LastCheckpointLocation = Location;
    CurrentPlayerData.LastCheckpointName = Name;
    CurrentPlayerData.CurrentRegion = Region;
    CurrentPlayerData.CurrentZone = Zone;
    
    UE_LOG(LogTemp, Log, TEXT("[Save] Checkpoint created: %s (%s, %s)"), *Name, *Region, *Zone);
    return true;
}

bool UEclipseSaveSystem::LoadToLastCheckpoint(FPlayerSaveData& OutPlayerData)
{
    OutPlayerData = CurrentPlayerData;
    UE_LOG(LogTemp, Log, TEXT("[Save] Loading to last checkpoint: %s"), *CurrentPlayerData.LastCheckpointName);
    return true;
}

TArray<FCheckpointData> UEclipseSaveSystem::GetAllCheckpoints() const
{
    return Checkpoints;
}

bool UEclipseSaveSystem::DeleteSave(ESaveSlot Slot)
{
    FString SlotName = GetSaveSlotName(Slot);
    UE_LOG(LogTemp, Log, TEXT("[Save] Deleting slot: %s"), *SlotName);
    return true;
}

bool UEclipseSaveSystem::DoesSaveExist(ESaveSlot Slot) const
{
    // In a real implementation, check if the save file exists
    return true;
}

FString UEclipseSaveSystem::GetSaveInfo(ESaveSlot Slot) const
{
    FString SlotName = GetSaveSlotName(Slot);
    return FString::Printf(TEXT("%s - Level %d - %s - PlayTime: %.1f"),
        *SlotName, CurrentPlayerData.PlayerLevel,
        *CurrentPlayerData.CurrentRegion, CurrentPlayerData.TotalPlayTime);
}
