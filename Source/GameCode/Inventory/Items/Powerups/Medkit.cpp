// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Items/Powerups/Medkit.h"
#include "Pawns/Character/GCBaseCharacter.h"
#include "Components/CharacterComponents/CharacterAttributeComponent.h"

bool UMedkit::Consume(AGCBaseCharacter* ConsumeTarget)
{
	UCharacterAttributeComponent* CharacterAttributes = ConsumeTarget->GetCharacterAttributeComponent_Muteble();
	CharacterAttributes->AddHealth(Health);
	this->ConditionalBeginDestroy();

	return true;
}
