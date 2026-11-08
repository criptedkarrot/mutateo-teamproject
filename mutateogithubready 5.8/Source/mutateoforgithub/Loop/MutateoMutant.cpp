#include "MutateoMutant.h"

#include "AIController.h"
#include "MutateoLoopLibrary.h"
#include "MutateoLoopTypes.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"

AMutateoMutant::AMutateoMutant()
{
	PrimaryActorTick.bCanEverTick = true;

	AIControllerClass = AAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
}

void AMutateoMutant::BeginPlay()
{
	Super::BeginPlay();
	Health = MaxHealth;
	TimeSinceAttack = AttackCooldown;
	GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;

	if (!GetController())
	{
		SpawnDefaultController();
	}
}

void AMutateoMutant::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bDead) return;

	TimeAlive += DeltaSeconds;
	TimeSinceAttack += DeltaSeconds;
	if (TimeAlive < WakeUpDelay) return;

	APawn* Target = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Target) return;

	AAIController* AI = Cast<AAIController>(GetController());
	const FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
	const float Distance = ToTarget.Size2D();

	if (Distance <= AttackRange)
	{
		if (AI) AI->StopMovement();

		const FRotator Face(0.f, ToTarget.Rotation().Yaw, 0.f);
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), Face, DeltaSeconds, 8.f));

		if (TimeSinceAttack >= AttackCooldown)
		{
			TimeSinceAttack = 0.f;
			UGameplayStatics::ApplyDamage(Target, AttackDamage, GetController(), this, UDamageType::StaticClass());
			OnAttack(Target);
		}
		return;
	}

	// Use the nav mesh if there is one; otherwise just walk straight at the player.
	if (!bUseDirectChase && AI)
	{
		if (AI->GetMoveStatus() == EPathFollowingStatus::Idle)
		{
			const EPathFollowingRequestResult::Type Result = AI->MoveToActor(Target, AttackRange * 0.7f);
			if (Result == EPathFollowingRequestResult::Failed)
			{
				bUseDirectChase = true;
			}
		}
	}
	else
	{
		AddMovementInput(ToTarget.GetSafeNormal2D(), 1.f);
	}
}

float AMutateoMutant::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bDead) return 0.f;

	const float Applied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (Applied <= 0.f) return 0.f;

	Health = FMath::Max(0.f, Health - Applied);
	OnDamaged(Applied, Health);

	if (Health <= 0.f)
	{
		Die();
	}
	return Applied;
}

void AMutateoMutant::Die()
{
	bDead = true;

	if (AAIController* AI = Cast<AAIController>(GetController())) AI->StopMovement();
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SpawnDrops();
	OnDeath();
	SetLifeSpan(FMath::Max(0.1f, CorpseLifeSpan));
}

void AMutateoMutant::SpawnDrops()
{
	if (!DropClass) return;

	const float YieldValue = UMutateoLoopLibrary::GetUpgradeValue(this, MutateoUpgradeIds::HarvestYield, static_cast<float>(BaseDropCount));
	const int32 Count = FMath::Max(1, FMath::RoundToInt(YieldValue));

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	for (int32 i = 0; i < Count; ++i)
	{
		const FVector2D Scatter = FMath::RandPointInCircle(60.f);
		const FVector Location = GetActorLocation() + FVector(Scatter.X, Scatter.Y, 0.f);
		GetWorld()->SpawnActor<AActor>(DropClass, Location, FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f), Params);
	}
}
