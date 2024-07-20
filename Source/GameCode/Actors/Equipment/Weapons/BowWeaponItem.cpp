// Fill out your copyright notice in the Description page of Project Settings.


#include "BowWeaponItem.h"

#include "Components/Weapon/WeaponBarellComponent.h"
#include "Pawns/Character/GCBaseCharacter.h"
#include "UI/Widget/BowstringWidget.h"


// Sets default values
ABowWeaponItem::ABowWeaponItem()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ArrowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Arrow"));
	ArrowMesh->SetupAttachment(WeaponMesh, NameArrowSocket);
	ArrowMesh->SetVisibility(false);
}
void ABowWeaponItem::BeginPlay()
{
	Super::BeginPlay();

	ArrowMesh->AttachToComponent(WeaponMesh, FAttachmentTransformRules::KeepRelativeTransform, NameArrowSocket);

}

void ABowWeaponItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateBowstring(DeltaTime);
}

void ABowWeaponItem::UpdateBowstring(float DeltaTime)
{
	if (bBowstring())
	{

		if (GetWorld()->GetTimerManager().IsTimerActive(TimerBowstring))
		{
			Tension = FMath::FInterpTo(Tension, 1.0f, DeltaTime, TimeTension);
		}
		else
		{
			Tension = 1.0f;
		}
		
		if (OnBowstringChangedEvent.IsBound())
		{
			OnBowstringChangedEvent.Broadcast(bBowstring(), Tension);
		}	
	}
	else
	{
		
		Tension = FMath::FInterpTo(Tension, 0.0f, DeltaTime, TimeTension);

		if (OnBowstringChangedEvent.IsBound())
		{
			OnBowstringChangedEvent.Broadcast(bBowstring(), Tension);
		}	
	}

}

void ABowWeaponItem::StartFire()
{
	if (!(bAiming() && GetWorld()->GetTimerManager().GetTimerElapsed(TimerBowstring) >= AfterTimeCanShoot || GetWorld()->GetTimerManager().GetTimerElapsed(TimerReadyToFire) >= AfterTimeCanShoot))
	{
		return;
	}
	
	Shot();
	
	Tension = 0;
}

void ABowWeaponItem::StopFire()
{

}

void ABowWeaponItem::Shot()
{
	//TODO Fire
	AGCBaseCharacter* CharacterOwner = GetCharacterOwner();
	if (!IsValid(CharacterOwner))
	{
		return;
	}
	
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
	
	SetAmmo(GetWeaponParameters().WeaponAmmo - 1);

	//TODO Bullet Spread angle

	float AngleInDegress = BowstringType == EBowstringType::Tension ? GetWeaponParameters().SpreadAngle : GetWeaponParameters().AimSpreadAngle;
	
	WeaponBarell->Shot(ShotLocation, ShotDirection, FMath::DegreesToRadians(AngleInDegress));
	TimeShotFire = GetWorld()->GetTimerManager().GetTimerElapsed(TimerReadyToFire);
	
	//TODO Reload
	StartReload();
	
	GetWorld()->GetTimerManager().SetTimer(FireTimer, this, &ABowWeaponItem::OnAimTimerElapsed, TimeReload, false);

	StopBowstring();
	
}

void ABowWeaponItem::OnAimTimerElapsed()
{
	if (bAiming())
	{
		StartBowstring();
	}
	else
	{
		StopFire();
	}
	
}


void ABowWeaponItem::StartAim()
{
	if (IsReloadnig())
	{
		return;	
	}
	
	Super::StartAim();

	StartBowstring();
	
}

void ABowWeaponItem::StopAim()
{
	Super::StopAim();
	
	StopBowstring();

	GetWorld()->GetTimerManager().ClearTimer(FireTimer);
}

void ABowWeaponItem::StartBowstring()
{
	if (IsValid(ArrowMesh))
	{
		ArrowMesh->SetVisibility(true);
	}

	bIsBowstring = true;

	BowstringType = EBowstringType::Tension;
	
	GetWorld()->GetTimerManager().SetTimer(TimerBowstring, this,  &ABowWeaponItem::StartTimers, TimeTension, false);
}

void ABowWeaponItem::StopBowstring()
{
	if (IsValid(ArrowMesh))
	{
		ArrowMesh->SetVisibility(false);
	}

	bIsBowstring = false;
		
	ClearTimers();
}

void ABowWeaponItem::StartTimers()
{
	BowstringType = EBowstringType::Hold;
	
	GetWorld()->GetTimerManager().SetTimer(TimerReadyToFire, this, &ABowWeaponItem::ReadyToFire, TimeReadyToFire, false);
	GetWorld()->GetTimerManager().SetTimer(TimerShakeAim, this, &ABowWeaponItem::ShakeAim, TimeShakeAim, false);
	
}

void ABowWeaponItem::ClearTimers()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerBowstring);
	GetWorld()->GetTimerManager().ClearTimer(TimerReadyToFire);
	GetWorld()->GetTimerManager().ClearTimer(TimerShakeAim);
	GetWorld()->GetTimerManager().ClearTimer(TimerRecoil);
	
}

float ABowWeaponItem::GetTimeTimerReadyToFire()
{
	return TimeShotFire;
}

void ABowWeaponItem::ShakeAim()
{
	//TODO random shift aim
	GetWorld()->GetTimerManager().SetTimer(TimerRecoil, this, &ABowWeaponItem::StartRecoil, 0.1f, true);
}

void ABowWeaponItem::ReadyToFire()
{
	StartFire();
}

void ABowWeaponItem::StartRecoil()
{
	APlayerController* Controller = GetCharacterOwner()->GetController<APlayerController>();
	if (!Controller)
	{
		return;
	}
//
	FVector ShotLocation;
	FRotator ShotRotation;
	Controller->GetPlayerViewPoint(ShotLocation, ShotRotation);

	ShotRotation = ShotRotation.RotateVector(FVector::ForwardVector).ToOrientationRotator();

	float Angle = 0.0f;
	if (GetWeaponParameters().RecoilAngleCurve)
	{
		float Timer = GetWorld()->GetTimerManager().GetTimerElapsed(TimerReadyToFire);
		if (Timer > TimeShakeAim)
		{
			Angle = GetWeaponParameters().RecoilAngleCurve->GetFloatValue(Timer);
		}
		else
		{
			Angle = 0.1f;
		}
	}
	else
	{
		Angle = 0.1f;
	}
	float SpreadSize = FMath::Tan(Angle);
	float RotationAngle = FMath::RandRange(0.0f, 2 * PI);

	float SpreadY = FMath::Cos(RotationAngle);
	float SpreadZ = FMath::Sin(RotationAngle);

	FVector Result = (ShotRotation.RotateVector(FVector::UpVector) * SpreadZ + ShotRotation.RotateVector(FVector::RightVector) * SpreadY) * SpreadSize;

	GetCharacterOwner()->AddControllerPitchInput(Result.Y);
	GetCharacterOwner()->AddControllerYawInput(Result.Z);
	
	
}

