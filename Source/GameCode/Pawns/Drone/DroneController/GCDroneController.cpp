// Fill out your copyright notice in the Description page of Project Settings.


#include "GCDroneController.h"

#include "Pawns/Drone/GCBaseDrone.h"

void AGCDroneController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);
	
	CachedBaseDrone = Cast<AGCBaseDrone>(InPawn);
	
}

void AGCDroneController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAxis("Drone_MoveForward", this, &AGCDroneController::MoveForward);
	InputComponent->BindAxis("Drone_MoveRight", this, &AGCDroneController::MoveRight);
	InputComponent->BindAxis("Drone_MoveUp", this, &AGCDroneController::MoveUp);
	InputComponent->BindAxis("Turn", this, &AGCDroneController::Turn);
	InputComponent->BindAxis("LookUp", this, &AGCDroneController::LookUp);

	InputComponent->BindAction("Drone_Detonation", IE_Pressed, this, &AGCDroneController::Detonation);
}

void AGCDroneController::MoveForward(float Value)
{
	if (CachedBaseDrone.IsValid())
	{
		CachedBaseDrone->MoveForward(Value);
	}
}

void AGCDroneController::MoveRight(float Value)
{
	if (CachedBaseDrone.IsValid())
	{
		CachedBaseDrone->MoveRight(Value);
	}
}

void AGCDroneController::MoveUp(float Value)
{
	if (CachedBaseDrone.IsValid())
	{
		CachedBaseDrone->MoveUp(Value);
	}
}

void AGCDroneController::Turn(float Value)
{
	if (CachedBaseDrone.IsValid())
	{
		CachedBaseDrone->Turn(Value);
	}
}

void AGCDroneController::LookUp(float Value)
{
	if (CachedBaseDrone.IsValid())
	{
		CachedBaseDrone->LookUp(Value);
	}
}

void AGCDroneController::Detonation()
{
	if (CachedBaseDrone.IsValid())
	{
		CachedBaseDrone->Detonation();
	}
}
