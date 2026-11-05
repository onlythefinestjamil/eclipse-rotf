#include "EclipseDRSystem.h"

UEclipseDRSystem::UEclipseDRSystem()
{
    ResetDuration = 15.0f;
}

float UEclipseDRSystem::GetDRMultiplier(EDRCategory Category) const
{
    const FDRTracker* Tracker = Trackers.Find(Category);
    if (!Tracker) return 1.0f;

    switch (Tracker->Stage)
    {
    case 0: return 1.0f;
    case 1: return 1.0f;
    case 2: return 0.5f;
    case 3: return 0.25f;
    default: return 0.0f;
    }
}

float UEclipseDRSystem::ApplyCC(EDRCategory Category, float BaseDuration)
{
    FDRTracker* Tracker = Trackers.Find(Category);
    if (!Tracker)
    {
        FDRTracker NewTracker;
        NewTracker.Category = Category;
        NewTracker.Stage = 1;
        NewTracker.ResetDuration = ResetDuration;
        Trackers.Add(Category, NewTracker);
        Tracker = Trackers.Find(Category);
    }
    else
    {
        Tracker->Stage = FMath::Min(Tracker->Stage + 1, 4);
    }

    float Multiplier = GetDRMultiplier(Category);
    return BaseDuration * Multiplier;
}

bool UEclipseDRSystem::IsImmune(EDRCategory Category) const
{
    const FDRTracker* Tracker = Trackers.Find(Category);
    return Tracker && Tracker->Stage >= 4;
}

void UEclipseDRSystem::ResetCategory(EDRCategory Category)
{
    if (Trackers.Contains(Category))
    {
        Trackers[Category].Stage = 0;
    }
}

TArray<FDRTracker> UEclipseDRSystem::GetActiveTrackers() const
{
    TArray<FDRTracker> Result;
    Trackers.GenerateValueArray(Result);
    return Result;
}
