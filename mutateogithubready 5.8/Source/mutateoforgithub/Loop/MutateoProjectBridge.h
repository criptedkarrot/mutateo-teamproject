// Reads this project's existing Blueprint data directly, so no Blueprint wiring is needed:
//  - Inventory: any component with an "Inventoryitems" array of {count, iteminstance} structs
//    (inventorymanagercomponant). Calls its "Oninventorychanged" dispatcher after changes.
//  - Plots: any actor with a "Spawnedplant" actor variable (BP_Plantplot); readiness is the
//    plant's "Isready" bool (bp_carrotplant).
// If a Blueprint implements the Mutateo Inventory / Mutateo Crop Plot interface, that is used instead.

#pragma once

#include "CoreMinimal.h"
#include "mutateoforgithub/Items/ItemDefinition.h"

namespace MutateoBridge
{
	// ---- Inventory ----
	/** True if the actor (or its pawn/controller) has an inventory we can read. */
	bool HasInventory(AActor* Owner);
	int32 CountItems(AActor* Owner, TSubclassOf<UItemDefinition> Item);
	bool ConsumeItems(AActor* Owner, TSubclassOf<UItemDefinition> Item, int32 Count);

	// ---- Plots ----
	bool IsPlot(const AActor* Actor);
	bool PlotHasCrop(AActor* Plot);
	bool PlotIsCropReady(AActor* Plot);
	void PlotClearCrop(AActor* Plot);
	/** Every plot in the world. */
	void GetAllPlots(const UObject* WorldContext, TArray<AActor*>& OutPlots);
}
