// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "equiptmentdefinition.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType, Abstract, Const)
class MUTATEOFORGITHUB_API Uequiptmentdefinition : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "equiptmentactor")
	TSubclassOf<AActor> equiptmentactorclass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "equiptmentactor")
	FName attachsocketname;
};
