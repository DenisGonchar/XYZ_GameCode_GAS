// Fill out your copyright notice in the Description page of Project Settings.


#include "APlatformInvocator.h"


void AAPlatformInvocator::Invoke()
{


	if (OnInvocatorActivated.IsBound())
	{
		OnInvocatorActivated.Broadcast();
		GEngine->AddOnScreenDebugMessage(-1,2.0f,FColor::White,TEXT("Start"));
	}
}

