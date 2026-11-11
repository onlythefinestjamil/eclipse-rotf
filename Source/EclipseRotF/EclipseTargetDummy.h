#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EclipseTargetDummy.generated.h"

class UStaticMeshComponent;
class UAbilitySystemComponent;
class UEclipseAttributeSet;

/** Berserker target with one telegraphed swing so parry / block / i-frames can be observed. */
UCLASS()
class ECLIPSEROTF_API AEclipseTargetDummy : public AActor
{
    GENERATED_BODY()

public:
    AEclipseTargetDummy();

    virtual void BeginPlay() override;
    virtual void PostInitializeComponents() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Eclipse|Dummy") UStaticMeshComponent* Mesh = nullptr;

    /** Receives equipment status effects (Status.<Affinity>) and buildup. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Eclipse|GAS") TObjectPtr<UAbilitySystemComponent> AbilitySystem = nullptr;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Eclipse|GAS") TObjectPtr<UEclipseAttributeSet> StatusAttributes = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Dummy") float MaxHealth = 500.0f;
    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Dummy") float Health = 500.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Dummy") float MaxPoise = 100.0f;
    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Dummy") float Poise = 100.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Dummy") float AttackInterval = 3.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Dummy") float WindupSeconds = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Dummy") float AttackDamage = 40.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Dummy") float AttackRange = 350.0f;
    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Dummy") bool bWindingUp = false;

private:
    void ExecuteTelegraphAttack();

    float TimeUntilAttack = 0.0f;
    float WindupRemaining = 0.0f;
    bool bAttackPending = false;
    bool bStaggered = false;
    float StaggerRemaining = 0.0f;
    FVector BaseLocation = FVector::ZeroVector;
};
