#include "EclipseCharacter.h"

#include "TimerManager.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "UObject/ConstructorHelpers.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"

#include "EclipseCombatComponent.h"
#include "EclipseAbilityComponent.h"
#include "EclipsePerfectDodgeComponent.h"
#include "Engine/GameInstance.h"
#include "FrontEnd/EclipseFrontEndSubsystem.h"
#include "AbilitySystemComponent.h"
#include "EclipseAttributeSet.h"
#include "EclipseEquipmentComponent.h"
#include "EclipseEquipmentTypes.h"

AEclipseCharacter::AEclipseCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    GlobalCooldownDuration = 1.0f;

    GetCapsuleComponent()->InitCapsuleSize(42.0f, 88.0f);

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    UCharacterMovementComponent* Move = GetCharacterMovement();
    Move->bOrientRotationToMovement = true;
    Move->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    Move->JumpZVelocity = 520.0f;
    Move->AirControl = 0.35f;
    Move->MaxWalkSpeed = WalkSpeed;
    Move->BrakingDecelerationWalking = 2048.0f;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(GetCapsuleComponent());
    CameraBoom->TargetArmLength = 340.0f;
    CameraBoom->SocketOffset = FVector(0.0f, 40.0f, 70.0f);
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bDoCollisionTest = true;
    CameraBoom->ProbeSize = 12.0f;

    TPPCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TPP_Camera"));
    TPPCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    TPPCamera->bUsePawnControlRotation = false;
    TPPCamera->FieldOfView = 90.0f;

    HeroMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeroMesh"));
    HeroMesh->SetupAttachment(GetCapsuleComponent());
    HeroMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    HeroMesh->SetGenerateOverlapEvents(false);
    HeroMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> HeroFinder(TEXT("/Game/Characters/AETHERBORN_HERO_ALT_AAA/StaticMeshes/AETHERBORN_HERO_AAA.AETHERBORN_HERO_AAA"));
    if (HeroFinder.Succeeded())
    {
        HeroMesh->SetStaticMesh(HeroFinder.Object);
    }

    WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
    WeaponMesh->SetupAttachment(GetMesh(), WeaponAttachSocket);
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponMesh->SetGenerateOverlapEvents(false);
    WeaponMesh->SetRelativeLocation(WeaponAttachOffset);
    WeaponMesh->SetRelativeRotation(WeaponAttachRotation);
    WeaponMesh->SetRelativeScale3D(WeaponAttachScale);

    CombatComponent = CreateDefaultSubobject<UEclipseCombatComponent>(TEXT("CombatComponent"));
    AbilityComponent = CreateDefaultSubobject<UEclipseAbilityComponent>(TEXT("AbilityComponent"));
    PerfectDodgeComponent = CreateDefaultSubobject<UEclipsePerfectDodgeComponent>(TEXT("PerfectDodgeComponent"));

    AbilitySystem = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));
    EquipmentAttributes = CreateDefaultSubobject<UEclipseAttributeSet>(TEXT("EquipmentAttributes"));
    EquipmentComponent = CreateDefaultSubobject<UEclipseEquipmentComponent>(TEXT("EquipmentComponent"));
}

void AEclipseCharacter::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    if (AbilitySystem)
    {
        AbilitySystem->InitAbilityActorInfo(this, this);
        UE_LOG(LogTemp, Log, TEXT("[GAS] %s ASC initialised (equipment attributes %s)"), *GetName(), EquipmentAttributes ? TEXT("registered") : TEXT("missing"));
    }
}

void AEclipseCharacter::SetWeaponMesh(UStaticMesh* InWeaponMesh)
{
    if (!WeaponMesh)
    {
        return;
    }

    WeaponMesh->SetStaticMesh(InWeaponMesh);
    WeaponMesh->SetVisibility(InWeaponMesh != nullptr, true);
    WeaponMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocket);
    WeaponMesh->SetRelativeLocation(WeaponAttachOffset);
    WeaponMesh->SetRelativeRotation(WeaponAttachRotation);
    WeaponMesh->SetRelativeScale3D(WeaponAttachScale);

    UE_LOG(LogTemp, Log, TEXT("[WEAPON] %s socketed to %s"),
        InWeaponMesh ? *InWeaponMesh->GetName() : TEXT("none"), *WeaponAttachSocket.ToString());
}

void AEclipseCharacter::BeginPlay()
{
    Super::BeginPlay();

    EnsureInputAssets();
    AddDefaultMappingContext();
    CreateAbilityBarWidget();

    // Front end: the player's pawn takes the identity of the character chosen on the character select screen.
    // Next tick, because the default pawn is possessed after its BeginPlay.
    GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
    {
        if (!IsPlayerControlled())
        {
            return;
        }
        const UGameInstance* GI = GetGameInstance();
        const UEclipseFrontEndSubsystem* FrontEnd = GI ? GI->GetSubsystem<UEclipseFrontEndSubsystem>() : nullptr;
        FEclipseCharacterSlot ActiveSlot;
        if (FrontEnd && FrontEnd->GetActiveCharacter(ActiveSlot))
        {
            Ancestry = ActiveSlot.Ancestry;
            CharacterLevel = ActiveSlot.Level;
            UE_LOG(LogEclipseFrontEnd, Log, TEXT("[FrontEnd] Pawn %s took identity '%s' (%s, Lv %d)"), *GetName(),
                *ActiveSlot.CharacterName, *StaticEnum<EAncestry>()->GetNameStringByValue(static_cast<int64>(Ancestry)), CharacterLevel);
        }
    }));

    if (CombatComponent)
    {
        CombatComponent->OnCastFinished.AddDynamic(this, &AEclipseCharacter::OnCombatCastFinished);
    }

    // Fit the imported AAA mesh to the capsule height so scale mismatches from GLB import do not matter.
    if (HeroMesh && HeroMesh->GetStaticMesh())
    {
        const FBoxSphereBounds Bounds = HeroMesh->GetStaticMesh()->GetBounds();
        const float MeshHeight = Bounds.BoxExtent.Z * 2.0f;
        if (MeshHeight > 1.0f)
        {
            const float UniformScale = 176.0f / MeshHeight;
            HeroMesh->SetRelativeScale3D(FVector(UniformScale));
            HeroMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
            UE_LOG(LogTemp, Log, TEXT("[VISUAL] Hero mesh fitted: raw height %.1f -> scale %.4f"), MeshHeight, UniformScale);
        }
    }

    // The static placeholder body only stands down once a rigged skeletal mesh is actually assigned to Mesh.
    const USkeletalMeshComponent* SkelMesh = GetMesh();
    const bool bHasSkeletalBody = (SkelMesh && SkelMesh->GetSkeletalMeshAsset() != nullptr);
    if (HeroMesh)
    {
        HeroMesh->SetVisibility(!bHasSkeletalBody, true);
        HeroMesh->SetHiddenInGame(bHasSkeletalBody, true);
    }
    UE_LOG(LogTemp, Log, TEXT("[ANIM] %s"), bHasSkeletalBody ? TEXT("skeletal hero") : TEXT("static placeholder"));

    // The CDO is constructed before a child Blueprint assigns its skeletal mesh, so re-bind the
    // weapon against the now-loaded mesh. Attaching to a bone name needs no authored socket.
    if (WeaponMesh)
    {
        WeaponMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocket);
        WeaponMesh->SetRelativeLocation(WeaponAttachOffset);
        WeaponMesh->SetRelativeRotation(WeaponAttachRotation);
        WeaponMesh->SetRelativeScale3D(WeaponAttachScale);
        UE_LOG(LogTemp, Log, TEXT("[WEAPON] %s on %s"),
            WeaponMesh->GetStaticMesh() ? *WeaponMesh->GetStaticMesh()->GetName() : TEXT("empty"),
            *WeaponAttachSocket.ToString());
    }

    UE_LOG(LogTemp, Log, TEXT("[RESOURCE] Start Mana=%.0f Stamina=%.0f Rage=%.0f Aether=%.0f | Ancestry=%d Class=%d"),
        Mana, Stamina, Rage, Aether, (int32)Ancestry, (int32)CharacterClass);
}

void AEclipseCharacter::CreateAbilityBarWidget()
{
    if (AbilityBarWidget || !IsLocallyControlled())
    {
        return;
    }

    TSubclassOf<UUserWidget> BarClass = AbilityBarWidgetClass;
    if (!BarClass)
    {
        BarClass = LoadClass<UUserWidget>(nullptr, TEXT("/Game/UI/WBP_AbilityBar.WBP_AbilityBar_C"));
    }
    if (!BarClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("[HUD] Ability bar class not found - /Game/UI/WBP_AbilityBar missing"));
        return;
    }

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC && GetWorld())
    {
        PC = GetWorld()->GetFirstPlayerController();
    }
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("[HUD] Ability bar deferred - no player controller yet"));
        return;
    }

    AbilityBarWidget = CreateWidget<UUserWidget>(PC, BarClass);
    if (AbilityBarWidget)
    {
        AbilityBarWidget->AddToViewport(10);
        UE_LOG(LogTemp, Log, TEXT("[HUD] Ability bar %s added to viewport"), *BarClass->GetName());
    }
}

UAnimInstance* AEclipseCharacter::GetHeroAnimInstance() const
{
    const USkeletalMeshComponent* SkelMesh = GetMesh();
    return SkelMesh ? SkelMesh->GetAnimInstance() : nullptr;
}

void AEclipseCharacter::EnsureInputAssets()
{
    if (bInputAssetsLoaded)
    {
        return;
    }

    DefaultMappingContext = LoadObject<UInputMappingContext>(nullptr, TEXT("/Game/Input/IMC_Default.IMC_Default"));
    MoveAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/IA_Move.IA_Move"));
    LookAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/IA_Look.IA_Look"));
    SprintAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/IA_Sprint.IA_Sprint"));
    DodgeAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/IA_Dodge.IA_Dodge"));
    LightAttackAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/IA_LightAttack.IA_LightAttack"));
    HeavyAttackAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/IA_HeavyAttack.IA_HeavyAttack"));
    BlockAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/IA_Block.IA_Block"));
    LockOnAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/IA_LockOn.IA_LockOn"));
    Ability1Action = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/IA_Ability1.IA_Ability1"));
    Ability2Action = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/IA_Ability2.IA_Ability2"));
    Ability3Action = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/IA_Ability3.IA_Ability3"));
    Ability4Action = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/IA_Ability4.IA_Ability4"));
    UltimateAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/IA_Ultimate.IA_Ultimate"));
    WeaponArtAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/IA_WeaponArt.IA_WeaponArt"));
    ToggleKeybindsAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/IA_ToggleKeybinds.IA_ToggleKeybinds"));
    RotationAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/IA_Rotation.IA_Rotation"));
    if (!CycleWeaponAction)
    {
        CycleWeaponAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/IA_CycleWeapon.IA_CycleWeapon"));
    }

    bInputAssetsLoaded = true;

    if (!DefaultMappingContext || !MoveAction || !LookAction)
    {
        UE_LOG(LogTemp, Error, TEXT("[INPUT] Input assets missing - check /Game/Input"));
    }
}

void AEclipseCharacter::AddDefaultMappingContext()
{
    EnsureInputAssets();

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC && GetWorld())
    {
        PC = GetWorld()->GetFirstPlayerController();
    }
    if (!PC || !DefaultMappingContext)
    {
        UE_LOG(LogTemp, Warning, TEXT("[INPUT] Cannot add mapping context yet (PC=%s IMC=%s)"), PC ? TEXT("valid") : TEXT("null"), DefaultMappingContext ? TEXT("valid") : TEXT("null"));
        return;
    }

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
    {
        Subsystem->AddMappingContext(DefaultMappingContext, 0);
        UE_LOG(LogTemp, Log, TEXT("[INPUT] Mapping context IMC_Default applied"));
    }
}

void AEclipseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    EnsureInputAssets();

    UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (!EIC)
    {
        UE_LOG(LogTemp, Error, TEXT("[INPUT] PlayerInputComponent is not an EnhancedInputComponent"));
        return;
    }

    if (MoveAction) EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AEclipseCharacter::OnMoveInput);
    if (LookAction) EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AEclipseCharacter::OnLookInput);
    if (SprintAction)
    {
        EIC->BindAction(SprintAction, ETriggerEvent::Started, this, &AEclipseCharacter::OnSprintStart);
        EIC->BindAction(SprintAction, ETriggerEvent::Completed, this, &AEclipseCharacter::OnSprintStop);
    }
    if (DodgeAction) EIC->BindAction(DodgeAction, ETriggerEvent::Started, this, &AEclipseCharacter::OnDodgeInput);
    if (LightAttackAction) EIC->BindAction(LightAttackAction, ETriggerEvent::Started, this, &AEclipseCharacter::OnLightAttackInput);
    if (HeavyAttackAction)
    {
        EIC->BindAction(HeavyAttackAction, ETriggerEvent::Started, this, &AEclipseCharacter::OnHeavyPressedInput);
        EIC->BindAction(HeavyAttackAction, ETriggerEvent::Completed, this, &AEclipseCharacter::OnHeavyReleasedInput);
    }
    if (Ability1Action) EIC->BindAction(Ability1Action, ETriggerEvent::Started, this, &AEclipseCharacter::OnAbility1Input);
    if (Ability2Action) EIC->BindAction(Ability2Action, ETriggerEvent::Started, this, &AEclipseCharacter::OnAbility2Input);
    if (Ability3Action) EIC->BindAction(Ability3Action, ETriggerEvent::Started, this, &AEclipseCharacter::OnAbility3Input);
    if (Ability4Action) EIC->BindAction(Ability4Action, ETriggerEvent::Started, this, &AEclipseCharacter::OnAbility4Input);
    if (UltimateAction) EIC->BindAction(UltimateAction, ETriggerEvent::Started, this, &AEclipseCharacter::OnUltimateInput);
    if (WeaponArtAction) EIC->BindAction(WeaponArtAction, ETriggerEvent::Started, this, &AEclipseCharacter::OnWeaponArtInput);
    if (RotationAction) EIC->BindAction(RotationAction, ETriggerEvent::Started, this, &AEclipseCharacter::OnRotationInput);
    if (ToggleKeybindsAction) EIC->BindAction(ToggleKeybindsAction, ETriggerEvent::Started, this, &AEclipseCharacter::OnToggleKeybindsInput);
    if (CycleWeaponAction)
    {
        EIC->BindAction(CycleWeaponAction, ETriggerEvent::Started, this, &AEclipseCharacter::OnCycleWeaponInput);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[INPUT] IA_CycleWeapon missing - weapon cycling unbound"));
    }

    AddDefaultMappingContext();

    UE_LOG(LogTemp, Log, TEXT("[INPUT] Player input bound (%d action bindings)"), EIC->GetActionEventBindings().Num());
}

void AEclipseCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    const bool bWingShield = AbilityComponent && AbilityComponent->bWingShieldActive;

    Mana = FMath::Clamp(Mana + 5.0f * DeltaSeconds, 0.0f, MaxMana);

    if (!CombatComponent || !CombatComponent->IsActing())
    {
        Stamina = FMath::Clamp(Stamina + 10.0f * DeltaSeconds, 0.0f, MaxStamina);
    }

    if (bSprinting && !bWingShield)
    {
        Stamina = FMath::Clamp(Stamina - SprintStaminaPerSecond * DeltaSeconds, 0.0f, MaxStamina);
        if (Stamina <= 0.0f)
        {
            bSprinting = false;
            GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
            UE_LOG(LogTemp, Log, TEXT("[RESOURCE] Stamina exhausted - sprint cancelled"));
        }
    }
}

void AEclipseCharacter::OnMoveInput(const FInputActionValue& Value)
{
    MoveInputAxis = Value.Get<FVector2D>();

    if (AbilityComponent && AbilityComponent->bWingShieldActive)
    {
        return;
    }

    if (!Controller)
    {
        return;
    }

    const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
    const FRotationMatrix YawMatrix(YawRotation);
    AddMovementInput(YawMatrix.GetUnitAxis(EAxis::X), MoveInputAxis.Y);
    AddMovementInput(YawMatrix.GetUnitAxis(EAxis::Y), MoveInputAxis.X);
}

void AEclipseCharacter::OnLookInput(const FInputActionValue& Value)
{
    const FVector2D Axis = Value.Get<FVector2D>();
    AddControllerYawInput(Axis.X);
    AddControllerPitchInput(Axis.Y);
}

void AEclipseCharacter::OnSprintStart()
{
    if (Stamina <= 1.0f)
    {
        return;
    }
    bSprinting = true;
    GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
    UE_LOG(LogTemp, Log, TEXT("[COMBAT] Sprint started"));
}

void AEclipseCharacter::OnSprintStop()
{
    bSprinting = false;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AEclipseCharacter::OnDodgeInput()
{
    if (CombatComponent)
    {
        CombatComponent->InputDodge(MoveInputAxis);
    }
}

void AEclipseCharacter::OnLightAttackInput()
{
    if (CombatComponent)
    {
        CombatComponent->InputLightAttack();
    }
}

void AEclipseCharacter::OnHeavyPressedInput()
{
    if (CombatComponent)
    {
        CombatComponent->InputHeavyPressed();
    }
}

void AEclipseCharacter::OnHeavyReleasedInput()
{
    if (CombatComponent)
    {
        CombatComponent->InputHeavyReleased();
    }
}

void AEclipseCharacter::OnAbility1Input()
{
    if (!CombatComponent)
    {
        if (AbilityComponent) AbilityComponent->ActivateAbility(TEXT("AetherBolt"));
        return;
    }

    // The combat component owns cast timing, so the spell cannot outrun its own animation state.
    if (!CombatComponent->InputCast(AetherBoltCastSeconds))
    {
        UE_LOG(LogTemp, Log, TEXT("[CAST] AetherBolt refused - combat busy"));
    }
}

void AEclipseCharacter::OnCombatCastFinished(bool bCompleted)
{
    if (!bCompleted)
    {
        UE_LOG(LogTemp, Log, TEXT("[CAST] AetherBolt interrupted - no spell released"));
        return;
    }

    if (AbilityComponent)
    {
        AbilityComponent->ActivateAbility(TEXT("AetherBolt"));
    }
}

void AEclipseCharacter::OnAbility2Input()
{
    if (AbilityComponent) AbilityComponent->ActivateAbility(TEXT("FractureStrike"));
}

void AEclipseCharacter::OnAbility3Input()
{
    if (AbilityComponent) AbilityComponent->ActivateAbility(TEXT("WingShield"));
}

void AEclipseCharacter::OnAbility4Input()
{
    if (AbilityComponent) AbilityComponent->ActivateAbility(TEXT("PhaseStep"));
}

void AEclipseCharacter::OnUltimateInput()
{
    if (AbilityComponent) AbilityComponent->ActivateAbility(TEXT("AetherStorm"));
}

void AEclipseCharacter::OnWeaponArtInput()
{
    if (AbilityComponent) AbilityComponent->ActivateAbility(TEXT("FractureBlade"));
}

void AEclipseCharacter::OnRotationInput()
{
    if (AbilityComponent)
    {
        AbilityComponent->TryActivateRotation();
    }
}

void AEclipseCharacter::OnToggleKeybindsInput()
{
    UClass* WidgetClass = LoadClass<UUserWidget>(nullptr, TEXT("/Game/UI/WBP_KeyBindings.WBP_KeyBindings_C"));
    if (!WidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("[KEYBIND] WBP_KeyBindings not found yet"));
        return;
    }

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC)
    {
        return;
    }

    if (UUserWidget* Widget = CreateWidget<UUserWidget>(PC, WidgetClass))
    {
        Widget->AddToViewport();
        PC->SetShowMouseCursor(true);
        UE_LOG(LogTemp, Log, TEXT("[KEYBIND] Key bindings menu opened"));
    }
}

void AEclipseCharacter::OnCycleWeaponInput()
{
    if (EquipmentComponent)
    {
        EquipmentComponent->CycleWeapon();
    }
}

float AEclipseCharacter::GetResourceValue(EResourceType Type) const
{
    switch (Type)
    {
    case EResourceType::Mana: return Mana;
    case EResourceType::Stamina: return Stamina;
    case EResourceType::Rage: return Rage;
    case EResourceType::Aether: return Aether;
    default: return 0.0f;
    }
}

bool AEclipseCharacter::HasResource(EResourceType Type, float Amount) const
{
    return GetResourceValue(Type) >= Amount;
}

bool AEclipseCharacter::TrySpendResource(EResourceType Type, float Amount)
{
    if (!HasResource(Type, Amount))
    {
        return false;
    }

    switch (Type)
    {
    case EResourceType::Mana: Mana -= Amount; break;
    case EResourceType::Stamina: Stamina -= Amount; break;
    case EResourceType::Rage: Rage -= Amount; break;
    case EResourceType::Aether: Aether -= Amount; break;
    default: return false;
    }

    UE_LOG(LogTemp, Log, TEXT("[RESOURCE] spent %.0f of type %d -> Mana=%.0f Stamina=%.0f Rage=%.0f Aether=%.0f"),
        Amount, (int32)Type, Mana, Stamina, Rage, Aether);
    return true;
}

void AEclipseCharacter::AddResourceValue(EResourceType Type, float Amount)
{
    switch (Type)
    {
    case EResourceType::Mana: Mana = FMath::Clamp(Mana + Amount, 0.0f, MaxMana); break;
    case EResourceType::Stamina: Stamina = FMath::Clamp(Stamina + Amount, 0.0f, MaxStamina); break;
    case EResourceType::Rage: Rage = FMath::Clamp(Rage + Amount, 0.0f, MaxRage); break;
    case EResourceType::Aether: Aether = FMath::Clamp(Aether + Amount, 0.0f, MaxAether); break;
    default: break;
    }
}

void AEclipseCharacter::ApplyIncomingDamage(float Amount)
{
    if (Amount <= 0.0f)
    {
        return;
    }
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - Amount);
    Rage = FMath::Clamp(Rage + 10.0f, 0.0f, MaxRage);
    UE_LOG(LogTemp, Log, TEXT("[RESOURCE] Health=%.1f/%.1f (took %.1f) Rage=%.1f"), CurrentHealth, MaxHealth, Amount, Rage);
}

bool AEclipseCharacter::IsOnGCD() const
{
    if (!GetWorld()) return false;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastGCDTriggerTime) < GlobalCooldownDuration;
}

void AEclipseCharacter::TriggerGCD()
{
    if (!GetWorld()) return;
    LastGCDTriggerTime = GetWorld()->GetTimeSeconds();
}

void AEclipseCharacter::AddFractureFlow(float Amount)
{
    FractureFlow = FMath::Clamp(FractureFlow + Amount, 0.0f, MaxFractureFlow);
}

void AEclipseCharacter::SpendFractureFlow(float Amount)
{
    FractureFlow = FMath::Clamp(FractureFlow - Amount, 0.0f, MaxFractureFlow);
}

void AEclipseCharacter::AddKill(bool bIsHonorKill, bool bIsBossKill)
{
    TotalKills++;
    if (bIsHonorKill) HonorKills++;
    if (bIsBossKill) BossKills++;

    int32 NewLevel = FMath::Clamp(FMath::Min(TotalKills / 1000, 50), 1, 50);
    if (NewLevel > WeaponLevel)
    {
        WeaponLevel = NewLevel;
        UE_LOG(LogTemp, Log, TEXT("Weapon leveled up to %d! (Kills: %d, Honor: %d, Boss: %d)"),
            WeaponLevel, TotalKills, HonorKills, BossKills);
    }
}
