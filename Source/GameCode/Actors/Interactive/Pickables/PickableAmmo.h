// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickableItem.h"
#include "PickableAmmo.generated.h"

UCLASS(Blueprintable)
class GAMECODE_API APickableAmmo : public APickableItem
{
	GENERATED_BODY()

public:
	APickableAmmo();

	virtual void Interact(AGCBaseCharacter* Character) override;

	virtual FName GetActionEventName() const override;

	int32 GetAmmoCount() const { return AmmoCount; }

	void SetAmmoCount(int32 NewCount);
protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* AmmoMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 AmmoCount = 1.0f;
	
};
