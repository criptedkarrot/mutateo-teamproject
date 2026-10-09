// Fill out your copyright notice in the Description page of Project Settings.


#include "iteminstance.h"
#include "mutateoforgithub/itemdeffragments/inventoryitemfragments.h"
#include "mutateoforgithub/items/itemdefinition.h"

float Uiteminstance::GetStatvalue(FGameplayTag stattag)
{
	return statsmap.FindRef(stattag);
}

void Uiteminstance::setstatvalue(FGameplayTag stattag, float statvalue)
{
	statsmap.Add(stattag, statvalue);
}

void Uiteminstance::initialize(TSubclassOf<Uitemdefinition> itemdef)
{
	if (!itemdef) return; 
	
	itemdefinition = itemdef;
	
	Uitemdefinition* ItemCDO = itemdefinition.GetDefaultObject();
	
	for (const TObjectPtr<Uinventoryitemfragments>& Fragment : ItemCDO->Fragments)
	{
		Fragment->OnInstanceCreated(this);
	}
}
{
	const TSubclassOf<Uinventoryitemfragments> Fragmentclass);

}
{
}



