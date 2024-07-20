// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/GCDataTableUtils.h"
#include "Engine/DataTable.h"
#include <Inventory/Items/InventoryItem.h>


FWeaponTableRow* GCDataTableUtils::FindWeaponData(const FName WeaponID)
{
	static const FString contextString(TEXT("Find Weapon Data"));

	UDataTable* WeaponDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/GameCode/Core/Data/DataTables/DT_WeaponList.DT_WeaponList"));
	
	if (WeaponDataTable == nullptr)
	{
		return nullptr;
	}
	return WeaponDataTable->FindRow<FWeaponTableRow>(WeaponID, contextString);
}

FItemTableRow* GCDataTableUtils::FindInventoryItemData(const FName ItemID)
{
	static const FString contextString(TEXT("Find Item Data"));

	UDataTable* InventoryDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/GameCode/Core/Data/DataTables/DT_InventoryItemList.DT_InventoryItemList"));

	if (InventoryDataTable == nullptr)
	{
		return nullptr;
	}
	return InventoryDataTable->FindRow<FItemTableRow>(ItemID, contextString);
}

FAmmoTableRow* GCDataTableUtils::FindAmmoItemData(const FName ItemID)
{
	static const FString contextString(TEXT("Find Ammo Item Data"));

	UDataTable* AmmoInventoryDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/GameCode/Core/Data/DataTables/DT_AmmoItemList.DT_AmmoItemList"));

	if (AmmoInventoryDataTable == nullptr)
	{
		return nullptr;
	}
	return AmmoInventoryDataTable->FindRow<FAmmoTableRow>(ItemID, contextString);
	
}
