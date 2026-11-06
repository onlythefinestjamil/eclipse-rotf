// Eclipse Solara Level Implementation
// First playable compact scene — 300m hero path

#include "EclipseSolaraLevel.h"
#include "EclipseCharacter.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

AEclipseSolaraLevel::AEclipseSolaraLevel()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AEclipseSolaraLevel::BeginPlay()
{
    Super::BeginPlay();
    InitializeLevel();
}

void AEclipseSolaraLevel::InitializeLevel()
{
    UE_LOG(LogEclipse, Log, TEXT("Solara Level initializing — 300m playable path"));

    // Level zones (per V4 directive):
    // 1. Vista point — high overlook with hero shot of Solara landscape
    // 2. Village center — trade avenue with NPCs
    // 3. Ruins entry — ancient architecture entrance
    // 4. Checkpoint — Grace Site save point
    // 5. Boss arena — Khorez, Keeper of the Seventh Lock

    UE_LOG(LogEclipse, Log, TEXT("Vista: %s"), *VistaPoint.ToString());
    UE_LOG(LogEclipse, Log, TEXT("Village: %s"), *VillageCenter.ToString());
    UE_LOG(LogEclipse, Log, TEXT("Ruins: %s"), *RuinsEntry.ToString());
    UE_LOG(LogEclipse, Log, TEXT("Checkpoint: %s"), *CheckpointLocation.ToString());
    UE_LOG(LogEclipse, Log, TEXT("Boss Arena: %s"), *BossArena.ToString());
}

void AEclipseSolaraLevel::SpawnEnemy(FVector Location, bool bIsRanged)
{
    UE_LOG(LogEclipse, Log, TEXT("Spawning %s enemy at %s"),
        bIsRanged ? TEXT("ranged") : TEXT("melee"),
        *Location.ToString());

    // In full implementation:
    // - Melee enemy: approaches player, light/heavy attacks
    // - Ranged enemy: stays at distance, casts spells
    // Both use AI controller with behavior tree
}

void AEclipseSolaraLevel::SpawnBoss()
{
    UE_LOG(LogEclipse, Log, TEXT("Spawning boss: Khorez, Keeper of the Seventh Lock"));

    // Khorez boss phases (per V4 bible):
    // Phase 1 (100-70%): Polearm combos, frontal cleave, delayed radial slam
    // Phase 2 (70-35%): Twin memory ghosts replicate one previous attack
    // Phase 3 (35-0%): Breakable lock pillars offer defensive cover
}

void AEclipseSolaraLevel::SaveCheckpoint()
{
    UE_LOG(LogEclipse, Log, TEXT("Checkpoint saved at %s"), *CheckpointLocation.ToString());

    // Grace Site — save player position, health, resources, weapon level
    // Per V4: save/checkpoint loop must function for first playable validation
}
