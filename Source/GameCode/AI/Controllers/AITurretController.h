// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GCAIController.h"
#include "AITurretController.generated.h"

class ATurret;

UCLASS()
class GAMECODE_API AAITurretController : public AGCAIController
{
	GENERATED_BODY()
	
public:
	
	virtual void SetPawn(APawn* InPawn) override;

	virtual void ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors) override;

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	void SetClosestActor(AActor* NewActor);
	
private:
	TWeakObjectPtr<ATurret> CachedTurret;
};
