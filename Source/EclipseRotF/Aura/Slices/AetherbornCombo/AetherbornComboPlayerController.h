#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AetherbornComboTypes.h"
#include "AetherbornComboPlayerController.generated.h"

class UAetherbornComboHUDWidget;
class AAetherbornComboArena;
class AAetherbornComboDummy;
class AAetherbornComboFighter;
class UComboMachine;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

/** Single director: owns the combo machine, the input and the per-frame driver loop. */
UCLASS()
class AAetherbornComboPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AAetherbornComboPlayerController();

	virtual void BeginPlay() override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherborn|UI")
	TSubclassOf<UAetherbornComboHUDWidget> HUDClass;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherborn|Input")
	TObjectPtr<UInputAction> AttackAction;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherborn|Input")
	TObjectPtr<UInputAction> ResetAction;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherborn|Input")
	TObjectPtr<UInputMappingContext> DefaultIMC;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn|Combo")
	TObjectPtr<UComboMachine> Machine;

	UFUNCTION(BlueprintPure, Category = "Aetherborn|Combo")
	FComboSnapshot GetLastSnapshot() const;

protected:
	void AttackPressed();
	void ResetPressed();
	void OnAttackAction(const FInputActionValue& Value);
	void OnResetAction(const FInputActionValue& Value);

	UFUNCTION()
	void HandleSwing(int32 Step);

	UFUNCTION()
	void HandleHit(int32 Step, float Damage);

	UFUNCTION()
	void HandleChain(int32 Step);

	UFUNCTION()
	void HandleDrop(FString Reason);

	UPROPERTY()
	TObjectPtr<UAetherbornComboHUDWidget> HUDWidget;

	UPROPERTY()
	TObjectPtr<AAetherbornComboArena> Arena;

	UPROPERTY()
	TObjectPtr<AAetherbornComboDummy> Dummy;

	UPROPERTY()
	TObjectPtr<AAetherbornComboFighter> Fighter;

	UPROPERTY()
	FComboSnapshot LastSnapshot;
};
