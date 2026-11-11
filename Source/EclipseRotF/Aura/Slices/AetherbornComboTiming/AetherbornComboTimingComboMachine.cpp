// AetherbornComboTiming — the combat / timing simulation (port of src/combo.js).
#include "AetherbornComboTimingComboMachine.h"

DEFINE_LOG_CATEGORY(LogAetherbornComboTiming);

namespace ACT = AetherbornComboTiming;

namespace
{
	/** Guard against a stalled frame handing in a multi-second dt. */
	constexpr float MaxDeltaSeconds = 1.0f;
	constexpr float TimingEps = 1e-6f;
	/** Speed is floored so no timing ever divides by zero (contracts.speedOf). */
	constexpr float SpeedFloor = 0.1f;

	/** Every timing of one attack, in seconds from its press (speed already applied). */
	struct FAttackTiming
	{
		EAetherbornComboTimingKind Kind = EAetherbornComboTimingKind::Light;
		int32 Step = 1;
		float Damage = 0.f;
		float Speed = 1.f;
		float Dur = 0.f;
		float WindupEnd = 0.f;
		float StrikeEnd = 0.f;
		float HitAt = 0.f;
		float RecoveryStart = 0.f;
		float RecoveryEnd = 0.f;
		float RecoveryLength = 0.f;
		float WindowRequested = 0.f;
		float WindowDur = 0.f;
		bool bWindowClamped = false;
		float WindowOpen = 0.f;
		float WindowClose = 0.f;
		float HitStop = 0.f;
		float ParryEnd = 0.f;
	};

	/** Playback speed multiplier (never below SpeedFloor). */
	float SpeedOf(const FAetherbornComboTimingTuning& Tuning)
	{
		const float Raw = FMath::IsFinite(Tuning.SpeedMul) ? Tuning.SpeedMul : 1.f;
		return FMath::Max(SpeedFloor, Raw);
	}

	/** Combo-window DURATION asked for by the tuning, clamped to the slider range. */
	float WindowRequestedOf(const FAetherbornComboTimingTuning& Tuning)
	{
		const float Raw = FMath::IsFinite(Tuning.ComboWindow) ? Tuning.ComboWindow : 0.28f;
		return FMath::Clamp(Raw, ACT::ComboWindowMin, ACT::ComboWindowMax);
	}

	int32 ClampStep(int32 Step)
	{
		return FMath::Clamp(Step, 1, ACT::ChainMax);
	}

	/** Per-step light damage: 40 / 44 / 48 / 52 / 56. */
	float LightDamageFor(int32 Step)
	{
		const float S = static_cast<float>(ClampStep(Step));
		const float Raw = ACT::LightDamageBase * (1.f + ACT::LightDamageStepPct * (S - 1.f));
		return static_cast<float>(FMath::RoundToInt(Raw));
	}

	/**
	 * Light chain step 1..5. Everything is divided by the speed except the hit-stop
	 * (a fixed global pause) and the window DURATION (absolute seconds). The window
	 * opens at the hit and its length extends the recovery, so the band always tracks
	 * the slider.
	 */
	FAttackTiming MakeLightTiming(int32 Step, const FAetherbornComboTimingTuning& Tuning)
	{
		FAttackTiming T;
		const int32 S = ClampStep(Step);
		const float Speed = SpeedOf(Tuning);
		const float BaseRecovery = ACT::LightRecovery / Speed;
		const float Requested = WindowRequestedOf(Tuning);
		const float RecoveryLength = FMath::Max(BaseRecovery, Requested);

		T.Kind = EAetherbornComboTimingKind::Light;
		T.Step = S;
		T.Damage = LightDamageFor(S);
		T.Speed = Speed;
		T.WindupEnd = ACT::LightWindup / Speed;
		T.HitAt = ACT::LightHitAt / Speed;
		T.StrikeEnd = T.HitAt;
		T.RecoveryStart = T.HitAt;
		T.Dur = T.HitAt + RecoveryLength;
		T.RecoveryEnd = T.Dur;
		T.RecoveryLength = RecoveryLength;
		T.WindowRequested = Requested;
		T.WindowDur = Requested;
		T.bWindowClamped = Requested > BaseRecovery + TimingEps;
		T.WindowOpen = T.HitAt;
		T.WindowClose = T.HitAt + Requested;
		T.HitStop = ACT::LightHitStop;
		T.ParryEnd = 0.f;
		return T;
	}

	/**
	 * The heavy cleave: 0.48 s windup + 0.12 s strike (hit at 0.60) + 0.75 s recovery.
	 * The hit is a cancel point: the same combo window as the light opens there, so a
	 * light pressed inside it drops the heavy's remaining recovery and starts a fresh
	 * light chain (heavy -> light). The 0.10 s parry window still starts at the press.
	 */
	FAttackTiming MakeHeavyTiming(const FAetherbornComboTimingTuning& Tuning)
	{
		FAttackTiming T;
		const float Speed = SpeedOf(Tuning);
		const float BaseRecovery = ACT::HeavyRecovery / Speed;
		const float Requested = WindowRequestedOf(Tuning);
		const float RecoveryLength = FMath::Max(BaseRecovery, Requested);

		T.Kind = EAetherbornComboTimingKind::Heavy;
		T.Step = 0;
		T.Damage = ACT::HeavyDamage;
		T.Speed = Speed;
		T.WindupEnd = ACT::HeavyWindup / Speed;
		T.HitAt = ACT::HeavyHitAt / Speed;
		T.StrikeEnd = T.HitAt;
		T.RecoveryStart = T.HitAt;
		T.Dur = T.HitAt + RecoveryLength;
		T.RecoveryEnd = T.Dur;
		T.RecoveryLength = RecoveryLength;
		T.WindowRequested = Requested;
		T.WindowDur = Requested;
		T.bWindowClamped = Requested > BaseRecovery + TimingEps;
		T.WindowOpen = T.HitAt;
		T.WindowClose = T.HitAt + Requested;
		T.HitStop = ACT::HeavyHitStop;
		T.ParryEnd = ACT::ParryWindow / Speed;
		return T;
	}

	/** Timing of any attack; the idle machine reports the first light swing. */
	FAttackTiming MakeTiming(EAetherbornComboTimingKind Kind, int32 Step, const FAetherbornComboTimingTuning& Tuning)
	{
		if (Kind == EAetherbornComboTimingKind::Heavy)
		{
			return MakeHeavyTiming(Tuning);
		}

		const int32 ShownStep = (Kind == EAetherbornComboTimingKind::Light) ? Step : 1;
		return MakeLightTiming(ShownStep, Tuning);
	}

	/** Hero pose slot: 1..5 for light steps, 3 for the heavy cleave, 0 idle. */
	int32 PoseStepFor(EAetherbornComboTimingKind Kind, int32 Step)
	{
		if (Kind == EAetherbornComboTimingKind::Heavy)
		{
			return 3;
		}
		if (Kind == EAetherbornComboTimingKind::Light)
		{
			return ClampStep(Step);
		}
		return 0;
	}

	/** Phase of the attack Sec seconds into it. */
	void PhaseAt(const FAttackTiming& T, float Sec, EAetherbornComboTimingPhase& OutPhase, float& OutPhaseT)
	{
		const float S = FMath::Clamp(Sec, 0.f, T.Dur);
		if (S < T.WindupEnd)
		{
			OutPhase = EAetherbornComboTimingPhase::Windup;
			OutPhaseT = (T.WindupEnd > 0.f) ? (S / T.WindupEnd) : 1.f;
			return;
		}
		if (S < T.StrikeEnd)
		{
			const float Span = FMath::Max(TimingEps, T.StrikeEnd - T.WindupEnd);
			OutPhase = EAetherbornComboTimingPhase::Strike;
			OutPhaseT = (S - T.WindupEnd) / Span;
			return;
		}
		const float Span = FMath::Max(TimingEps, T.Dur - T.StrikeEnd);
		OutPhase = EAetherbornComboTimingPhase::Recovery;
		OutPhaseT = (S - T.StrikeEnd) / Span;
	}
}

float UAetherbornComboTimingComboMachine::LightDamageForStep(int32 Step)
{
	return LightDamageFor(Step);
}

void UAetherbornComboTimingComboMachine::SetInput(EAetherbornComboTimingVerdict Verdict, EAetherbornComboTimingRefuse Reason)
{
	// Every resolved press bumps the sequence so the HUD flashes on each one, even
	// when the same verdict repeats (a five-hit chain is five Hit verdicts).
	LastInput = Verdict;
	++LastInputSeq;
	RefuseReason = (Verdict == EAetherbornComboTimingVerdict::Refused) ? Reason : EAetherbornComboTimingRefuse::None;
}

void UAetherbornComboTimingComboMachine::StartLight(int32 NextStep)
{
	Kind = EAetherbornComboTimingKind::Light;
	Step = ClampStep(NextStep);
	Elapsed = 0.f;
	bLanded = false;
	bQueued = false;
}

void UAetherbornComboTimingComboMachine::StartChain()
{
	Combo = 0;
	LastHitClock = Clock;
	StartLight(1);
}

void UAetherbornComboTimingComboMachine::StartHeavy()
{
	Combo = 0;
	LastHitClock = Clock;
	Kind = EAetherbornComboTimingKind::Heavy;
	Step = 0;
	Elapsed = 0.f;
	bLanded = false;
	bQueued = false;
}

void UAetherbornComboTimingComboMachine::Land()
{
	// Only the Eclipse Hit Window frame deals damage, and it starts the hit-stop.
	if (Kind == EAetherbornComboTimingKind::Idle || bLanded)
	{
		return;
	}

	const FAttackTiming T = MakeTiming(Kind, Step, LastTuning);
	const bool bHeavy = (Kind == EAetherbornComboTimingKind::Heavy);

	bLanded = true;
	++Combo;
	Best = FMath::Max(Best, Combo);
	LastHitClock = Clock;

	FAetherbornComboTimingEvent HitEvent;
	HitEvent.Type = EAetherbornComboTimingEventType::Hit;
	HitEvent.Step = bHeavy ? 0 : Step;
	HitEvent.Damage = T.Damage;
	HitEvent.bHeavy = bHeavy;
	HitEvent.Combo = Combo;
	HitEvent.Duration = 0.f;
	Events.Add(HitEvent);

	FreezeT = T.HitStop;
	FreezeDur = T.HitStop;

	FAetherbornComboTimingEvent StopEvent;
	StopEvent.Type = EAetherbornComboTimingEventType::HitStop;
	StopEvent.bHeavy = bHeavy;
	StopEvent.Combo = Combo;
	StopEvent.Duration = T.HitStop;
	Events.Add(StopEvent);
}

void UAetherbornComboTimingComboMachine::EndAttack()
{
	const bool bHadHits = (Combo > 0);
	const int32 EndedCombo = Combo;
	const bool bWasHeavy = (Kind == EAetherbornComboTimingKind::Heavy);

	Kind = EAetherbornComboTimingKind::Idle;
	Step = 1;
	Elapsed = 0.f;
	bQueued = false;
	bQueuedHeavy = false;
	bLanded = false;
	FreezeT = 0.f;

	FAetherbornComboTimingEvent Event;
	Event.Type = bHadHits ? EAetherbornComboTimingEventType::ChainEnd : EAetherbornComboTimingEventType::Dropped;
	Event.Step = 0;
	Event.Damage = 0.f;
	Event.bHeavy = bWasHeavy;
	Event.Combo = bHadHits ? EndedCombo : 0;
	Event.Duration = 0.f;
	Events.Add(Event);

	Combo = 0; // Best is kept
}

void UAetherbornComboTimingComboMachine::Update(float DeltaSeconds, bool bLightPress, bool bHeavyPress, const FAetherbornComboTimingTuning& Tuning)
{
	Events.Reset();
	LastTuning = Tuning;

	const float D = (FMath::IsFinite(DeltaSeconds) && DeltaSeconds > 0.f) ? FMath::Min(DeltaSeconds, MaxDeltaSeconds) : 0.f;
	Clock += D;

	// 1. hit-stop: the whole sim (playhead included) stands still. Incoming time is
	//    burned by the freeze first; if any freeze is left the frame resolves nothing,
	//    but the press is stashed so a hit-stop never swallows input aimed at a window.
	float DSim = D;
	if (FreezeT > 0.f)
	{
		const float Burn = FMath::Min(FreezeT, DSim);
		FreezeT -= Burn;
		DSim -= Burn;
		if (FreezeT < 1e-6f)
		{
			FreezeT = 0.f;
		}
		if (FreezeT > 0.f)
		{
			bStashedLight = bStashedLight || bLightPress;
			bStashedHeavy = bStashedHeavy || bHeavyPress;
			return;
		}
	}

	if (Kind != EAetherbornComboTimingKind::Idle)
	{
		Elapsed += DSim;
	}

	const bool bLight = bLightPress || bStashedLight;
	const bool bHeavy = bHeavyPress || bStashedHeavy;
	bStashedLight = false;
	bStashedHeavy = false;

	FAttackTiming T = MakeTiming(Kind, Step, Tuning);

	// 2. resolve the presses against the notify markers.
	if (Kind == EAetherbornComboTimingKind::Idle)
	{
		if (bLight)
		{
			StartChain();
			SetInput(EAetherbornComboTimingVerdict::Start);
		}
		else if (bHeavy)
		{
			StartHeavy();
			SetInput(EAetherbornComboTimingVerdict::Start);
		}
		T = MakeTiming(Kind, Step, Tuning);
	}
	else if (Kind == EAetherbornComboTimingKind::Light)
	{
		if (bLight)
		{
			if (Elapsed < T.WindowOpen)
			{
				// Before the Eclipse Combo Window: no damage, no chain — it only sits
				// in the buffer when buffering is enabled.
				if (Tuning.bBuffer)
				{
					bQueued = true;
					SetInput(EAetherbornComboTimingVerdict::Buffered);
				}
				else
				{
					SetInput(EAetherbornComboTimingVerdict::Early);
				}
			}
			else if (Elapsed <= T.WindowClose + TimingEps)
			{
				if (Step >= ACT::ChainMax)
				{
					SetInput(EAetherbornComboTimingVerdict::Max);
				}
				else
				{
					Land(); // the outgoing swing finishes its own hit window first
					StartLight(Step + 1);
					SetInput(EAetherbornComboTimingVerdict::Hit);
					T = MakeTiming(Kind, Step, Tuning);
				}
			}
			else
			{
				SetInput(EAetherbornComboTimingVerdict::Late);
			}
		}
		else if (bHeavy)
		{
			// A heavy pressed inside a light swing is a real cancel, not a refusal:
			// before the window opens it buffers, inside the window the light lands
			// its hit and the heavy finisher starts, after the window it is late.
			if (Elapsed < T.WindowOpen)
			{
				if (Tuning.bBuffer)
				{
					bQueued = true;
					bQueuedHeavy = true;
					SetInput(EAetherbornComboTimingVerdict::Buffered);
				}
				else
				{
					SetInput(EAetherbornComboTimingVerdict::Early);
				}
			}
			else if (Elapsed <= T.WindowClose + TimingEps)
			{
				Land();
				StartHeavy();
				SetInput(EAetherbornComboTimingVerdict::Hit);
				T = MakeTiming(Kind, Step, Tuning);
			}
			else
			{
				SetInput(EAetherbornComboTimingVerdict::Late);
			}
		}
	}
	else
	{
		// The heavy cleave is committed, but its hit window is still a cancel point:
		// a light pressed inside it drops into a fresh light chain (heavy -> light).
		// A second heavy never interrupts the first, and past the window the
		// recovery is locked.
		if (bHeavy)
		{
			SetInput(EAetherbornComboTimingVerdict::Refused,
				Elapsed < T.RecoveryStart ? EAetherbornComboTimingRefuse::HeavySwing : EAetherbornComboTimingRefuse::HeavyRecovery);
		}
		else if (bLight)
		{
			if (Elapsed < T.WindowOpen)
			{
				if (Tuning.bBuffer)
				{
					bQueued = true;
					bQueuedHeavy = false;
					SetInput(EAetherbornComboTimingVerdict::Buffered);
				}
				else
				{
					SetInput(EAetherbornComboTimingVerdict::Early);
				}
			}
			else if (Elapsed <= T.WindowClose + TimingEps)
			{
				Land();
				StartLight(1);
				SetInput(EAetherbornComboTimingVerdict::Hit);
				T = MakeTiming(Kind, Step, Tuning);
			}
			else
			{
				SetInput(EAetherbornComboTimingVerdict::Refused, EAetherbornComboTimingRefuse::HeavyRecovery);
			}
		}
	}

	// 3. a buffered press fires the moment the combo window opens.
	if (bQueued && Kind != EAetherbornComboTimingKind::Idle)
	{
		T = MakeTiming(Kind, Step, Tuning);
		const bool bWantHeavy = bQueuedHeavy;
		if (!bWantHeavy && Kind == EAetherbornComboTimingKind::Light && Step >= ACT::ChainMax)
		{
			bQueued = false;
			bQueuedHeavy = false;
		}
		else if (Elapsed >= T.WindowOpen)
		{
			const bool bFromHeavy = (Kind == EAetherbornComboTimingKind::Heavy);
			bQueued = false;
			bQueuedHeavy = false;
			Land();
			if (bWantHeavy)
			{
				StartHeavy();
			}
			else
			{
				StartLight(bFromHeavy ? 1 : Step + 1);
			}
			SetInput(EAetherbornComboTimingVerdict::Hit);
			T = MakeTiming(Kind, Step, Tuning);
		}
	}

	// 4. damage only on the Eclipse Hit Window notify.
	if (Kind != EAetherbornComboTimingKind::Idle && !bLanded)
	{
		T = MakeTiming(Kind, Step, Tuning);
		if (Elapsed >= T.HitAt)
		{
			Land();
		}
	}

	// 5. the attack ends.
	if (Kind != EAetherbornComboTimingKind::Idle)
	{
		T = MakeTiming(Kind, Step, Tuning);
		if (Elapsed >= T.Dur)
		{
			EndAttack();
		}
	}
}

void UAetherbornComboTimingComboMachine::Reset()
{
	Kind = EAetherbornComboTimingKind::Idle;
	Step = 1;
	Elapsed = 0.f;
	Combo = 0;
	Best = 0;
	bQueued = false;
	bQueuedHeavy = false;
	bLanded = false;
	FreezeT = 0.f;
	FreezeDur = 0.f;
	bStashedLight = false;
	bStashedHeavy = false;
	Clock = 0.f;
	LastHitClock = 0.f;
	LastInput = EAetherbornComboTimingVerdict::None;
	RefuseReason = EAetherbornComboTimingRefuse::None;
	++LastInputSeq;
	Events.Reset();

	RebuildSnapshot(LastTuning);
}

const FAetherbornComboTimingSnapshot& UAetherbornComboTimingComboMachine::GetSnapshot() const
{
	RebuildSnapshot(LastTuning);
	return CachedSnapshot;
}

const TArray<FAetherbornComboTimingEvent>& UAetherbornComboTimingComboMachine::GetEvents() const
{
	return Events;
}

void UAetherbornComboTimingComboMachine::RebuildSnapshot(const FAetherbornComboTimingTuning& Tuning) const
{
	FAetherbornComboTimingSnapshot& S = CachedSnapshot;

	const bool bAttacking = (Kind != EAetherbornComboTimingKind::Idle);
	const FAttackTiming T = MakeTiming(Kind, Step, Tuning);

	S.Kind = Kind;

	EAetherbornComboTimingPhase Phase = EAetherbornComboTimingPhase::Neutral;
	float PhaseT = 0.f;
	if (bAttacking)
	{
		PhaseAt(T, Elapsed, Phase, PhaseT);
	}
	S.Phase = Phase;
	S.PhaseT = PhaseT;

	S.LastInput = LastInput;
	S.RefuseReason = RefuseReason;
	S.Step = (Kind == EAetherbornComboTimingKind::Light) ? Step : 0;
	S.PoseStep = bAttacking ? PoseStepFor(Kind, Step) : 0;
	S.Combo = Combo;
	S.Best = Best;
	S.LastInputSeq = LastInputSeq;
	S.ChainMax = ACT::ChainMax;

	S.AttackElapsed = bAttacking ? Elapsed : 0.f;
	S.AttackDuration = T.Dur;
	S.HitAt = T.HitAt;
	S.RecoveryStart = T.RecoveryStart;
	S.RecoveryEnd = T.RecoveryEnd;

	const bool bWindowOpen = (Kind != EAetherbornComboTimingKind::Idle) && (Elapsed >= T.WindowOpen) && (Elapsed <= T.WindowClose);
	const float Span = FMath::Max(TimingEps, T.WindowClose - T.WindowOpen);

	S.WindowStart = T.WindowOpen;
	S.WindowEnd = T.WindowClose;
	S.WindowDur = T.WindowDur;
	S.bWindowClamped = T.bWindowClamped;
	S.bWindowOpen = bWindowOpen;
	S.WindowT = bWindowOpen ? FMath::Clamp((Elapsed - T.WindowOpen) / Span, 0.f, 1.f) : 0.f;

	S.ParryEnd = (Kind == EAetherbornComboTimingKind::Heavy) ? T.ParryEnd : 0.f;
	S.bParryOpen = (Kind == EAetherbornComboTimingKind::Heavy) && (Elapsed <= T.ParryEnd + TimingEps);

	S.HitStopDur = T.HitStop;
	S.bFrozen = (FreezeT > 0.f);
	S.FreezeT = FreezeT;
	S.SinceHitMs = FMath::Max(0.f, (Clock - LastHitClock) * 1000.f);

	S.Damage = T.Damage;
	S.bHitLanded = bAttacking ? bLanded : false;
	S.bQueued = bQueued;
}

FAetherbornComboTimingSnapshot UAetherbornComboTimingComboMachine::MakeIdlePreview(const FAetherbornComboTimingTuning& Tuning)
{
	// Built from the tuning alone, so it is safe before the first Update().
	FAetherbornComboTimingSnapshot S;
	const FAttackTiming T = MakeLightTiming(1, Tuning);

	S.Kind = EAetherbornComboTimingKind::Idle;
	S.Phase = EAetherbornComboTimingPhase::Neutral;
	S.LastInput = EAetherbornComboTimingVerdict::None;
	S.RefuseReason = EAetherbornComboTimingRefuse::None;
	S.Step = 0;
	S.PoseStep = 0;
	S.Combo = 0;
	S.Best = 0;
	S.LastInputSeq = 0;
	S.ChainMax = ACT::ChainMax;

	S.PhaseT = 0.f;
	S.AttackElapsed = 0.f;
	S.AttackDuration = T.Dur;
	S.HitAt = T.HitAt;
	S.RecoveryStart = T.RecoveryStart;
	S.RecoveryEnd = T.RecoveryEnd;
	S.WindowStart = T.WindowOpen;
	S.WindowEnd = T.WindowClose;
	S.WindowDur = T.WindowDur;
	S.WindowT = 0.f;
	S.ParryEnd = 0.f;
	S.HitStopDur = T.HitStop;
	S.FreezeT = 0.f;
	S.SinceHitMs = 0.f;
	S.Damage = T.Damage;

	S.bHitLanded = false;
	S.bWindowOpen = false;
	S.bWindowClamped = T.bWindowClamped;
	S.bParryOpen = false;
	S.bFrozen = false;
	S.bQueued = false;

	return S;
}
