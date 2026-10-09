// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "inventoryitemfragments.generated.h"

class Uiteminstance;
/**
 * 
 */
UCLASS(Blueprintable, BlueprintType, Abstract, DefaultToInstanced, EditInlineNew)
class MUTATEOFORGITHUB_API Uinventoryitemfragments : public UObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintNativeEvent)
	void OnInstanceCreated(Uiteminstance* ItemInstance);
};

inline void Uinventoryitemfragments::OnInstanceCreated_Implementation(Uiteminstance* ItemInstance) {}

