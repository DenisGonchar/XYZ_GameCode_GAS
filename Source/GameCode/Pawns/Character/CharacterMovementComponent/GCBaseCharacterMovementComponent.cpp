// Fill out your copyright notice in the Description page of Project Settings.


#include "GCBaseCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Curves/CurveVector.h"
#include "../../../Actors/Interactive/Environment/Ladder.h"
#include "../../../Actors/Interactive/Environment/Zipline.h"
#include "../GCBaseCharacter.h"
#include "../PlayerCharacter.h"
#include <Kismet/KismetMathLibrary.h>
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "Subsystems/DebugSubsystem.h"
#include <Utils/GCTraceUtils.h>

#include "ChaosInterfaceWrapperCore.h"
#include "Components/CharacterComponents/CharacterMoveComponent.h"

FNetworkPredictionData_Client_Character* UGCBaseCharacterMovementComponent::GetPredictionData_Client() const
{
	if (ClientPredictionData == nullptr)
	{
		UGCBaseCharacterMovementComponent* MutableThis = const_cast<UGCBaseCharacterMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Character_GC(*this);

	}
	return ClientPredictionData;
}

void UGCBaseCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);
	
	/*
		FLAG_Reserved_1 = 0x04,	// Reserved for future use
		FLAG_Reserved_2 = 0x08,	// Reserved for future use
		// Remaining bit masks are available for custom flags.
		FLAG_Custom_0 = 0x10, - Sprinting flag
		FLAG_Custom_1 = 0x20, - Mantling
		FLAG_Custom_2 = 0x40, - Slide
		FLAG_Custom_3 = 0x80,
	*/

	bool bWasMantling = GetBaseCharacterOwner()->GetCharacterMoveComponent()->bIsMantling;
	bool bWasSlide = GetBaseCharacterOwner()->GetCharacterMoveComponent()->bIsSliding;
	
	bIsSprinting = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
	bool bIsMantling = (Flags & FSavedMove_Character::FLAG_Custom_1) != 0;
	bool bIsSlide = (Flags & FSavedMove_Character::FLAG_Custom_2) != 0;
	
	if (GetBaseCharacterOwner()->GetLocalRole() == ROLE_Authority)
	{
		if (!bWasMantling && bIsMantling)
		{
			GetBaseCharacterOwner()->Mantle(true);
		}

		if (!bWasSlide && bIsSlide)
		{
			GetBaseCharacterOwner()->Slide();
		}
	}

}

void UGCBaseCharacterMovementComponent::PhysicsRotation(float DeltaTime)
{
	if (GetBaseCharacterOwner()->GetLocalRole() == ROLE_SimulatedProxy)
	{
		return;
	}
			
	if (bForceRotation)
	{
		FRotator CurrentRotation = UpdatedComponent->GetComponentRotation(); // Normalized
		CurrentRotation.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): CurrentRotation"));

		FRotator DeltaRot = GetDeltaRotation(DeltaTime);
		DeltaRot.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): GetDeltaRotation"));

		// Accumulate a desired new rotation.
		const float AngleTolerance = 1e-3f;

		if (!CurrentRotation.Equals(ForceTargetRotation, AngleTolerance))
		{
			FRotator DesiredRotation = ForceTargetRotation;

			// PITCH
			if (!FMath::IsNearlyEqual(CurrentRotation.Pitch, DesiredRotation.Pitch, AngleTolerance))
			{
				DesiredRotation.Pitch = FMath::FixedTurn(CurrentRotation.Pitch, DesiredRotation.Pitch, DeltaRot.Pitch);
			}

			// YAW
			if (!FMath::IsNearlyEqual(CurrentRotation.Yaw, DesiredRotation.Yaw, AngleTolerance))
			{
				DesiredRotation.Yaw = FMath::FixedTurn(CurrentRotation.Yaw, DesiredRotation.Yaw, DeltaRot.Yaw);
			}

			// ROLL
			if (!FMath::IsNearlyEqual(CurrentRotation.Roll, DesiredRotation.Roll, AngleTolerance))
			{
				DesiredRotation.Roll = FMath::FixedTurn(CurrentRotation.Roll, DesiredRotation.Roll, DeltaRot.Roll);
			}

			// Set the new rotation.
			DesiredRotation.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): DesiredRotation"));
			MoveUpdatedComponent(FVector::ZeroVector, DesiredRotation, /*bSweep*/ false);
		}
		else
		{	
			ForceTargetRotation = FRotator::ZeroRotator;
			bForceRotation = false;
		}
		return;
	}
	
	
	if (IsOnLadder())
	{
		return;

	}

	if (IsOnZipline())
	{	
		return;
	}
	
	if (IsSliding())
	{
		return;
	}


	Super::PhysicsRotation(DeltaTime);
}

void UGCBaseCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	BaseCharacterOwner = Cast<AGCBaseCharacter>(GetOwner());
}

void UGCBaseCharacterMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FVector PendingInput = GetPendingInputVector().GetClampedToMaxSize(1.0f);
	FVector CharacterSpacePendingInput = UKismetMathLibrary::InverseTransformDirection(GetOwner()->GetTransform(), PendingInput);

	if (IsSliding())
	{

		FVector SlidingMovingDirection = GetOwner()->GetActorForwardVector();
		FRotator SlidingMovingRotator = SlidingMovingDirection.Rotation();
		
		SlidingMovingRotator.Yaw += SlideStrafeAngleValue * CharacterSpacePendingInput.Y * DeltaTime;
		SlidingMovingDirection = SlidingMovingRotator.Vector();
		
		Velocity = SlidingMovingDirection * GetMaxSpeed();

		GetOwner()->SetActorRotation(Velocity.ToOrientationRotator());
		UpdateComponentVelocity();
		
	}
}

float UGCBaseCharacterMovementComponent::GetMaxSpeed() const
{
	float Result = Super::GetMaxSpeed();
	
	if (bIsSprinting)
	{
		Result = SprintSpeed;
	}
	else if (bIsOutOfStamina)
	{
		Result = OutOfStaminaSpeed;
	}
	else if(IsProning())
	{
		Result = OutOfProne;
	}
	else if (IsOnLadder())
	{
		Result = ClimbingOnLadderMaxSpeed;
	}
	else if (IsOnZipline())
	{
		Result = DescentOnZiplineMaxSpeed;
	}
	else if (IsWallRuning())
	{
		Result = WallRunSpeed;
	}
	else if (IsSliding())
	{
		Result = SlideSpeed;
	}
	else if (GetBaseCharacterOwner()->IsAiming())
	{
		Result = GetBaseCharacterOwner()->GetAimingMovementSpeed();

	}

	return Result;
}

void UGCBaseCharacterMovementComponent::StartSprint()
{
	bIsSprinting = true;
	bForceMaxAccel = 1;
}

void UGCBaseCharacterMovementComponent::StopSprint()
{
	bIsSprinting = false;
	bForceMaxAccel = 0;
}

FORCEINLINE bool UGCBaseCharacterMovementComponent::IsProning() const
{
	return BaseCharacterOwner && BaseCharacterOwner->bIsProned;
}

void UGCBaseCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);

	if (BaseCharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)
	{
		const bool bIsProne = IsProning();
		if (bIsProne && (!bWantsToProne || !CanProneInCurrentState()))
		{
			UnProne(false);
		}
		else if (!bIsProne && bWantsToProne && CanProneInCurrentState())
		{
			Prone(false);
		}
	}

}

bool UGCBaseCharacterMovementComponent::CanProneInCurrentState() const
{
	return  IsMovingOnGround() && UpdatedComponent && !UpdatedComponent->IsSimulatingPhysics();
}

void UGCBaseCharacterMovementComponent::Prone(bool bClientSimulation /*= false*/)
{
	if (!HasValidData())
	{
		return;
	}

	if (!CanProneInCurrentState())
	{
		return;
	}

	//if (CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() == PronedHalfHeight)
	if(FMath::IsNearlyEqual(BaseCharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight(), PronedHalfHeight))
	{
		if (!bClientSimulation)
		{
			BaseCharacterOwner->bIsProned = true;
			bWantsToCrouch = false;
			BaseCharacterOwner->bIsCrouched = false;
		}
		BaseCharacterOwner->OnStartProne(0.f, 0.f);
		return;
	}
	//
	// Change collision size to crouching dimensions
	const float ComponentScale = BaseCharacterOwner->GetCapsuleComponent()->GetShapeScale();
	const float OldUnscaledHalfHeight = BaseCharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float OldUnscaledRadius = BaseCharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	// Height is not allowed to be smaller than radius.

	
	const float ClampedPronedHalfHeight = (FMath::Max3(0.f, OldUnscaledRadius, CrouchedHalfHeight))/2;
	BaseCharacterOwner->GetCapsuleComponent()->SetCapsuleSize(OldUnscaledRadius, ClampedPronedHalfHeight);
	
	
	float HalfHeightAdjust = (OldUnscaledHalfHeight - ClampedPronedHalfHeight);
	float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;

	if (!bClientSimulation)
	{
		// Crouching to a larger height? (this is rare)
		if (ClampedPronedHalfHeight > OldUnscaledHalfHeight)
		{
			FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(ProneTrace), false, CharacterOwner);
			FCollisionResponseParams ResponseParam;
			InitCollisionParams(CapsuleParams, ResponseParam);
			const bool bEncroached = GetWorld()->OverlapBlockingTestByChannel(UpdatedComponent->GetComponentLocation() - FVector(0.f, 0.f, ScaledHalfHeightAdjust), FQuat::Identity,
				UpdatedComponent->GetCollisionObjectType(), GetPawnCapsuleCollisionShape(SHRINK_None), CapsuleParams, ResponseParam);

			// If encroached, cancel
			if (bEncroached)
			{
				BaseCharacterOwner->GetCapsuleComponent()->SetCapsuleSize(OldUnscaledRadius, OldUnscaledHalfHeight);
				return;
			}
		}

		if (bProneMaintainsBaseLocation)
		{
			// Intentionally not using MoveUpdatedComponent, where a horizontal plane constraint would prevent the base of the capsule from staying at the same spot.
			UpdatedComponent->MoveComponent(FVector(0.f, 0.f, -ScaledHalfHeightAdjust), UpdatedComponent->GetComponentQuat(), true, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
		}

		BaseCharacterOwner->bIsProned = true;
		bWantsToCrouch = false;
		BaseCharacterOwner->bIsCrouched = false;
	}

	bForceNextFloorCheck = true;

	// OnStartCrouch takes the change from the Default size, not the current one (though they are usually the same).
	//const float MeshAdjust = ScaledHalfHeightAdjust;
	ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
	HalfHeightAdjust = (DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() - CrouchedHalfHeight - ClampedPronedHalfHeight);
	ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;

	AdjustProxyCapsuleSize();
	BaseCharacterOwner->OnStartProne(HalfHeightAdjust, ScaledHalfHeightAdjust);

	//GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Green, FString::Printf(TEXT("HalfHeightAdjust = %.2f, ScaledHalfHeightAdjust = %.2f "), HalfHeightAdjust, ScaledHalfHeightAdjust));
}

void UGCBaseCharacterMovementComponent::UnProne(bool bClientSimulation)
{
	if (!HasValidData())
	{
		return;
	}

	ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();

	float DesiredHeight = bIsFullHeight ? DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() : CrouchedHalfHeight;

	// See if collision is already at desired size.
	if (CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() == DesiredHeight)
	{
		if (!bClientSimulation)
		{
			BaseCharacterOwner->bIsCrouched = false;
			bWantsToCrouch = !bIsFullHeight;
			BaseCharacterOwner->bIsCrouched = !bIsFullHeight;
		}
		BaseCharacterOwner->OnEndProne(0.f, 0.f);
		return;
	}

	const float CurrentPronedHalfHeight = BaseCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	const float ComponentScale = BaseCharacterOwner->GetCapsuleComponent()->GetShapeScale();
	const float OldUnscaledHalfHeight = BaseCharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	float HalfHeightAdjust = DesiredHeight - OldUnscaledHalfHeight;
	float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;
	const FVector PawnLocation = UpdatedComponent->GetComponentLocation();

	// Grow to uncrouched size.
	check(CharacterOwner->GetCapsuleComponent());

	if (!bClientSimulation)
	{
		// Try to stay in place and see if the larger capsule fits. We use a slightly taller capsule to avoid penetration.
		const UWorld* MyWorld = GetWorld();
		const float SweepInflation = KINDA_SMALL_NUMBER * 10.f;
		FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(CrouchTrace), false, CharacterOwner);
		FCollisionResponseParams ResponseParam;
		InitCollisionParams(CapsuleParams, ResponseParam);

		// Compensate for the difference between current capsule size and standing size
		const FCollisionShape StandingCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, -SweepInflation - ScaledHalfHeightAdjust); // Shrink by negative amount, so actually grow it.
		const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();
		bool bEncroached = true;

		if (!bProneMaintainsBaseLocation)
		{
			// Expand in place
			bEncroached = MyWorld->OverlapBlockingTestByChannel(PawnLocation, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);

			if (bEncroached)
			{
				// Try adjusting capsule position to see if we can avoid encroachment.
				if (ScaledHalfHeightAdjust > 0.f)
				{
					// Shrink to a short capsule, sweep down to base to find where that would hit something, and then try to stand up from there.
					float PawnRadius, PawnHalfHeight;
					BaseCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleSize(PawnRadius, PawnHalfHeight);
					const float ShrinkHalfHeight = PawnHalfHeight - PawnRadius;
					const float TraceDist = PawnHalfHeight - ShrinkHalfHeight;
					const FVector Down = FVector(0.f, 0.f, -TraceDist);

					FHitResult Hit(1.f);
					const FCollisionShape ShortCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, ShrinkHalfHeight);
					const bool bBlockingHit = MyWorld->SweepSingleByChannel(Hit, PawnLocation, PawnLocation + Down, FQuat::Identity, CollisionChannel, ShortCapsuleShape, CapsuleParams);
					if (Hit.bStartPenetrating)
					{
						bEncroached = true;
					}
					else
					{
						// Compute where the base of the sweep ended up, and see if we can stand there
						const float DistanceToBase = (Hit.Time * TraceDist) + ShortCapsuleShape.Capsule.HalfHeight;
						const FVector NewLoc = FVector(PawnLocation.X, PawnLocation.Y, PawnLocation.Z - DistanceToBase + StandingCapsuleShape.Capsule.HalfHeight + SweepInflation + MIN_FLOOR_DIST / 2.f);
						bEncroached = MyWorld->OverlapBlockingTestByChannel(NewLoc, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);
						if (!bEncroached)
						{
							// Intentionally not using MoveUpdatedComponent, where a horizontal plane constraint would prevent the base of the capsule from staying at the same spot.
							UpdatedComponent->MoveComponent(NewLoc - PawnLocation, UpdatedComponent->GetComponentQuat(), false, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
						}
					}
				}
			}
		}
		else
		{
			// Expand while keeping base location the same.
			FVector StandingLocation = PawnLocation + FVector(0.f, 0.f, StandingCapsuleShape.GetCapsuleHalfHeight() - CurrentPronedHalfHeight);
			bEncroached = MyWorld->OverlapBlockingTestByChannel(StandingLocation, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);

			if (bEncroached)
			{
				if (IsMovingOnGround())
				{
					// Something might be just barely overhead, try moving down closer to the floor to avoid it.
					const float MinFloorDist = KINDA_SMALL_NUMBER * 10.f;
					if (CurrentFloor.bBlockingHit && CurrentFloor.FloorDist > MinFloorDist)
					{
						StandingLocation.Z -= CurrentFloor.FloorDist - MinFloorDist;
						bEncroached = MyWorld->OverlapBlockingTestByChannel(StandingLocation, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);
					}
				}
			}

			if (!bEncroached)
			{
				// Commit the change in location.
				UpdatedComponent->MoveComponent(StandingLocation - PawnLocation, UpdatedComponent->GetComponentQuat(), false, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
				bForceNextFloorCheck = true;
			}
		}

		// If still encroached then abort.
		if (bEncroached)
		{
			return;
		}

		BaseCharacterOwner->bIsProned = false;
		bWantsToCrouch = !bIsFullHeight;
		BaseCharacterOwner->bIsCrouched = !bIsFullHeight;

	}
	else
	{
		bShrinkProxyCapsule = true;
	}

	// Now call SetCapsuleSize() to cause touch/untouch events and actually grow the capsule
	BaseCharacterOwner->GetCapsuleComponent()->SetCapsuleSize(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), DesiredHeight, true);

	HalfHeightAdjust = bIsFullHeight ? CrouchedHalfHeight - PronedHalfHeight : DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() - DesiredHeight - PronedHalfHeight;

	ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;

	AdjustProxyCapsuleSize();
	BaseCharacterOwner->OnEndProne(HalfHeightAdjust, ScaledHalfHeightAdjust);

}

void UGCBaseCharacterMovementComponent::StartMantle(const FMantlingMovementParameters& MantlingParameters)
{
	CurrentMantlingParameters = MantlingParameters;
	SetMovementMode(EMovementMode::MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_Mantling);
	
}

void UGCBaseCharacterMovementComponent::EndMantle()
{
	GetBaseCharacterOwner()->GetCharacterMoveComponent()->bIsMantling = false;
	SetMovementMode(MOVE_Walking);
}

bool UGCBaseCharacterMovementComponent::IsMantling() const
{
	return UpdatedComponent && MovementMode == MOVE_Custom && CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_Mantling;
}

void UGCBaseCharacterMovementComponent::AttachToRockClimbing(const FMantlingMovementParameters& RockClimbingParameters)
{
	CurrentRockClimbingParameters = RockClimbingParameters;
	
	FVector ClimbingUpVector = CurrentRockClimbingParameters.PlatformMesh->GetUpVector();
	float Projection = GetActorToCurrentRockClimbingProjection(RockClimbingParameters.TargetLocation);
	//NewClimbingCharacterLocation = RockClimbingParameters.PlatforMeshLocation + Projection * ClimbingUpVector + RockClimbingOffset * RockClimbingParameters.PlatformMesh->GetForwardVector();
	NewClimbingCharacterLocation = RockClimbingParameters.TargetLocation;
	
	//Point(GetWorld(), NewClimbingCharacterLocation, 20, FColor::Orange, true, 100);
	//DrawDebugLine(GetWorld(), GetOwner()->GetActorLocation(), NewClimbingCharacterLocation, FColor::Green, true, 100);
	
	SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_RockClimbing);
}

float UGCBaseCharacterMovementComponent::GetActorToCurrentRockClimbingProjection(const FVector& Location) const
{
	FVector ClimbingUpVector = CurrentRockClimbingParameters.PlatformMesh->GetUpVector();
	FVector ClimbingToCharacterDistance = Location - CurrentRockClimbingParameters.PlatforMeshLocation;
	
	return FVector::DotProduct(ClimbingUpVector, ClimbingToCharacterDistance);
}

void UGCBaseCharacterMovementComponent::DetachFromRockClimbing(
	EDetachFromRockClimbingMethod DetachFromRockClimbingMethod)
{
	switch (DetachFromRockClimbingMethod)
	{
		case EDetachFromRockClimbingMethod::Fall:
			{
				SetMovementMode(MOVE_Falling);
				break;
			}
			
		case EDetachFromRockClimbingMethod::JumpOff:
			{
				FVector JumpDirection = CurrentRockClimbingParameters.PlatformActor->GetActorRightVector() * -1;

				SetMovementMode(MOVE_Falling);
				
				FVector JumpVelocity = JumpDirection * JumpOffFromLadderSpeed;

				ForceTargetRotation = JumpDirection.ToOrientationRotator();
				bForceRotation = true;
				
				Launch(JumpVelocity);

				break;
			}
		
	}
	
}

bool UGCBaseCharacterMovementComponent::IsRockClimbing() const
{
	return UpdatedComponent && MovementMode == MOVE_Custom && CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_RockClimbing;

}

void UGCBaseCharacterMovementComponent::StopAnimClimbing()
{
	
}

void UGCBaseCharacterMovementComponent::AttachToLadder(const class ALadder* Ladder)
{
	CurrentLadder = Ladder;
	FRotator TargetOrintationRotation = CurrentLadder->GetActorForwardVector().ToOrientationRotator();
	TargetOrintationRotation.Yaw += 180.0f;
	
	FVector LadderUpVector = CurrentLadder->GetActorUpVector();
	FVector LadderForwardActor = CurrentLadder->GetActorForwardVector();

	float Projection = GetActorToCurrentLadderProjection(GetActorLocation());

	FVector NewCharacterLocation = CurrentLadder->GetActorLocation() + Projection * LadderUpVector + LadderToCharacterOffset * LadderForwardActor;

	if (CurrentLadder->GetIsOnTop())
	{
		NewCharacterLocation = CurrentLadder->GetAttachTopAnimMontageStartingLocation();
	}

	GetOwner()->SetActorLocation(FVector(NewCharacterLocation.X, NewCharacterLocation.Y, NewCharacterLocation.Z + 10.0f));
	GetOwner()->SetActorRotation(TargetOrintationRotation);

	SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_Ladder);
}

float UGCBaseCharacterMovementComponent::GetActorToCurrentLadderProjection(const FVector& Location) const
{
	checkf(IsValid(CurrentLadder), TEXT("UGCBaseCharacterMovementComponent::GetCharacterToCurrentLadderProjection() cannot be invoked when current ladder is null"));

	FVector LadderUpVector = CurrentLadder->GetActorUpVector();
	FVector LadderToCharacterDistance = Location - CurrentLadder->GetActorLocation();
	return FVector::DotProduct(LadderUpVector, LadderToCharacterDistance);

}

float UGCBaseCharacterMovementComponent::GetLadderSpeedRatio() const
{
	checkf(IsValid(CurrentLadder), TEXT("UGCBaseCharacterMovementComponent::GetCharacterToCurrentLadderProjection() cannot be invoked when current ladder is null"));

	FVector LadderUpVector = CurrentLadder->GetActorUpVector();
	
	return FVector::DotProduct(LadderUpVector, Velocity) / ClimbingOnLadderMaxSpeed;
}

void UGCBaseCharacterMovementComponent::DetachFromLadder(EDetachFromLadderMethod DetachFromLadderMethod /*= EDetachFromLadderMethod::Fall*/)
{	
	switch (DetachFromLadderMethod)
	{
	
		case EDetachFromLadderMethod::ReachingTheTop:
		{
			GetBaseCharacterOwner()->Mantle(true);

			break;
		}	
		case EDetachFromLadderMethod::ReachingTheBottom:
		{
			SetMovementMode(MOVE_Walking);

			break;
		}
		case EDetachFromLadderMethod::JumpOff:
		{
			FVector JumpDirection = CurrentLadder->GetActorForwardVector();

			SetMovementMode(MOVE_Falling);
			
			FVector JumpVelocity = JumpDirection * JumpOffFromLadderSpeed;

			ForceTargetRotation = JumpDirection.ToOrientationRotator();
			bForceRotation = true;
			
			Launch(JumpVelocity);

			break;
		}
		case EDetachFromLadderMethod::Fall:
		default:
		{
			SetMovementMode(MOVE_Falling);
			break;
		}
	}
}

bool UGCBaseCharacterMovementComponent::IsOnLadder() const
{
	return UpdatedComponent && MovementMode == MOVE_Custom && CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_Ladder;

}
const class ALadder* UGCBaseCharacterMovementComponent::GetCurrentLadder()
{
	return CurrentLadder;
}

void UGCBaseCharacterMovementComponent::AttachToZipline(const class AZipline* Zipline)
{
	
	CurrentZipline = Zipline;

	FVector StartRailUpPostionWorldLocation = CurrentZipline->StartBoxComponent->GetComponentLocation();
	FVector EndRailUpPostionWorldLocation = CurrentZipline->EndBoxComponent->GetComponentLocation();
	FVector ZiplineCableWorldLocation = CurrentZipline->CableMechComponent->GetComponentLocation();

	FVector MovingDirection = EndRailUpPostionWorldLocation - StartRailUpPostionWorldLocation;
	MovingDirection.Normalize();
	
	//Rotation
	FRotator TargetOrintationRotation = MovingDirection.ToOrientationRotator();
	GetOwner()->SetActorRotation(FRotator( 0.0f, TargetOrintationRotation.Yaw, 0.0f));

	//Location
	
	float Project = GetActorToCurrentZiplineProjection(GetActorLocation());

	FVector NewCharacterLocation = StartRailUpPostionWorldLocation + Project * MovingDirection  - ZiplineToCharacterOffset * CurrentZipline->StartRailMechComponent->GetUpVector();
	GetOwner()->SetActorRelativeLocation(NewCharacterLocation);

	SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_Zipline);
}

float UGCBaseCharacterMovementComponent::GetActorToCurrentZiplineProjection(const FVector& Location) const
{
	checkf(IsValid(CurrentZipline), TEXT("UGCBaseCharacterMovementComponent::GetCharacterToCurrentZiplineProjection() cannot be invoked when current Zipline is null"));

	FVector ZiplineToCharacterVector = Location - CurrentZipline->StartBoxComponent->GetComponentLocation();

	FVector StartRailUpPostionWorldLocation = CurrentZipline->StartBoxComponent->GetComponentLocation();
	FVector EndRailUpPostionWorldLocation = CurrentZipline->EndBoxComponent->GetComponentLocation();
	FVector MovingDirection = EndRailUpPostionWorldLocation - StartRailUpPostionWorldLocation;
	MovingDirection.Normalize();

	return FVector::DotProduct(MovingDirection, ZiplineToCharacterVector);

}

float UGCBaseCharacterMovementComponent::GetZiplineSpeedRatio() const
{
	checkf(IsValid(CurrentZipline), TEXT("UGCBaseCharacterMovementComponent::GetCharacterToCurrentZiplineProjection() cannot be invoked when current Zipline is null"));

	FVector ZiplineVector = CurrentZipline->GetActorUpVector();

	return FVector::DotProduct(ZiplineVector, Velocity) / DescentOnZiplineMaxSpeed;
}

bool UGCBaseCharacterMovementComponent::IsOnZipline() const
{
	return UpdatedComponent && MovementMode == MOVE_Custom && CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_Zipline;
}

void UGCBaseCharacterMovementComponent::DetachFromZipline(EDetachFromZiplineMethod DetachFromZiplineMethod /*= EDetachFromZiplineMethod::Fall*/)
{
	switch (DetachFromZiplineMethod)
	{

	
	case EDetachFromZiplineMethod::JumpOff:
	{
		
		SetMovementMode(MOVE_Falling);

		break;
	}
	case EDetachFromZiplineMethod::Fall:
	default:
	{
		SetMovementMode(MOVE_Falling);
		break;
	}
	}
}

const class AZipline* UGCBaseCharacterMovementComponent::GetCurrentZipline()
{
	return CurrentZipline;
}


void UGCBaseCharacterMovementComponent::AttachToWallRun()
{
	UCapsuleComponent* CharacterCapsule = GetBaseCharacterOwner()->GetCapsuleComponent();

	FHitResult TraceHitResult;

#if ENABLE_DRAW_DEBUG
	UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
	bool IsDebugEnabled = DebugSubsystem->IsCategoryEnabled(DebugCategoryWallRun);
#else
	bool IsDebugEnabled = false;
#endif
	
	if (!DetectWallRun(TraceHitResult, CharacterCapsule->GetComponentLocation()))
	{
		return;
	}

	FVector HitNormal = TraceHitResult.ImpactNormal.GetSafeNormal();

	bool bIsAngleCorrect = false;
	float CharacterAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(-HitNormal, CharacterCapsule->GetForwardVector())));
	
	if ((CharacterAngle > WallRunMinAngleDeg) && (CharacterAngle < WallRunMaxAngleDeg))
	{
		bIsAngleCorrect = true;
	}
	
	if (IsDebugEnabled)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Purple, FString::Printf(TEXT("Angle is not corrent. Angle = %.2f"), CharacterAngle));

	}

	if (FVector::DotProduct(HitNormal, CharacterCapsule->GetRightVector()) > 0)
	{
		CurrentWallRunSide = EWallRunSide::Left;
	}
	else
	{
		CurrentWallRunSide = EWallRunSide::Right;
	}

	if (CurrentWallRunSide == PreviousWallRunSide)
	{
		return;
	}

	if (IsDebugEnabled)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Purple, FString::Printf(TEXT("WallRun side: %s"), *UEnum::GetValueAsString(CurrentWallRunSide)));
	}

	StartWallRun(CharacterCapsule, TraceHitResult);
}

bool UGCBaseCharacterMovementComponent::DetectWallRun(FHitResult& OutHit, FVector CharacterLocation)
{

#if ENABLE_DRAW_DEBUG
	UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
	bool IsDebugEnabled = DebugSubsystem->IsCategoryEnabled(DebugCategoryWallRun);
#else
	bool IsDebugEnabled = false;
#endif
	
	FCollisionQueryParams QuerryParams;
	QuerryParams.bTraceComplex = true;
	QuerryParams.AddIgnoredActor(GetOwner());

	UCapsuleComponent* CharacterCapsule = GetBaseCharacterOwner()->GetCapsuleComponent();

	FVector LeftCheckStart = CharacterLocation;
	FVector LeftCheckEnd = CharacterLocation - CharacterCapsule->GetRightVector() * WallRunTraceLenght;

	bool bIsHitedLeft = GCTraceUtils::LineTraceSingleByChannel(GetWorld(), OutHit, LeftCheckStart, LeftCheckEnd, ECC_WallRannable, QuerryParams, FCollisionResponseParams::DefaultResponseParam, IsDebugEnabled, 10);
	if (OutHit.bBlockingHit)
	{
		return bIsHitedLeft;

	}

	FVector RightCheckStart = LeftCheckStart;
	FVector RightCheckEnd = CharacterLocation + CharacterCapsule->GetRightVector() * WallRunTraceLenght;

	bool bIsHitedRight = GCTraceUtils::LineTraceSingleByChannel(GetWorld(), OutHit, RightCheckStart, RightCheckEnd, ECC_WallRannable, QuerryParams, FCollisionResponseParams::DefaultResponseParam, IsDebugEnabled, 10);

	return bIsHitedRight;
}

void UGCBaseCharacterMovementComponent::JumpOffWall()
{
	StopWallRun();

	FVector MovingDirection;
	FVector JumpOffVelocity;

	if (CurrentWallRunSide == EWallRunSide::Left)
	{
		MovingDirection = GetOwner()->GetActorRightVector() + Velocity.GetSafeNormal();
	}
	else
	{
		MovingDirection = -GetOwner()->GetActorRightVector() + Velocity.GetSafeNormal();
	}

	JumpOffVelocity = MovingDirection * JumpOffWallRunHorizontalVelocity;
	ForceTargetRotation = MovingDirection.ToOrientationRotator();

	bForceRotation = true;

	JumpOffVelocity += GetOwner()->GetActorUpVector() * JumpOffWallRunVerticalVelocity;
	Launch(JumpOffVelocity);

}

void UGCBaseCharacterMovementComponent::StartWallRun(class UCapsuleComponent* CharacterCapsule, FHitResult& Hit)
{
	GetWorld()->GetTimerManager().SetTimer(WallRunTimer, this, &UGCBaseCharacterMovementComponent::StopWallRun, WallRunTime, false);
	
	FVector CharacterStartLocation = Hit.ImpactPoint + Hit.ImpactNormal.GetSafeNormal() * CharacterCapsule->GetScaledCapsuleRadius();
	CharacterStartLocation.Z = CharacterCapsule->GetComponentLocation().Z;
	GetOwner()->SetActorLocation(CharacterStartLocation);

	FRotator CharacterRotation = GetWallRunCharacterMovingDirection(Hit).ToOrientationRotator();
	GetOwner()->SetActorRotation(CharacterRotation);
	SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_WallRun);

}
void UGCBaseCharacterMovementComponent::StopWallRun()
{
	GetWorld()->GetTimerManager().ClearTimer(WallRunTimer);

	PreviousWallRunSide = CurrentWallRunSide;
	
	SetMovementMode(MOVE_Falling);

}

bool UGCBaseCharacterMovementComponent::IsWallRuning() const
{
	return UpdatedComponent && MovementMode == MOVE_Custom && CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_WallRun;
}

EWallRunSide UGCBaseCharacterMovementComponent::GetCurrentWallRunSide() const
{
	return CurrentWallRunSide;
}

FVector UGCBaseCharacterMovementComponent::GetWallRunCharacterMovingDirection(FHitResult& Hit)
{
	return CurrentWallRunSide == EWallRunSide::Left ? FVector::CrossProduct(Hit.ImpactNormal, FVector::UpVector) : FVector::CrossProduct(FVector::UpVector, Hit.ImpactNormal);
}

void UGCBaseCharacterMovementComponent::StartSlide()
{
	bIsSliding = true;

	GetWorld()->GetTimerManager().SetTimer(SlideTimer, this, &UGCBaseCharacterMovementComponent::StopSlide, SlideMaxTime, false);

	//Capsule
	const float ComponentScale = BaseCharacterOwner->GetCapsuleComponent()->GetShapeScale();
	const float OldUnscaledHalfHeight = BaseCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const float OldUnscaledRadius = BaseCharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	
	const float ScaledHalfHeight = SlideCapsuleHalfHeight * ComponentScale;
	float HalfHeightAdjust = OldUnscaledHalfHeight - ScaledHalfHeight;
	
	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, FString::Printf(TEXT("Start : HalfHeightAdjust( %.2f ) = OldUnscaledHalfHeight ( %.2f ) - ScaleHalfHeight ( %.2f ) "), HalfHeightAdjust, OldUnscaledHalfHeight, ScaledHalfHeight));

	BaseCharacterOwner->GetCapsuleComponent()->SetCapsuleSize(OldUnscaledRadius, ScaledHalfHeight);

	APlayerCharacter* CachedCharacter = StaticCast<APlayerCharacter*>(GetOwner());
	CachedCharacter->StartSlide(HalfHeightAdjust);
}

void UGCBaseCharacterMovementComponent::StopSlide()
{
	if (!IsSliding())
	{
		return;
	}

	bIsSliding = false;
	GetBaseCharacterOwner()->GetCharacterMoveComponent()->bIsSliding = false;
	GetWorld()->GetTimerManager().ClearTimer(SlideTimer);

	ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();

	const float ScaledHalfHeight = BaseCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const float DefaultScaledHalfHeight = DefaultCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const float DefaultUnscaledHalfHeight = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float DefaultUnscaledCapsuleRadius = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius();

	const float HalfHeightAdjust = DefaultScaledHalfHeight - ScaledHalfHeight;
	
	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, FString::Printf(TEXT("End : HalfHeightAdjust( %.2f ) = DefaultScaledHalfHeight ( %.2f ) - ScaleHalfHeight ( %.2f ) "), HalfHeightAdjust, DefaultScaledHalfHeight, ScaledHalfHeight));

	FVector CharacterLocation = UpdatedComponent->GetComponentLocation();
	CharacterLocation.Z += HalfHeightAdjust;

	BaseCharacterOwner->GetCapsuleComponent()->SetRelativeLocation(CharacterLocation);
	BaseCharacterOwner->GetCapsuleComponent()->SetCapsuleSize(DefaultUnscaledCapsuleRadius, DefaultUnscaledHalfHeight, true);

	APlayerCharacter* CachedCharacter = StaticCast<APlayerCharacter*>(GetOwner());
	CachedCharacter->EndSlide(HalfHeightAdjust);
}

bool UGCBaseCharacterMovementComponent::IsSliding() const
{
	return bIsSliding;
}

UAnimMontage* UGCBaseCharacterMovementComponent::GetSlideAnimMontage() const
{
	return SlideAnimMontage;
}

void UGCBaseCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	if (GetBaseCharacterOwner()->GetLocalRole() == ROLE_SimulatedProxy)
	{
		return;
	}

	switch (CustomMovementMode)
	{
		case (uint8) ECustomMovementMode::CMOVE_Mantling:
		{
			PhysMantling(deltaTime, Iterations);

			break;
		}
		case (uint8)ECustomMovementMode::CMOVE_Ladder:
		{
			PhysLadder(deltaTime, Iterations);

			break;
		}
		case(uint8) ECustomMovementMode::CMOVE_Zipline:
		{
			PhysZipline(deltaTime, Iterations);
			
			break;
		}
		case(uint8) ECustomMovementMode::CMOVE_WallRun:
		{
			PhysWallRun(deltaTime, Iterations);

			break;
		}

		case (uint8) ECustomMovementMode::CMOVE_RockClimbing:
		{
			PhysRockClimbing(deltaTime, Iterations);

				break;
		}
		
		default:
			break;
	}



	Super::PhysCustom(deltaTime, Iterations);
}

void UGCBaseCharacterMovementComponent::PhysMantling(float DeltaTime, int32 Iterations)
{
	float ElapsedTime = GetWorld()->GetTimerManager().GetTimerElapsed(MantlingTimer) + CurrentMantlingParameters.StartTime;

	FVector MantlingCurveValue = CurrentMantlingParameters.MantlingCurve->GetVectorValue(ElapsedTime);

	float PositionAlpha = MantlingCurveValue.X;
	float XYCorrectionAlpha = MantlingCurveValue.Y;
	float ZCorrectAlpha = MantlingCurveValue.Z;

	FVector  CurrectedInitialLocation = FMath::Lerp(CurrentMantlingParameters.InitialLocation, CurrentMantlingParameters.InitialAnimationLocation, XYCorrectionAlpha);
	CurrectedInitialLocation.Z = FMath::Lerp(CurrentMantlingParameters.InitialLocation.Z, CurrentMantlingParameters.InitialAnimationLocation.Z, ZCorrectAlpha);
	
	FVector NewLocation = FVector::ZeroVector;
	FRotator NewRotation = FRotator::ZeroRotator;

	UStaticMeshComponent* Mesh = CurrentMantlingParameters.PlatformMesh;

	if (IsValid(Mesh))
	{
		FVector GetRelativePlatform = CurrentMantlingParameters.PlatformMesh->GetRelativeLocation() - CurrentMantlingParameters.PlatforMeshLocation;
		FVector NewTargetLocation = CurrentMantlingParameters.TargetLocation + GetRelativePlatform;

		NewLocation = FMath::Lerp(CurrectedInitialLocation, NewTargetLocation, PositionAlpha);
		NewRotation = FMath::Lerp(CurrentMantlingParameters.InitialRotation, CurrentMantlingParameters.TargetRotation, PositionAlpha);

	}
	else
	{

		NewLocation = FMath::Lerp(CurrectedInitialLocation, CurrentMantlingParameters.TargetLocation, PositionAlpha);
		NewRotation = FMath::Lerp(CurrentMantlingParameters.InitialRotation, CurrentMantlingParameters.TargetRotation, PositionAlpha);

	}


	FVector Delta = NewLocation - GetActorLocation();
	Velocity = Delta / DeltaTime;

	FHitResult Hit;

	SafeMoveUpdatedComponent(Delta, NewRotation, false, Hit);
}

void UGCBaseCharacterMovementComponent::PhysLadder(float DeltaTime, int32 Iterations)
{
	
	CalcVelocity(DeltaTime, 1.0f, false, ClimbingOnLadderBrakingDecelartion);
	FVector Delta = Velocity * DeltaTime;

	if (HasAnimRootMotion())
	{
		FHitResult Hit;
		SafeMoveUpdatedComponent(Delta, GetOwner()->GetActorRotation(), false, Hit);
		return;
	}

	FVector NewPos = GetActorLocation() + Delta;
	float  NewPosProjection = GetActorToCurrentLadderProjection(NewPos);

	if (NewPosProjection < MinLadderBottomOffset)
	{
		DetachFromLadder(EDetachFromLadderMethod::ReachingTheBottom);
		
		return;
	}
	else if (NewPosProjection > (CurrentLadder->GetLadderHeight() - MaxLadderTopOffset))
	{
		DetachFromLadder(EDetachFromLadderMethod::ReachingTheTop);
		return;
	}

	FHitResult Hit;
	SafeMoveUpdatedComponent(Delta, GetOwner()->GetActorRotation(), true, Hit);
}

void UGCBaseCharacterMovementComponent::PhysZipline(float DeltaTime, int32 Iterations)
{
	CalcVelocity(DeltaTime, 1.0f, false, ClimbingOnZiplineBrakingDecelartion);
	FVector Delta = Velocity * DeltaTime;
	
	FVector NewPos = GetActorLocation() + Delta;
	
	float DistenceEndRail = UKismetMathLibrary::Vector_Distance(GetActorLocation(), CurrentZipline->EndRailMechComponent->GetComponentLocation());
	
	//DrawDebugLine(GetWorld(), GetActorLocation(), CurrentZipline->EndRailMechComponent->GetComponentLocation(), FColor::Green);

	if (DistenceEndRail <= 120.0f)
	{
		DetachFromZipline(EDetachFromZiplineMethod::JumpOff);

	}

	FHitResult Hit;
	SafeMoveUpdatedComponent(Delta, GetOwner()->GetActorRotation(), true, Hit);
}

void UGCBaseCharacterMovementComponent::PhysWallRun(float DeltaTime, int32 Iterations)
{
	FHitResult WallHit;
	if (!DetectWallRun(WallHit, GetOwner()->GetActorLocation()))
	{
		StopWallRun();
		return;
	}

	FVector MovingDirection = GetWallRunCharacterMovingDirection(WallHit);

	FRotator CharacterRotation = UKismetMathLibrary::RInterpTo(GetOwner()->GetActorRotation(), MovingDirection.ToOrientationRotator(), DeltaTime, WallRunRotationInterpSpeed);
	GetOwner()->SetActorRotation(CharacterRotation);
	
	Velocity = MovingDirection * WallRunSpeed;
	FVector Delta = DeltaTime * Velocity;
	SlideAlongSurface(Delta, 1, WallHit.ImpactNormal, WallHit, true);

	if (WallHit.bBlockingHit)
	{
		StopWallRun();
		return;

	}

}

void UGCBaseCharacterMovementComponent::PhysRockClimbing(float DeltaTime, int32 Iterations)
{
	FVector NewLocation = FMath::VInterpTo(GetActorLocation(), NewClimbingCharacterLocation, DeltaTime, ClimbingTime);

	
	//FRotator NewRotation = FMath::Lerp(CurrentRockClimbingParameters.InitialRotation, CurrentRockClimbingParameters.TargetRotation, 0);
	FRotator NewRotation = FMath::RInterpTo(CurrentRockClimbingParameters.InitialRotation, CurrentRockClimbingParameters.TargetRotation * -1, DeltaTime, 1);
	
	FVector Delta = NewLocation - GetActorLocation();
	Velocity = Delta / DeltaTime;
	
	FHitResult Hit; 
	SafeMoveUpdatedComponent(Delta, NewRotation, false, Hit);
	
}


void UGCBaseCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{

	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
	
	if (MovementMode == MOVE_Walking)
	{
		PreviousWallRunSide = EWallRunSide::Nome;
	}

	if (MovementMode == MOVE_Swimming)
	{
		CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(SwimmingCapsuleRadius, SwimmingCapsuleHalfHeight);
		
	}
	else if (PreviousMovementMode == MOVE_Swimming)
	{
		ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
		CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight(), true);
	}

	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == (uint8)ECustomMovementMode::CMOVE_Ladder)
	{
		CurrentLadder = nullptr;
	}
	
	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == (uint8)ECustomMovementMode::CMOVE_Zipline)
	{
		CurrentZipline = nullptr;
	}

	if (MovementMode == MOVE_Custom)
	{
		Velocity = FVector::ZeroVector;
		
		switch (CustomMovementMode)
		{
			case (uint8)ECustomMovementMode::CMOVE_Mantling:
			{
				
				GetWorld()->GetTimerManager().SetTimer(MantlingTimer, this, &UGCBaseCharacterMovementComponent::EndMantle, CurrentMantlingParameters.Duration, false);
				break;
			}

			default:
				break;
		}
	}


}

AGCBaseCharacter* UGCBaseCharacterMovementComponent::GetBaseCharacterOwner() const
{
	return StaticCast<AGCBaseCharacter*>(CharacterOwner);
}

void UGCBaseCharacterMovementComponent::SetIsOutOfStamina(bool bIsOutOfStamina_In)
{
	bIsOutOfStamina = bIsOutOfStamina_In;
	if (bIsOutOfStamina)
	{
		StopSprint();
		
	}
}

void FSavedMove_GC::Clear()
{
	Super::Clear();

	bSavedIsSprinting = 0;
	bSavedIsMantling = 0;
	bSavedIsPressingSlide = 0;

}

uint8 FSavedMove_GC::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	/*
		FLAG_Reserved_1 = 0x04,	// Reserved for future use
		FLAG_Reserved_2 = 0x08,	// Reserved for future use
		// Remaining bit masks are available for custom flags.
		FLAG_Custom_0 = 0x10, - Sprinting flag
		FLAG_Custom_1 = 0x20, - Mantling
		FLAG_Custom_2 = 0x40, - Slide
		FLAG_Custom_3 = 0x80,
	*/

	
	if (bSavedIsSprinting)
	{
		Result |= FLAG_Custom_0;
	}

	if (bSavedIsMantling)
	{
		Result &= ~FLAG_JumpPressed;
		Result |= FLAG_Custom_1;
	}

	if (bSavedIsPressingSlide)
	{
		Result &= ~FLAG_Custom_0;
		Result |= FLAG_Custom_2;
	}
	
	return Result;
}

bool FSavedMove_GC::CanCombineWith(const FSavedMovePtr& NewMovePtr, ACharacter* InCharacter, float MaxDelta) const
{
	const FSavedMove_GC* NewMove = StaticCast<const FSavedMove_GC*>(NewMovePtr.Get());

	if (bSavedIsSprinting != NewMove->bSavedIsSprinting 
		|| bSavedIsMantling != NewMove->bSavedIsMantling
		|| bSavedIsPressingSlide != NewMove->bSavedIsPressingSlide)
	{
		return false;
	}

	return Super::CanCombineWith(NewMovePtr, InCharacter, MaxDelta);
}

void FSavedMove_GC::SetMoveFor(ACharacter* InCharacter, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(InCharacter, InDeltaTime, NewAccel, ClientData);

	check(InCharacter->IsA<AGCBaseCharacter>());
	AGCBaseCharacter* InBaseCharacter = StaticCast<AGCBaseCharacter*>(InCharacter);
	UGCBaseCharacterMovementComponent* MovementComponent = InBaseCharacter->GetBaseCharacterMovementComponent();

	bSavedIsSprinting = MovementComponent->bIsSprinting;
	bSavedIsMantling = InBaseCharacter->GetCharacterMoveComponent()->bIsMantling;
	bSavedIsPressingSlide = InBaseCharacter->GetCharacterMoveComponent()->bIsSliding;
}

void FSavedMove_GC::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	UGCBaseCharacterMovementComponent* MovementComponent = StaticCast<UGCBaseCharacterMovementComponent*>(Character->GetMovementComponent());

	MovementComponent->bIsSprinting = bSavedIsSprinting;
	MovementComponent->bIsSliding = bSavedIsPressingSlide;

}

FNetworkPredictionData_Client_Character_GC::FNetworkPredictionData_Client_Character_GC(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
	
}

FSavedMovePtr FNetworkPredictionData_Client_Character_GC::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_GC());
}
