// Fill out your copyright notice in the Description page of Project Settings.


#include "mutateoforgithub/items/itemdefinition.h"

#include "mutateoforgithub/itemdeffragments/inventoryitemfragments.h"

const Uinventoryitemfragments* Uitemdefinition::findfragmentbyclass(const TSubclassOf<Uitemdefinition> itemdefinition,
                                                                    const TSubclassOf<Uinventoryitemfragments> FragmentClass)
{
	if (itemdefinition && FragmentClass)
	{
		Uitemdefinition* ItemCDO = itemdefinition.GetDefaultObject();
		for (const TObjectPtr<Uinventoryitemfragments>& fragment : ItemCDO->Fragments)
		{
			if (fragment->IsA(FragmentClass))
			{
				return fragment;
			}
		}
	}
	return nullptr;
}
