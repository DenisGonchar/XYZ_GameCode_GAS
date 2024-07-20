// Fill out your copyright notice in the Description page of Project Settings.


#include "ArrowProjectile.h"


AArrowProjectile::AArrowProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AArrowProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

void AArrowProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AArrowProjectile::OnCollisionHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::OnCollisionHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
	
	//TODO Spawn PowerUp  
	FVector NewLocation = Hit.Location + 20.0f * GetDirection();
	FRotator NewRotation = GetDirection().ToOrientationRotator();
	FTransform NewTransform(NewRotation, NewLocation);
	
	GetWorld()->SpawnActor<AActor>(PickableItem, NewTransform);
	
}

