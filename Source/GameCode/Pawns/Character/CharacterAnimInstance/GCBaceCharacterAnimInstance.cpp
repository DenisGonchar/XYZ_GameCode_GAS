// Fill out your copyright notice in the Description page of Project Settings.


#include "GCBaceCharacterAnimInstance.h"
#include "../GCBaseCharacter.h"
#include "../CharacterMovementComponent/GCBaseCharacterMovementComponent.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "Kismet/KismetMathLibrary.h"

void UGCBaceCharacterAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	checkf(TryGetPawnOwner()->IsA<AGCBaseCharacter>(),TEXT("UGCBaceCharacterAnimInstance::NativeBeginPlay() UGCBaceCharacterAnimInstance can be used only with UGCBaceCharacterAnimInstance"));
	CachedBaseCharacter = StaticCast<AGCBaseCharacter*>(TryGetPawnOwner());

}

void UGCBaceCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (!CachedBaseCharacter.IsValid())
	{
		return;
	}

	bIsAiming = CachedBaseCharacter->IsAiming();

	UGCBaseCharacterMovementComponent* CharacterMovement = CachedBaseCharacter->GetBaseCharacterMovementComponent();

	Speed = CharacterMovement->Velocity.Size();
	bIsFalling = CharacterMovement->IsFalling();
	bIsCrouching = CharacterMovement->IsCrouching();
	bIsSprinting = CharacterMovement->IsSprinting();
	bIsOutOfStamina = CharacterMovement->IsOutOfStamina();
	bIsOutOfProne = CharacterMovement->IsProning();
	bIsSwimming = CharacterMovement->IsSwimming();
	bIsOnLadder = CharacterMovement->IsOnLadder();
	bIsOnZipline = CharacterMovement->IsOnZipline();
	bIsOnWallRun = CharacterMovement->IsWallRuning();
	bIsOnRockClimbing = CharacterMovement->IsRockClimbing();
	
	if (bIsOnLadder)
	{
		LadderSpeedRatio = CharacterMovement->GetLadderSpeedRatio();

	}

	if (bIsOnWallRun)
	{
		WallRunSide = CharacterMovement->GetCurrentWallRunSide();
	}


	bIsStrafing = !CharacterMovement->bOrientRotationToMovement;
	Direction = CalculateDirection(CharacterMovement->Velocity, CachedBaseCharacter->GetActorRotation());

	AimRotation = CachedBaseCharacter->GetAimOffset();

	//
	RightFootEffectorLocation = FVector((CachedBaseCharacter->GetIKRightFootSocketOffset() + CachedBaseCharacter->GetIKPelvisSocketOffset()), 0.0f, 0.0f);
	LeftFootEffectorLocation = FVector(-(CachedBaseCharacter->GetIKLeftFootSocketOffset() + CachedBaseCharacter->GetIKPelvisSocketOffset()), 0.0f, 0.0f);
	PelvisEffectorLocation = FVector(0.0f, 0.0f ,CachedBaseCharacter->GetIKPelvisSocketOffset());
	//

	const UCharacterEquipmentComponent* CharacterEquipment = CachedBaseCharacter->GetCharacterEquipmentComponent();
	CurrentEquipbleItemType = CharacterEquipment->GetCurrentEquipperItemType();

	ARangeWeaponItem* CurrentRangeWeapon = CharacterEquipment->GetCurrentRangeWeapon();
	if (IsValid(CurrentRangeWeapon))
	{
		ForeGripSocketTransform = CurrentRangeWeapon->GetForeGripTransform();
	}
	
	FRotator CurrentRotation = CachedBaseCharacter->GetActorRotation();
	RotationSpeed = UKismetMathLibrary::NormalizedDeltaRotator(CurrentRotation, PreviousCharacterRotation);
	RotationSpeed *= 1.0f / DeltaSeconds;
	PreviousCharacterRotation = CurrentRotation;
	
}
