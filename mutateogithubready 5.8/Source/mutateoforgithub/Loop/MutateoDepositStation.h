// Buys the resources the mutants dropped, leaving a few behind for replanting.

#pragma once

#include "CoreMinimal.h"
#include "MutateoStation.h"
#include "MutateoLoopTypes.h"
#include "MutateoDepositStation.generated.h"

UCLASS(Blueprintable)
class MUTATEOFORGITHUB_API AMutateoDepositStation : public AMutateoStation
{
	GENERATED_BODY()

public:
	AMutateoDepositStation();

	/** Items this station buys, their coin value, and how many the player keeps for replanting. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Deposit")
	TArray<FMutateoSellableItem> SellableItems;

	UFUNCTION(BlueprintImplementableEvent, Category = "Mutateo|Deposit")
	void OnDeposited(int32 CoinsEarned, int32 ItemsSold);

protected:
	virtual bool HandleUse(AActor* User) override;
};
