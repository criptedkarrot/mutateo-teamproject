// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemDefinition.h"
#include "mutateoforgithub//Items/ItemDefinition.h"
#include "mutateoforgithub//Fragments/InventoryItemFragment.h"
#include "mutateoforgithub/Fragments/InventoryItemFragment.h"

class UItemDefinition;
class UInventoryItemFragment;

const UInventoryItemFragment* UItemDefinition::FindFragmentByClass(const TSubclassOf<UItemDefinition> ItemDefinition,
                                                                   const TSubclassOf<UInventoryItemFragment> FragmentClass)
{
	if (ItemDefinition && FragmentClass)
	{
		UItemDefinition* ItemCDO = ItemDefinition.GetDefaultObject();
		
		for (const TObjectPtr<UInventoryItemFragment>& Fragment : ItemCDO->Fragments)
		{
			if (Fragment->IsA(FragmentClass))
			{
				return Fragment;
			}
		}
	}
	return nullptr;
}
