// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "mutateoforgithub/itemdeffragments/inventoryitemfragments.h"
#include "mutateoforgithub/itemdeffragments/equiptment/equiptmentinstance.h"
#include "inventoryfragmentequipable.generated.h"

class Uequiptmentdefinition;


UCLASS()
class MUTATEOFORGITHUB_API Uinventoryfragmentequipable : public Uinventoryitemfragments
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "equiptment")
	TSubclassOf<Uequiptmentdefinition> equiptmentdefinition;
};
