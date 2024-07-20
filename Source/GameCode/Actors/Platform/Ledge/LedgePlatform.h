// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LedgePlatform.generated.h"

UCLASS()
class GAMECODE_API ALedgePlatform : public AActor
{
	GENERATED_BODY()

public:
	ALedgePlatform();

	bool IsFinalPlatform() const {return bIsFinalUpPlatform; };
	
public:
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	UStaticMeshComponent* PlatformMesh;

	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	USceneComponent* PositionForCharacter;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsFinalUpPlatform = false;

};
