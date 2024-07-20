// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawns/Character/FPPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "CharacterMovementComponent/GCBaseCharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameCodeTypes.h"
#include "CharacterController/GCPlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/CharacterComponents/CharacterMoveComponent.h"


AFPPlayerCharacter::AFPPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	FirstPersonMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
	FirstPersonMeshComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, -128.0f));
	FirstPersonMeshComponent->CastShadow = false;
	FirstPersonMeshComponent->bCastDynamicShadow = false;
	FirstPersonMeshComponent->SetOnlyOwnerSee(true);

	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMeshComponent, SocketFPCamera);
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	
	
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->bCastHiddenShadow = true;

	CameraComponent->bAutoActivate = false;

	SpringArmComponent->bAutoActivate = false;
	SpringArmComponent->bUsePawnControlRotation = false;
	
	GetCharacterMovement()->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = true;
}

void AFPPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	GCPlayerController = Cast<AGCPlayerController>(NewController);

}

void AFPPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsFPMontagePlaying() && GCPlayerController.IsValid())
	{
		FRotator TargetControlRotation = GCPlayerController->GetControlRotation();
		TargetControlRotation.Pitch = 0.0f;

		float BlendSpeed = 10.0f;
		
		FRotator CurrentControlRotaion = GCPlayerController->GetControlRotation();
		
		TargetControlRotation = FMath::RInterpTo(CurrentControlRotaion, TargetControlRotation, DeltaTime, BlendSpeed);
		GCPlayerController->SetControlRotation(TargetControlRotation);

	}

}

void AFPPlayerCharacter::OnStartCrouch(float HalfHeightAbjust, float ScaledHalfHeightAnjust)
{
	Super::OnStartCrouch(HalfHeightAbjust, ScaledHalfHeightAnjust);
	
	const AFPPlayerCharacter* DefaultCharacter = GetDefault<AFPPlayerCharacter>(GetClass());

	FVector& FirstPersonMeshRelativeLocation = FirstPersonMeshComponent->GetRelativeLocation_DirectMutable();

	FirstPersonMeshRelativeLocation.Z = DefaultCharacter->FirstPersonMeshComponent->GetRelativeLocation().Z + HalfHeightAbjust;


}

void AFPPlayerCharacter::OnEndCrouch(float HalfHeightAbjust, float ScaledHalfHeightAnjust)
{
	Super::OnEndCrouch(HalfHeightAbjust, ScaledHalfHeightAnjust);

	const AFPPlayerCharacter* DefaultCharacter = GetDefault<AFPPlayerCharacter>(GetClass());

	FVector& FirstPersonMeshRelativeLocation = FirstPersonMeshComponent->GetRelativeLocation_DirectMutable();

	FirstPersonMeshRelativeLocation.Z = DefaultCharacter->FirstPersonMeshComponent->GetRelativeLocation().Z;

}

void AFPPlayerCharacter::OnSprintStart_Implementation()
{
	Super::OnSprintStart_Implementation();

}

void AFPPlayerCharacter::OnSprintEnd_Implementation()
{
	Super::OnSprintEnd_Implementation();

}

void AFPPlayerCharacter::HardLanding()
{
	Super::HardLanding();

	UAnimInstance* FPAnimInstance = FirstPersonMeshComponent->GetAnimInstance();
	if (!IsValid(FPAnimInstance))
	{
		return;
	}

	if (IsValid(FPHardLandingAnimMontage))
	{
		FPAnimInstance->Montage_Play(FPHardLandingAnimMontage);

	}


}

void AFPPlayerCharacter::LimitControl()
{
	Super::LimitControl();

	GetController()->SetIgnoreLookInput(true);
}

void AFPPlayerCharacter::UnLimitControl()
{
	Super::UnLimitControl();

	GetController()->SetIgnoreLookInput(false);
}

FRotator AFPPlayerCharacter::GetViewRotation() const
{
	FRotator Result = Super::GetViewRotation();

	if (IsFPMontagePlaying())
	{
		FRotator SocketRotation = FirstPersonMeshComponent->GetSocketRotation(SocketFPCamera);
		
		Result.Pitch += SocketRotation.Pitch;
		Result.Yaw = SocketRotation.Yaw;
		Result.Roll = SocketRotation.Roll;

	}

	return Result;
}

void AFPPlayerCharacter::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode /*= 0 */)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if (GetBaseCharacterMovementComponent()->IsOnLadder())
	{
		OnLadderStarted();

	}
	else if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == (uint8)ECustomMovementMode::CMOVE_Ladder)
	{
		OnLadderStopped();

	}

	//Zipline
	if (GetBaseCharacterMovementComponent()->IsOnZipline())
	{
		OnZiplineStarted();
	}
	else if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == (uint8)ECustomMovementMode::CMOVE_Zipline)
	{
		OnZiplineStoped();
	}

}

void AFPPlayerCharacter::OnMantle(const FMantlingSetting& MantlingSetting, float MantlingAnimationStartTime)
{
	Super::OnMantle(MantlingSetting, MantlingAnimationStartTime);

	UAnimInstance* FPAnimInstance = FirstPersonMeshComponent->GetAnimInstance();
	if (IsValid(FPAnimInstance) && MantlingSetting.FPMantlingMontage)
	{	
		if (GCPlayerController.IsValid())
		{
			GCPlayerController->SetIgnoreLookInput(true);
			GCPlayerController->SetIgnoreMoveInput(true);
		}

		float MontageDuration = FPAnimInstance->Montage_Play(MantlingSetting.FPMantlingMontage, 1.0f, EMontagePlayReturnType::Duration, MantlingAnimationStartTime);
		GetWorld()->GetTimerManager().SetTimer(FPMontageTimer, this, &AFPPlayerCharacter::OnFPMontageTimerElapsed, MontageDuration, false);
	}
}



void AFPPlayerCharacter::OnLadderStarted()
{
	if (GCPlayerController.IsValid())
	{
		GCPlayerController->SetIgnoreCameraPitch(true);
		bUseControllerRotationYaw = false;

		APlayerCameraManager* CurrentCameraManager = GCPlayerController->PlayerCameraManager;
		CurrentCameraManager->ViewPitchMin = LadderCameraMinPitch;
		CurrentCameraManager->ViewPitchMax = LadderCameraMaxPitch;
		CurrentCameraManager->ViewYawMin = LadderCameraMinYaw;
		CurrentCameraManager->ViewYawMax = LadderCameraMaxYaw;

	}
}

void AFPPlayerCharacter::OnLadderStopped()
{
	if (GCPlayerController.IsValid())
	{
		GCPlayerController->SetIgnoreCameraPitch(false);
		bUseControllerRotationYaw = true;

		APlayerCameraManager* CurrentCameraManager = GCPlayerController->PlayerCameraManager;
		APlayerCameraManager* DefaultCameraManager = CurrentCameraManager->GetClass()->GetDefaultObject<APlayerCameraManager>();

		CurrentCameraManager->ViewPitchMin = DefaultCameraManager->ViewPitchMin;
		CurrentCameraManager->ViewPitchMax = DefaultCameraManager->ViewPitchMax;
		CurrentCameraManager->ViewYawMin = DefaultCameraManager->ViewYawMin;
		CurrentCameraManager->ViewYawMax = DefaultCameraManager->ViewYawMax;
	}
}

void AFPPlayerCharacter::OnZiplineStarted()
{
	if (GCPlayerController.IsValid())
	{
		GCPlayerController->SetIgnoreCameraPitch(true);
		bUseControllerRotationYaw = false;

		APlayerCameraManager* CurrentCameraManager = GCPlayerController->PlayerCameraManager;
		
		CurrentCameraManager->ViewPitchMin = ZiplineCameraMinPitch;
		CurrentCameraManager->ViewPitchMax = ZiplineCameraMaxPitch;
		CurrentCameraManager->ViewYawMin = ZiplineCameraMinYaw;
		CurrentCameraManager->ViewYawMax = ZiplineCameraMaxYaw;
	}
}

void AFPPlayerCharacter::OnZiplineStoped()
{
	if (GCPlayerController.IsValid())
	{
		GCPlayerController->SetIgnoreCameraPitch(false);
		bUseControllerRotationYaw = true;

		APlayerCameraManager* CurrentCameraManager = GCPlayerController->PlayerCameraManager;
		APlayerCameraManager* DefaultCameraManager = CurrentCameraManager->GetClass()->GetDefaultObject<APlayerCameraManager>();

		CurrentCameraManager->ViewPitchMin = DefaultCameraManager->ViewPitchMin;
		CurrentCameraManager->ViewPitchMax = DefaultCameraManager->ViewPitchMax;
		CurrentCameraManager->ViewYawMin = DefaultCameraManager->ViewYawMin;
		CurrentCameraManager->ViewYawMax = DefaultCameraManager->ViewYawMax;
	}
}

bool AFPPlayerCharacter::IsFPMontagePlaying() const
{
	UAnimInstance* FPAnimInstance = FirstPersonMeshComponent->GetAnimInstance();
	return IsValid(FPAnimInstance) && FPAnimInstance->IsAnyMontagePlaying();

}

void AFPPlayerCharacter::OnFPMontageTimerElapsed()
{
	if (GCPlayerController.IsValid())
	{
		GCPlayerController->SetIgnoreLookInput(false);
		GCPlayerController->SetIgnoreMoveInput(false);
	}
}
