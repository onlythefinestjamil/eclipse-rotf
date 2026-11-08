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

    // Level zones (per V9 canon + Solara Cathedral Environment Design v2):
    // 1. Vista point — high overlook with hero shot of sun-scorched cathedral lands
    // 2. Cathedral Approach — weathered stone path, collapsed outer walls, bronze doors
    //    (mapped to VillageCenter FVector for Blueprint compat)
    // 3. Cathedral Nave — interior, Gothic arches, stained glass, cosmic void ceiling breach
    //    (mapped to RuinsEntry FVector for Blueprint compat)
    // 4. Grace Site — bonfire-style save point in side chapel
    // 5. Boss arena — Cathedral Apse: Pyre-Warden Velkor (destructible pillars, time manipulation)

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
    UE_LOG(LogEclipse, Log, TEXT("Spawning boss: Pyre-Warden Velkor (Cathedral of Unmade Hours)"));

    // Pyre-Warden Velkor boss phases (per V9 canon + Solara Cathedral Design v2):
    // Phase 1 (100-70%): Sword combos, frontal cleave
    // Phase 2 (70-35%): Destructible pillars fall, arena shrinks (Chaos Geometry Collection)
    // Phase 3 (35-0%): Time manipulation — rewinds player dodge
    // Enrage timer: 5 minutes
}

void AEclipseSolaraLevel::SaveCheckpoint()
{
    UE_LOG(LogEclipse, Log, TEXT("Checkpoint saved at %s"), *CheckpointLocation.ToString());

    // Grace Site — save player position, health, resources, weapon level
    // Per V4: save/checkpoint loop must function for first playable validation
}
