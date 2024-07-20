// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryAmmoItem.h"

#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "Pawns/Character/GCBaseCharacter.h"

bool UInventoryAmmoItem::Consume(AGCBaseCharacter* ConsumeTarget)
{
	return false;
}

void UInventoryAmmoItem::AddAmmunition(AGCBaseCharacter* Character)
{
	UCharacterEquipmentComponent* CharacterEquipment = Character->GetCharacterEquipmentComponent_Muteble();
	CharacterEquipment->AddAmmunitionAmmo(AmmunitionType, AmmoCount);
	this->ConditionalBeginDestroy();

}

void UInventoryAmmoItem::RemoveAmmunition(AGCBaseCharacter* Character)
{
	UCharacterEquipmentComponent* CharacterEquipment = Character->GetCharacterEquipmentComponent_Muteble();
	CharacterEquipment->RemoveAmmunitionAmmo(AmmunitionType, AmmoCount);
	this->ConditionalBeginDestroy();
}

void UInventoryAmmoItem::SetAmmoCount(int32 Count)
{
	AmmoCount = Count;
}

void UInventoryAmmoItem::SetAmmunitionType(EAmmunitionType Ammunition)
{
	AmmunitionType = Ammunition;
}

void UInventoryAmmoItem::SetIsAmmoVisibility(bool Visibility)
{
	bIsAmmoVisibility = Visibility;
}
