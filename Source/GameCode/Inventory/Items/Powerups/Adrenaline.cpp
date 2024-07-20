// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Items/Powerups/Adrenaline.h"
#include "Pawns/Character/GCBaseCharacter.h"
#include "Components/CharacterComponents/CharacterAttributeComponent.h"

bool UAdrenaline::Consume(AGCBaseCharacter* ConsumeTarget)
{
	UCharacterAttributeComponent* CharacterAttributes = ConsumeTarget->GetCharacterAttributeComponent_Muteble();
	CharacterAttributes->RestoreFullStamina();
	this->ConditionalBeginDestroy();

	return true;
}
