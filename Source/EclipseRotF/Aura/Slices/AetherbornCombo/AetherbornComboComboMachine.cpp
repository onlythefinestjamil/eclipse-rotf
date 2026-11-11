// Aetherborn combo: pure timing state machine (port of Saved/.Aura/Slices/aetherborn-combo/src/combo.js).

#include "AetherbornComboComboMachine.h"

DEFINE_LOG_CATEGORY(LogAetherbornCombo);

namespace AetherbornComboMachineLocal
{
	/** JS toDt(): non-positive deltas are ignored, everything else is capped at 1/30 s. */
	static float ToDeltaSeconds(float InDeltaSeconds)
	{
		if (!(InDeltaSeconds > 0.0f))
		{
			return 0.0f;
		}
		return FMath::Min(InDeltaSeconds, 1.0f / 30.0f);
	}

	/** Lower-case kind, matching the JS history payload the HUD chips read back. */
	static const TCHAR* AttemptKindText(EComboAttemptKind Kind)
	{
		switch (Kind)
		{
		case EComboAttemptKind::Hit:      return TEXT("hit");
		case EComboAttemptKind::Buffered: return TEXT("buffered");
		case EComboAttemptKind::Early:    return TEXT("early");
		case EComboAttemptKind::Late:     return TEXT("late");
		case EComboAttemptKind::Timeout:  return TEXT("timeout");
		default:                          return TEXT("unknown");
		}
	}

	static const TCHAR* PhaseText(EComboPhase InPhase)
	{
		switch (InPhase)
		{
		case EComboPhase::Idle:     return TEXT("idle");
		case EComboPhase::Windup:   return TEXT("windup");
		case EComboPhase::Active:   return TEXT("active");
		case EComboPhase::Recovery: return TEXT("recovery");
		default:                    return TEXT("unknown");
		}
	}

	/** Base duration constant of a phase, exactly as config.js TIMING is indexed. */
	static float BasePhaseDuration(EComboPhase InPhase)
	{
		switch (InPhase)
		{
		case EComboPhase::Windup:   return AetherbornComboTiming::Windup;
		case EComboPhase::Active:   return AetherbornComboTiming::Active;
		case EComboPhase::Recovery: return AetherbornComboTiming::Recovery;
		case EComboPhase::Idle:
		default:                    return 0.2f; // JS (TIMING[p] || 0.2)
		}
	}

	/** Damage for a chain step: DAMAGE[min(step, len) - 1] || DAMAGE[0]. */
	static float DamageForStep(int32 InStep)
	{
		const int32 Count = UE_ARRAY_COUNT(AetherbornComboTiming::Damage);
		const int32 Index = FMath::Min(InStep, Count) - 1;
		if (Index >= 0 && Index < Count)
		{
			const float Value = AetherbornComboTiming::Damage[Index];
			return Value != 0.0f ? Value : AetherbornComboTiming::Damage[0];
		}
		return AetherbornComboTiming::Damage[0];
	}
}

float UComboMachine::Speed() const
{
	// JS: Math.max(0.2, Number(settings.speedMult) || 1)
	const float Value = (Settings.SpeedMult != 0.0f) ? Settings.SpeedMult : 1.0f;
	return FMath::Max(0.2f, Value);
}

float UComboMachine::DurOf(EComboPhase InPhase) const
{
	return AetherbornComboMachineLocal::BasePhaseDuration(InPhase) / Speed();
}

float UComboMachine::WinLen() const
{
	// JS: Math.max(0.02, Number(settings.windowLen) || TIMING.window)
	const float Value = (Settings.WindowLen != 0.0f) ? Settings.WindowLen : AetherbornComboTiming::Window;
	return FMath::Max(0.02f, Value);
}

void UComboMachine::SetCue(const FString& Text, const FString& Kind)
{
	Cue = Text;
	CueKind = Kind.IsEmpty() ? TEXT("info") : Kind;
	CueT = AetherbornComboTiming::CueTime;
}

void UComboMachine::LogAttempt(EComboAttemptKind Kind, const FString& Label)
{
	FComboAttempt Attempt;
	Attempt.Step = Step;
	Attempt.Kind = Kind;
	Attempt.Label = Label;
	History.Add(Attempt);

	// History is capped, oldest entries leave first.
	if (History.Num() > AetherbornComboTiming::MaxHistory)
	{
		History.RemoveAt(0, History.Num() - AetherbornComboTiming::MaxHistory);
	}

	bRecorded = true;

	UE_LOG(LogAetherbornCombo, Log, TEXT("Attempt [%s] on step %d: %s (history %d/%d)"),
		AetherbornComboMachineLocal::AttemptKindText(Kind), Step, *Label, History.Num(), AetherbornComboTiming::MaxHistory);
}

void UComboMachine::StartSwing(int32 NewStep)
{
	Step = NewStep;
	Phase = EComboPhase::Windup;
	T = 0.0f;
	bWindowOpen = false;
	WindowT = 0.0f;
	bBuffered = false;
	bRecorded = false;
	bDropped = false;

	if (NewStep > Longest)
	{
		Longest = NewStep;
	}

	UE_LOG(LogAetherbornCombo, Log, TEXT("Swing %d start: windup %.3fs, active %.3fs, recovery %.3fs, window %.3fs (speed x%.2f)"),
		NewStep, DurOf(EComboPhase::Windup), DurOf(EComboPhase::Active), DurOf(EComboPhase::Recovery), WinLen(), Speed());
}

void UComboMachine::EndRecovery()
{
	bWindowOpen = false;
	WindowT = 0.0f;

	if (bBuffered)
	{
		bBuffered = false;

		if (Step < 3)
		{
			const int32 Next = Step + 1;
			StartSwing(Next);
			SetCue(TEXT("CHAINED"), TEXT("hit"));

			UE_LOG(LogAetherbornCombo, Log, TEXT("Chain: swing %d -> swing %d"), Next - 1, Next);
			OnChain.Broadcast(Next);
		}
		else
		{
			if (Longest < 3)
			{
				Longest = 3;
			}

			Step = 0;
			Phase = EComboPhase::Idle;
			T = 0.0f;
			bRecorded = false;
			bDropped = false;
			SetCue(TEXT("COMBO COMPLETE"), TEXT("hit"));

			UE_LOG(LogAetherbornCombo, Log, TEXT("Combo complete: 3/3 hits, longest %d"), Longest);
			OnChain.Broadcast(3);
		}

		return;
	}

	if (!bRecorded)
	{
		// Recovery ended with the window open and no press at all: one miss, once.
		LogAttempt(EComboAttemptKind::Timeout, TEXT("Missed window"));
		SetCue(TEXT("COMBO DROPPED"), TEXT("miss"));

		UE_LOG(LogAetherbornCombo, Log, TEXT("Drop: timeout (combo dropped on step %d)"), Step);
		OnDrop.Broadcast(TEXT("timeout"));
	}

	UE_LOG(LogAetherbornCombo, Log, TEXT("Recovery end on step %d -> idle (longest %d)"), Step, Longest);

	Step = 0;
	Phase = EComboPhase::Idle;
	T = 0.0f;
	bRecorded = false;
	bDropped = false;
}

void UComboMachine::Update(float DeltaSeconds, const FComboSettings& NewSettings)
{
	Settings = NewSettings;

	const float D = AetherbornComboMachineLocal::ToDeltaSeconds(DeltaSeconds);

	if (CueT > 0.0f)
	{
		CueT -= D;
		if (CueT <= 0.0f)
		{
			Cue.Reset();
			CueKind = TEXT("info");
		}
	}

	if (Phase == EComboPhase::Idle || Step == 0)
	{
		return;
	}

	if (bWindowOpen)
	{
		WindowT += D;
		if (WindowT >= WinLen())
		{
			bWindowOpen = false;
			LogAttempt(EComboAttemptKind::Timeout, TEXT("Missed window"));
			SetCue(TEXT("WINDOW CLOSED"), TEXT("miss"));

			UE_LOG(LogAetherbornCombo, Log, TEXT("Window closed without a press on step %d (drop: timeout)"), Step);
			OnDrop.Broadcast(TEXT("timeout"));
		}
	}

	T += D;

	// Guard matches the JS: at most six phase transitions per update.
	int32 Guard = 0;
	while (Guard++ < 6)
	{
		const float Dur = DurOf(Phase);
		if (T < Dur)
		{
			break;
		}

		T -= Dur;

		if (Phase == EComboPhase::Windup)
		{
			Phase = EComboPhase::Active;
			LastDamage = AetherbornComboMachineLocal::DamageForStep(Step);

			UE_LOG(LogAetherbornCombo, Log, TEXT("Step %d: windup -> active, hit for %.1f damage"), Step, LastDamage);
			OnHit.Broadcast(Step, LastDamage);
		}
		else if (Phase == EComboPhase::Active)
		{
			Phase = EComboPhase::Recovery;
			bWindowOpen = true;
			WindowT = 0.0f;
			bRecorded = false;
			SetCue(TEXT("COMBO WINDOW"), TEXT("window"));

			UE_LOG(LogAetherbornCombo, Log, TEXT("Step %d: active -> recovery, window open for %.3fs"), Step, WinLen());
		}
		else
		{
			UE_LOG(LogAetherbornCombo, Log, TEXT("Step %d: recovery finished"), Step);
			EndRecovery();
			break;
		}
	}
}

void UComboMachine::SetSettings(const FComboSettings& NewSettings)
{
	Settings = NewSettings;

	UE_LOG(LogAetherbornCombo, Verbose, TEXT("Settings: window %.3fs, speed x%.2f, early buffers %s"),
		Settings.WindowLen, Settings.SpeedMult, Settings.bEarlyBuffers ? TEXT("on") : TEXT("off"));
}

void UComboMachine::Press()
{
	UE_LOG(LogAetherbornCombo, Log, TEXT("Press in phase [%s] on step %d (buffered %s, window %s)"),
		AetherbornComboMachineLocal::PhaseText(Phase), Step, bBuffered ? TEXT("yes") : TEXT("no"), bWindowOpen ? TEXT("open") : TEXT("closed"));

	if (Phase == EComboPhase::Idle || Step == 0)
	{
		StartSwing(1);
		SetCue(TEXT("SWING 1"), TEXT("hit"));
		OnSwing.Broadcast(1);
		return;
	}

	if (Phase == EComboPhase::Windup || Phase == EComboPhase::Active)
	{
		// Too early.
		if (Settings.bEarlyBuffers)
		{
			if (!bBuffered)
			{
				bBuffered = true;
				LogAttempt(EComboAttemptKind::Buffered, TEXT("Buffered"));
			}
			SetCue(TEXT("BUFFERED"), TEXT("hit"));
		}
		else if (!bDropped)
		{
			bDropped = true;
			LogAttempt(EComboAttemptKind::Early, TEXT("Too early"));
			SetCue(TEXT("TOO EARLY"), TEXT("miss"));

			UE_LOG(LogAetherbornCombo, Log, TEXT("Drop: early (pressed during [%s] on step %d)"),
				AetherbornComboMachineLocal::PhaseText(Phase), Step);
			OnDrop.Broadcast(TEXT("early"));
		}
		else
		{
			SetCue(TEXT("TOO EARLY"), TEXT("miss"));
		}
		return;
	}

	// Recovery.
	if (bWindowOpen)
	{
		if (!bBuffered)
		{
			bBuffered = true;
			LogAttempt(EComboAttemptKind::Hit, TEXT("In window"));
		}

		// A successful press consumes the window, so no late timeout follows it.
		bWindowOpen = false;
		WindowT = 0.0f;
		SetCue(TEXT("CHAIN BUFFERED"), TEXT("hit"));
		UE_LOG(LogAetherbornCombo, Log, TEXT("Chain buffered on step %d (window consumed)"), Step);
		return;
	}

	// Recovery with the window already closed: too late, restart the chain.
	LogAttempt(EComboAttemptKind::Late, TEXT("Too late"));
	SetCue(TEXT("TOO LATE"), TEXT("miss"));

	UE_LOG(LogAetherbornCombo, Log, TEXT("Drop: late (window closed on step %d), restarting at swing 1"), Step);
	OnDrop.Broadcast(TEXT("late"));

	StartSwing(1);
	OnSwing.Broadcast(1);
}

void UComboMachine::Reset(bool bClearHistory)
{
	UE_LOG(LogAetherbornCombo, Log, TEXT("Reset from step %d (phase [%s], clear history %s)"),
		Step, AetherbornComboMachineLocal::PhaseText(Phase), bClearHistory ? TEXT("yes") : TEXT("no"));

	Step = 0;
	Phase = EComboPhase::Idle;
	T = 0.0f;
	bWindowOpen = false;
	WindowT = 0.0f;
	bBuffered = false;
	bRecorded = false;
	bDropped = false;
	Cue.Reset();
	CueKind = TEXT("info");
	CueT = 0.0f;
	LastDamage = 0.0f;

	if (bClearHistory)
	{
		History.Reset();
	}
}

FComboSnapshot UComboMachine::GetSnapshot() const
{
	FComboSnapshot Snapshot;

	const float Dur = DurOf(Phase);

	Snapshot.Step = Step;
	Snapshot.Phase = Phase;
	Snapshot.PhaseProgress = (Phase == EComboPhase::Idle || Dur <= 0.0f) ? 0.0f : FMath::Clamp(T / Dur, 0.0f, 1.0f);
	Snapshot.bWindowOpen = bWindowOpen;

	const float Win = WinLen();
	Snapshot.WindowProgress = (bWindowOpen && Win > 0.0f) ? FMath::Clamp(WindowT / Win, 0.0f, 1.0f) : -1.0f;

	Snapshot.bBuffered = bBuffered;
	Snapshot.Longest = Longest;
	Snapshot.History = History;
	Snapshot.Cue = Cue;
	Snapshot.CueKind = CueKind;
	Snapshot.LastDamage = LastDamage;

	return Snapshot;
}
