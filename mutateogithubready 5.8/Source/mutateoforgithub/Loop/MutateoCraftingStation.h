// Takes the fetch-quest materials and spawns the weapon pickup.

#pragma once

#include "CoreMinimal.h"
#include "MutateoStation.h"
#include "MutateoLoopTypes.h"
#include "MutateoCraftingStation.generated.h"

UCLASS(Blueprintable)
class MUTATEOFORGITHUB_API AMutateoCraftingStation : public AMutateoStation
{
	GENERATED_BODY()

public:
	AMutateoCraftingStation();

	/** What the player must bring (e.g. quest_collectable_scrap x1, rope x1, plastic x1). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Crafting")
	TArray<FMutateoItemRequirement> Recipe;

	/** Spawned when crafting succeeds, e.g. a pickup Blueprint set up to give the water gun. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Crafting")
	TSubclassOf<AActor> ResultClass;

	/** Move this in the Blueprint viewport to choose where the result appears. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mutateo|Crafting")
	TObjectPtr<USceneComponent> ResultSpawnPoint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Crafting")
	bool bCraftOnlyOnce = true;

	UFUNCTION(BlueprintCallable, Category = "Mutateo|Crafting")
	bool HasAllMaterials(AActor* User) const;

	UFUNCTION(BlueprintPure, Category = "Mutateo|Crafting")
	bool HasCrafted() const { return bHasCrafted; }

	UFUNCTION(BlueprintImplementableEvent, Category = "Mutateo|Crafting")
	void OnCrafted(AActor* SpawnedResult);

protected:
	virtual bool HandleUse(AActor* User) override;

private:
	bool bHasCrafted = false;
};
