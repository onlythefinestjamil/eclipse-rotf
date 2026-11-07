#include "EclipseCharacter.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

AEclipseCharacter::AEclipseCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AEclipseCharacter::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("Eclipse Character spawned - Ancestry: %d, Class: %d"), 
        (int32)Ancestry, (int32)CharacterClass);
}

bool AEclipseCharacter::IsOnGCD() const
{
    if (!GetWorld()) return false;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastGCDTriggerTime) < GlobalCooldownDuration;
}

void AEclipseCharacter::TriggerGCD()
{
    if (!GetWorld()) return;
    LastGCDTriggerTime = GetWorld()->GetTimeSeconds();
}

void AEclipseCharacter::AddFractureFlow(float Amount)
{
    FractureFlow = FMath::Clamp(FractureFlow + Amount, 0.0f, MaxFractureFlow);
}

void AEclipseCharacter::SpendFractureFlow(float Amount)
{
    FractureFlow = FMath::Clamp(FractureFlow - Amount, 0.0f, MaxFractureFlow);
}

void AEclipseCharacter::AddKill(bool bIsHonorKill, bool bIsBossKill)
{
    TotalKills++;
    if (bIsHonorKill) HonorKills++;
    if (bIsBossKill) BossKills++;

    int32 NewLevel = FMath::Clamp(FMath::Min(TotalKills / 1000, 50), 1, 50);
    if (NewLevel > WeaponLevel)
    {
        WeaponLevel = NewLevel;
        UE_LOG(LogTemp, Log, TEXT("Weapon leveled up to %d! (Kills: %d, Honor: %d, Boss: %d)"),
            WeaponLevel, TotalKills, HonorKills, BossKills);
    }
}
