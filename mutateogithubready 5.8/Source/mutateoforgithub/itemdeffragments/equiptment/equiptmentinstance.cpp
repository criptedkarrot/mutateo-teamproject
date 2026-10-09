// Fill out your copyright notice in the Description page of Project Settings.


#include "equiptmentinstance.h"
#include "mutateoforgithub/items/iteminstance.h"



void Uequiptmentinstance::Initialize(Uiteminstance* iteminstance)
{
	if (iteminstance) return;
	
	sourceiteminstance = iteminstance;
	
	iteminstance->FindFragmentByClass(Uinventoryitemfragments::StaticClass()
}

void Uequiptmentinstance::handleequipitem(ACharacter* character)
{
}

void Uequiptmentinstance::handleunequipitem(ACharacter* character)
{
}
