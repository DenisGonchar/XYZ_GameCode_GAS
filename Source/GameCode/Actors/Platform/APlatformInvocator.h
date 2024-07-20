// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "APlatformInvocator.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnInvocatorActivated);

UCLASS()
class GAMECODE_API AAPlatformInvocator : public AActor
{
	GENERATED_BODY()
	
public:
	
	//UPROPERTY(BlueprintAssignable)
	FOnInvocatorActivated OnInvocatorActivated;
	
	UFUNCTION(BlueprintCallable)
	void Invoke();
	

	
};
