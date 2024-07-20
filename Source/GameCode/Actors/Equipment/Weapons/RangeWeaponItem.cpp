// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "Components/Weapon/WeaponBarellComponent.h"
#include "GameCodeTypes.h"
#include "MatineeCameraShake.h"
#include "Pawns/Character/GCBaseCharacter.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"

ARangeWeaponItem::ARangeWeaponItem()
{
	PrimaryActorTick.bCanEverTick = true;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Weapon"));

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);

	WeaponBarell = CreateDefaultSubobject<UWeaponBarellComponent>(TEXT("WeaponBarell"));
	WeaponBarell->SetupAttachment(WeaponMesh, SocketWeaponMuzzle);

	ReticleType = EReticleType::Default;

	EquippedSocketName = SocketCharacterWeapon;
}

void ARangeWeaponItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARangeWeaponItem, bIsReloading);
	DOREPLIFETIME(ARangeWeaponItem, CurrentWeaponParameters);
}

void ARangeWeaponItem::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ProcessRecoil(DeltaSeconds);
	ProcessRecoilback(DeltaSeconds);

}

void ARangeWeaponItem::StartFire()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(ShotTimer))
	{
		return;
	}

	bIsFiring = true;

	StopRecoilRollback();
	
	MakeShot();
	
	
}

void ARangeWeaponItem::StopFire()
{
	bIsFiring = false;

	if (!GetCharacterOwner()->IsPlayerControlled())
	{
		return;
	}

	RecoilRollbackTime = AccumulatedShots * 60.0f / CurrentWeaponParameters.RecoilParameters.RecoilRollbackSpeed;
	if (AccumulatedRecoilPitch != 0.0f || AccumulatedRecoilYaw != 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(RecoilRollbackTimer, this, &ARangeWeaponItem::StopRecoilRollback, RecoilRollbackTime, false);
	}
	
}

bool ARangeWeaponItem::bAiming()
{
	return bIsAiming;
}

void ARangeWeaponItem::SetAmmo(int32 NewAmmo)
{
	CurrentWeaponParameters.WeaponAmmo = NewAmmo;
	if (OnAmmoChanged.IsBound())
	{
		OnAmmoChanged.Broadcast(CurrentWeaponParameters.WeaponAmmo);
	}
}

bool ARangeWeaponItem::CanShoot() const
{
	return CurrentWeaponParameters.WeaponAmmo > 0;
}


void ARangeWeaponItem::StartReload()
{
	AGCBaseCharacter* CharacterOwner = GetCharacterOwner();
	if (!IsValid(CharacterOwner))
	{
		return;
	}

	bIsReloading = true;

	if (IsValid(CurrentWeaponParameters.CharacterReloadMontage))
	{
		float MontageDuration = CharacterOwner->PlayAnimMontage(CurrentWeaponParameters.CharacterReloadMontage);
		
		PlayAnimMontage(CurrentWeaponParameters.WeaponReloadMontage);
		if (CurrentWeaponParameters.ReloadType == EReloadType::FullClip)
		{
			GetWorld()->GetTimerManager().SetTimer(ReloadTimer, [this]() {EndReload(true); }, MontageDuration, false);

		}	
	}
	else
	{
		EndReload(true);
	}

	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_ReloadWeapon();
	}
}

void ARangeWeaponItem::EndReload(bool bIsSuccess)
{
	if (!bIsReloading)
	{
		return;
	}

	AGCBaseCharacter* CharacterOwner = GetCharacterOwner();

	if (!bIsSuccess)
	{
		if (IsValid(CharacterOwner))
		{
			CharacterOwner->StopAnimMontage(CurrentWeaponParameters.CharacterReloadMontage);

		}
		StopAnimMontage(CurrentWeaponParameters.WeaponReloadMontage);
	}
	
	if (CurrentWeaponParameters.ReloadType == EReloadType::ByBullet)
	{
		UAnimInstance* CharacterAnimInstance = IsValid(CharacterOwner) ? CharacterOwner->GetMesh()->GetAnimInstance() : nullptr;
		if (IsValid(CharacterAnimInstance))
		{
			CharacterAnimInstance->Montage_JumpToSection(SectionMontageReloadEnd, CurrentWeaponParameters.CharacterReloadMontage);
		}

		UAnimInstance* WeaponAnimInstance = WeaponMesh->GetAnimInstance();
		if (IsValid(WeaponAnimInstance))
		{
			WeaponAnimInstance->Montage_JumpToSection(SectionMontageReloadEnd, CurrentWeaponParameters.WeaponReloadMontage);
		}
	}

	GetWorld()->GetTimerManager().ClearTimer(ReloadTimer);

	bIsReloading = false;
	if (bIsSuccess && OnReloadComplete.IsBound())
	{
		OnReloadComplete.Broadcast();
	}
}

bool ARangeWeaponItem::IsReloadnig() const
{
	return bIsReloading;
}

EReticleType ARangeWeaponItem::GetReticleType() const
{
	return bIsAiming ? CurrentWeaponParameters.AimReticleType : ReticleType;
}

// void ARangeWeaponItem::OnLevelDeserialized_Implementation()
// {
// 	SetActorRelativeTransform(FTransform(FRotator::ZeroRotator, FVector::ZeroVector));
//
// 	if (OnAmmoChanged.IsBound())
// 	{
// 		OnAmmoChanged.Broadcast(CurrentWeaponParameters.WeaponAmmo);
// 	}
// }

void ARangeWeaponItem::StopRecoilRollback()
{
	if (RecoilRollbackTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(RecoilRollbackTimer);
	}
	
	AccumulatedRecoilPitch = 0.0f;
	AccumulatedRecoilYaw = 0.0f;
	AccumulatedShots = 0;
	
}

bool ARangeWeaponItem::SwitchAmmoType()
{
	if (!bIsAdditional)
	{
		WeaponDefaultParameters = CurrentWeaponParameters;
		CurrentWeaponParameters = WeaponAdditionalParameters;
		
		bIsAdditional = true;
	}
	else
	{
		WeaponAdditionalParameters = CurrentWeaponParameters;
		CurrentWeaponParameters = WeaponDefaultParameters;
		
		bIsAdditional = false;
	}

	if (OnAmmoChanged.IsBound())
	{
		OnAmmoChanged.Broadcast(CurrentWeaponParameters.WeaponAmmo);
	}

	
	WeaponBarell->SwitchAdditionalRegistrationType();
	return true;
	
}

void ARangeWeaponItem::Server_ReloadWeapon_Implementation()
{
	
	Multicast_Reload();
}

void ARangeWeaponItem::Multicast_Reload_Implementation()
{

	StartReload();
}

void ARangeWeaponItem::BeginPlay()
{
	Super::BeginPlay();

	SetAllAmmo();

	CurrentWeaponParameters = WeaponDefaultParameters;

	SetAmmo(CurrentWeaponParameters.MaxAmmo);

}

void ARangeWeaponItem::SetAllAmmo()
{
	WeaponDefaultParameters.WeaponAmmo = WeaponDefaultParameters.MaxAmmo;
	WeaponDefaultParameters.WeaponMaxAmmo = WeaponDefaultParameters.MaxAmmo;

	WeaponAdditionalParameters.WeaponAmmo = WeaponAdditionalParameters.MaxAmmo;
	WeaponAdditionalParameters.WeaponMaxAmmo = WeaponAdditionalParameters.MaxAmmo;

}

void ARangeWeaponItem::ProcessRecoil(float DeltaTime)
{
	if (!GetWorld()->GetTimerManager().IsTimerActive(RecoilTimer))
	{
		return;
	}
	
	AGCBaseCharacter* CharacterOwner = GetCharacterOwner();
	if (!IsValid(CharacterOwner))
	{
		return;
	}

	if (!CharacterOwner->IsPlayerControlled() || !CharacterOwner->IsLocallyControlled())
	{
		return;
	}
	
	float RecoilPitch = -CurrentWeaponParameters.RecoilParameters.RecoilPitch / GetRecoilTimeInterval() * DeltaTime;
	CharacterOwner->AddControllerPitchInput(RecoilPitch);
	AccumulatedRecoilPitch += RecoilPitch;
	
	float RecoilYaw = CurrentWeaponParameters.RecoilParameters.RecoilYaw / GetRecoilTimeInterval() * DeltaTime;
	CharacterOwner->AddControllerYawInput(RecoilYaw);
	AccumulatedRecoilYaw += RecoilYaw;
	
}

void ARangeWeaponItem::ProcessRecoilback(float DeltaTime)
{
	if (!GetWorld()->GetTimerManager().IsTimerActive(RecoilRollbackTimer))
	{
		return;
	}

	AGCBaseCharacter* CharacterOwner = GetCharacterOwner();
	if (!IsValid(CharacterOwner))
	{
		return;
	}

	if (!CharacterOwner->IsPlayerControlled() || !CharacterOwner->IsLocallyControlled())
	{
		return;
	}
	
	float RollbackPitch = -AccumulatedRecoilPitch / RecoilRollbackTime * DeltaTime;
	CharacterOwner->AddControllerPitchInput(RollbackPitch);

	float RollbackYaw = -AccumulatedRecoilYaw / RecoilRollbackTime * DeltaTime;
	CharacterOwner->AddControllerYawInput(RollbackYaw);

}

float ARangeWeaponItem::GetCurrentBulletSpreadAngle() const
{
	float AngleInDegress = bIsAiming ? CurrentWeaponParameters.AimSpreadAngle : CurrentWeaponParameters.SpreadAngle;
	
	return FMath::DegreesToRadians(AngleInDegress);
}

void ARangeWeaponItem::MakeShot()
{
	AGCBaseCharacter* CharacterOwner = GetCharacterOwner();
	if (!IsValid(CharacterOwner))
	{
		return;
	}

	if (!CanShoot())
	{
		StopFire();
		if (CurrentWeaponParameters.WeaponAmmo == 0 && CurrentWeaponParameters.bAutoReload)
		{
			CharacterOwner->Reload();
		}
		return;
	}

	EndReload(false);

	CharacterOwner->PlayAnimMontage(CurrentWeaponParameters.CharacterFireMontage);
	PlayAnimMontage(CurrentWeaponParameters.WeaponFireMontage);
	
	FVector ShotLocation;
	FRotator ShotRotation;
	if (CharacterOwner->IsPlayerControlled())
	{
		APlayerController* Controller = CharacterOwner->GetController<APlayerController>();
		Controller->GetPlayerViewPoint(ShotLocation, ShotRotation);

	}
	else
	{
		ShotLocation = WeaponBarell->GetComponentLocation();
		ShotRotation = CharacterOwner->GetBaseAimRotation();
	}

	FVector ShotDirection = ShotRotation.RotateVector(FVector::ForwardVector);
	
	SetAmmo(CurrentWeaponParameters.WeaponAmmo - 1);

	WeaponBarell->Shot(ShotLocation, ShotDirection, GetCurrentBulletSpreadAngle());

	GetWorld()->GetTimerManager().SetTimer(ShotTimer, this, &ARangeWeaponItem::OnShotTimerElapsed, GetShotTimerInterval(), false);
	GetWorld()->GetTimerManager().SetTimer(RecoilTimer, GetRecoilTimeInterval(),false);
	
	if (IsValid(CurrentWeaponParameters.ShotCameraShakeClass) && GetCharacterOwner()->IsPlayerControlled())
	{
		APlayerController* Controller = CharacterOwner->GetController<APlayerController>();
		Controller->PlayerCameraManager->StartCameraShake(CurrentWeaponParameters.ShotCameraShakeClass);
	}
	
	++AccumulatedShots;
}

void ARangeWeaponItem::OnShotTimerElapsed()
{
	if (!bIsFiring)
	{
		return;
	}

	switch (CurrentWeaponParameters.WeaponFireMode)
	{
		case EWeaponFireMode::Single:
		{
			StopFire();
			break;
			
		}
		
		case EWeaponFireMode::FullAuto:
		{
			MakeShot();

		}

	}
}

float ARangeWeaponItem::GetShotTimerInterval() const
{
	return 60.0f / CurrentWeaponParameters.RateOfFire;
}

float ARangeWeaponItem::GetRecoilTimeInterval() const
{
	return FMath::Min(GetShotTimerInterval(), CurrentWeaponParameters.RecoilParameters.RecoilTime);

	
}

float ARangeWeaponItem::PlayAnimMontage(UAnimMontage* AnimMontage)
{
	float Result = 0.0f;
	
	UAnimInstance* WeaponAnimInstance = WeaponMesh->GetAnimInstance();
	if (IsValid(WeaponAnimInstance))
	{
		Result = WeaponAnimInstance->Montage_Play(AnimMontage);
	
	}

	return Result;
}

void ARangeWeaponItem::StopAnimMontage(UAnimMontage* AnimMontage, float BlendOutTime /*= 0.0f*/)
{
	UAnimInstance* WeaponAnimInstance = WeaponMesh->GetAnimInstance();
	if (IsValid(WeaponAnimInstance))
	{
		WeaponAnimInstance->Montage_Stop(BlendOutTime, AnimMontage);
		
	}
}
