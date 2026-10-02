// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "inventoryitemfragments.h"
#include "inventoryitemfragment_worldrep.generated.h"

/**
 * 
 */
UCLASS()

class MUTATEOFORGITHUB_API Uinventoryitemfragment_worldrep : public Uinventoryitemfragments
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "mesh")
	TObjectPtr<UStaticMesh> Itemmesh;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "options")
	bool bcanbedropped = true;
};
