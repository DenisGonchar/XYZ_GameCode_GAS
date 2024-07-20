// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Controllers/AITurretController.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AIPerceptionComponent.h"
#include "AI/Characters/Turret.h"
#include "Perception/AISenseConfig.h"
#include "Perception/AISense_Damage.h"

void AAITurretController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	if (IsValid(InPawn))
	{
		checkf(InPawn->IsA<ATurret>(),TEXT("AAITurretController::SetPawn AAITurretController can possess only Turrets"));
		CachedTurret = StaticCast<ATurret*>(InPawn);
	}
	else
	{
		CachedTurret = nullptr;
	}

}

void AAITurretController::ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{	
	Super::ActorsPerceptionUpdated(UpdatedActors);

	if (!CachedTurret.IsValid())
	{
		return;
	}

	bool bIsClosest = false;
	
	if(AActor* ClosestActor = GetClosestSensedActor(UAISense_Damage::StaticClass()))
	{
		bIsClosest = true;
		
		SetClosestActor(ClosestActor);
	}
	
	if(AActor* ClosestActor = GetClosestSensedActor(UAISense_Sight::StaticClass()))
	{
		bIsClosest = true;
		
		SetClosestActor(ClosestActor);
	}

	if (!bIsClosest)
	{
		SetClosestActor(nullptr);
	}

	
}

float AAITurretController::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	AActor* ActorInstigator = Cast<AActor>(EventInstigator->GetPawn());
	UAISense_Damage::ReportDamageEvent(GetWorld(), this, ActorInstigator, DamageAmount, ActorInstigator->GetActorLocation(), GetTargetLocation());

	CachedTurret->SetTookDamage(true);

	
	return DamageAmount;
}

void AAITurretController::SetClosestActor(AActor* NewActor)
{
	CachedTurret->CurrentTarget = NewActor;
	CachedTurret->OnCurrentTargetSet();
}
