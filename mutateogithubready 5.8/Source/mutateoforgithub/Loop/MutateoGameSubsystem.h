// Coins and upgrade levels for the current level. Reach it in Blueprint with "Get Mutateo Game Subsystem".

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MutateoLoopTypes.h"
#include "MutateoGameSubsystem.generated.h"

class AMutateoLoopDirector;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMutateoCoinsChanged, int32, NewCoins);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMutateoUpgradePurchased, FName, UpgradeId, int32, NewLevel);

UCLASS()
class MUTATEOFORGITHUB_API UMutateoGameSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Mutateo|Economy")
	FMutateoCoinsChanged OnCoinsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Mutateo|Economy")
	FMutateoUpgradePurchased OnUpgradePurchased;

	// ---- Coins ----
	UFUNCTION(BlueprintPure, Category = "Mutateo|Economy")
	int32 GetCoins() const { return Coins; }

	UFUNCTION(BlueprintCallable, Category = "Mutateo|Economy")
	void AddCoins(int32 Amount);

	/** Returns false (and spends nothing) if there aren't enough coins. */
	UFUNCTION(BlueprintCallable, Category = "Mutateo|Economy")
	bool SpendCoins(int32 Amount);

	// ---- Upgrades ----
	void RegisterUpgrades(const TArray<FMutateoUpgradeDef>& Defs);

	UFUNCTION(BlueprintPure, Category = "Mutateo|Upgrades")
	int32 GetUpgradeLevel(FName UpgradeId) const;

	/** BaseValue + ValuePerLevel * Level. Returns Fallback if no upgrade with this Id exists. */
	UFUNCTION(BlueprintPure, Category = "Mutateo|Upgrades")
	float GetUpgradeValue(FName UpgradeId, float Fallback = 1.f) const;

	/** Cost of the next level, or -1 if maxed or unknown. */
	UFUNCTION(BlueprintPure, Category = "Mutateo|Upgrades")
	int32 GetUpgradeCost(FName UpgradeId) const;

	UFUNCTION(BlueprintPure, Category = "Mutateo|Upgrades")
	bool IsUpgradeMaxed(FName UpgradeId) const;

	UFUNCTION(BlueprintPure, Category = "Mutateo|Upgrades")
	bool GetUpgradeDefinition(FName UpgradeId, FMutateoUpgradeDef& OutDefinition) const;

	/** Spends coins and raises the level. Prefer UpgradeStation->PurchaseUpgrade, which also applies side effects. */
	UFUNCTION(BlueprintCallable, Category = "Mutateo|Upgrades")
	bool TryPurchaseUpgrade(FName UpgradeId);

	// ---- Director ----
	void SetDirector(AMutateoLoopDirector* InDirector);

	UFUNCTION(BlueprintPure, Category = "Mutateo|Loop")
	AMutateoLoopDirector* GetDirector() const;

private:
	int32 Coins = 0;
	TMap<FName, FMutateoUpgradeDef> Upgrades;
	TMap<FName, int32> Levels;
	TWeakObjectPtr<AMutateoLoopDirector> Director;
};
