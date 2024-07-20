// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "BasePlatforms.generated.h"

UENUM(BlueprintType)
enum class EPlatformBehaviors : uint8
{
	OnDemand = 0,
	Loop
};

class APlatformTrigger;
class UStaticMeshComponent;

UCLASS()
class GAMECODE_API ABasePlatforms : public AActor
{
	GENERATED_BODY()
	
public:	
	ABasePlatforms();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void MovePlatform();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform movement")
	UStaticMeshComponent* PlatformMesh;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Platform movement")
	APlatformTrigger* PlatformTrigger;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform movement")
	UCurveFloat* MovementCurve;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Platform movement", meta = (MakeEditWidget))
	FVector EndPlatformLocation;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Platform movement", meta = (MakeEditWidget))
	float ReturnTime = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform movement")
	EPlatformBehaviors PlatformBehavior;

	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnPlatformTriggerActivated(bool bIsActivated);

	UFUNCTION()
	void TickPlatformTimeline(float Value);

	UFUNCTION()
	void OnPlatfromEndReached();
	FTimeline PlatformTimeline;

	FVector StartLocation = FVector::ZeroVector;
	FVector EndLocation = FVector::ZeroVector;
	bool bIsMovingForward = false;

	FTimerHandle ReturnTimer;
};
