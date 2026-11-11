#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "EclipseCombatSystem.h"
#include "EclipseGameFeel.h"
#include "EclipseCombatComponent.generated.h"

class AEclipseCharacter;
class UAnimMontage;

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Idle,
    LightAttack,
    HeavyAttack,
    Dodging,
    Blocking,
    Casting,
    Stagger
};

/** Where the current swing is inside its montage. Driven by anim notifies, not by timers. */
UENUM(BlueprintType)
enum class EAttackPhase : uint8
{
    None,
    Windup,
    Active,
    Recovery
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEclipseHitSignature, float, Damage, bool, bCritical);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEclipseCastSignature, bool, bCompleted);

/** Owns the frame-critical attack / defense state machine for the playable Aetherborn. */
UCLASS(ClassGroup = (Eclipse), meta = (BlueprintSpawnableComponent))
class ECLIPSEROTF_API UEclipseCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEclipseCombatComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(BlueprintAssignable, Category = "Eclipse|Combat")
    FEclipseHitSignature OnHitLanded;

    UPROPERTY(BlueprintAssignable, Category = "Eclipse|Combat")
    FEclipseCastSignature OnCastFinished;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Combat")
    FGameFeelProfile Feel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Combat") float LightDamage = 40.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Combat") float HeavyDamage = 90.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Combat") float AttackRange = 300.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Combat") float AttackRadius = 110.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Combat") int32 MaxComboSteps = 5;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Combat") float BlockHoldThreshold = 0.25f;
    /** Damage at or above this breaks poise: the character plays a hit reaction and loses its action. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Combat") float StaggerDamageThreshold = 55.0f;
    /** Stagger lock length when no hit-reaction montage is assigned. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Combat") float StaggerFallbackSeconds = 0.35f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Combat") float ParryWindowSeconds = 0.10f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Combat") float HeavyRecoverySeconds = 0.75f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Combat") float DamageBuffMultiplier = 1.0f;

    // --- Animation montage layer. Assign montages on the BP CDO; an empty slot falls back to the fixed timers. ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Anim") bool bMontageDriven = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Anim") TArray<TObjectPtr<UAnimMontage>> LightAttackMontages;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Anim") TObjectPtr<UAnimMontage> HeavyAttackMontage = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Anim") TObjectPtr<UAnimMontage> DodgeMontage = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Anim") TObjectPtr<UAnimMontage> BlockMontage = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Anim") TObjectPtr<UAnimMontage> ParryMontage = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Anim") TObjectPtr<UAnimMontage> CastMontage = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eclipse|Anim") TArray<TObjectPtr<UAnimMontage>> HitReactionMontages;

    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Combat") ECombatState State = ECombatState::Idle;
    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Combat") int32 ComboIndex = 0;
    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Combat") bool bBlocking = false;
    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Combat") bool bDodgeIFrameActive = false;
    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Combat") bool bParryWindowOpen = false;
    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Combat") bool bGuardCounterOpen = false;
    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Combat") float HeavyRecoveryRemaining = 0.0f;
    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Combat") float DodgeCooldownRemaining = 0.0f;
    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Combat") float ComboWindowRemaining = 0.0f;

    /** Cast bar state. The bar, the montage and the gameplay payload all read these. */
    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Combat") bool bCastActive = false;
    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Combat") bool bCastInterrupted = false;
    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Combat") float CastDuration = 0.0f;
    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Combat") float CastElapsed = 0.0f;
    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Combat") float CastProgress = 0.0f;
    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Combat") FString LastReactionReason;

    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Combat") float LastDamageDealt = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Anim") EAttackPhase AttackPhase = EAttackPhase::None;
    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Anim") float MontageDuration = 0.0f;
    UPROPERTY(BlueprintReadOnly, Category = "Eclipse|Anim") FString ActiveMontageName;

    UFUNCTION(BlueprintCallable, Category = "Eclipse|Combat") bool InputLightAttack();
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Combat") void InputHeavyPressed();
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Combat") void InputHeavyReleased();
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Combat") bool InputDodge(const FVector2D& MoveAxis);
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Combat") float ReceiveDamage(float InDamage, AActor* Source);
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Combat") bool IsActing() const;
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Combat") void SetDamageBuff(float Multiplier, float DurationSeconds);
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Combat") void ResetParryWindow();

    /** Begin an interruptible cast. Refused while already acting. The cast bar is authoritative: it
     *  completes on CastDuration regardless of montage length, and any incoming damage cancels it. */
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Combat") bool InputCast(float InCastSeconds = 0.8f);

    /** Play a stagger reaction. Returns false when the hit was absorbed (i-frames, guard, committed swing). */
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Combat") bool PlayHitReaction(float InDamage, const FVector& SourceLocation);


    /** Called by the HitWindow anim notify: the swing's single damage sweep. */
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Anim") void OpenHitWindow();
    /** Called by the ComboWindow anim notify: opens the chain window for the next swing. */
    UFUNCTION(BlueprintCallable, Category = "Eclipse|Anim") void OpenComboWindow();
    UFUNCTION(BlueprintPure, Category = "Eclipse|Anim") UAnimInstance* GetOwnerAnimInstance() const;

private:
    void PerformSweep(float DamageAmount, const FString& TypeLabel);
    void DoHitStop(float Seconds);
    void EndHitStop();
    void CloseGuardCounter();
    void FinishCast(bool bCompleted);
    void StartHeavyAttack();
    AEclipseCharacter* GetOwnerChar() const;
    UAnimMontage* PickLightAttackMontage() const;
    bool PlayCombatMontage(UAnimMontage* Montage, float Damage, const FString& TypeLabel);
    void OnCombatMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    UPROPERTY()
    UEclipseCombatSystem* CombatSystem = nullptr;

    float AttackTimer = 0.0f;
    UPROPERTY() TObjectPtr<UAnimMontage> CurrentMontage = nullptr;
    float PendingSweepDamage = 0.0f;
    FString PendingSweepLabel;
    bool bSwingSwept = false;
    bool bMontageActive = false;
    float DodgeElapsed = 0.0f;
    float ParryTimer = 0.0f;
    float StaggerTimer = 0.0f;
    bool bStaggerFromMontage = false;
    float RMBHoldTimer = 0.0f;
    float BuffRemaining = 0.0f;
    bool bRMBHeld = false;
    FTimerHandle GuardCounterHandle;
};
