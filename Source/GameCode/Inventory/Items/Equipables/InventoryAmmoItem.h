// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/Items/InventoryItem.h"
#include "InventoryAmmoItem.generated.h"

/**
 * 
 */
UCLASS()
class GAMECODE_API UInventoryAmmoItem : public UInventoryItem
{
	GENERATED_BODY()

public:
	virtual bool Consume(AGCBaseCharacter* ConsumeTarget) override;

	void AddAmmunition(AGCBaseCharacter* Character);

	void RemoveAmmunition(AGCBaseCharacter* Character);
	
	void SetAmmoCount(int32 Count);
	int32 GetAmmoCount() const {return AmmoCount;}

	EAmmunitionType GetAmmunitionType() const {return AmmunitionType; }
	void SetAmmunitionType(EAmmunitionType Ammunition);

	void SetIsAmmoVisibility( bool Visibility);
	
protected:
	int32 AmmoCount = 1;

	EAmmunitionType AmmunitionType;

	bool bIsAmmoVisibility = false;
};
