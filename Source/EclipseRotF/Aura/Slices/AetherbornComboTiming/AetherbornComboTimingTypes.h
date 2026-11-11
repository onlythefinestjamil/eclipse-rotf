// AetherbornComboTiming — shared types, palette and the RETUNED timing contract.
//
// Include order rule for this slice: this header includes nothing project specific,
// and every other header in the slice includes it first.
//
// The retuned contract (overrides the Slice's src/contracts.js literals):
//   light  0.52 s = windup 0.12 + strike 0.08 (hit at 0.20) + recovery 0.32
//   heavy  1.35 s = windup 0.48 + strike 0.12 (hit at 0.60) + recovery 0.75
//   combo window default 0.28 s, opened at the hit (start of recovery)
//   parry 0.10 s from the heavy press, hit-stop 0.035 light / 0.13 heavy
//   chain max 5, light damage 40/44/48/52/56, heavy damage 90
#pragma once

#include "CoreMinimal.h"

#include "AetherbornComboTimingTypes.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAetherbornComboTiming, Log, All);

namespace AetherbornComboTiming
{
	// ---------------------------------------------------------------- light swing
	// 0.52 s total: 0.12 windup + 0.08 strike (hit at 0.20) + 0.32 recovery.
	inline constexpr float LightTotal = 0.52f;
	inline constexpr float LightWindup = 0.12f;
	inline constexpr float LightStrike = 0.08f;
	inline constexpr float LightHitAt = 0.20f; // LightWindup + LightStrike
	inline constexpr float LightRecovery = 0.32f;
	inline constexpr float LightHitStop = 0.035f;

	// ---------------------------------------------------------------- heavy cleave
	// 1.35 s total: 0.48 windup + 0.12 strike (hit at 0.60) + 0.75 recovery.
	inline constexpr float HeavyTotal = 1.35f;
	inline constexpr float HeavyWindup = 0.48f;
	inline constexpr float HeavyStrike = 0.12f;
	inline constexpr float HeavyHitAt = 0.60f; // HeavyWindup + HeavyStrike
	inline constexpr float HeavyRecovery = 0.75f;
	inline constexpr float HeavyHitStop = 0.13f;
	inline constexpr float HeavyDamage = 90.f;

	// ---------------------------------------------------------------- windowing
	inline constexpr float ParryWindow = 0.10f;      // measured from the heavy press
	inline constexpr int32 ChainMax = 5;             // authored light chain length
	inline constexpr float LightDamageBase = 40.f;   // 40 / 44 / 48 / 52 / 56
	inline constexpr float LightDamageStepPct = 0.10f;
	inline constexpr float ComboWindowMin = 0.05f;
	inline constexpr float ComboWindowMax = 0.50f;
	inline constexpr float SpeedMin = 0.40f;
	inline constexpr float SpeedMax = 2.00f;

	// --------------------------------------------------------------------- palette
	// Authored sRGB values; FLinearColor(FColor) converts them to linear space.
	inline const FLinearColor Bg = FLinearColor(FColor(0x07, 0x09, 0x0f));
	inline const FLinearColor Fog = FLinearColor(FColor(0x0a, 0x0d, 0x15));
	inline const FLinearColor Ground = FLinearColor(FColor(0x10, 0x14, 0x20));
	inline const FLinearColor Grid = FLinearColor(FColor(0x27, 0x32, 0x4a));
	inline const FLinearColor KeyColor = FLinearColor(FColor(0x7f, 0xb2, 0xff));
	inline const FLinearColor Rim = FLinearColor(FColor(0xff, 0xb4, 0x6b));
	inline const FLinearColor HeroSteel = FLinearColor(FColor(0x39, 0x44, 0x5b));
	inline const FLinearColor HeroTrim = FLinearColor(FColor(0xc9, 0xa4, 0x5c));
	inline const FLinearColor HeroSeam = FLinearColor(FColor(0x6f, 0xd3, 0xff));
	inline const FLinearColor DummyColor = FLinearColor(FColor(0x2a, 0x31, 0x43));
	inline const FLinearColor DummyCore = FLinearColor(FColor(0xff, 0x8a, 0x4c));
	inline const FLinearColor LightWindow = FLinearColor(FColor(0x4f, 0xd6, 0xff));
	inline const FLinearColor CancelWindow = FLinearColor(FColor(0xff, 0xd1, 0x66));
	inline const FLinearColor HitWindow = FLinearColor(FColor(0xff, 0x5f, 0x6d));
	inline const FLinearColor HeavyRecoveryTint = FLinearColor(FColor(0xff, 0x8a, 0x4c));
	inline const FLinearColor Parry = FLinearColor(FColor(0xb3, 0x9d, 0xff));
	inline const FLinearColor HitNumber = FLinearColor(FColor(0xbf, 0xe8, 0xff));
	inline const FLinearColor HeavyNumber = FLinearColor(FColor(0xff, 0xb0, 0x66));
	inline const FLinearColor Danger = FLinearColor(FColor(0xff, 0x6b, 0x57));
	inline const FLinearColor Ok = FLinearColor(FColor(0x7e, 0xf0, 0xa8));
	inline const FLinearColor Text = FLinearColor(FColor(0xdf, 0xe7, 0xf5));
	inline const FLinearColor TextDim = FLinearColor(FColor(0x8b, 0x97, 0xb0));

	// ------------------------------------------------------- anim notify labels
	inline const FString HitNotifyLabel = TEXT("ECLIPSE HIT WINDOW");
	inline const FString ComboNotifyLabel = TEXT("ECLIPSE COMBO WINDOW");
}

/** Player-facing feel knobs. ComboWindow is the combo-window DURATION in seconds. */
USTRUCT(BlueprintType)
struct FAetherbornComboTimingTuning
{
	GENERATED_BODY()

	/** Combo-window duration in absolute seconds (clamped to [0.05, 0.50]). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherborn|Combo")
	float ComboWindow = 0.28f;

	/** Playback speed multiplier; every timing is divided by it (hit-stop and window duration are not). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherborn|Combo")
	float SpeedMul = 1.0f;

	/** Buffer a press made before the combo window opens. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherborn|Combo")
	bool bBuffer = true;
};

/** What is running right now. */
UENUM(BlueprintType)
enum class EAetherbornComboTimingKind : uint8
{
	Idle,
	Light,
	Heavy
};

/** Phase inside the running attack. */
UENUM(BlueprintType)
enum class EAetherbornComboTimingPhase : uint8
{
	Neutral,
	Windup,
	Strike,
	Recovery
};

/** Verdict of the last resolved press (drives the HUD flash). */
UENUM(BlueprintType)
enum class EAetherbornComboTimingVerdict : uint8
{
	None,
	Start,
	Hit,
	Buffered,
	Early,
	Late,
	Max,
	Refused,
	Idle
};

/** Why a press was refused. */
UENUM(BlueprintType)
enum class EAetherbornComboTimingRefuse : uint8
{
	None,
	LightSwing,
	LightRecovery,
	HeavySwing,
	HeavyRecovery
};

/** Events emitted by a single Update(). */
UENUM(BlueprintType)
enum class EAetherbornComboTimingEventType : uint8
{
	Hit,
	HitStop,
	ChainEnd,
	Dropped
};

/** Read-only view of the combo machine, published once per frame. */
USTRUCT(BlueprintType)
struct FAetherbornComboTimingSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn|Combo")
	EAetherbornComboTimingKind Kind = EAetherbornComboTimingKind::Idle;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn|Combo")
	EAetherbornComboTimingPhase Phase = EAetherbornComboTimingPhase::Neutral;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn|Combo")
	EAetherbornComboTimingVerdict LastInput = EAetherbornComboTimingVerdict::None;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn|Combo")
	EAetherbornComboTimingRefuse RefuseReason = EAetherbornComboTimingRefuse::None;

	/** Light chain step 1..5 while a light is running, else 0. */
	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn|Combo")
	int32 Step = 0;

	/** Hero pose slot: 1..5 light, 3 heavy, 0 idle. */
	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn|Combo")
	int32 PoseStep = 0;

	/** Hits landed in the current chain. */
	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn|Combo")
	int32 Combo = 0;

	/** Best chain this session. */
	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn|Combo")
	int32 Best = 0;

	/** Bumps on every resolved press so the HUD flashes even on a repeated verdict. */
	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn|Combo")
	int32 LastInputSeq = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn|Combo")
	int32 ChainMax = AetherbornComboTiming::ChainMax;

	/** 0..1 inside the current phase. */
	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn|Combo")
	float PhaseT = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn|Combo")
	float AttackElapsed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn|Combo")
	float AttackDuration = AetherbornComboTiming::LightTotal;

	/** Seconds: the Eclipse Hit Window notify (damage frame). */
	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn|Combo")
	float HitAt = AetherbornComboTiming::LightHitAt;

	/** Seconds: the Eclipse Combo Window notify opens here. */
	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn|Combo")
	float RecoveryStart = AetherbornComboTiming::LightHitAt;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn|Combo")
	float RecoveryEnd = AetherbornComboTiming::LightTotal;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn|Combo")
	float WindowStart = AetherbornComboTiming::LightHitAt;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn|Combo")
	float WindowEnd = AetherbornComboTiming::LightHitAt;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn|Combo")
	float WindowDur = 0.f;

	/** 0..1 progress across the combo window, 0 when closed. */
	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn|Combo")
	float WindowT = 0.f;

	/** Seconds: the parry window ends (heavy only, else 0). */
	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn|Combo")
	float ParryEnd = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn|Combo")
	float HitStopDur = AetherbornComboTiming::LightHitStop;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn|Combo")
	float FreezeT = 0.f;

	/** Milliseconds since the last landed hit (or since the chain started). */
	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn|Combo")
	float SinceHitMs = 0.f;

	/** Damage of the running (or next) attack. */
	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn|Combo")
	float Damage = AetherbornComboTiming::LightDamageBase;

	/** The running attack has already dealt its damage. */
	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn|Combo")
	bool bHitLanded = false;

	/** The combo window is accepting the next press right now. */
	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn|Combo")
	bool bWindowOpen = false;

	/** The requested window does not fit the recovery at this speed. */
	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn|Combo")
	bool bWindowClamped = false;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn|Combo")
	bool bParryOpen = false;

	/** The sim is in hit-stop this instant. */
	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn|Combo")
	bool bFrozen = false;

	/** An early press is sitting in the buffer. */
	UPROPERTY(BlueprintReadOnly, Category = "Aetherborn|Combo")
	bool bQueued = false;
};

/** One thing that happened during an Update(). */
USTRUCT()
struct FAetherbornComboTimingEvent
{
	GENERATED_BODY()

	UPROPERTY()
	EAetherbornComboTimingEventType Type = EAetherbornComboTimingEventType::Hit;

	/** Light chain step 1..5, else 0. */
	UPROPERTY()
	int32 Step = 0;

	UPROPERTY()
	float Damage = 0.f;

	UPROPERTY()
	bool bHeavy = false;

	UPROPERTY()
	int32 Combo = 0;

	/** Hit-stop length in seconds (Type == HitStop). */
	UPROPERTY()
	float Duration = 0.f;
};

/** Broadcast when a swing's hit window lands. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAetherbornComboTimingHit, FVector, Location, float, Damage, bool, bHeavy);
