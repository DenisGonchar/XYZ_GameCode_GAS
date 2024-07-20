// Fill out your copyright notice in the Description page of Project Settings.


#include "Drone.h"

bool UDrone::Consume(AGCBaseCharacter* ConsumeTarget)
{
	return false;
	
}

TSubclassOf<AGCBaseDrone> UDrone::GetDrone() const
{
	return Drone;
}

UAnimMontage* UDrone::GetCharacterEquipAnimMontage() const
{
	return CharacterEquipAnimMontage;
}


