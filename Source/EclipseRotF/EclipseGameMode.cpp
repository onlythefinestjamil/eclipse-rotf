// Eclipse Game Mode Implementation
// Online + Offline modes + Hybrid movement (Wukong + Elden Ring + WoW)

#include "EclipseGameMode.h"
#include "EclipseCharacter.h"
#include "Engine/Engine.h"

AEclipseGameMode::AEclipseGameMode()
{
    // Default to offline story mode
    CurrentMode = EGameModeType::OfflineStory;
    bIsOnline = false;
    MovementStyle = EMovementStyle::Hybrid;
}

void AEclipseGameMode::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogEclipse, Log, TEXT("Eclipse Game Mode: %s"), *UEnum::GetValueAsString(CurrentMode));
    UE_LOG(LogEclipse, Log, TEXT("Movement Style: %s"), *UEnum::GetValueAsString(MovementStyle));
    UE_LOG(LogEclipse, Log, TEXT("Online: %s"), bIsOnline ? TEXT("Yes") : TEXT("No"));
}

void AEclipseGameMode::SetGameMode(EGameModeType Mode)
{
    CurrentMode = Mode;

    switch (Mode)
    {
    case EGameModeType::OfflineStory:
        bIsOnline = false;
        MaxPlayers = 1;
        UE_LOG(LogEclipse, Log, TEXT("Mode: Offline Solo Story"));
        break;

    case EGameModeType::OnlineCoop:
        bIsOnline = true;
        MaxPlayers = 4;
        UE_LOG(LogEclipse, Log, TEXT("Mode: Online Co-op (max 4)"));
        break;

    case EGameModeType::OnlinePvE:
        bIsOnline = true;
        MaxPlayers = 10;
        UE_LOG(LogEclipse, Log, TEXT("Mode: Online PvE (max 10)"));
        break;

    case EGameModeType::OnlinePvP:
        bIsOnline = true;
        MaxPlayers = 10;
        UE_LOG(LogEclipse, Log, TEXT("Mode: Online PvP Arena"));
        break;

    case EGameModeType::OfflineCouch:
        bIsOnline = false;
        bIsCouchCoop = true;
        MaxPlayers = 4;
        UE_LOG(LogEclipse, Log, TEXT("Mode: Offline Couch Co-op (1-4 local)"));
        break;

    case EGameModeType::OfflineTraining:
        bIsOnline = false;
        MaxPlayers = 1;
        UE_LOG(LogEclipse, Log, TEXT("Mode: Offline Training"));
        break;
    }
}

void AEclipseGameMode::SetOnline(bool bOnline)
{
    bIsOnline = bOnline;
    if (!bOnline)
    {
        CurrentMode = EGameModeType::OfflineStory;
    }
    UE_LOG(LogEclipse, Log, TEXT("Online: %s"), bOnline ? TEXT("Enabled") : TEXT("Disabled"));
}

void AEclipseGameMode::SetMovementStyle(EMovementStyle Style)
{
    MovementStyle = Style;
    UE_LOG(LogEclipse, Log, TEXT("Movement Style: %s"), *UEnum::GetValueAsString(Style));
}

float AEclipseGameMode::GetDodgeIFrameDuration() const
{
    switch (MovementStyle)
    {
    case EMovementStyle::Wukong:    return Wukong_DodgeIFrames;
    case EMovementStyle::EldenRing: return EldenRing_DodgeIFrames;
    case EMovementStyle::WoW:      return WoW_DodgeIFrames;
    case EMovementStyle::Hybrid:   return Hybrid_DodgeIFrames;
    default: return 0.3f;
    }
}

float AEclipseGameMode::GetParryWindow() const
{
    switch (MovementStyle)
    {
    case EMovementStyle::Wukong:    return Wukong_ParryWindow;
    case EMovementStyle::EldenRing: return EldenRing_ParryWindow;
    case EMovementStyle::WoW:      return WoW_ParryWindow;
    case EMovementStyle::Hybrid:   return Hybrid_ParryWindow;
    default: return 0.14f;
    }
}

float AEclipseGameMode::GetAttackCommitment() const
{
    switch (MovementStyle)
    {
    case EMovementStyle::Wukong:    return Wukong_Commitment;
    case EMovementStyle::EldenRing: return EldenRing_Commitment;
    case EMovementStyle::WoW:      return WoW_Commitment;
    case EMovementStyle::Hybrid:   return Hybrid_Commitment;
    default: return 0.5f;
    }
}

float AEclipseGameMode::GetGCDForCurrentStyle() const
{
    switch (MovementStyle)
    {
    case EMovementStyle::Wukong:    return Wukong_GCD;
    case EMovementStyle::EldenRing: return EldenRing_GCD;
    case EMovementStyle::WoW:      return WoW_GCD;
    case EMovementStyle::Hybrid:   return Hybrid_GCD;
    default: return 1.0f;
    }
}

bool AEclipseGameMode::CanCancelAnimation() const
{
    // Wukong: can cancel most attacks (fluid)
    // Elden Ring: committed (can't cancel during active frames)
    // WoW: ability locked (can't cancel)
    // Hybrid: can cancel during windup but not during active frames

    switch (MovementStyle)
    {
    case EMovementStyle::Wukong:    return true;   // Fluid — cancel anytime
    case EMovementStyle::EldenRing: return false;  // Committed — no cancel
    case EMovementStyle::WoW:      return false;  // Locked — no cancel
    case EMovementStyle::Hybrid:   return true;   // Can cancel during windup only
    default: return true;
    }
}
