// Base for the crafting, deposit and upgrade stations and the harvest switch.
// Works on its own: walk up, a floating "[E] Craft" prompt appears, press Interact.
// (You can still call "Use Station" from your own interaction system if you prefer.)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MutateoLoopTypes.h"
#include "MutateoStation.generated.h"

class AMutateoLoopDirector;
class UInputAction;
class UStaticMeshComponent;
class UTextRenderComponent;

UCLASS(Abstract, Blueprintable)
class MUTATEOFORGITHUB_API AMutateoStation : public AActor
{
	GENERATED_BODY()

public:
	AMutateoStation();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mutateo|Station")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mutateo|Station")
	TObjectPtr<UStaticMeshComponent> Mesh;

	/** Floating "[E] Craft" text shown when the player is close. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mutateo|Station")
	TObjectPtr<UTextRenderComponent> PromptText;

	/** Word shown in the prompt, e.g. "Craft", "Deposit". */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Station")
	FText StationPrompt;

	/** The interact input action (IA_Interact). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Station")
	TObjectPtr<UInputAction> InteractAction;

	/** Key label shown in the prompt. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Station")
	FText InteractKeyLabel;

	/** How close the player must be to use the station. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Station", meta = (ClampMin = 50))
	float InteractRadius = 300.f;

	/** Returns true if the station did something. */
	UFUNCTION(BlueprintCallable, Category = "Mutateo|Station")
	bool UseStation(AActor* User);

	UFUNCTION(BlueprintPure, Category = "Mutateo|Station")
	FText GetStationPrompt() const { return StationPrompt; }

	/** Fires after every use, so you can play sounds / effects. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Mutateo|Station")
	void OnStationUsed(AActor* User, bool bSuccess);

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;
	virtual bool HandleUse(AActor* User) { return false; }

	AMutateoLoopDirector* GetDirector() const;
	void Say(const FMutateoVoiceLine& Line) const;
	bool IsPlayerInRange(const APawn* Player) const;

private:
	void BindInteractInput();
	void HandleInteractPressed();

	bool bInputBound = false;
};
