// Fill out your copyright notice in the Description page of Project Settings.


#include "GCBaseDrone.h"

#include "Components/BoxComponent.h"
#include "Components/ExplosionComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Pawns/Character/GCBaseCharacter.h"

AGCBaseDrone::AGCBaseDrone()
{
	PrimaryActorTick.bCanEverTick = true;

	MoverComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("MoverComponent"));
	SetRootComponent(MoverComponent);
	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	StaticMeshComponent->SetupAttachment(MoverComponent);
	
	ThirdSpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("ThirdSpringArmComponent"));
	ThirdSpringArmComponent->SetupAttachment(RootComponent);
	ThirdSpringArmComponent->bUsePawnControlRotation = true;
	
	ThirdCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdCameraComponent"));
	ThirdCameraComponent->SetupAttachment(ThirdSpringArmComponent);
	ThirdCameraComponent->bUsePawnControlRotation = false;

	FirstSpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("FirstSpringArmComponent"));
	FirstSpringArmComponent->SetupAttachment(RootComponent);
	FirstSpringArmComponent->bUsePawnControlRotation = true;
	
	FirstCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstCameraComponent"));
	FirstCameraComponent->SetupAttachment(FirstSpringArmComponent);
	FirstCameraComponent->bUsePawnControlRotation = false;
	
	ExlosionComponent = CreateDefaultSubobject<UExplosionComponent>(TEXT("ExplosionComponent"));
	ExlosionComponent->SetupAttachment(RootComponent);


	
}

void AGCBaseDrone::BeginPlay()
{
	Super::BeginPlay();

	
}

void AGCBaseDrone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

void AGCBaseDrone::MoveForward(float Value)
{
	if (!FMath::IsNearlyZero(Value,1e-6f))
	{
		CurrentSpeed = ForwardSpeed;

		FRotator YawRotator(0.0f, GetControlRotation().Yaw, 0.0f);
		FVector ForwardVector = YawRotator.RotateVector(FVector::ForwardVector);

		FVector NewLocation = GetActorLocation() + Value * ForwardSpeed * ForwardVector * GetWorld()->GetDeltaSeconds(); 
		SetActorLocation(NewLocation);

		//Rotation
		RotationAngle = FMath::RInterpTo(GetActorRotation(), FRotator(RotationPitchAngle * -Value, GetActorRotation().Yaw, GetActorRotation().Roll), GetWorld()->GetDeltaSeconds(), RotationRollAngleSpeed);
		SetActorRotation(RotationAngle);
		
	}
	else
	{
		RotationAngle = FMath::RInterpTo(GetActorRotation(), FRotator(0.0f,GetActorRotation().Yaw, GetActorRotation().Roll), GetWorld()->GetDeltaSeconds(), RotationRollAngleSpeed);
		SetActorRotation(RotationAngle);
	}
	
	OnRotatorAngleBind(RotationAngle);
	 
}

void AGCBaseDrone::MoveRight(float Value)
{
	if (!FMath::IsNearlyZero(Value,1e-6f))
	{
		CurrentSpeed = RightSpeed;

		FRotator YawRotator(0.0f, GetControlRotation().Yaw, 0.0f);
		FVector RightVector = YawRotator.RotateVector(FVector::RightVector);
		
		FVector NewLocation = GetActorLocation() + Value * RightSpeed * RightVector * GetWorld()->GetDeltaSeconds(); 
		SetActorLocation(NewLocation);

		//Rotation
		RotationAngle = FMath::RInterpTo(GetActorRotation(), FRotator(GetActorRotation().Pitch, GetActorRotation().Yaw, RotationRollAngle * Value), GetWorld()->GetDeltaSeconds(), RotationRollAngleSpeed);
		SetActorRotation(RotationAngle);
	}
	else
	{
		RotationAngle = FMath::RInterpTo(GetActorRotation(), FRotator(GetActorRotation().Pitch,GetActorRotation().Yaw,0.0f), GetWorld()->GetDeltaSeconds(), RotationRollAngleSpeed);
		SetActorRotation(RotationAngle);
	}
	
	OnRotatorAngleBind(RotationAngle);
	
}

void AGCBaseDrone::MoveUp(float Value)
{
	if (!FMath::IsNearlyZero(Value,1e-6f))
	{
		CurrentSpeed = UpSpeed;
	
		FRotator YawRotator(0.0f, GetControlRotation().Yaw, 0.0f);
		FVector UpVector = YawRotator.RotateVector(FVector::UpVector);
		
		FVector NewLocation = GetActorLocation() + Value * UpSpeed * UpVector * GetWorld()->GetDeltaSeconds(); 
		SetActorLocation(NewLocation);
	}
}

void AGCBaseDrone::Turn(float Value)
{
	AddControllerYawInput(Value * BaseTurnRate *  GetWorld()->GetDeltaSeconds() * CustomTimeDilation );
}

void AGCBaseDrone::LookUp(float Value)
{
	AddControllerPitchInput(Value * BaseLookUpRate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
}

void AGCBaseDrone::OnRotatorAngleBind(FRotator Rotation)
{
	if (OnRotationAngleChanged.IsBound())
	{
		OnRotationAngleChanged.Broadcast(Rotation);
	}
}

void AGCBaseDrone::Detonation()
{
	StaticMeshComponent->SetVisibility(false);
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RemoveDrone();

	if (IsPlayerControlled())
	{
		ExlosionComponent->Explode(GetController());
		BackToCharacter();
	}
	else
	{
		ExlosionComponent->Explode(OwnerCharacter->GetController());
	}

	Destroy();
	
}

void AGCBaseDrone::BackToCharacter()
{
	if (!OwnerCharacter)
	{
		return;
	}

	DefaultViewCamera();
	GetController()->Possess(OwnerCharacter);
	
}

void AGCBaseDrone::PreviousDrone()
{
	if (OwnerCharacter && OwnerCharacter->GetActiveDrones().Num() > 1)
	{
		int8 Index = OwnerCharacter->GetCurrentIndexActiveDrone();
		TArray<AGCBaseDrone* > Actors = OwnerCharacter->GetActiveDrones();
		
		if (Index == 0)
		{
			Index = Actors.Num() - 1;
			GetController()->Possess(Actors[Index]);
		}
		else
		{
			Index -= 1;
			GetController()->Possess(Actors[Index]);
		}

		OwnerCharacter->SetCurrentIndexActiveDrone(Index);
	}
}

void AGCBaseDrone::NextDrone()
{
	if (OwnerCharacter && OwnerCharacter->GetActiveDrones().Num() > 1)
	{
		int8 Index = OwnerCharacter->GetCurrentIndexActiveDrone();
		TArray<AGCBaseDrone* > Actors = OwnerCharacter->GetActiveDrones();
		
		if (Index == Actors.Num() - 1)
		{
			Index = 0;
			GetController()->Possess(Actors[Index]);
						
		}
		else
		{
			Index += 1; 
			GetController()->Possess(Actors[Index]);
		}

		OwnerCharacter->SetCurrentIndexActiveDrone(Index);
	}
}

void AGCBaseDrone::SetOwnerCharacter(AGCBaseCharacter* Character)
{
	OwnerCharacter = Character;
}


void AGCBaseDrone::RemoveDrone()
{
	if (GetOwnerCharacter())
	{
		OwnerCharacter->RemoveActiveDrones(this);
 		
	}
}

void AGCBaseDrone::SwitchCameara()
{
	if (ThirdCameraComponent->IsActive())
	{
		ThirdCameraComponent->Deactivate();
		FirstCameraComponent->Activate();
		OnViewFirstCamera();
		
	}
	else
	{
		FirstCameraComponent->Deactivate();
		ThirdCameraComponent->Activate();
		OnViewThirdCamera();
		
	}
	
}

void AGCBaseDrone::OnViewFirstCamera()
{
	APlayerCameraManager* CurrentCameraManager = Cast<APlayerController>(GetController())->PlayerCameraManager;
	if (!CurrentCameraManager)
	{
		return;
	}
		CurrentCameraManager->ViewPitchMax = 10.0f;
		SetSeeMesh(true);
	
}

void AGCBaseDrone::OnViewThirdCamera()
{
	
	APlayerCameraManager* CurrentCameraManager = Cast<APlayerController>(GetController())->PlayerCameraManager;
	if (!CurrentCameraManager)
	{
		return;
	}
	
	APlayerCameraManager* DefaultCameraManager = CurrentCameraManager->GetClass()->GetDefaultObject<APlayerCameraManager>();
	CurrentCameraManager->ViewPitchMax = DefaultCameraManager->ViewPitchMax;
	
	SetSeeMesh(false);
}

void AGCBaseDrone::DefaultViewCamera()
{
	if (!ThirdCameraComponent->IsActive())
	{
		FirstCameraComponent->Deactivate();
		ThirdCameraComponent->Activate();
		OnViewThirdCamera();
	}	
}

void AGCBaseDrone::SetSeeMesh(bool IsSee)
{
	StaticMeshComponent->SetOwnerNoSee(IsSee);
	StaticMeshComponent->bCastHiddenShadow = IsSee;
}

FGenericTeamId AGCBaseDrone::GetGenericTeamId() const
{
	return FGenericTeamId((uint8)Team);
}

float AGCBaseDrone::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	Detonation();
	
	return DamageAmount;
}

