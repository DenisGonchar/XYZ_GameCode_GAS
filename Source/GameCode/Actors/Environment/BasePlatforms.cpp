// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePlatforms.h"
#include "Components/StaticMeshComponent.h"
#include "Actors/Environment/PlatformTrigger.h"

ABasePlatforms::ABasePlatforms()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
	SetRootComponent(PlatformMesh);
}

void ABasePlatforms::BeginPlay()
{
	Super::BeginPlay();
	if (IsValid(MovementCurve))
	{
		FOnTimelineFloatStatic OnTimelineUpdate;
		OnTimelineUpdate.BindUFunction(this, FName("TickPlatformTimeline"));
		PlatformTimeline.AddInterpFloat(MovementCurve, OnTimelineUpdate);

		FOnTimelineEventStatic OnTimelineFinished;
		OnTimelineFinished.BindUFunction(this, FName("OnPlatfromEndReached"));
		PlatformTimeline.SetTimelineFinishedFunc(OnTimelineFinished);
	}

	if (PlatformBehavior == EPlatformBehaviors::Loop)
	{
		MovePlatform();
	}

	StartLocation = GetActorLocation();
	EndLocation = GetActorTransform().TransformPosition(EndPlatformLocation);

	if (IsValid(PlatformTrigger))
	{
		PlatformTrigger->OnTriggerActivated.AddDynamic(this, &ABasePlatforms::OnPlatformTriggerActivated);
	}
}

void ABasePlatforms::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	PlatformTimeline.TickTimeline(DeltaTime);
}

void ABasePlatforms::MovePlatform()
{
	if (bIsMovingForward)
	{
		PlatformTimeline.Reverse();
		bIsMovingForward = false;
	}
	else
	{
		PlatformTimeline.Play();
		bIsMovingForward = true;
	}
}

void ABasePlatforms::OnPlatformTriggerActivated(bool bIsActivated)
{
	MovePlatform();
}

void ABasePlatforms::TickPlatformTimeline(float Value)
{
	FVector NewLocation = FMath::Lerp(StartLocation, EndLocation, Value); 
	SetActorLocation(NewLocation);
}

void ABasePlatforms::OnPlatfromEndReached()
{
	if (PlatformBehavior == EPlatformBehaviors::Loop)
	{
		MovePlatform();
	}
	else if (ReturnTime > 0.0f )
	{
		GetWorld()->GetTimerManager().SetTimer(ReturnTimer, this, &ABasePlatforms::MovePlatform, ReturnTime, false);
		return;
	}
}

