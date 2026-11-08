// The "brain" of the vertical slice. Drop ONE of these into the level and fill in its references.
// It moves through each stage of the loop, plays the guide voice lines, highlights where the player
// should go next, and keeps the objective text up to date.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InputCoreTypes.h"
#include "MutateoLoopTypes.h"
#include "MutateoLoopDirector.generated.h"

class AMutateoCraftingStation;
class AMutateoDepositStation;
class AMutateoHarvestSwitch;
class AMutateoMutant;
class AMutateoUpgradeStation;
class UInputAction;
class UMaterialInterface;
class UNiagaraSystem;
class USoundBase;
class UMutateoSubtitleWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMutateoStageChanged, EMutateoLoopStage, NewStage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMutateoVoiceLineEvent, FText, Text, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMutateoObjectiveChanged, FText, Objective);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMutateoLoopCompleted, int32, LoopsCompleted);

UCLASS(Blueprintable)
class MUTATEOFORGITHUB_API AMutateoLoopDirector : public AActor
{
	GENERATED_BODY()

public:
	AMutateoLoopDirector();

	// ===================== Level references (set these on the placed actor) =====================

	/** The glowing starter crop(s) placed in front of the player spawn. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Mutateo|Level")
	TArray<TObjectPtr<AActor>> StarterPickups;

	/** Your BP_Plantplot actors. Leave empty to auto-find every actor with the Mutateo Crop Plot interface. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Mutateo|Level")
	TArray<TObjectPtr<AActor>> PlantPlots;

	/** Scrap / rope / plastic pickups to highlight during the fetch quest. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Mutateo|Level")
	TArray<TObjectPtr<AActor>> FetchPickups;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Mutateo|Level")
	TObjectPtr<AMutateoCraftingStation> CraftingStation;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Mutateo|Level")
	TObjectPtr<AMutateoHarvestSwitch> HarvestSwitch;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Mutateo|Level")
	TObjectPtr<AMutateoDepositStation> DepositStation;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Mutateo|Level")
	TObjectPtr<AMutateoUpgradeStation> UpgradeStation;

	// ===================== Setup =====================

	/** Item the starter pickup gives (e.g. item_carrot). Picking one up finishes the first step.
	 *  If empty, the step finishes when every Starter Pickup has been destroyed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Setup")
	TSubclassOf<UItemDefinition> StarterItem;

	/** Seconds after the level starts before the guide speaks. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Setup", meta = (ClampMin = 0))
	float StartDelay = 1.5f;

	/** Overlay material put on whatever the player should go to next (a bright, translucent, pulsing material works best). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Setup")
	TObjectPtr<UMaterialInterface> HighlightMaterial;

	/** HUD for subtitles, objective and coins. Leave empty for the built-in one. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Setup")
	TSubclassOf<UMutateoSubtitleWidget> SubtitleWidgetClass;

	/** Pause between queued voice lines. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Setup", meta = (ClampMin = 0))
	float VoiceLineGap = 0.4f;

	/** Untick to skip the guide entirely and start at the Replant stage (handy for testing combat). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Setup")
	bool bPlayTutorial = true;

	// ===================== Combat (water gun) =====================

	/** Fire input (IA_ATTACK). Pressing it starts the spray; it keeps firing while Fire Key is held. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Combat")
	TObjectPtr<UInputAction> AttackAction;

	/** Only fire once the weapon has been crafted and something is equipped (attached to the player). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Combat")
	bool bRequireEquippedWeapon = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Combat", meta = (ClampMin = 0))
	float WeaponDamage = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Combat", meta = (ClampMin = 100))
	float WeaponRange = 2500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Combat", meta = (ClampMin = 0))
	float AimAssistRadius = 20.f;

	/** Seconds between shots while holding fire. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Combat", meta = (ClampMin = 0.02))
	float FireInterval = 0.15f;

	/** Seconds after you stop firing before the water stream (and each impact splash) is removed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Combat", meta = (ClampMin = 0))
	float EffectDespawnTime = 3.f;

	/** Spawned where the water hits. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Combat")
	TObjectPtr<UNiagaraSystem> ImpactEffect;

	/** Key that must stay held to keep spraying (works even if the input action uses a Tap trigger). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Combat")
	FKey FireKey;

	/** Looping water stream attached to the gun while firing (e.g. NS_waterfall_Tube). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Combat")
	TObjectPtr<UNiagaraSystem> MuzzleEffect;

	/** Extra rotation for the stream if the effect doesn't point forward by default. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Combat")
	FRotator StreamRotationOffset = FRotator::ZeroRotator;

	/** Offset of the stream from the gun's origin, in the gun's local space. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Combat")
	FVector StreamLocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Combat")
	TObjectPtr<USoundBase> FireSound;

	// ===================== Voice =====================

	/** What the guide says when each stage starts, the first time through. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mutateo|Voice")
	TMap<EMutateoLoopStage, FMutateoVoiceLine> FirstLoopLines;

	/** Shorter lines used on every loop after the first. Missing/empty entries say nothing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mutateo|Voice")
	TMap<EMutateoLoopStage, FMutateoVoiceLine> RepeatLoopLines;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mutateo|Voice")
	FMutateoVoiceLine MissingMaterialsLine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mutateo|Voice")
	FMutateoVoiceLine NeedWeaponLine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mutateo|Voice")
	FMutateoVoiceLine NothingReadyLine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mutateo|Voice")
	FMutateoVoiceLine NothingToDepositLine;

	// ===================== Events =====================

	UPROPERTY(BlueprintAssignable, Category = "Mutateo|Events")
	FMutateoStageChanged OnStageChanged;

	UPROPERTY(BlueprintAssignable, Category = "Mutateo|Events")
	FMutateoVoiceLineEvent OnVoiceLine;

	UPROPERTY(BlueprintAssignable, Category = "Mutateo|Events")
	FMutateoObjectiveChanged OnObjectiveChanged;

	/** Fires each time the player replants and a new loop begins. Use it for a "slice complete" screen. */
	UPROPERTY(BlueprintAssignable, Category = "Mutateo|Events")
	FMutateoLoopCompleted OnLoopCompleted;

	// ===================== Queries =====================

	UFUNCTION(BlueprintPure, Category = "Mutateo|Loop")
	EMutateoLoopStage GetStage() const { return Stage; }

	UFUNCTION(BlueprintPure, Category = "Mutateo|Loop")
	int32 GetLoopsCompleted() const { return LoopsCompleted; }

	UFUNCTION(BlueprintPure, Category = "Mutateo|Loop")
	FText GetObjectiveText() const { return CurrentObjective; }

	UFUNCTION(BlueprintPure, Category = "Mutateo|Loop")
	bool HasCraftedWeapon() const { return bWeaponCrafted; }

	UFUNCTION(BlueprintCallable, Category = "Mutateo|Loop")
	TArray<AActor*> GetPlots() const;

	UFUNCTION(BlueprintCallable, Category = "Mutateo|Loop")
	bool AnyCropPlanted() const;

	UFUNCTION(BlueprintCallable, Category = "Mutateo|Loop")
	bool AnyCropReady() const;

	UFUNCTION(BlueprintCallable, Category = "Mutateo|Loop")
	int32 GetAliveMutantCount() const;

	/** Checked by the harvest switch. If false, OutReason is the line the guide should say. */
	bool CanActivateHarvestSwitch(FMutateoVoiceLine& OutReason) const;

	// ===================== Actions =====================

	/** Queue a line for the guide. Returns seconds until it has finished playing. */
	UFUNCTION(BlueprintCallable, Category = "Mutateo|Voice")
	float PlayVoiceLine(const FMutateoVoiceLine& Line);

	/** Re-check whether the current stage is complete. Runs automatically every 0.25s. */
	UFUNCTION(BlueprintCallable, Category = "Mutateo|Loop")
	void EvaluateStage();

	/** Jump straight to a stage (for testing). */
	UFUNCTION(BlueprintCallable, Category = "Mutateo|Loop", meta = (DevelopmentOnly))
	void DebugSkipToStage(EMutateoLoopStage NewStage, bool bMarkWeaponCrafted = true);

	// Called by the stations / switch.
	void NotifyWeaponCrafted();
	void NotifyMutantsReleased(const TArray<AMutateoMutant*>& Mutants);
	void NotifyDeposited(int32 CoinsEarned);
	void NotifyUpgradeStationVisited();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void StartLoop();
	void EnterStage(EMutateoLoopStage NewStage);
	void PlayStageLine(EMutateoLoopStage ForStage);
	void PlayNextQueuedLine();
	float GetLineDuration(const FMutateoVoiceLine& Line) const;

	void RefreshHighlights();
	void ClearHighlights();
	void RefreshObjective();
	FText BuildObjectiveText() const;

	bool HasCollectedStarter() const;
	bool AreFetchRequirementsMet() const;
	APawn* GetPlayerPawn() const;

	void SetupDefaultLines();
	void BindCombatInput();
	void HandleAttackInput();
	void FireTick();
	void FireOnce();
	void StopFiring();
	void DespawnStream();
	void CleanupImpactEffects();
	FTimerHandle StreamDespawnTimer;
	FTimerHandle ImpactCleanupTimer;
	TArray<TPair<TWeakObjectPtr<class UNiagaraComponent>, float>> SpawnedImpacts;
	AActor* GetEquippedWeapon() const;
	float LastImpactFxTime = -100.f;
	FTimerHandle FireTimer;

	UPROPERTY(Transient)
	TObjectPtr<class UNiagaraComponent> StreamComponent;

	EMutateoLoopStage Stage = EMutateoLoopStage::NotStarted;
	int32 LoopsCompleted = 0;
	bool bWeaponCrafted = false;
	FText CurrentObjective;

	TArray<TWeakObjectPtr<AMutateoMutant>> ActiveMutants;
	TArray<TWeakObjectPtr<AActor>> HighlightedActors;
	TArray<FMutateoVoiceLine> VoiceQueue;

	UPROPERTY(Transient)
	TObjectPtr<UMutateoSubtitleWidget> SubtitleWidget;

	FTimerHandle StartTimer;
	FTimerHandle PollTimer;
	FTimerHandle VoiceTimer;
	FTimerHandle IntroTimer;
};
