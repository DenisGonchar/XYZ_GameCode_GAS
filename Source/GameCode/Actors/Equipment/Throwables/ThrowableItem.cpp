// Fill out your copyright notice in the Description page of Project Settings.


#include "ThrowableItem.h"

#include "Pawns/Character/GCBaseCharacter.h"
#include "../../Projectiles/GCProjectile.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/Text/ISlateEditableTextWidget.h"


void AThrowableItem::BeginPlay()
{
	Super::BeginPlay();

	if (GetLocalRole() < ROLE_Authority)
	{
		return;
	}

	if (!IsValid(ProjectileClass))
	{
		return;
	}

	ProjectilePool.Reserve(ProjectilePoolSize);
	for (int32 i = 0; i < ProjectilePoolSize; ++i)
	{
		AGCProjectile* Projectile = GetWorld()->SpawnActor<AGCProjectile>(ProjectileClass, ProjectilePoolLocation, FRotator::ZeroRotator);
		Projectile->SetOwner(GetOwner());
		Projectile->SetProjectileActive(false);
		ProjectilePool.Add(Projectile);
	}
	
	SetThrowAmmo(MaxThrowAmmo);
}

void AThrowableItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AThrowableItem, ProjectilePool);
	DOREPLIFETIME(AThrowableItem, CurrentProjectileIndex);
	DOREPLIFETIME(AThrowableItem, ThrowAmmo);
}

void AThrowableItem::Throw()
{	
	
	AGCBaseCharacter* CharacterOwner = GetCharacterOwner();
	if (!IsValid(CharacterOwner))
	{
		return;
	}

	if (!CanThrow())
	{
		return;
	}

	FVector LaunchDirection;
	FVector SpawnLocation;

	if (CharacterOwner->IsPlayerControlled() && CharacterOwner->IsLocallyControlled())
	{
		FVector PlayerViewPoint;
		FRotator PlayerViewRotation;

		
		if(APlayerController* Controller = CharacterOwner->GetController<APlayerController>())
		{
			Controller->GetPlayerViewPoint(PlayerViewPoint, PlayerViewRotation);
		}

		
		FTransform PlayerViewTransform(PlayerViewRotation, PlayerViewPoint);

		FVector ViewDirection = PlayerViewRotation.RotateVector(FVector::ForwardVector);
		FVector ViewUpVector = PlayerViewRotation.RotateVector(FVector::UpVector);

		LaunchDirection = ViewDirection + FMath::Tan(FMath::DegreesToRadians(ThrowAngle)) * ViewUpVector;

		FVector ThrowableSocketLocation = CharacterOwner->GetMesh()->GetSocketLocation(SocketCharacterThrowable);
		FVector SocketInViewSpace = PlayerViewTransform.InverseTransformPosition(ThrowableSocketLocation);

		SpawnLocation = PlayerViewPoint + ViewDirection * SocketInViewSpace.X;

	}
	else
	{
		LaunchDirection = CharacterOwner->GetActorForwardVector() + FMath::Tan(FMath::DegreesToRadians(ThrowAngle)) * CharacterOwner->GetActorUpVector();
		SpawnLocation = CharacterOwner->GetMesh()->GetSocketLocation(SocketCharacterThrowable);
	}


	AGCProjectile* Projectile = ProjectilePool[CurrentProjectileIndex];
	
	Projectile->SetActorLocation(SpawnLocation);

	Projectile->SetProjectileActive(true);
	Projectile->OnProjectileHit.AddDynamic(this, &AThrowableItem::ProcessProjectileHit);
	Projectile->LaunchProjectile(LaunchDirection.GetSafeNormal());

	SetThrowAmmo(--ThrowAmmo);
	
	++CurrentProjectileIndex;
	if (CurrentProjectileIndex == ProjectilePool.Num())
	{
		CurrentProjectileIndex = 0;
	}

	
}

void AThrowableItem::SetThrowAmmo(int32 NewAmmo)
{
	ThrowAmmo = FMath::Clamp(NewAmmo, 0, MaxThrowAmmo);
	
	if (OnThrowAmmoChanged.IsBound())
	{
		OnThrowAmmoChanged.Broadcast(ThrowAmmo);
	}
}

void AThrowableItem::OnRep_ThrowAmmo()
{
	SetThrowAmmo(ThrowAmmo);
}

void AThrowableItem::ProcessProjectileHit(AGCProjectile* Projectile, const FHitResult& HitResult,
                                          const FVector& Direction)
{
	Projectile->SetProjectileActive(false);
	Projectile->SetActorLocation(ProjectilePoolLocation);
	Projectile->SetActorRotation(FRotator::ZeroRotator);
	Projectile->OnProjectileHit.RemoveAll(this);

}
