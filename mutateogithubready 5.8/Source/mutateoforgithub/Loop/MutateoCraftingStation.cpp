#include "MutateoCraftingStation.h"

#include "MutateoLoopDirector.h"
#include "MutateoLoopLibrary.h"
#include "MutateoProjectBridge.h"
#include "Engine/World.h"

#define LOCTEXT_NAMESPACE "MutateoLoop"

AMutateoCraftingStation::AMutateoCraftingStation()
{
	StationPrompt = LOCTEXT("CraftPrompt", "Craft");

	ResultSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("ResultSpawnPoint"));
	ResultSpawnPoint->SetupAttachment(Root);
	ResultSpawnPoint->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
}

bool AMutateoCraftingStation::HasAllMaterials(AActor* User) const
{
	if (!User) return false;
	for (const FMutateoItemRequirement& Req : Recipe)
	{
		if (!Req.Item) continue;
		if (UMutateoLoopLibrary::GetInventoryItemCount(User, Req.Item) < Req.Count) return false;
	}
	return true;
}

bool AMutateoCraftingStation::HandleUse(AActor* User)
{
	if (bCraftOnlyOnce && bHasCrafted) return false;

	if (!MutateoBridge::HasInventory(User))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: %s has no inventory component."), *GetName(), *GetNameSafe(User));
		return false;
	}

	if (!HasAllMaterials(User))
	{
		if (const AMutateoLoopDirector* Director = GetDirector()) Say(Director->MissingMaterialsLine);
		return false;
	}

	for (const FMutateoItemRequirement& Req : Recipe)
	{
		if (Req.Item) UMutateoLoopLibrary::ConsumeInventoryItems(User, Req.Item, Req.Count);
	}

	AActor* Spawned = nullptr;
	if (ResultClass)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		Spawned = GetWorld()->SpawnActor<AActor>(ResultClass, ResultSpawnPoint->GetComponentTransform(), Params);
	}

	bHasCrafted = true;
	OnCrafted(Spawned);

	if (AMutateoLoopDirector* Director = GetDirector()) Director->NotifyWeaponCrafted();
	return true;
}

#undef LOCTEXT_NAMESPACE
