// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTServices/BTService_Granade.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Pawns/Character/GCBaseCharacter.h"
#include <Actors/Equipment/Throwables/ThrowableItem.h>
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "GameCodeTypes.h"

UBTService_Granade::UBTService_Granade()
{
	NodeName = "Granade";

}

void UBTService_Granade::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!IsValid(AIController) || !IsValid(Blackboard))
	{
		return;
	}

	AGCBaseCharacter* Character = Cast<AGCBaseCharacter>(AIController->GetPawn());
	if (!IsValid(Character))
	{
		return;
	}

	const UCharacterEquipmentComponent* EquipmentComponent = Character->GetCharacterEquipmentComponent();
	AThrowableItem* Granade = EquipmentComponent->GetCurrentThrowableWeapon();
	if (!IsValid(Granade))
	{
		return;
	}

	AActor* CurrentTarget = Cast<AActor>(Blackboard->GetValueAsObject(TargetKey.SelectedKeyName));
	if (!IsValid(CurrentTarget))
	{
		return;
	}
	
	float DistSq = FVector::DistSquared(CurrentTarget->GetActorLocation(), Character->GetActorLocation());
	if (DistSq > FMath::Square(MinGranadeDIstance) && DistSq < FMath::Square(MaxGranadeDIstance))
	{
		
		Character->StopFire();
		Character->EquipPrimaryItem();
	}

}
