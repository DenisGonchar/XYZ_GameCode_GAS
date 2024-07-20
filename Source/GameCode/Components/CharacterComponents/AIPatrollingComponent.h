// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AIPatrollingComponent.generated.h"

class APatrollingPath;

UENUM()
enum class EPatrollingType : uint8
{
	Circle,
	PingPong

};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMECODE_API UAIPatrollingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	bool CanPatrol() const;
	FVector SelectClossestWaypoint();
	FVector SelectNextWaypoint();


protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Path")
	APatrollingPath* PatrollingPath;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrolling")
	EPatrollingType PatrollingType = EPatrollingType::Circle;

private:
	int32 CurrentWayPointIndex = -1;

	bool bIsNextWayPoint = true;
};
