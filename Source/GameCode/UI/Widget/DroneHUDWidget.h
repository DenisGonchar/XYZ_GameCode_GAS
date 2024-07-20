// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DroneHUDWidget.generated.h"

class UReticleWidget;

UCLASS()
class GAMECODE_API UDroneHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UReticleWidget* GetReticalWidget();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget names")
	FName ReticleWidgetName;

};
