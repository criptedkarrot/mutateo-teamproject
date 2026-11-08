// Lets the loop ask your plant plots what state they are in.
// Add this interface to BP_Plantplot (Class Settings > Implemented Interfaces).

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MutateoCropPlotInterface.generated.h"

UINTERFACE(BlueprintType, Blueprintable, meta = (DisplayName = "Mutateo Crop Plot"))
class MUTATEOFORGITHUB_API UMutateoCropPlotInterface : public UInterface
{
	GENERATED_BODY()
};

class MUTATEOFORGITHUB_API IMutateoCropPlotInterface
{
	GENERATED_BODY()

public:
	/** True if something is planted here (growing or grown). */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Mutateo|Crops")
	bool HasCrop();

	/** True if the crop here is fully grown. */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Mutateo|Crops")
	bool IsCropReady();

	/** Destroy the plant and reset the plot to empty. Called when the harvest switch turns it into a mutant. */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Mutateo|Crops")
	void ClearCrop();
};
