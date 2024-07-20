// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Projectiles/GCProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
AGCProjectile::AGCProjectile()
{
 	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
    CollisionComponent->InitSphereRadius(5.0f);
    CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
    CollisionComponent->bReturnMaterialOnMove = true;
    SetRootComponent(CollisionComponent);

    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MovementComponent"));
    ProjectileMovementComponent->InitialSpeed = 2000.0f;
    ProjectileMovementComponent->bAutoActivate = false;
    SetReplicates(true);
    SetReplicateMovement(true);
}

void AGCProjectile::LaunchProjectile(FVector Direction)
{
    ProjectileMovementComponent->Velocity = Direction * ProjectileMovementComponent->InitialSpeed;
    CollisionComponent->IgnoreActorWhenMoving(GetOwner(), true);

    ActorDirection = Direction;
    
    OnProjectileLaunched();

}

void AGCProjectile::SetProjectileActive_Implementation(bool bIsProjectileActive)
{
    ProjectileMovementComponent->SetActive(bIsProjectileActive);
}

void AGCProjectile::SetSpeedProjectile(float Speed)
{
    ProjectileMovementComponent->InitialSpeed = Speed;
}

void AGCProjectile::BeginPlay()
{
    Super::BeginPlay();

    CollisionComponent->OnComponentHit.AddDynamic(this, &AGCProjectile::OnCollisionHit);

}

void AGCProjectile::OnProjectileLaunched()
{
    
} 

void AGCProjectile::OnCollisionHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (OnProjectileHit.IsBound())
    {
        OnProjectileHit.Broadcast(this, Hit, ProjectileMovementComponent->Velocity.GetSafeNormal());
    }
}

