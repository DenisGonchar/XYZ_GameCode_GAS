// Fill out your copyright notice in the Description page of Project Settings.


#include "ExplosiveProjectile.h"
#include "Components/ExplosionComponent.h"

AExplosiveProjectile::AExplosiveProjectile()
{
	ExlosionComponent = CreateDefaultSubobject<UExplosionComponent>(TEXT("ExlosionComponent"));
	ExlosionComponent->SetupAttachment(GetRootComponent());

	ExlosionComponent->SetIsReplicated(true);
}

void AExplosiveProjectile::OnProjectileLaunched()
{
	Super::OnProjectileLaunched();

	GetWorld()->GetTimerManager().SetTimer(DitonationTimer, this, &AExplosiveProjectile::OnDetonationTimerElapsed, DetonationTime, false);

}

void AExplosiveProjectile::OnCollisionHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	
	if (ProjectileType == EProjectileType::Additional)
	{
		OnDetonationTimerElapsed();
	}

	
}

void AExplosiveProjectile::OnDetonationTimerElapsed()
{
	
	ExlosionComponent->Explode(GetController());

	if (OnProjectileHit.IsBound())
	{
		const FHitResult Hit;
		OnProjectileHit.Broadcast(this, Hit, FVector::ZeroVector);
	}

	Destroy();
	
}

AController* AExplosiveProjectile::GetController()
{
	APawn* PawnOwner = Cast<APawn>(GetOwner());
	return IsValid(PawnOwner) ? PawnOwner->GetController() : nullptr;


}
