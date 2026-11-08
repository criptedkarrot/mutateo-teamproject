#include "MutateoGameSubsystem.h"

#include "MutateoLoopDirector.h"

void UMutateoGameSubsystem::SetDirector(AMutateoLoopDirector* InDirector)
{
	Director = InDirector;
}

AMutateoLoopDirector* UMutateoGameSubsystem::GetDirector() const
{
	return Director.Get();
}

void UMutateoGameSubsystem::AddCoins(int32 Amount)
{
	if (Amount == 0) return;
	Coins = FMath::Max(0, Coins + Amount);
	OnCoinsChanged.Broadcast(Coins);
}

bool UMutateoGameSubsystem::SpendCoins(int32 Amount)
{
	if (Amount < 0 || Coins < Amount) return false;
	Coins -= Amount;
	OnCoinsChanged.Broadcast(Coins);
	return true;
}

void UMutateoGameSubsystem::RegisterUpgrades(const TArray<FMutateoUpgradeDef>& Defs)
{
	for (const FMutateoUpgradeDef& Def : Defs)
	{
		if (Def.Id.IsNone()) continue;
		Upgrades.Add(Def.Id, Def);
		Levels.FindOrAdd(Def.Id);
	}
}

int32 UMutateoGameSubsystem::GetUpgradeLevel(FName UpgradeId) const
{
	return Levels.FindRef(UpgradeId);
}

float UMutateoGameSubsystem::GetUpgradeValue(FName UpgradeId, float Fallback) const
{
	const FMutateoUpgradeDef* Def = Upgrades.Find(UpgradeId);
	if (!Def) return Fallback;
	return Def->BaseValue + Def->ValuePerLevel * GetUpgradeLevel(UpgradeId);
}

int32 UMutateoGameSubsystem::GetUpgradeCost(FName UpgradeId) const
{
	const FMutateoUpgradeDef* Def = Upgrades.Find(UpgradeId);
	if (!Def) return -1;
	const int32 Level = GetUpgradeLevel(UpgradeId);
	if (Level >= Def->MaxLevel) return -1;
	return FMath::RoundToInt(Def->BaseCost * FMath::Pow(Def->CostGrowth, static_cast<float>(Level)));
}

bool UMutateoGameSubsystem::IsUpgradeMaxed(FName UpgradeId) const
{
	const FMutateoUpgradeDef* Def = Upgrades.Find(UpgradeId);
	return !Def || GetUpgradeLevel(UpgradeId) >= Def->MaxLevel;
}

bool UMutateoGameSubsystem::GetUpgradeDefinition(FName UpgradeId, FMutateoUpgradeDef& OutDefinition) const
{
	if (const FMutateoUpgradeDef* Def = Upgrades.Find(UpgradeId))
	{
		OutDefinition = *Def;
		return true;
	}
	return false;
}

bool UMutateoGameSubsystem::TryPurchaseUpgrade(FName UpgradeId)
{
	const int32 Cost = GetUpgradeCost(UpgradeId);
	if (Cost < 0 || Coins < Cost) return false;

	// Raise the level before broadcasting the coin change so listeners (the menu) see the new level.
	int32& Level = Levels.FindOrAdd(UpgradeId);
	++Level;
	Coins -= Cost;
	OnCoinsChanged.Broadcast(Coins);
	OnUpgradePurchased.Broadcast(UpgradeId, Level);
	return true;
}
