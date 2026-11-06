// Eclipse Class Roles - Tank, DPS, Healer, Hybrid
// Some classes can choose Tank/DPS/Healer, others are DPS-only

#pragma once
#include "CoreMinimal.h"
#include "EclipseClassRoles.generated.h"

UENUM(BlueprintType)
enum class EClassRole : uint8
{
    Tank        UMETA(DisplayName = "Tank - absorbs damage, holds aggro, protects party"),
    DPS         UMETA(DisplayName = "DPS - deals damage, fragile, position-dependent"),
    Healer      UMETA(DisplayName = "Healer - restores health, cleanses, resurrects"),
    Hybrid      UMETA(DisplayName = "Hybrid - can switch between Tank/DPS or DPS/Healer"),
    Support     UMETA(DisplayName = "Support - buffs, debuffs, utility, crowd control"),
};

UENUM(BlueprintType)
enum class ERoleFlexibility : uint8
{
    Fixed       UMETA(DisplayName = "Fixed - locked to one role"),
    Flexible    UMETA(DisplayName = "Flexible - can choose from multiple roles"),
};

USTRUCT(BlueprintType)
struct FClassRoleInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) ERoleFlexibility Flexibility = ERoleFlexibility::Fixed;
    UPROPERTY(BlueprintReadWrite) TArray<EClassRole> AvailableRoles;
    UPROPERTY(BlueprintReadWrite) FString Description = "";
};

UCLASS(Blueprintable)
class ECLIPSEROTF_API UEclipseClassRoles : public UObject
{
    GENERATED_BODY()

public:
    UEclipseClassRoles();

    // Get role info for a class
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Roles")
    FClassRoleInfo GetClassRoleInfo(EClass CharacterClass) const;

    // Can this class be a tank?
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Roles")
    bool CanBeTank(EClass CharacterClass) const;

    // Can this class be a healer?
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Roles")
    bool CanBeHealer(EClass CharacterClass) const;

    // Can this class be DPS?
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Roles")
    bool CanBeDPS(EClass CharacterClass) const;

    // Is this class locked to one role?
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Roles")
    bool IsFixedRole(EClass CharacterClass) const;

private:
    TMap<EClass, FClassRoleInfo> ClassRoles;

    void InitializeRoles();
};
