// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BowstringWidget.generated.h"

/**
 * 
 */
UCLASS()
class GAMECODE_API UBowstringWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	
	void SetBowstring(bool NewBowstring, float NewTension);

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsBowstring = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Tension = 0.0f;
	
};
