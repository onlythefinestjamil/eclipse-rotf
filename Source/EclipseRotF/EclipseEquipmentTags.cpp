#include "EclipseEquipmentTags.h"
#include "NativeGameplayTags.h"

// Affinity.* (canon EElementalAffinity, 11)
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Affinity_Fire, "Affinity.Fire");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Affinity_Ice, "Affinity.Ice");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Affinity_Lightning, "Affinity.Lightning");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Affinity_Shadow, "Affinity.Shadow");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Affinity_Holy, "Affinity.Holy");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Affinity_Water, "Affinity.Water");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Affinity_Soul, "Affinity.Soul");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Affinity_Earth, "Affinity.Earth");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Affinity_Wind, "Affinity.Wind");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Affinity_Poison, "Affinity.Poison");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Affinity_Temporal, "Affinity.Temporal");

// Status.*
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Status_Fire, "Status.Fire");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Status_Ice, "Status.Ice");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Status_Lightning, "Status.Lightning");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Status_Shadow, "Status.Shadow");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Status_Holy, "Status.Holy");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Status_Water, "Status.Water");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Status_Soul, "Status.Soul");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Status_Earth, "Status.Earth");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Status_Wind, "Status.Wind");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Status_Poison, "Status.Poison");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Status_Temporal, "Status.Temporal");

// Status.Immune.*
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Immune_Fire, "Status.Immune.Fire");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Immune_Ice, "Status.Immune.Ice");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Immune_Lightning, "Status.Immune.Lightning");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Immune_Shadow, "Status.Immune.Shadow");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Immune_Holy, "Status.Immune.Holy");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Immune_Water, "Status.Immune.Water");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Immune_Soul, "Status.Immune.Soul");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Immune_Earth, "Status.Immune.Earth");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Immune_Wind, "Status.Immune.Wind");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Immune_Poison, "Status.Immune.Poison");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Immune_Temporal, "Status.Immune.Temporal");

// Item.Family.* (EWeaponType, 12 incl. Proposed OneHandAxe/Tome)
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Family_Greatsword, "Item.Family.Greatsword");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Family_Daggers, "Item.Family.Daggers");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Family_Spear, "Item.Family.Spear");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Family_GreatAxe, "Item.Family.GreatAxe");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Family_Hammer, "Item.Family.Hammer");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Family_Staff, "Item.Family.Staff");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Family_Bow, "Item.Family.Bow");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Family_Shield, "Item.Family.Shield");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Family_Scythe, "Item.Family.Scythe");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Family_FistWeapons, "Item.Family.FistWeapons");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Family_OneHandAxe, "Item.Family.OneHandAxe");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Family_Tome, "Item.Family.Tome");

// Item.Slot.*
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Slot_MainHand, "Item.Slot.MainHand");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Slot_OffHand, "Item.Slot.OffHand");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Slot_Head, "Item.Slot.Head");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Slot_Chest, "Item.Slot.Chest");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Slot_Hands, "Item.Slot.Hands");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Slot_Legs, "Item.Slot.Legs");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Slot_Feet, "Item.Slot.Feet");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Slot_Neck, "Item.Slot.Neck");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Slot_Ring, "Item.Slot.Ring");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Slot_Trinket, "Item.Slot.Trinket");

namespace EclipseEquipmentTags
{
    FString AffinityName(EElementalAffinity Affinity)
    {
        return StaticEnum<EElementalAffinity>()->GetNameStringByValue(static_cast<int64>(Affinity));
    }

    FGameplayTag StatusTagFor(EElementalAffinity Affinity)
    {
        switch (Affinity)
        {
        case EElementalAffinity::Fire: return TAG_Status_Fire;
        case EElementalAffinity::Ice: return TAG_Status_Ice;
        case EElementalAffinity::Lightning: return TAG_Status_Lightning;
        case EElementalAffinity::Shadow: return TAG_Status_Shadow;
        case EElementalAffinity::Holy: return TAG_Status_Holy;
        case EElementalAffinity::Water: return TAG_Status_Water;
        case EElementalAffinity::Soul: return TAG_Status_Soul;
        case EElementalAffinity::Earth: return TAG_Status_Earth;
        case EElementalAffinity::Wind: return TAG_Status_Wind;
        case EElementalAffinity::Poison: return TAG_Status_Poison;
        case EElementalAffinity::Temporal: return TAG_Status_Temporal;
        default: return FGameplayTag();
        }
    }

    FGameplayTag ImmunityTagFor(EElementalAffinity Affinity)
    {
        switch (Affinity)
        {
        case EElementalAffinity::Fire: return TAG_Immune_Fire;
        case EElementalAffinity::Ice: return TAG_Immune_Ice;
        case EElementalAffinity::Lightning: return TAG_Immune_Lightning;
        case EElementalAffinity::Shadow: return TAG_Immune_Shadow;
        case EElementalAffinity::Holy: return TAG_Immune_Holy;
        case EElementalAffinity::Water: return TAG_Immune_Water;
        case EElementalAffinity::Soul: return TAG_Immune_Soul;
        case EElementalAffinity::Earth: return TAG_Immune_Earth;
        case EElementalAffinity::Wind: return TAG_Immune_Wind;
        case EElementalAffinity::Poison: return TAG_Immune_Poison;
        case EElementalAffinity::Temporal: return TAG_Immune_Temporal;
        default: return FGameplayTag();
        }
    }

    FGameplayTag AffinityTagFor(EElementalAffinity Affinity)
    {
        switch (Affinity)
        {
        case EElementalAffinity::Fire: return TAG_Affinity_Fire;
        case EElementalAffinity::Ice: return TAG_Affinity_Ice;
        case EElementalAffinity::Lightning: return TAG_Affinity_Lightning;
        case EElementalAffinity::Shadow: return TAG_Affinity_Shadow;
        case EElementalAffinity::Holy: return TAG_Affinity_Holy;
        case EElementalAffinity::Water: return TAG_Affinity_Water;
        case EElementalAffinity::Soul: return TAG_Affinity_Soul;
        case EElementalAffinity::Earth: return TAG_Affinity_Earth;
        case EElementalAffinity::Wind: return TAG_Affinity_Wind;
        case EElementalAffinity::Poison: return TAG_Affinity_Poison;
        case EElementalAffinity::Temporal: return TAG_Affinity_Temporal;
        default: return FGameplayTag();
        }
    }
}
