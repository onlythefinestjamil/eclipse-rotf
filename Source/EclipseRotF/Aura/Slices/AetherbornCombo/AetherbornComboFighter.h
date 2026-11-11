#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AetherbornComboTypes.h"
#include "AetherbornComboFighter.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

/** One pose of the procedural duelist, in radians, mirroring src/fighter.js. */
USTRUCT(BlueprintType)
struct FComboPose
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherborn|Pose") float sx = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherborn|Pose") float sy = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherborn|Pose") float sz = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherborn|Pose") float ex = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherborn|Pose") float ey = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherborn|Pose") float tx = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherborn|Pose") float ty = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherborn|Pose") float tz = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherborn|Pose") float ry = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherborn|Pose") float lean = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherborn|Pose") float bob = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherborn|Pose") float lx = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherborn|Pose") float ly = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherborn|Pose") float lz = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherborn|Pose") float lex = 0.0f;
};

USTRUCT(BlueprintType)
struct FComboSwingPose
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherborn|Pose") FComboPose Wound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherborn|Pose") FComboPose Strike;
};

/** Box-built duelist with procedurally posed sword swings. */
UCLASS()
class AAetherbornComboFighter : public AActor
{
	GENERATED_BODY()

public:
	AAetherbornComboFighter();

	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherborn|Materials")
	TObjectPtr<UMaterialInterface> SteelMat;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherborn|Materials")
	TObjectPtr<UMaterialInterface> SteelDarkMat;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherborn|Materials")
	TObjectPtr<UMaterialInterface> ClothMat;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherborn|Materials")
	TObjectPtr<UMaterialInterface> LeatherMat;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherborn|Materials")
	TObjectPtr<UMaterialInterface> EmberMat;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherborn|Materials")
	TObjectPtr<UMaterialInterface> FullerMat;

	UFUNCTION(BlueprintCallable, Category = "Aetherborn|Fighter")
	void SetStep(int32 NewStep);

	UFUNCTION(BlueprintCallable, Category = "Aetherborn|Fighter")
	void UpdatePose(float DeltaSeconds, const FComboSnapshot& Snapshot);

	UFUNCTION(BlueprintPure, Category = "Aetherborn|Fighter")
	FVector GetHitPoint() const;

protected:
	UStaticMeshComponent* MakePart(USceneComponent* InParent, const FString& InName, const FVector& SizeCm, const FVector& LocCm, const FRotator& Rot, UMaterialInterface* Mat);
	USceneComponent* MakeJoint(USceneComponent* InParent, const FString& InName, const FVector& LocCm);
	void BuildBody();

	UPROPERTY()
	TObjectPtr<USceneComponent> Root;

	UPROPERTY()
	TObjectPtr<USceneComponent> Rig;

	UPROPERTY()
	TObjectPtr<USceneComponent> Hips;

	UPROPERTY()
	TObjectPtr<USceneComponent> Torso;

	UPROPERTY()
	TObjectPtr<USceneComponent> Head;

	UPROPERTY()
	TObjectPtr<USceneComponent> ShoulderR;

	UPROPERTY()
	TObjectPtr<USceneComponent> ElbowR;

	UPROPERTY()
	TObjectPtr<USceneComponent> ShoulderL;

	UPROPERTY()
	TObjectPtr<USceneComponent> ElbowL;

	UPROPERTY()
	TObjectPtr<USceneComponent> Sword;

	UPROPERTY()
	TObjectPtr<USceneComponent> SwordTip;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> Parts;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> EmberDMI;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> FullerDMI;

	UPROPERTY(EditAnywhere, Category = "Aetherborn|Pose")
	FComboPose GuardPose;

	UPROPERTY(EditAnywhere, Category = "Aetherborn|Pose")
	FComboSwingPose SwingPoses[3];

	UPROPERTY()
	FComboPose Posed;

	UPROPERTY()
	FComboPose GuardLive;

	UPROPERTY()
	float Clock = 0.0f;

	UPROPERTY()
	int32 HintedStep = 0;

	void ApplyPose(const FComboPose& Pose);
};
