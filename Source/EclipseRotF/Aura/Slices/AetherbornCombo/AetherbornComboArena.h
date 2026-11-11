#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AetherbornComboArena.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class UMaterialInterface;

/** Courtyard: ground, ring dressing, lights and the camera rig the pawn reads. */
UCLASS()
class AAetherbornComboArena : public AActor
{
	GENERATED_BODY()

public:
	AAetherbornComboArena();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherborn|Materials")
	TObjectPtr<UMaterialInterface> GroundMat;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherborn|Materials")
	TObjectPtr<UMaterialInterface> StoneMat;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherborn|Materials")
	TObjectPtr<UMaterialInterface> StoneAltMat;

	UFUNCTION(BlueprintCallable, Category = "Aetherborn|Arena")
	void Punch(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Aetherborn|Arena")
	void UpdateRig(float DeltaSeconds);

	UFUNCTION(BlueprintPure, Category = "Aetherborn|Arena")
	FVector GetCameraLocation() const;

	UFUNCTION(BlueprintPure, Category = "Aetherborn|Arena")
	FVector GetCameraTarget() const;

protected:
	/** Creates a cube part sized in cm and attaches it to InParent. */
	UStaticMeshComponent* MakePart(USceneComponent* InParent, const FString& InName, const FVector& SizeCm, const FVector& LocCm, const FRotator& Rot, UMaterialInterface* Mat);

	UPROPERTY()
	TObjectPtr<USceneComponent> Root;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Ground;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> Parts;

	UPROPERTY()
	float RigTime = 0.0f;

	UPROPERTY()
	float ShakeEnergy = 0.0f;

	UPROPERTY()
	FVector RigOffset = FVector::ZeroVector;
};
