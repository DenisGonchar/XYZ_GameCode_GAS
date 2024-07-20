// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CharacterComponents/AIPatrollingComponent.h"
#include "Actors/Navigation/PatrollingPath.h"

bool UAIPatrollingComponent::CanPatrol() const
{
	return IsValid(PatrollingPath) && PatrollingPath->GetWaypoints().Num() > 0;
}

FVector UAIPatrollingComponent::SelectClossestWaypoint()
{
	FVector OwnerLocation = GetOwner()->GetActorLocation();
	const TArray<FVector> WayPoints = PatrollingPath->GetWaypoints();
	FTransform PathTransform = PatrollingPath->GetActorTransform();
	
	FVector ClosestWayPoint;
	float MinSqDistance = FLT_MAX;
	
	for (int32 i = 0; i < WayPoints.Num(); ++i)
	{
		FVector WayPointWorld = PathTransform.TransformPosition(WayPoints[i]);
		float CurrentSqDistance = (OwnerLocation - WayPointWorld).SizeSquared();
		if (CurrentSqDistance < MinSqDistance)
		{
			MinSqDistance = CurrentSqDistance;
			ClosestWayPoint = WayPointWorld;
			CurrentWayPointIndex = i;
		}

	}

	return ClosestWayPoint;

}

FVector UAIPatrollingComponent::SelectNextWaypoint()
{
	const TArray<FVector> WayPoints = PatrollingPath->GetWaypoints();

	switch (PatrollingType)
	{
		case EPatrollingType::Circle:
		{
			++CurrentWayPointIndex;
		
			if (CurrentWayPointIndex == PatrollingPath->GetWaypoints().Num())
			{
				CurrentWayPointIndex = 0;
			}

			break;
		}
		case EPatrollingType::PingPong:
		{
			if (bIsNextWayPoint)
			{
				++CurrentWayPointIndex;
				if (CurrentWayPointIndex == PatrollingPath->GetWaypoints().Num())
				{
					CurrentWayPointIndex -= 2;
					bIsNextWayPoint = false;
				}
				
			}
			else
			{
				--CurrentWayPointIndex;
				if (CurrentWayPointIndex == -1)
				{
					CurrentWayPointIndex += 2;
					bIsNextWayPoint = true;
				}
				
			}
			break;
		}
	}

	FTransform PathTransform = PatrollingPath->GetActorTransform();	
	FVector WayPoint = PathTransform.TransformPosition(WayPoints[CurrentWayPointIndex]);

	return WayPoint;

}

