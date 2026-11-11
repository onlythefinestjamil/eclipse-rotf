#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AetherbornComboDummy.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

/** Training dummy: stacked boxes, damped-spring recoil and a hit flash. */
UCLASS()
class AAetherbornComboDummy : public AActor
{
	GENERATED_BODY()

public:
	AAetherbornComboDummy();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherborn|Materials")
	TObjectPtr<UMaterialInterface> WoodMat;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherborn|Materials")
	TObjectPtr<UMaterialInterface> RopeMat;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherborn|Materials")
	TObjectPtr<UMaterialInterface> StoneMat;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherborn|Materials")
	TObjectPtr<UMaterialInterface> StrawMat;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherborn|Materials")
	TObjectPtr<UMaterialInterface> RustMat;

	UFUNCTION(BlueprintCallable, Category = "Aetherborn|Dummy")
	void Hit(float Damage);

	UFUNCTION(BlueprintPure, Category = "Aetherborn|Dummy")
	float GetFlash() const { return Flash; }

	UFUNCTION(BlueprintPure, Category = "Aetherborn|Dummy")
	FVector GetHitPoint() const;

protected:
	UStaticMeshComponent* MakePart(USceneComponent* InParent, const FString& InName, const FVector& SizeCm, const FVector& LocCm, const FRotator& Rot, UMaterialInterface* Mat);

	UPROPERTY()
	TObjectPtr<USceneComponent> Root;

	UPROPERTY()
	TObjectPtr<USceneComponent> Body;

	UPROPERTY()
	TObjectPtr<USceneComponent> Chest;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> Parts;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> StrawDMI;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> RustDMI;

	UPROPERTY()
	float Flash = 0.0f;

	UPROPERTY()
	float Recoil = 0.0f;

	UPROPERTY()
	float RecoilVel = 0.0f;
};
