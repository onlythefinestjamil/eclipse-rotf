#include "EclipseTargetDummy.h"

#include "EclipseCharacter.h"
#include "EclipseCombatComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/OverlapResult.h"
#include "DrawDebugHelpers.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemComponent.h"
#include "EclipseAttributeSet.h"

DEFINE_LOG_CATEGORY_STATIC(LogEclipseDummy, Log, All);

AEclipseTargetDummy::AEclipseTargetDummy()
{
    PrimaryActorTick.bCanEverTick = true;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;
    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Mesh->SetCollisionObjectType(ECC_Pawn);
    Mesh->SetCollisionResponseToAllChannels(ECR_Block);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(TEXT("/Game/Characters/BERSERKER/StaticMeshes/BERSERKER.BERSERKER"));
    if (MeshFinder.Succeeded())
    {
        Mesh->SetStaticMesh(MeshFinder.Object);
    }

    AbilitySystem = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));
    StatusAttributes = CreateDefaultSubobject<UEclipseAttributeSet>(TEXT("StatusAttributes"));
}

void AEclipseTargetDummy::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    if (AbilitySystem)
    {
        AbilitySystem->InitAbilityActorInfo(this, this);
    }
}

void AEclipseTargetDummy::BeginPlay()
{
    Super::BeginPlay();
    Health = MaxHealth;
    Poise = MaxPoise;
    TimeUntilAttack = AttackInterval;
    BaseLocation = GetActorLocation();
    UE_LOG(LogEclipseDummy, Log, TEXT("[DUMMY] %s ready (HP %.0f, poise %.0f, telegraph attack every %.1fs)"), *GetName(), Health, Poise, AttackInterval);
}

void AEclipseTargetDummy::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (bStaggered)
    {
        StaggerRemaining -= DeltaSeconds;
        if (StaggerRemaining <= 0.0f)
        {
            bStaggered = false;
            Poise = MaxPoise;
            UE_LOG(LogEclipseDummy, Log, TEXT("[DUMMY] recovered from stagger"));
        }
        return;
    }

    if (bAttackPending)
    {
        WindupRemaining -= DeltaSeconds;
        if (WindupRemaining <= 0.0f)
        {
            bAttackPending = false;
            bWindingUp = false;
            ExecuteTelegraphAttack();
            TimeUntilAttack = AttackInterval;
        }
        return;
    }

    TimeUntilAttack -= DeltaSeconds;
    if (TimeUntilAttack <= 0.0f)
    {
        bAttackPending = true;
        bWindingUp = true;
        WindupRemaining = WindupSeconds;
        UE_LOG(LogEclipseDummy, Log, TEXT("[DUMMY] TELEGRAPH - overhead smash winding up for %.2fs (red glow)"), WindupSeconds);
        DrawDebugSphere(GetWorld(), GetActorLocation() + FVector(0, 0, 150.0f), 120.0f, 12, FColor::Red, false, WindupSeconds);
    }
}

void AEclipseTargetDummy::ExecuteTelegraphAttack()
{
    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(DummySwing), false, this);
    const FVector Origin = GetActorLocation();
    GetWorld()->OverlapMultiByChannel(Overlaps, Origin, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(AttackRange), Params);
    DrawDebugSphere(GetWorld(), Origin, AttackRange, 16, FColor::Orange, false, 0.4f);

    bool bHitPlayer = false;
    for (const FOverlapResult& Overlap : Overlaps)
    {
        if (AEclipseCharacter* Player = Cast<AEclipseCharacter>(Overlap.GetActor()))
        {
            if (UEclipseCombatComponent* Combat = Player->GetCombat())
            {
                const float Final = Combat->ReceiveDamage(AttackDamage, this);
                Player->ApplyIncomingDamage(Final);
                bHitPlayer = true;
            }
        }
    }

    if (!bHitPlayer)
    {
        UE_LOG(LogEclipseDummy, Log, TEXT("[DUMMY] swing missed (no player inside %.0fcm)"), AttackRange);
    }
}

float AEclipseTargetDummy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    const float Incoming = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    Health = FMath::Max(0.0f, Health - Incoming);

    Poise = FMath::Max(0.0f, Poise - Incoming * 0.5f);
    if (Poise <= 0.0f && !bStaggered)
    {
        bStaggered = true;
        StaggerRemaining = 2.0f;
        bAttackPending = false;
        bWindingUp = false;
        UE_LOG(LogEclipseDummy, Log, TEXT("[DUMMY] POISE BROKEN - staggered 2s, open for critical"));
    }

    UE_LOG(LogEclipseDummy, Log, TEXT("[DUMMY] %s took %.1f (HP %.1f, poise %.1f)"), *GetName(), Incoming, Health, Poise);
    return Incoming;
}
