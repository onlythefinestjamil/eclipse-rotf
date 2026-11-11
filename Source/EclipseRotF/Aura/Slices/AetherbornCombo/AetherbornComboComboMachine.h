#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AetherbornComboTypes.h"
#include "AetherbornComboComboMachine.generated.h"

/** Pure timing state machine for the three-hit Aetherborn combo (port of src/combo.js). */
UCLASS(BlueprintType)
class UComboMachine : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Aetherborn|Combo")
	FComboSwingSignature OnSwing;

	UPROPERTY(BlueprintAssignable, Category = "Aetherborn|Combo")
	FComboHitSignature OnHit;

	UPROPERTY(BlueprintAssignable, Category = "Aetherborn|Combo")
	FComboChainSignature OnChain;

	UPROPERTY(BlueprintAssignable, Category = "Aetherborn|Combo")
	FComboDropSignature OnDrop;

	UFUNCTION(BlueprintCallable, Category = "Aetherborn|Combo")
	void Update(float DeltaSeconds, const FComboSettings& NewSettings);

	UFUNCTION(BlueprintCallable, Category = "Aetherborn|Combo")
	void SetSettings(const FComboSettings& NewSettings);

	UFUNCTION(BlueprintCallable, Category = "Aetherborn|Combo")
	void Press();

	UFUNCTION(BlueprintCallable, Category = "Aetherborn|Combo")
	void Reset(bool bClearHistory = true);

	UFUNCTION(BlueprintPure, Category = "Aetherborn|Combo")
	FComboSnapshot GetSnapshot() const;

	UFUNCTION(BlueprintPure, Category = "Aetherborn|Combo")
	int32 GetStep() const { return Step; }

protected:
	UPROPERTY()
	FComboSettings Settings;

	UPROPERTY()
	int32 Step = 0;

	UPROPERTY()
	EComboPhase Phase = EComboPhase::Idle;

	UPROPERTY()
	float T = 0.0f;

	UPROPERTY()
	bool bWindowOpen = false;

	UPROPERTY()
	float WindowT = 0.0f;

	UPROPERTY()
	bool bBuffered = false;

	UPROPERTY()
	int32 Longest = 0;

	UPROPERTY()
	float LastDamage = 0.0f;

	UPROPERTY()
	FString Cue;

	UPROPERTY()
	FString CueKind;

	UPROPERTY()
	float CueT = 0.0f;

	UPROPERTY()
	TArray<FComboAttempt> History;

	UPROPERTY()
	bool bRecorded = false;

	UPROPERTY()
	bool bDropped = false;

	float Speed() const;
	float DurOf(EComboPhase InPhase) const;
	float WinLen() const;
	void SetCue(const FString& Text, const FString& Kind);
	void LogAttempt(EComboAttemptKind Kind, const FString& Label);
	void StartSwing(int32 NewStep);
	void EndRecovery();
};
