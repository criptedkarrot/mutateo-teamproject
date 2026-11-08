#include "MutateoUpgradeStation.h"

#include "MutateoGameSubsystem.h"
#include "MutateoLoopDirector.h"
#include "MutateoLoopLibrary.h"
#include "MutateoUpgradeMenu.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"

#define LOCTEXT_NAMESPACE "MutateoLoop"

AMutateoUpgradeStation::AMutateoUpgradeStation()
{
	StationPrompt = LOCTEXT("UpgradePrompt", "Upgrades");
	MenuClass = UMutateoUpgradeMenu::StaticClass();

	Upgrades.Add(FMutateoUpgradeDef(MutateoUpgradeIds::WeaponDamage,
		LOCTEXT("U_Dmg", "Weapon Damage"), LOCTEXT("U_DmgDesc", "+25% weapon damage per level."),
		10, 1.5f, 5, 1.f, 0.25f));

	Upgrades.Add(FMutateoUpgradeDef(MutateoUpgradeIds::HarvestYield,
		LOCTEXT("U_Yield", "Harvest Yield"), LOCTEXT("U_YieldDesc", "Each mutant drops one more resource per level."),
		15, 1.6f, 4, 5.f, 1.f));

	Upgrades.Add(FMutateoUpgradeDef(MutateoUpgradeIds::GrowthSpeed,
		LOCTEXT("U_Grow", "Growth Speed"), LOCTEXT("U_GrowDesc", "Crops grow 25% faster per level."),
		12, 1.5f, 4, 1.f, 0.25f));

	Upgrades.Add(FMutateoUpgradeDef(MutateoUpgradeIds::MoveSpeed,
		LOCTEXT("U_Move", "Running Shoes"), LOCTEXT("U_MoveDesc", "Move faster around the island."),
		8, 1.5f, 3, 0.f, 75.f));
}

void AMutateoUpgradeStation::BeginPlay()
{
	Super::BeginPlay();
	if (UMutateoGameSubsystem* Subsystem = UMutateoLoopLibrary::GetMutateoSubsystem(this))
	{
		Subsystem->RegisterUpgrades(Upgrades);
	}
}

bool AMutateoUpgradeStation::HandleUse(AActor* User)
{
	OpenUpgradeMenu(User);
	if (AMutateoLoopDirector* Director = GetDirector())
	{
		Director->NotifyUpgradeStationVisited();
	}
	return true;
}

void AMutateoUpgradeStation::OpenUpgradeMenu_Implementation(AActor* User)
{
	if (OpenMenu && OpenMenu->IsInViewport()) return;

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC || !MenuClass) return;

	OpenMenu = CreateWidget<UMutateoUpgradeMenu>(PC, MenuClass);
	if (OpenMenu)
	{
		OpenMenu->Open(this, User);
	}
}

bool AMutateoUpgradeStation::PurchaseUpgrade(FName UpgradeId, AActor* Buyer)
{
	UMutateoGameSubsystem* Subsystem = UMutateoLoopLibrary::GetMutateoSubsystem(this);
	if (!Subsystem || !Subsystem->TryPurchaseUpgrade(UpgradeId)) return false;

	// Built-in side effects.
	if (UpgradeId == MutateoUpgradeIds::MoveSpeed)
	{
		ACharacter* Character = Cast<ACharacter>(Buyer);
		if (!Character)
		{
			if (const AController* Controller = Cast<AController>(Buyer)) Character = Cast<ACharacter>(Controller->GetPawn());
		}
		FMutateoUpgradeDef Def;
		if (Character && Subsystem->GetUpgradeDefinition(UpgradeId, Def))
		{
			Character->GetCharacterMovement()->MaxWalkSpeed += Def.ValuePerLevel;
		}
	}

	OnUpgradePurchased(UpgradeId, Subsystem->GetUpgradeLevel(UpgradeId));
	return true;
}

#undef LOCTEXT_NAMESPACE
