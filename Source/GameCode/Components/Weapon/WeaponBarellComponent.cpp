// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Weapon/WeaponBarellComponent.h"
#include "GameCodeTypes.h"
#include "DrawDebugHelpers.h"
#include <Subsystems/DebugSubsystem.h>

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Actors/Equipment/Weapons/BowWeaponItem.h"
#include "Components/DecalComponent.h"
#include "Actors/Projectiles/GCProjectile.h"
#include "DataAsset/ImpactFXData.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Net/UnrealNetwork.h"


UWeaponBarellComponent::UWeaponBarellComponent()
{
	SetIsReplicatedByDefault(true);

}

void UWeaponBarellComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwnerRole() < ROLE_Authority)
	{
		return;
	}

	SaveCurveDamage = DamageAmount;

	if (!IsValid(ProjectileClass))
	{
		return;
	}

	ProjectilePool.Reserve(ProjectilePoolSize);
	for (int32 i = 0; i < ProjectilePoolSize; ++i)
	{
		AGCProjectile* Projectile = GetWorld()->SpawnActor<AGCProjectile>(ProjectileClass, ProjectilePoolLocation, FRotator::ZeroRotator);
		Projectile->SetOwner(GetOwnerPawn());
		Projectile->SetProjectileActive(false);
		ProjectilePool.Add(Projectile);

	}


	
}

void UWeaponBarellComponent::Shot(FVector ShotStart, FVector ShotDirection, float SpreadAngle)
{
	TArray<FShotInfo> ShotInfo;
	
	for (int i = 0; i < BulletPerShot; i++)
	{
		if (bIsRandRange)
		{
			ShotDirection += GetBulletSpreadOffset(FMath::RandRange(0.0f, SpreadAngle), ShotDirection.ToOrientationRotator());
		}
		else
		{
			ShotDirection += GetBulletSpreadOffset(SpreadAngle, ShotDirection.ToOrientationRotator());
		}
		
		ShotDirection = ShotDirection.GetSafeNormal();
		
		ShotInfo.Emplace(ShotStart, ShotDirection);
	
	}

	if (GetOwner()->GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_Shot(ShotInfo);
	}
	
	ShotInternal(ShotInfo);

}

void UWeaponBarellComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams RepParams;

	RepParams.Condition = COND_SimulatedOnly;
	RepParams.RepNotifyCondition = REPNOTIFY_Always;

	DOREPLIFETIME_WITH_PARAMS(UWeaponBarellComponent, LastShotsInfo, RepParams);
	DOREPLIFETIME(UWeaponBarellComponent, ProjectilePool);
	DOREPLIFETIME(UWeaponBarellComponent, CurrentProjectileIndex);
	DOREPLIFETIME(UWeaponBarellComponent, SaveCurveDamage);

}

bool UWeaponBarellComponent::SwitchAdditionalRegistrationType()
{
	if (AdditionalProjectileClass == nullptr)
	{
		return false;
	}

	if (HitRegistration == EHitRegistrationType::HitScan)
	{
		HitRegistration = EHitRegistrationType::Projectile;
		bIsAddional = true;
	}
	else
	{
		HitRegistration = EHitRegistrationType::HitScan;
		bIsAddional = false;
	}
	return true;
}

bool UWeaponBarellComponent::HitScan(FVector ShotStart, OUT FVector& ShotEnd, FVector ShotDirection)
{
	FHitResult ShotResult;
	
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnPhysicalMaterial = true;
	QueryParams.AddIgnoredActor(GetOwner());
	
	if (GetWorld()->LineTraceSingleByChannel(ShotResult, ShotStart, ShotEnd, ECC_Bullet, QueryParams))
	{
		ShotEnd = ShotResult.ImpactPoint;

		float ForwardDistance = (MuzzleLocation - ShotEnd).Size();

		if (!FalloffDiagrama)
		{
			return true;
		}
		
		float CurveValue = FalloffDiagrama->GetFloatValue(ForwardDistance);

		if (CurveValue > 0.0f && SaveCurveDamage > 0.0f)
		{
			DamageAmount = SaveCurveDamage + (CurveValue * SaveCurveDamage);
		}
	}
		
	ProcessHit(ShotResult, ShotDirection);

	return false;
}


void UWeaponBarellComponent::ProcessProjectileHit(AGCProjectile* Projectile, const FHitResult& HitResult, const FVector& Direction)
{
	Projectile->SetProjectileActive(false);
	Projectile->SetActorLocation(ProjectilePoolLocation);
	Projectile->SetActorRotation(FRotator::ZeroRotator);
	Projectile->OnProjectileHit.RemoveAll(this);

	ProcessHit(HitResult, Direction);
}

void UWeaponBarellComponent::ProcessHit(const FHitResult& HitResult, const FVector& Direction)
{
	AActor* HitActor = HitResult.GetActor();
	if (GetOwner()->HasAuthority() && IsValid(HitActor))
	{
		float Damage = DamageAmount;
		ABowWeaponItem* Weapon = Cast<ABowWeaponItem>(GetOwner());
		if (Weapon && DamageCurve)
		{
			switch (Weapon->GetBowstringType())
			{
				case EBowstringType::Tension:
				{
					Damage = MinDamageAmount;
					break;
				}

				case EBowstringType::Hold:
				{
					float TimeTimer = Weapon->GetTimeTimerReadyToFire();
					Damage = DamageCurve->GetFloatValue(TimeTimer); 
					break;
				}
			}
		}
			
		IAbilitySystemInterface* AbilitySystemActor = Cast<IAbilitySystemInterface>(HitActor);
		if (AbilitySystemActor != nullptr && IsValid(	DamageEffectClass))
		{
			UGameplayEffect* DamageEffect = DamageEffectClass->GetDefaultObject<UGameplayEffect>();
			FGameplayEffectContext* DamageEffectContext = new FGameplayEffectContext(GetController(), GetOwner());

			DamageEffectContext->AddHitResult(HitResult);
			FGameplayEffectContextHandle DamageEffectContextHandle(DamageEffectContext);
			FGameplayEffectSpec DamageEffectSpec(DamageEffect, DamageEffectContextHandle);

			DamageEffectSpec.SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Abilities.Attributes.Health")), Damage);

			UAbilitySystemComponent* AbilitySystem = AbilitySystemActor->GetAbilitySystemComponent();
			AbilitySystem->ApplyGameplayEffectSpecToSelf(DamageEffectSpec);
		}
		
		
	}

	UDecalComponent* DecalComponent = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), DefaultDecalInfo.DecalMaterial, DefaultDecalInfo.DecalSize, HitResult.ImpactPoint, HitResult.ImpactNormal.ToOrientationRotator());
	if (IsValid(DecalComponent))
	{
		DecalComponent->SetFadeScreenSize(0.0001f);
		DecalComponent->SetFadeOut(DefaultDecalInfo.DecalLifeTime, DefaultDecalInfo.DecalFadeOutTime);
	}

	
	if (IsValid(ImpactFXData))
	{
		float FXScale = 1.0f;
		if (HitResult.BoneName == FName("head"))
		{
			FXScale = 2.0f;
	
		}
		ImpactFXData->PlayImpactFX(HitResult, FXScale);
	}
}


void UWeaponBarellComponent::LaunchProjectile(const FVector& LaunchStart, const FVector& LaunchDirection)
{
	
	if (bIsAddional)
	{
		AGCProjectile* Projectile = GetWorld()->SpawnActor<AGCProjectile>(AdditionalProjectileClass, LaunchStart, LaunchDirection.ToOrientationRotator());
		
		Projectile->SetOwner(GetOwner());
		Projectile->LaunchProjectile(LaunchDirection.GetSafeNormal());
		Projectile->SetProjectileActive(true);

		return;
	}
	
	AGCProjectile* Projectile = ProjectilePool[CurrentProjectileIndex];

	ABowWeaponItem* Weapon = Cast<ABowWeaponItem>(GetOwner());
	if (Weapon)
	{
		if (Weapon->GetBowstringType() == EBowstringType::Tension)
		{
			Projectile->SetSpeedProjectile(MinSpeedProjectile);
		}
		else
		{
			Projectile->SetSpeedProjectile(MaxSpeedProjectile);
		}
		
	}
	
	Projectile->SetActorLocation(LaunchStart);
	Projectile->SetActorRotation(LaunchDirection.ToOrientationRotator());

	Projectile->SetProjectileActive(true);
	Projectile->OnProjectileHit.AddDynamic(this, &UWeaponBarellComponent::ProcessProjectileHit);
	Projectile->LaunchProjectile(LaunchDirection.GetSafeNormal());
	
	
	++CurrentProjectileIndex;
	if (CurrentProjectileIndex == ProjectilePool.Num())
	{
		CurrentProjectileIndex = 0;
	}

}

void UWeaponBarellComponent::ShotInternal(const TArray<FShotInfo>& ShotsInfo)
{	
	if(GetOwner()->HasAuthority())
	{
		LastShotsInfo = ShotsInfo;	
	}

	MuzzleLocation = GetComponentLocation();
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), MuzzleFlashFX, MuzzleLocation, GetComponentRotation());

	for (const FShotInfo& ShotInfo : ShotsInfo)
	{
		FVector ShotStart = ShotInfo.GetLocation();// + 50 * FVector::ForwardVector;
		FVector ShotDirection = ShotInfo.GetDirection();
		FVector ShotEnd = ShotStart + FiringRange * ShotDirection;

#if ENABLE_DRAW_DEBUG
		UDebugSubsystem* DebugSubSystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
		bool bIsDebugEnabled = DebugSubSystem->IsCategoryEnabled(DebugCategoryRangeWeapon);
#else
		bool bIsDebugEnabled = false;
#endif
		switch (HitRegistration)
		{
			case EHitRegistrationType::HitScan:
			{
				bool bHasHit = HitScan(ShotStart, ShotEnd, ShotDirection);

				if (bIsDebugEnabled && bHasHit)
				{
					DrawDebugSphere(GetWorld(), ShotEnd, 10.0f, 24, FColor::Red, false, 5.0f);
				}

				break;
			}

			case EHitRegistrationType::Projectile:
			{
				LaunchProjectile(ShotStart, ShotDirection);

				break;
			}

			default:
				break;
		}



		UNiagaraComponent* TraceFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), TraceFX, MuzzleLocation, GetComponentRotation());
		if (IsValid(TraceFXComponent))
		{
			TraceFXComponent->SetVectorParameter(FXParamTraceEnd, ShotEnd);
		}

		if (bIsDebugEnabled)
		{
			DrawDebugLine(GetWorld(), MuzzleLocation, ShotEnd, FColor::Red, false, 1.0f, 0, 5.0f);

		}
	}
}

void UWeaponBarellComponent::Server_Shot_Implementation(const TArray<FShotInfo>& ShotsInfo)
{
	ShotInternal(ShotsInfo);

}

void UWeaponBarellComponent::OnRep_LastShotsInfo()
{
	ShotInternal(LastShotsInfo);
}

APawn* UWeaponBarellComponent::GetOwnerPawn() const
{
	APawn* PawnOwner = Cast<APawn>(GetOwner());
	if (!IsValid(PawnOwner))
	{
		PawnOwner = PawnOwner = Cast<APawn>(GetOwner()->GetOwner());
	}
	return PawnOwner;
}

AController* UWeaponBarellComponent::GetController() const
{
	APawn* PawnOwner = GetOwnerPawn();
	return IsValid(PawnOwner) ? PawnOwner->GetController() : nullptr;
}

FVector UWeaponBarellComponent::GetBulletSpreadOffset(float Angle, FRotator ShotRotation) const
{
	float SpreadSize = FMath::Tan(Angle);
	float RotationAngle = FMath::RandRange(0.0f, 2 * PI);

	float SpreadY = FMath::Cos(RotationAngle);
	float SpreadZ = FMath::Sin(RotationAngle);

	FVector Result = (ShotRotation.RotateVector(FVector::UpVector) * SpreadZ + ShotRotation.RotateVector(FVector::RightVector) * SpreadY) * SpreadSize;

	return Result;
}

