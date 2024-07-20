// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePlatform.h"
#include "../Platform/APlatformInvocator.h"

// Sets default values
ABasePlatform::ABasePlatform()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* DefaultPlatformRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Platform root"));
	RootComponent = DefaultPlatformRoot;

	PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Platform"));
	PlatformMesh->SetupAttachment(DefaultPlatformRoot);
	
	
}

// Called when the game starts or when spawned
void ABasePlatform::BeginPlay()
{
	Super::BeginPlay();
	
	StartLocation = PlatformMesh->GetRelativeLocation();

	if (IsValid(TimelineCurve))
	{
		FOnTimelineFloatStatic PlatformMovementTimelineUpdate;
		PlatformMovementTimelineUpdate.BindUObject(this, &ABasePlatform::PlatformTimelineUpdate);
		PlatformTimeline.AddInterpFloat(TimelineCurve, PlatformMovementTimelineUpdate);
		
	}

	if (IsValid(PlatformInvocator))
	{
	
	PlatformInvocator->OnInvocatorActivated.AddUObject(this, &ABasePlatform::MovePlatform);
	
	}
	
}

// Called every frame
void ABasePlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PlatformTimeline.TickTimeline(DeltaTime);
	
	TickLoop(DeltaTime);
}


void ABasePlatform::TickLoop(float DeltaTime)
{
	if (PlatformBehavior == EPlatformBehavior::Loop)
	{
		
		if(PlatformTimeline.GetPlaybackPosition() == PlatformTimeline.GetTimelineLength())
		{
			PlatformTimelineLoop();
		}
	}
	
}


void ABasePlatform::PlatformTimelineUpdate(float Alpha)
{
	const FVector PlatformTargetLocation = FMath::Lerp(StartLocation, EndLocation, Alpha);
	PlatformMesh->SetRelativeLocation(PlatformTargetLocation);
}


void ABasePlatform::PlatformTimelineOnDemand()
{
	PlatformTimeline.Play();

}

void ABasePlatform::PlatformTimelineLoop()
{
	if (PlatformTimeline.GetPlaybackPosition() == 0.0f)
	{
		PlatformTimeline.Play();
	}
	else if(PlatformTimeline.GetPlaybackPosition() == PlatformTimeline.GetTimelineLength())
	{
		PlatformTimeline.Reverse();
	}

}

void ABasePlatform::MovePlatform()
{
		if (PlatformBehavior == EPlatformBehavior::OnDemand)
		{
			PlatformTimelineOnDemand();
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("OnDemand"));
		}
		else if (PlatformBehavior == EPlatformBehavior::Loop)
		{
			PlatformTimelineLoop();
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Loop"));
		}
}


