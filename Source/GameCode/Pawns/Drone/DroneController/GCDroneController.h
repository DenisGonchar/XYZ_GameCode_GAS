// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GCDroneController.generated.h"

/**
 * 
 */
UCLASS()
class GAMECODE_API AGCDroneController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void SetPawn(APawn* InPawn) override;

	virtual void SetupInputComponent() override;

private:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void MoveUp(float Value);
	void Turn(float Value);
	void LookUp(float Value);

	void Detonation();
	
private:
	TSoftObjectPtr<class AGCBaseDrone> CachedBaseDrone;

};
