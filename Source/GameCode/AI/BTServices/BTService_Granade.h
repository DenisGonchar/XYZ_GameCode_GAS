// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_Granade.generated.h"

/**
 * 
 */
UCLASS()
class GAMECODE_API UBTService_Granade : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTService_Granade();

protected:
	
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	FBlackboardKeySelector TargetKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	float MaxGranadeDIstance = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	float MinGranadeDIstance = 300.0f;

};
