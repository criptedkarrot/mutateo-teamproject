// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "mutateoforgithub/itemdeffragments/inventoryitemfragments.h"
#include "UObject/Object.h"
#include "iteminstance.generated.h"

class Uitemdefinition;
/**
 * 
 */
UCLASS(BlueprintType)
class MUTATEOFORGITHUB_API Uiteminstance : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly)
	TSubclassOf<Uitemdefinition> itemdefinition;
	
	UPROPERTY(BlueprintReadOnly)
	TMap<FGameplayTag, float> statsmap;
	
	UFUNCTION(blueprintCallable, BlueprintPure, Category = "item stats")
	float GetStatvalue(FGameplayTag stattag);
	
	UFUNCTION(blueprintCallable, Category = "item stats")
	void setstatvalue (FGameplayTag stattag, float statvalue);
	
	UFUNCTION(BlueprintCallable, Category = "item instance", meta = (AutoCreateRefTerm = "initialstats"))
	void initialize(TSubclassOf<Uitemdefinition> itemdef);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, meta= (DeterminesOutputType = "fragmentclass"))
	const Uinventoryitemfragments* FindFragmentByClass(const TSubclassOf<Uinventoryitemfragments> Fragmentclass);
};
