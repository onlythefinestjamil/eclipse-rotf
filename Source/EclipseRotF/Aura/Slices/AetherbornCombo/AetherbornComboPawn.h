#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AetherbornComboPawn.generated.h"

class UCameraComponent;
class AAetherbornComboArena;

/** Camera-only pawn: the arena owns the rig math, this follows it. */
UCLASS()
class AAetherbornComboPawn : public APawn
{
	GENERATED_BODY()

public:
	AAetherbornComboPawn();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Aetherborn|Camera")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY()
	TObjectPtr<AAetherbornComboArena> Arena;
};
