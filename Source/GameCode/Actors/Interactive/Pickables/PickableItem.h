// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Interface/Interactive.h"
#include "PickableItem.generated.h"

class IInteractable;

UCLASS(Abstract, NotBlueprintable)
class GAMECODE_API APickableItem : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:	
	const FName& GetDataTableID() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FName DataTableID = NAME_None;
	

};
