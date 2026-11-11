// Native gameplay tags for equipment: affinities, statuses, status immunity, families, slots.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "EclipseCharacter.h"

namespace EclipseEquipmentTags
{
    /** Status.<Affinity> */
    ECLIPSEROTF_API FGameplayTag StatusTagFor(EElementalAffinity Affinity);
    /** Status.Immune.<Affinity> */
    ECLIPSEROTF_API FGameplayTag ImmunityTagFor(EElementalAffinity Affinity);
    /** Affinity.<Affinity> */
    ECLIPSEROTF_API FGameplayTag AffinityTagFor(EElementalAffinity Affinity);
    ECLIPSEROTF_API FString AffinityName(EElementalAffinity Affinity);
}
