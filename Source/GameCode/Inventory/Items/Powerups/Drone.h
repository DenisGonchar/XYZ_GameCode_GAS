// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/Items/InventoryItem.h"
#include "Drone.generated.h"

class AGCBaseDrone;

UCLASS()
class GAMECODE_API UDrone : public UInventoryItem
{
	GENERATED_BODY()

public:
	virtual bool Consume(AGCBaseCharacter* ConsumeTarget) override;

	TSubclassOf<AGCBaseDrone> GetDrone() const;

	UAnimMontage* GetCharacterEquipAnimMontage() const;
protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TSubclassOf<AGCBaseDrone> Drone;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	UAnimMontage* CharacterEquipAnimMontage;
	
};
