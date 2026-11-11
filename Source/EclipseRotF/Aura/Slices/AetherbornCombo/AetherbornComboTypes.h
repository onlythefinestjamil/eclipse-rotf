#pragma once

#include "CoreMinimal.h"
#include "AetherbornComboTypes.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAetherbornCombo, Log, All);

UENUM(BlueprintType)
enum class EComboPhase : uint8
{
	Idle     UMETA(DisplayName = "Idle"),
	Windup   UMETA(DisplayName = "Windup"),
	Active   UMETA(DisplayName = "Active"),
	Recovery UMETA(DisplayName = "Recovery")
};

UENUM(BlueprintType)
enum class EComboAttemptKind : uint8
{
	Hit      UMETA(DisplayName = "Hit"),
	Buffered UMETA(DisplayName = "Buffered"),
	Early    UMETA(DisplayName = "Early"),
	Late     UMETA(DisplayName = "Late"),
	Timeout  UMETA(DisplayName = "Timeout")
};

USTRUCT(BlueprintType)
struct FComboSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherborn")
	float WindowLen = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherborn")
	float SpeedMult = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherborn")
	bool bEarlyBuffers = false;
};

USTRUCT(BlueprintType)
struct FComboAttempt
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherborn")
	int32 Step = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherborn")
	EComboAttemptKind Kind = EComboAttemptKind::Timeout;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherborn")
	FString Label;
};

USTRUCT(BlueprintType)
struct FComboSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn")
	int32 Step = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn")
	EComboPhase Phase = EComboPhase::Idle;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn")
	float PhaseProgress = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn")
	bool bWindowOpen = false;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn")
	float WindowProgress = -1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn")
	bool bBuffered = false;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn")
	int32 Longest = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn")
	TArray<FComboAttempt> History;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn")
	FString Cue;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn")
	FString CueKind;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn")
	float LastDamage = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FComboSwingSignature, int32, Step);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FComboHitSignature, int32, Step, float, Damage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FComboChainSignature, int32, Step);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FComboDropSignature, FString, Reason);

namespace AetherbornComboTiming
{
	constexpr float Windup = 0.18f;
	constexpr float Active = 0.12f;
	constexpr float Recovery = 0.34f;
	constexpr float Window = 0.25f;
	constexpr float ChainLinger = 0.45f;
	constexpr float CueTime = 0.85f;
	constexpr int32 MaxHistory = 10;
	constexpr float Damage[3] = { 12.0f, 16.0f, 24.0f };
}
