// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "equiptmentdefinition.h"
#include "mutateoforgithub/items/iteminstance.h"
#include "UObject/Object.h"
#include "equiptmentinstance.generated.h"


UCLASS(BlueprintType)
class MUTATEOFORGITHUB_API Uequiptmentinstance : public UObject
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(BlueprintReadOnly)
	TSubclassOf<Uequiptmentdefinition> equiptmentdefinition;
	
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> spawnequiptactor;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<Uiteminstance> sourceiteminstance;
	
	UFUNCTION(BlueprintCallable)
	void Initialize(Uiteminstance* iteminstance);
	
	UFUNCTION(BlueprintCallable)
	void handleequipitem(ACharacter* character);
	
	UFUNCTION(BlueprintCallable)
	void handleunequipitem(ACharacter* character);
};
