// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GrenadeWidget.generated.h"

/**
 * 
 */
UCLASS()
class GAMECODE_API UGrenadeWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo")
	int32 Ammo;

	UFUNCTION()
	void UpdateGrenadeCount(int32 NewAmmo);

protected:

	
};
