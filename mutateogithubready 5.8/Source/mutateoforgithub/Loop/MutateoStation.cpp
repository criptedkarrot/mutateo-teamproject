#include "MutateoStation.h"

#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "MutateoLoopDirector.h"
#include "MutateoLoopLibrary.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

#define LOCTEXT_NAMESPACE "MutateoLoop"

AMutateoStation::AMutateoStation()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.05f;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Root);
	Mesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);

	PromptText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("PromptText"));
	PromptText->SetupAttachment(Root);
	PromptText->SetRelativeLocation(FVector(0.f, 0.f, 220.f));
	PromptText->SetHorizontalAlignment(EHTA_Center);
	PromptText->SetVerticalAlignment(EVRTA_TextCenter);
	PromptText->SetWorldSize(36.f);
	PromptText->SetTextRenderColor(FColor(255, 230, 120));
	PromptText->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PromptText->SetHiddenInGame(true);

	InteractKeyLabel = LOCTEXT("InteractKey", "F");
}

void AMutateoStation::BeginPlay()
{
	Super::BeginPlay();
	PromptText->SetText(FText::Format(LOCTEXT("PromptFmt", "[{0}] {1}"), InteractKeyLabel, StationPrompt));
	BindInteractInput();
}

void AMutateoStation::BindInteractInput()
{
	if (bInputBound || !InteractAction) return;

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC)
	{
		// Player not spawned yet; try again shortly.
		FTimerHandle Retry;
		GetWorldTimerManager().SetTimer(Retry, this, &AMutateoStation::BindInteractInput, 0.5f, false);
		return;
	}

	EnableInput(PC);
	if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputComponent))
	{
		Input->BindAction(InteractAction, ETriggerEvent::Started, this, &AMutateoStation::HandleInteractPressed);
		bInputBound = true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: input component is not an Enhanced Input component."), *GetName());
	}
}

bool AMutateoStation::IsPlayerInRange(const APawn* Player) const
{
	return Player && FVector::DistSquared(Player->GetActorLocation(), GetActorLocation()) <= FMath::Square(InteractRadius);
}

void AMutateoStation::HandleInteractPressed()
{
	APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (IsPlayerInRange(Player))
	{
		UseStation(Player);
	}
}

void AMutateoStation::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	const bool bShow = IsPlayerInRange(Player);
	PromptText->SetHiddenInGame(!bShow);

	if (bShow)
	{
		if (const APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(this, 0))
		{
			const FVector ToCamera = Camera->GetCameraLocation() - PromptText->GetComponentLocation();
			PromptText->SetWorldRotation(FRotator(0.f, ToCamera.Rotation().Yaw, 0.f));
		}
	}
}

bool AMutateoStation::UseStation(AActor* User)
{
	const bool bSuccess = HandleUse(User);
	OnStationUsed(User, bSuccess);
	return bSuccess;
}

AMutateoLoopDirector* AMutateoStation::GetDirector() const
{
	return UMutateoLoopLibrary::GetLoopDirector(this);
}

void AMutateoStation::Say(const FMutateoVoiceLine& Line) const
{
	if (AMutateoLoopDirector* Director = GetDirector())
	{
		Director->PlayVoiceLine(Line);
	}
}

#undef LOCTEXT_NAMESPACE
