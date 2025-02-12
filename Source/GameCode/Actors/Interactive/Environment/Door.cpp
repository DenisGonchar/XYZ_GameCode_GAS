// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Interactive/Environment/Door.h"
#include "Components/TimelineComponent.h"
#include "GameCodeTypes.h"

ADoor::ADoor()
{	
	USceneComponent* DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DoorRoot"));
	SetRootComponent(DefaultSceneRoot);

	DoorPivot = CreateDefaultSubobject<USceneComponent>(TEXT("DoorPivot"));
	DoorPivot->SetupAttachment(GetRootComponent());

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(DoorPivot);

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void ADoor::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsValid(DoorAnimationCurve))
	{
		FOnTimelineFloatStatic DoorAnimationDelegate;
		DoorAnimationDelegate.BindUObject(this, &ADoor::UpdateDoorAnimation);
		DoorOpenAnimTimeline.AddInterpFloat(DoorAnimationCurve, DoorAnimationDelegate);

		FOnTimelineEventStatic DoorOpenedDelegate;
		DoorOpenedDelegate.BindUObject(this, &ADoor::OnDoorAnimationFinished);
		DoorOpenAnimTimeline.SetTimelineFinishedFunc(DoorOpenedDelegate);

	}

}

void ADoor::InteractWithDoor()
{
	SetActorTickEnabled(true);
	if (bIsOpnened)
	{
		DoorOpenAnimTimeline.Reverse();
	}
	else
	{
		DoorOpenAnimTimeline.Play();
	}

	bIsOpnened = !bIsOpnened;

}

void ADoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DoorOpenAnimTimeline.TickTimeline(DeltaTime);

}

void ADoor::Interact(AGCBaseCharacter* Character)
{
	ensureMsgf(IsValid(DoorAnimationCurve), TEXT("Door animation curve is not set"));
	InteractWithDoor();
	
	if (OnInteractionEvent.IsBound())
	{
		OnInteractionEvent.Broadcast();

	}

}

FName ADoor::GetActionEventName() const
{
	return ActionInteract;
}

bool ADoor::HasOnInteractionCallback() const
{
	return true;
}

FDelegateHandle ADoor::AddOnInteractionUFunction(UObject* Object, const FName& FunctionName)
{
	return OnInteractionEvent.AddUFunction(Object, FunctionName);
}

void ADoor::RemoveOnInteractionDelegate(FDelegateHandle DelegateHandle)
{
	OnInteractionEvent.Remove(DelegateHandle);
}

// void ADoor::OnLevelDeserialized_Implementation()
// {
// 	float YawAngle = bIsOpnened ? AngleOpened : AngleClosed;
// 	DoorPivot->SetRelativeRotation(FRotator(0.0f, YawAngle, 0.0f));
//
// }

void ADoor::UpdateDoorAnimation(float Alpha)
{
	float YawAngle = FMath::Lerp(AngleClosed, AngleOpened, FMath::Clamp(Alpha, 0.0f, 1.0f));
	DoorPivot->SetRelativeRotation(FRotator(0.0f, YawAngle, 0.0f));
}

void ADoor::OnDoorAnimationFinished()
{
	SetActorTickEnabled(false);
}


