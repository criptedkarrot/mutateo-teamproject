// Subtitle / objective / coins HUD. Works as-is (it builds a simple layout itself).
// To restyle it, make a Widget Blueprint with this as its parent and add Text widgets named
// exactly SubtitleText, ObjectiveText and CoinsText.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MutateoSubtitleWidget.generated.h"

class UTextBlock;

UCLASS()
class MUTATEOFORGITHUB_API UMutateoSubtitleWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Mutateo|HUD")
	void ShowSubtitle(const FText& Text, float Duration);

	UFUNCTION(BlueprintCallable, Category = "Mutateo|HUD")
	void SetObjective(const FText& Text);

	UFUNCTION(BlueprintCallable, Category = "Mutateo|HUD")
	void SetCoins(int32 Coins);

	/** Hook for a fade-in animation. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Mutateo|HUD")
	void OnSubtitleShown(const FText& Text);

	UFUNCTION(BlueprintImplementableEvent, Category = "Mutateo|HUD")
	void OnSubtitleHidden();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SubtitleText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ObjectiveText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CoinsText;

private:
	UFUNCTION()
	void HandleCoinsChanged(int32 NewCoins);

	void HideSubtitle();

	FTimerHandle HideTimer;
};
