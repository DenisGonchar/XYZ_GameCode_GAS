// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterMoveComponent.h"

#include "Actors/Interactive/Environment/Ladder.h"
#include "Actors/Interactive/Environment/Zipline.h"
#include "Actors/Platform/Ledge/LedgePlatform.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Curves/CurveVector.h"
#include "Net/UnrealNetwork.h"
#include "Pawns/Character/GCBaseCharacter.h"
#include "Pawns/Character/CharacterMovementComponent/GCBaseCharacterMovementComponent.h"
#include "Components/LedgeDetectorComponent.h"

UCharacterMoveComponent::UCharacterMoveComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UCharacterMoveComponent::BeginPlay()
{
	Super::BeginPlay();

	CachedBaseCharacter = Cast<AGCBaseCharacter>(GetOwner());
	
	BaseTranslationOffset = CachedBaseCharacter->GetMesh()->GetRelativeLocation();

	CurrentBaseCharacterMovementComponent = CachedBaseCharacter->GetBaseCharacterMovementComponent();
	CurrentLedgeDetectorComponent = CachedBaseCharacter->GetLedgeDetectorComponent();
	
	CachedBaseCharacter->GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &UCharacterMoveComponent::OnPlayrCapsuleHit);

}

void UCharacterMoveComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                            FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentBaseCharacterMovementComponent->IsOnZipline())
	{
		
		ClimbZipline(1.0f);
	}
}

void UCharacterMoveComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCharacterMoveComponent, bIsMantling);
	DOREPLIFETIME_CONDITION(UCharacterMoveComponent, bIsSliding, COND_SimulatedOnly);
}

void UCharacterMoveComponent::OnPlayrCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                                UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	WallRun();
}

void UCharacterMoveComponent::Slide()
{
	// if (CanSlide() || (bIsSliding || CachedBaseCharacter->GetLocalRole() == ROLE_Authority))
	if (CanSlide())
	{
		CurrentBaseCharacterMovementComponent->StartSlide();
		
		UAnimInstance* AnimInstance = CachedBaseCharacter->GetMesh()->GetAnimInstance();
		CachedBaseCharacter->PlayAnimMontage(CurrentBaseCharacterMovementComponent->GetSlideAnimMontage());

		
		if (CachedBaseCharacter->IsLocallyControlled() || CachedBaseCharacter->GetLocalRole() == ROLE_Authority)
		{
			CurrentBaseCharacterMovementComponent->StartSlide();
		}
	}
}

void UCharacterMoveComponent::StartSlide(float HalfHeightAbjust)
{
	bIsSliding = true;
	
	if (CachedBaseCharacter->GetMesh())
	{
		FVector MeshRelativeLocation = CachedBaseCharacter->GetMesh()->GetRelativeLocation();
		MeshRelativeLocation.Z = CachedBaseCharacter->GetMesh()->GetRelativeLocation().Z + HalfHeightAbjust;
		BaseTranslationOffset.Z = MeshRelativeLocation.Z;
		CachedBaseCharacter->GetMesh()->SetRelativeLocation(MeshRelativeLocation);

	}
	else
	{
		BaseTranslationOffset.Z = CachedBaseCharacter->GetBaseTranslationOffset().Z + HalfHeightAbjust;
	}

}

void UCharacterMoveComponent::EndSlide(float HalfHeightAbjust)
{
	bIsSliding = false;
	
	ACharacter* DefaultCharacter = CachedBaseCharacter->GetClass()->GetDefaultObject<ACharacter>();

	if (CachedBaseCharacter->GetMesh())
	{
		FVector MeshRelativeLocation = CachedBaseCharacter->GetMesh()->GetRelativeLocation();
		MeshRelativeLocation.Z = DefaultCharacter->GetMesh()->GetRelativeLocation().Z;
		BaseTranslationOffset.Z = MeshRelativeLocation.Z - DefaultCharacter->GetMesh()->GetRelativeLocation().Z;
		CachedBaseCharacter->GetMesh()->SetRelativeLocation(MeshRelativeLocation);

	}
}

bool UCharacterMoveComponent::CanSlide()
{
	
	return CurrentBaseCharacterMovementComponent->IsSprinting() &&
		   !CurrentBaseCharacterMovementComponent->IsSliding();
	
}

void UCharacterMoveComponent::OnRep_InSlide(bool bWasSliding)
{
	if (CachedBaseCharacter->GetLocalRole() == ROLE_SimulatedProxy && !bWasSliding && bIsSliding)
	{
		Slide();
	}
}

void UCharacterMoveComponent::WallRun()
{
	if (!CanWallRun())
	{
		return;
	}

	if (CurrentBaseCharacterMovementComponent->IsWallRuning())
	{
		CurrentBaseCharacterMovementComponent->JumpOffWall();
			
		return;
	}

	if (!CurrentBaseCharacterMovementComponent->IsFalling())
	{
		return;
	}

	CurrentBaseCharacterMovementComponent->AttachToWallRun();
	
}

bool UCharacterMoveComponent::CanWallRun()
{
	return !CurrentBaseCharacterMovementComponent->IsMantling() && 
 			CurrentBaseCharacterMovementComponent->MovementMode != MOVE_Walking &&
 			CurrentBaseCharacterMovementComponent->MovementMode != MOVE_Swimming;
}

void UCharacterMoveComponent::ClimbZipline(float Value)
{
	if (CurrentBaseCharacterMovementComponent->IsOnZipline() && !FMath::IsNearlyZero(Value))
	{
		FVector ZiplineVector = CurrentBaseCharacterMovementComponent->GetCurrentZipline()->GetZiplineInteractionBox()->GetForwardVector();

		CachedBaseCharacter->AddMovementInput(ZiplineVector, Value);
	}
}

void UCharacterMoveComponent::InteractWithZipline()
{
	if (CurrentBaseCharacterMovementComponent->IsOnZipline())
	{
		CurrentBaseCharacterMovementComponent->DetachFromZipline(EDetachFromZiplineMethod::JumpOff);
	}
	else
	{

		const AZipline* AvailableZipline = GetAvailableZipline();
		if (IsValid(AvailableZipline))
		{
			CurrentBaseCharacterMovementComponent->AttachToZipline(AvailableZipline);
			
		}
	}
}

const class AZipline* UCharacterMoveComponent::GetAvailableZipline() const
{
	const AZipline* Result = nullptr;
	
	for (auto InteractiveActor : CachedBaseCharacter->GetAvaibleInteractiveActors())
	{
		if (InteractiveActor->IsA<AZipline>())
		{
			Result = Cast<AZipline>(InteractiveActor);
			break;
		}
	}
	return Result;
}

void UCharacterMoveComponent::ClimbLadderUp(float Value)
{
	
	if (CurrentBaseCharacterMovementComponent && CurrentBaseCharacterMovementComponent->IsOnLadder() && !FMath::IsNearlyZero(Value))
	{
		FVector LadderUpVector =  CurrentBaseCharacterMovementComponent->GetCurrentLadder()->GetActorUpVector();
		CachedBaseCharacter->AddMovementInput(LadderUpVector, Value);
	}
}

void UCharacterMoveComponent::InteractWithLadder()
{
	if (CurrentBaseCharacterMovementComponent->IsOnLadder())
	{
		CurrentBaseCharacterMovementComponent->DetachFromLadder(EDetachFromLadderMethod::JumpOff);

	}
	else
	{
		const ALadder* AvailableLadder = GetAvailableLadder();
		if (IsValid(AvailableLadder))
		{
			if (AvailableLadder->GetIsOnTop())
			{
				CachedBaseCharacter->PlayAnimMontage(AvailableLadder->GetAttachFromTopAnimMontage());
			}
			CurrentBaseCharacterMovementComponent->AttachToLadder(AvailableLadder);
		}

	}

}

const class ALadder* UCharacterMoveComponent::GetAvailableLadder() const
{
	const ALadder* Result = nullptr;
	
	for (auto InteractiveActor : CachedBaseCharacter->GetAvaibleInteractiveActors())
	{
		
		if (InteractiveActor->IsA<ALadder>())
		{
			Result = Cast<const ALadder>(InteractiveActor);
			break;
		}
	}

	return Result;
}


void UCharacterMoveComponent::Mantle(bool bForce /*= false*/)
{
	if (!(CanMantle() || bForce))
	{
		return;
	}

	FLedgeDescription LedgeDescription;

	if (CurrentLedgeDetectorComponent->DetectLedge(LedgeDescription) 
			&& !(CurrentBaseCharacterMovementComponent->MovementMode == MOVE_Falling) /* && CanJumpInternal_Implementation()*/)
	{	
		bIsMantling = true;

		FMantlingMovementParameters MantlingParameters;
		MantlingParameters.MantlingCurve = HighMantleSettings.MantlingCurve;
		MantlingParameters.InitialLocation = CachedBaseCharacter->GetActorLocation();
		MantlingParameters.InitialRotation = CachedBaseCharacter->GetActorRotation();
		MantlingParameters.TargetLocation = LedgeDescription.Location;
		MantlingParameters.TargetRotation = LedgeDescription.Rotation;
		MantlingParameters.PlatformMesh = LedgeDescription.PlatformStaticMesh;
		MantlingParameters.PlatforMeshLocation = LedgeDescription.PlatformMeshTargetLLocation;

		float BottomZOffset = 2.0f;
		FVector CharacterBottom = MantlingParameters.InitialLocation - (CachedBaseCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - BottomZOffset) * FVector::UpVector;
		
		//float MantlingHeight = (MantlingParameters.TargetLocation - MantlingParameters.InitialLocation).Z;
		float MantlingHeight = (LedgeDescription.LedgeHeight - CharacterBottom).Z;

		const FMantlingSetting& MantlingSettings = GetMantlingSetting(MantlingHeight);

		float MinRange;
		float MaxRange;
		MantlingSettings.MantlingCurve->GetTimeRange(MinRange, MaxRange);

		MantlingParameters.Duration = MaxRange - MinRange;

		MantlingParameters.MantlingCurve = MantlingSettings.MantlingCurve;

		FVector2D SourceRange(MantlingSettings.MinHeight, MantlingSettings.MaxHeight);
		FVector2D TargetRange(MantlingSettings.MinHeightStartTime, MantlingSettings.MaxHeightStartTime);
		MantlingParameters.StartTime = FMath::GetMappedRangeValueClamped(SourceRange, TargetRange, MantlingHeight);
		
		MantlingParameters.InitialAnimationLocation = MantlingParameters.TargetLocation - MantlingSettings.AnimationCorrectionZ * FVector::UpVector + MantlingSettings.AnimationCorrectionXY * LedgeDescription.LedgeNormal;

		if (CachedBaseCharacter->IsLocallyControlled() || CachedBaseCharacter->GetLocalRole() == ROLE_Authority)
		{
			CurrentBaseCharacterMovementComponent->StartMantle(MantlingParameters);
		}
		
		UAnimInstance* AnimInstance = CachedBaseCharacter->GetMesh()->GetAnimInstance();
		AnimInstance->Montage_Play(MantlingSettings.MantlingMontage, 1.0f, EMontagePlayReturnType::Duration, MantlingParameters.StartTime);

		CachedBaseCharacter->OnMantle(MantlingSettings, MantlingParameters.StartTime);
	
	}

}

void UCharacterMoveComponent::OnRep_IsMantlong(bool bWasMantling)
{
	if (CachedBaseCharacter->GetLocalRole() == ROLE_SimulatedProxy && !bWasMantling && bIsMantling)
	{
		Mantle(true);
	}
}

void UCharacterMoveComponent::RockClimbing(EMoveRockClimbing Move)
{
	if (!CurrentBaseCharacterMovementComponent->IsRockClimbing() || GetWorld()->GetTimerManager().IsTimerActive(ClimbingTimer))
	{
		return;
	}

	FLedgeDescription LedgeDescription;
	
	switch (Move)
	{
	case EMoveRockClimbing::Up:
		{
			if (CurrentLedgeActor->IsFinalPlatform())
			{
				Mantle();
				return;
			}

			if (CurrentLedgeDetectorComponent->DetectLedgeMoveUp(LedgeDescription, bIsDrawDebugUp))
			{
				FMantlingMovementParameters MantlingParameters;
				MantlingParameters.InitialLocation = CachedBaseCharacter->GetActorLocation();
				MantlingParameters.InitialRotation = CachedBaseCharacter->GetActorRotation();
				MantlingParameters.TargetLocation = LedgeDescription.Location;
				MantlingParameters.TargetRotation = LedgeDescription.Rotation;
				MantlingParameters.PlatformMesh = LedgeDescription.PlatformStaticMesh;
				MantlingParameters.PlatforMeshLocation = LedgeDescription.PlatformMeshTargetLLocation;
				MantlingParameters.PlatformActor = LedgeDescription.PlatformActor;

				if (UpClimbingMontage)
				{
					UAnimInstance* AnimInstance = CachedBaseCharacter->GetMesh()->GetAnimInstance();
					MantlingParameters.StartTime = AnimInstance->Montage_Play(UpClimbingMontage) + UpClimbingMontageTime;
					GetWorld()->GetTimerManager().ClearTimer(ClimbingTimer);
					GetWorld()->GetTimerManager().SetTimer(ClimbingTimer, MantlingParameters.StartTime, false);
				}
				else
				{
					MantlingParameters.StartTime = UpClimbingMontageTime;
					
					GetWorld()->GetTimerManager().ClearTimer(ClimbingTimer);
					GetWorld()->GetTimerManager().SetTimer(ClimbingTimer, MantlingParameters.StartTime, false);
				}

				CurrentLedgeActor = Cast<ALedgePlatform>(LedgeDescription.PlatformActor);
				CurrentBaseCharacterMovementComponent->AttachToRockClimbing(MantlingParameters);
				
			}
			
			break;
		}
	case EMoveRockClimbing::Down:
		{
			
			if (CurrentLedgeDetectorComponent->DetectLedgeMoveDown(LedgeDescription, bIsDrawDebugDonw))
			{
				FMantlingMovementParameters MantlingParameters;
				MantlingParameters.InitialLocation = CachedBaseCharacter->GetActorLocation();
				MantlingParameters.InitialRotation = CachedBaseCharacter->GetActorRotation();
				MantlingParameters.TargetLocation = LedgeDescription.Location;
				MantlingParameters.TargetRotation = LedgeDescription.Rotation;
				MantlingParameters.PlatformMesh = LedgeDescription.PlatformStaticMesh;
				MantlingParameters.PlatforMeshLocation = LedgeDescription.PlatformMeshTargetLLocation;
				MantlingParameters.PlatformActor = LedgeDescription.PlatformActor;

				if (DownClimbingMontage)
				{
					UAnimInstance* AnimInstance = CachedBaseCharacter->GetMesh()->GetAnimInstance();
					MantlingParameters.StartTime = AnimInstance->Montage_Play(DownClimbingMontage) + DownClimbingMontageTime;
					GetWorld()->GetTimerManager().ClearTimer(ClimbingTimer);
					GetWorld()->GetTimerManager().SetTimer(ClimbingTimer, MantlingParameters.StartTime, false);
				}
				else
				{
					MantlingParameters.StartTime = DownClimbingMontageTime;
					
					GetWorld()->GetTimerManager().ClearTimer(ClimbingTimer);
					GetWorld()->GetTimerManager().SetTimer(ClimbingTimer, MantlingParameters.StartTime, false);
				}

				CurrentLedgeActor = Cast<ALedgePlatform>(LedgeDescription.PlatformActor);
				CurrentBaseCharacterMovementComponent->AttachToRockClimbing(MantlingParameters);
				
			}
			
			break;
		}
	}

	
}

void UCharacterMoveComponent::StartRockClimbing()
{
	if (CurrentBaseCharacterMovementComponent->IsRockClimbing())
	{
		CurrentBaseCharacterMovementComponent->DetachFromRockClimbing(EDetachFromRockClimbingMethod::JumpOff);
	}
	else
	{
		
		FLedgeDescription LedgeDescription;

		if (CurrentLedgeDetectorComponent->DetectLedgePlatform(LedgeDescription))
		{
			bIsRockClimbing = true;
			
			FMantlingMovementParameters MantlingParameters;
			MantlingParameters.InitialLocation = CachedBaseCharacter->GetActorLocation();
			MantlingParameters.InitialRotation = CachedBaseCharacter->GetActorRotation();
			MantlingParameters.TargetLocation = LedgeDescription.Location;
			MantlingParameters.TargetRotation = LedgeDescription.Rotation;
			MantlingParameters.PlatformMesh = LedgeDescription.PlatformStaticMesh;
			MantlingParameters.PlatforMeshLocation = LedgeDescription.PlatformMeshTargetLLocation;
			MantlingParameters.PlatformActor = LedgeDescription.PlatformActor;
			
			if (StartClimbingMontage)
			{
				UAnimInstance* AnimInstance = CachedBaseCharacter->GetMesh()->GetAnimInstance();
				MantlingParameters.StartTime = AnimInstance->Montage_Play(StartClimbingMontage);
				AnimInstance->Montage_Play(StartClimbingMontage);
			
				GetWorld()->GetTimerManager().ClearTimer(ClimbingTimer);
				GetWorld()->GetTimerManager().SetTimer(ClimbingTimer, MantlingParameters.StartTime, false);
			}
			else
			{
				MantlingParameters.StartTime = UpClimbingMontageTime;
				GetWorld()->GetTimerManager().ClearTimer(ClimbingTimer);
				GetWorld()->GetTimerManager().SetTimer(ClimbingTimer, MantlingParameters.StartTime, false);
			}
			
			CurrentLedgeActor = Cast<ALedgePlatform>(LedgeDescription.PlatformActor);
			CurrentBaseCharacterMovementComponent->AttachToRockClimbing(MantlingParameters);
			
			
		}
	}
}

bool UCharacterMoveComponent::CanRockClimbing() const
{
	return !CurrentBaseCharacterMovementComponent->IsOnLadder() &&
		!CurrentBaseCharacterMovementComponent->IsOnZipline() &&
			!CurrentBaseCharacterMovementComponent->IsMantling();
}


const FMantlingSetting& UCharacterMoveComponent::GetMantlingSetting(float LedgeHeight) const
{
	
	return LedgeHeight > LowMantleMaxHeight ? HighMantleSettings : LowMantleSettings;
}

bool UCharacterMoveComponent::CanMantle() const
{
	return !CurrentBaseCharacterMovementComponent->IsOnLadder() && !CurrentBaseCharacterMovementComponent->IsOnZipline()
			/*TODO !IsOnRockClimbing*/;
}