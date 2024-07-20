// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Actors/Interactive/Interface/Interactive.h"
#include "Components/TimelineComponent.h"
#include "Subsystems/SaveSubsystem/SaveSubsystemInterface.h"
#include "Door.generated.h"


UCLASS()
class GAMECODE_API ADoor : public AActor, public IInteractable /*, public ISaveSubsystemInterface*/
{
	GENERATED_BODY()
	
public:	
	ADoor();

	virtual void Tick(float DeltaTime) override;

	virtual void Interact(AGCBaseCharacter* Character) override;
	
	virtual FName GetActionEventName() const override;

	virtual bool HasOnInteractionCallback() const override;

	virtual FDelegateHandle AddOnInteractionUFunction(UObject* Object, const FName& FunctionName) override;

	virtual void RemoveOnInteractionDelegate(FDelegateHandle DelegateHandle) override;

	//virtual void OnLevelDeserialized_Implementation() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactive | Door")
	UStaticMeshComponent* DoorMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactive | Door")
	USceneComponent* DoorPivot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactive | Door")
	float AngleClosed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactive | Door")
	float AngleOpened = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactive | Door")
	UCurveFloat* DoorAnimationCurve;

	virtual void BeginPlay() override;

	IInteractable::FOnInteraction OnInteractionEvent;

protected:
	void InteractWithDoor();
	
	UFUNCTION()
	void UpdateDoorAnimation(float Alpha);

	UFUNCTION()
	void OnDoorAnimationFinished();

	FTimeline DoorOpenAnimTimeline;

	UPROPERTY(SaveGame)
	bool bIsOpnened = false;
	
};
