#include "MutateoDepositStation.h"

#include "MutateoGameSubsystem.h"
#include "MutateoLoopDirector.h"
#include "MutateoLoopLibrary.h"

#define LOCTEXT_NAMESPACE "MutateoLoop"

AMutateoDepositStation::AMutateoDepositStation()
{
	StationPrompt = LOCTEXT("DepositPrompt", "Deposit");
}

bool AMutateoDepositStation::HandleUse(AActor* User)
{
	int32 Coins = 0;
	int32 Sold = 0;

	for (const FMutateoSellableItem& Entry : SellableItems)
	{
		if (!Entry.Item) continue;
		const int32 Have = UMutateoLoopLibrary::GetInventoryItemCount(User, Entry.Item);
		const int32 ToSell = Have - Entry.KeepForReplanting;
		if (ToSell <= 0) continue;

		if (UMutateoLoopLibrary::ConsumeInventoryItems(User, Entry.Item, ToSell))
		{
			Sold += ToSell;
			Coins += ToSell * Entry.CoinValue;
		}
	}

	AMutateoLoopDirector* Director = GetDirector();
	if (Sold == 0)
	{
		if (Director) Say(Director->NothingToDepositLine);
		return false;
	}

	if (UMutateoGameSubsystem* Subsystem = UMutateoLoopLibrary::GetMutateoSubsystem(this))
	{
		Subsystem->AddCoins(Coins);
	}
	OnDeposited(Coins, Sold);
	if (Director) Director->NotifyDeposited(Coins);
	return true;
}

#undef LOCTEXT_NAMESPACE
