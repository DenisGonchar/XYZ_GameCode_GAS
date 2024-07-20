// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"

#include "BasePlatform.generated.h"


UENUM(BlueprintType)
enum class EPlatformBehavior : uint8
{
	OnDemand = 0,
	Loop
};

UCLASS()
class GAMECODE_API ABasePlatform : public AActor
{
	GENERATED_BODY()


public:	
	// Sets default values for this actor's properties
	ABasePlatform();

	FTimeline PlatformTimeline;
	

	UPROPERTY(EditInstanceOnly,BlueprintReadOnly)
	class AAPlatformInvocator* PlatformInvocator;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	UStaticMeshComponent* PlatformMesh;

	
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	UCurveFloat* TimelineCurve;

	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	EPlatformBehavior PlatformBehavior = EPlatformBehavior::OnDemand;

	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	void TickLoop(float DeltaTime);

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Transient)
	FVector StartLocation;

	
	UPROPERTY(EditInstanceOnly,BlueprintReadOnly,meta = (MakeEditWidget))
	FVector EndLocation;

	void PlatformTimelineUpdate(float Alpha);
	
	void PlatformTimelineOnDemand();
	
	void PlatformTimelineLoop();
	
	UFUNCTION()
	void MovePlatform();


};
