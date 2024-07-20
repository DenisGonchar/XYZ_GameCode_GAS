// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GCLimbActor.generated.h"

class USceneComponent;
class USkeletalMeshComponent;

UCLASS()
class GAMECODE_API AGCLimbActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AGCLimbActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* TransformComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USkeletalMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	AActor* PrototyActor;

	USkeletalMeshComponent* GetMeshComponent() const { return MeshComponent; };

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

};
