#include "MutateoHarvestSwitch.h"

#include "MutateoCropPlotInterface.h"
#include "MutateoLoopDirector.h"
#include "MutateoMutant.h"
#include "MutateoProjectBridge.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

#define LOCTEXT_NAMESPACE "MutateoLoop"

AMutateoHarvestSwitch::AMutateoHarvestSwitch()
{
	StationPrompt = LOCTEXT("HarvestPrompt", "Harvest");
}

bool AMutateoHarvestSwitch::HandleUse(AActor* User)
{
	AMutateoLoopDirector* Director = GetDirector();
	if (Director)
	{
		FMutateoVoiceLine Reason;
		if (!Director->CanActivateHarvestSwitch(Reason))
		{
			Say(Reason);
			return false;
		}
	}

	if (!MutantClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: set Mutant Class on the harvest switch."), *GetName());
		return false;
	}

	TArray<AActor*> Plots;
	if (Director) Plots = Director->GetPlots();
	else MutateoBridge::GetAllPlots(this, Plots);

	const FVector SwitchLocation = GetActorLocation();
	TArray<AMutateoMutant*> Spawned;

	for (AActor* Plot : Plots)
	{
		if (!MutateoBridge::PlotIsCropReady(Plot)) continue;

		const FVector Location = Plot->GetActorLocation() + SpawnOffset;
		const FRotator FacingSwitch = (SwitchLocation - Location).GetSafeNormal2D().Rotation();

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		if (AMutateoMutant* Mutant = GetWorld()->SpawnActor<AMutateoMutant>(MutantClass, Location, FacingSwitch, Params))
		{
			Spawned.Add(Mutant);
		}

		MutateoBridge::PlotClearCrop(Plot);
	}

	if (Spawned.IsEmpty()) return false;

	OnMutantsReleased(Spawned);
	if (Director) Director->NotifyMutantsReleased(Spawned);
	return true;
}

#undef LOCTEXT_NAMESPACE
