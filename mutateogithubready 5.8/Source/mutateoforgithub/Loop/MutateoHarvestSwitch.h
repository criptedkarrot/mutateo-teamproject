// The switch next to the farm. Every fully grown crop turns into a mutant and its plot is cleared.

#pragma once

#include "CoreMinimal.h"
#include "MutateoStation.h"
#include "MutateoHarvestSwitch.generated.h"

class AMutateoMutant;

UCLASS(Blueprintable)
class MUTATEOFORGITHUB_API AMutateoHarvestSwitch : public AMutateoStation
{
	GENERATED_BODY()

public:
	AMutateoHarvestSwitch();

	/** Your mutant Blueprint (a child of MutateoMutant). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Harvest")
	TSubclassOf<AMutateoMutant> MutantClass;

	/** Offset from each plot's origin where its mutant appears. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Harvest")
	FVector SpawnOffset = FVector(0.f, 0.f, 100.f);

	/** Fires with the mutants that were spawned (play a rumble, camera shake, sound...). */
	UFUNCTION(BlueprintImplementableEvent, Category = "Mutateo|Harvest")
	void OnMutantsReleased(const TArray<AMutateoMutant*>& Mutants);

protected:
	virtual bool HandleUse(AActor* User) override;
};
