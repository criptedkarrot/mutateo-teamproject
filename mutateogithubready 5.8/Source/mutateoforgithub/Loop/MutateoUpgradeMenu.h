// A ready-made upgrade shop menu, built in code. The upgrade station opens it automatically.
// To make your own, implement "Open Upgrade Menu" in your upgrade station Blueprint instead.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MutateoUpgradeMenu.generated.h"

class AMutateoUpgradeStation;
class UButton;
class UTextBlock;
class UVerticalBox;

/** One line in the menu: name, level, description and a Buy button. */
UCLASS()
class MUTATEOFORGITHUB_API UMutateoUpgradeRow : public UUserWidget
{
	GENERATED_BODY()

public:
	void Setup(AMutateoUpgradeStation* InStation, AActor* InBuyer, FName InUpgradeId);
	void Refresh();

protected:
	virtual void NativeOnInitialized() override;

private:
	UFUNCTION()
	void HandleBuyClicked();

	UPROPERTY(Transient) TObjectPtr<UTextBlock> NameText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> DescriptionText;
	UPROPERTY(Transient) TObjectPtr<UButton> BuyButton;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> BuyText;

	TWeakObjectPtr<AMutateoUpgradeStation> Station;
	TWeakObjectPtr<AActor> Buyer;
	FName UpgradeId;
};

UCLASS()
class MUTATEOFORGITHUB_API UMutateoUpgradeMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Fills the menu and switches to mouse control. */
	void Open(AMutateoUpgradeStation* InStation, AActor* InBuyer);

	UFUNCTION(BlueprintCallable, Category = "Mutateo|Upgrades")
	void CloseMenu();

	void RefreshAll();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void HandleCloseClicked();

	UFUNCTION()
	void HandleCoinsChanged(int32 NewCoins);

	UPROPERTY(Transient) TObjectPtr<UVerticalBox> RowBox;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> CoinsText;
	UPROPERTY(Transient) TObjectPtr<UButton> CloseButton;
	UPROPERTY(Transient) TArray<TObjectPtr<UMutateoUpgradeRow>> Rows;
};
