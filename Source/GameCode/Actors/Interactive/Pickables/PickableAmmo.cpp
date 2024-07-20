// Fill out your copyright notice in the Description page of Project Settings.


#include "PickableAmmo.h"

#include "Inventory/Items/InventoryItem.h"
#include "Inventory/Items/Equipables/InventoryAmmoItem.h"
#include "Pawns/Character/GCBaseCharacter.h"
#include "Utils/GCDataTableUtils.h"


// Sets default values
APickableAmmo::APickableAmmo()
{
	AmmoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AmmoMesh"));
	SetRootComponent(AmmoMesh);

}

void APickableAmmo::Interact(AGCBaseCharacter* Character)
{
	FAmmoTableRow* AmmoRow = GCDataTableUtils::FindAmmoItemData(DataTableID);
	if (AmmoRow)
	{
		TWeakObjectPtr<UInventoryAmmoItem> Item = NewObject<UInventoryAmmoItem>(Character);
		Item->Initialize(DataTableID, AmmoRow->InventoryItemDescription);
		Item->SetAmmoCount(AmmoCount);
		Item->SetAmmunitionType(AmmoRow->AmunitionType);
		
		const bool bPickedUp = Character->PickupItem(Item, AmmoCount);
		if (bPickedUp)
		{
			Item->AddAmmunition(Character);
			
			Destroy();
		}
	}
}

FName APickableAmmo::GetActionEventName() const
{
	return ActionInteract; 
}

void APickableAmmo::SetAmmoCount(int32 NewCount)
{
	AmmoCount = NewCount;
}
