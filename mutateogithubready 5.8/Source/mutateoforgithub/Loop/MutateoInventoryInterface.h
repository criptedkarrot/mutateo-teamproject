// Lets the C++ loop read and remove items from your Blueprint inventory component.
// Add this interface to "inventorymanagercomponant" (Class Settings > Implemented Interfaces)
// and fill in the two functions.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "mutateoforgithub/Items/ItemDefinition.h"
#include "MutateoInventoryInterface.generated.h"

UINTERFACE(BlueprintType, Blueprintable, meta = (DisplayName = "Mutateo Inventory"))
class MUTATEOFORGITHUB_API UMutateoInventoryInterface : public UInterface
{
	GENERATED_BODY()
};

class MUTATEOFORGITHUB_API IMutateoInventoryInterface
{
	GENERATED_BODY()

public:
	/** Return how many of this item the inventory holds, across all stacks. */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Mutateo|Inventory")
	int32 CountItemsOfType(TSubclassOf<UItemDefinition> Item);

	/** Remove Count of this item (across stacks). Return true if they were removed. */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Mutateo|Inventory")
	bool ConsumeItemsOfType(TSubclassOf<UItemDefinition> Item, int32 Count);
};
