// Holds the upgrade list and lets the player buy upgrades. Using the station opens the built-in
// upgrade menu; override "Open Upgrade Menu" in a Blueprint child to use your own widget instead.

#pragma once

#include "CoreMinimal.h"
#include "MutateoStation.h"
#include "MutateoLoopTypes.h"
#include "MutateoUpgradeStation.generated.h"

class UMutateoUpgradeMenu;

UCLASS(Blueprintable)
class MUTATEOFORGITHUB_API AMutateoUpgradeStation : public AMutateoStation
{
	GENERATED_BODY()

public:
	AMutateoUpgradeStation();

	/** Comes pre-filled with WeaponDamage, HarvestYield, GrowthSpeed and MoveSpeed. Add your own freely. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Upgrades")
	TArray<FMutateoUpgradeDef> Upgrades;

	UFUNCTION(BlueprintPure, Category = "Mutateo|Upgrades")
	TArray<FMutateoUpgradeDef> GetUpgrades() const { return Upgrades; }

	/** Spends coins, raises the level and applies built-in side effects (MoveSpeed). */
	UFUNCTION(BlueprintCallable, Category = "Mutateo|Upgrades")
	bool PurchaseUpgrade(FName UpgradeId, AActor* Buyer);

	/** Widget used by the default Open Upgrade Menu. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Upgrades")
	TSubclassOf<UMutateoUpgradeMenu> MenuClass;

	/** Opens the built-in menu by default. Override in Blueprint to show your own widget. */
	UFUNCTION(BlueprintNativeEvent, Category = "Mutateo|Upgrades")
	void OpenUpgradeMenu(AActor* User);

	UFUNCTION(BlueprintImplementableEvent, Category = "Mutateo|Upgrades")
	void OnUpgradePurchased(FName UpgradeId, int32 NewLevel);

protected:
	virtual void BeginPlay() override;
	virtual bool HandleUse(AActor* User) override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UMutateoUpgradeMenu> OpenMenu;
};
