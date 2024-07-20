// Fill out your copyright notice in the Description page of Project Settings.


#include "GCAICharacterController.h"
#include "AI/Characters/GCAICharacter.h"
#include "Perception/AISense_Sight.h"
#include "Components/CharacterComponents/AIPatrollingComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameCodeTypes.h"


void AGCAICharacterController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	if (IsValid(InPawn))
	{
		checkf(InPawn->IsA<AGCAICharacter>(), TEXT("AGCAICharacterController::SetPawn AICharacterController can possess only GCAICharacter"));
		CachedAICharacter = StaticCast<AGCAICharacter*>(InPawn);
		RunBehaviorTree(CachedAICharacter->GetBehaviorTree());
		SetupPatrolling();
	}
	else
	{
		CachedAICharacter = nullptr;
	}


}

void AGCAICharacterController::ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	Super::ActorsPerceptionUpdated(UpdatedActors);
	if (!CachedAICharacter.IsValid())
	{
		return;
	}
	TryMoveToNextTarget();


}

void AGCAICharacterController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);
	if (!Result.IsSuccess())
	{
		return;
	}
	TryMoveToNextTarget();
}

void AGCAICharacterController::SetupPatrolling()
{
	UAIPatrollingComponent* PatrollingComponet = CachedAICharacter->GetPatrollingComponent();

	if (PatrollingComponet->CanPatrol())
	{
		FVector ClossestWayPoint = PatrollingComponet->SelectClossestWaypoint();
		if (IsValid(Blackboard))
		{
			Blackboard->SetValueAsVector(BB_NextLocation, ClossestWayPoint);
			Blackboard->SetValueAsObject(BB_CurrentTarget, nullptr);
		}
		bIsPatrolling = true;
	}
}

void AGCAICharacterController::TryMoveToNextTarget()
{
	UAIPatrollingComponent* PatrollingComponet = CachedAICharacter->GetPatrollingComponent();

	AActor* ClosestActor = GetClosestSensedActor(UAISense_Sight::StaticClass());
	if (IsValid(ClosestActor))
	{
		if (IsValid(Blackboard))
		{
			Blackboard->SetValueAsObject(BB_CurrentTarget, ClosestActor);
			SetFocus(ClosestActor, EAIFocusPriority::Gameplay);

		}
		bIsPatrolling = false;

	}
	else if (PatrollingComponet->CanPatrol())
	{
		FVector WayPoint = bIsPatrolling ? PatrollingComponet->SelectNextWaypoint() : PatrollingComponet->SelectClossestWaypoint();
		if (IsValid(Blackboard))
		{
			ClearFocus(EAIFocusPriority::Gameplay);
			Blackboard->SetValueAsVector(BB_NextLocation, WayPoint);
			Blackboard->SetValueAsObject(BB_CurrentTarget, nullptr);
		}

		bIsPatrolling = true;
	}

	
}

bool AGCAICharacterController::IsTargetReached(FVector TargetLocation) const
{
	return (TargetLocation - CachedAICharacter->GetActorLocation()).SizeSquared() <= FMath::Square(TargetReachRadius);

}
