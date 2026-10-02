// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "itemdefinition.generated.h"
class Uinventoryitemfragments;
/**
 * 
 */
UCLASS(Blueprintable, BlueprintType, Abstract, Const)
class MUTATEOFORGITHUB_API Uitemdefinition : public UObject
{
	GENERATED_BODY()
public:


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display")
	FText ItemName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display")
	FText ItemDescription;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display")
	TObjectPtr<UTexture2D> ItemIcon;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "fragarray")
	TArray<TObjectPtr<Uinventoryitemfragments>> Fragments;

UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DeterminesOutputType = "fragmentclass"))
	static const Uinventoryitemfragments* findfragmentbyclass(const TSubclassOf<Uitemdefinition> itemdefinition, const TSubclassOf<Uinventoryitemfragments> FragmentClass);
	
};
