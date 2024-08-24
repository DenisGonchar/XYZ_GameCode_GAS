// Fill out your copyright notice in the Description page of Project Settings.


#include "GCBaseCharacter.h"
#include "AIController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/PhysicsVolume.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CharacterMovementComponent/GCBaseCharacterMovementComponent.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "Components/CharacterComponents/CharacterAttributeComponent.h"
#include "Components/CharacterComponents/CharacterInventoryComponent.h"
#include "AbilitySystemComponent/GCAbilitySystemComponent.h"
#include "Components/LedgeDetectorComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Actors/Interactive/InteractiveActor.h"
#include "Actors/Interactive/Interface/Interactive.h"
#include "Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "Actors/Equipment/Weapons/MeleeWeaponItem.h"
#include "UI/Widget/World/GCAttributeProgressBar.h"
#include "Inventory/Items/InventoryItem.h"
#include "GameCodeTypes.h"
#include "SignificanceManager.h"
#include "AbilitySystemComponent/AttributeSets/GCCharacterAttributeSet.h"
#include "Components/CharacterComponents/CharacterHitReactionComponent.h"
#include "Components/CharacterComponents/CharacterMoveComponent.h"
#include "Inventory/Items/Powerups/Drone.h"
#include "Pawns/Drone/GCBaseDrone.h"


AGCBaseCharacter::AGCBaseCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UGCBaseCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	GCBaseCharacterMovementComponent = StaticCast<UGCBaseCharacterMovementComponent*>(GetCharacterMovement());

	//
	IKScale = GetActorScale3D().Z;
	IKTraceDistance = GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * IKScale;
	
	BaseTranslationOffset = GetMesh()->GetRelativeLocation();

	LedgeDetertorComponent = CreateDefaultSubobject<ULedgeDetectorComponent>(TEXT("LedgeDetertor"));

	GetMesh()->CastShadow = true;
	GetMesh()->bCastDynamicShadow = true;

	AvaibleInteractiveActors.Reserve(10);
	CharacterAttributeComponent = CreateDefaultSubobject<UCharacterAttributeComponent>(TEXT("CharacterAttribute"));
	CharacterEquipmentComponent = CreateDefaultSubobject<UCharacterEquipmentComponent>(TEXT("CharacterEquipment"));
	CharacterInventoryComponent = CreateDefaultSubobject<UCharacterInventoryComponent>(TEXT("InventoryComponent"));
	CharacterHitReactionComponent = CreateDefaultSubobject<UCharacterHitReactionComponent>(TEXT("HitReactionComponent"));

	CharacterBaseMoveComponent = CreateDefaultSubobject<UCharacterMoveComponent>(TEXT("CharacterBaseMoveComponent"));
	
	HealthBarProgressComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarProgressComponent"));
	HealthBarProgressComponent->SetupAttachment(GetCapsuleComponent());

	AbilitySystemComponent = CreateDefaultSubobject<UGCAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AttributeSet = CreateDefaultSubobject<UGCCharacterAttributeSet>(TEXT("AttributeSet"));
}

void AGCBaseCharacter::BeginPlay()
{
	Super::BeginPlay();


	CharacterAttributeComponent->OnDeathEvent.AddUObject(this, &AGCBaseCharacter::OnDeath);
	CharacterAttributeComponent->OutOfStaminaEventSignature.AddUObject(this, &AGCBaseCharacter::Stamina);

	InitializeHealthProgress();

	if (bIsSignificanceEnabled)
	{
		USignificanceManager* SignificanceManager = FSignificanceManagerModule::Get(GetWorld());
		if (IsValid(SignificanceManager))
		{
			SignificanceManager->RegisterObject(
					this,
					SignificanceTagCharacter,
					[this](USignificanceManager::FManagedObjectInfo* ObjectInfo, const FTransform& ViewPoint) -> float
					{
						return SingnificanceFunction(ObjectInfo, ViewPoint);
					},
					USignificanceManager::EPostSignificanceType::Sequential,
					[this](USignificanceManager::FManagedObjectInfo* ObjectInfo, float OldSignificance, float Significance, bool bFinal)
					{
						PostSignificanceFunction(ObjectInfo, OldSignificance, Significance, bFinal);
					}
			);
		}
	}
}

void AGCBaseCharacter::EndPlay(const EEndPlayReason::Type Reason)
{
	if (OnInteractableObjectFound.IsBound())
	{
		OnInteractableObjectFound.Unbind();

	}

	Super::EndPlay(Reason);
}

void AGCBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TryChangeSprintState(DeltaTime);
	UpdateIkSetting(DeltaTime);

	TraceLineOfSight();
		
}
float AGCBaseCharacter::InternalTakePointDamage(float Damage, FPointDamageEvent const& PointDamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (CharacterAttributeComponent->IsAlive())
	{
		CharacterAttributeComponent->TakeDamage(Damage, EventInstigator, DamageCauser, PointDamageEvent.HitInfo.BoneName);

	}	
	return 0.0f;
}


UAbilitySystemComponent* AGCBaseCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AGCBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

}

void AGCBaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AAIController* AIController = Cast<AAIController>(NewController);
	if (IsValid(AIController))
	{
		FGenericTeamId TeamId((uint8)Team);
		AIController->SetGenericTeamId(TeamId);
	}
	
	InitGameplayAbilitySystem(NewController);
	
}



void AGCBaseCharacter::ChangeCrouchState()
{
	if (!AbilitySystemComponent->IsAbilityActive(CrouchAbilityTag) && !GCBaseCharacterMovementComponent->IsProning() && (GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling()))
	{
		AbilitySystemComponent->TryActivateAbilityWithTag(CrouchAbilityTag);
	}
	else
	{
		AbilitySystemComponent->TryCancelAbilityWithTag(CrouchAbilityTag);
	}

}

void AGCBaseCharacter::ChangeProneState()
{
	if (GetCharacterMovement()->IsCrouching() && !GCBaseCharacterMovementComponent->IsProning())
	{
		Prone();
	}
	else if(!GetCharacterMovement()->IsCrouching() && GCBaseCharacterMovementComponent->IsProning())
	{
		UnProne(false);
	}
}

void AGCBaseCharacter::Prone(bool bClientSimulation /*= false*/)
{
	if (GCBaseCharacterMovementComponent)
	{
		if (CanProne())
		{
			GCBaseCharacterMovementComponent->bWantsToProne = true; //---

		}
	}

}

void AGCBaseCharacter::UnProne(bool bIsFullHeight, bool bClientSimulation)
{
	if (GCBaseCharacterMovementComponent)
	{
		GCBaseCharacterMovementComponent->bWantsToProne = false;
		GCBaseCharacterMovementComponent->bIsFullHeight = bIsFullHeight;
	}
}

bool AGCBaseCharacter::CanProne() const
{
	return !bIsProned && GCBaseCharacterMovementComponent && GetRootComponent() && !GetRootComponent()->IsSimulatingPhysics();
}

void AGCBaseCharacter::OnStartProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	RecalculateBaseEyeHeight();

	const ACharacter* DefaultChar = GetDefault<ACharacter>(GetClass());
	if (GetMesh() && DefaultChar->GetMesh())
	{
		FVector& MeshRelativeLocation = GetMesh()->GetRelativeLocation_DirectMutable();
		MeshRelativeLocation.Z = GetMesh()->GetRelativeLocation().Z + HalfHeightAdjust + GCBaseCharacterMovementComponent->CrouchedHalfHeight - 34.0f;
		BaseTranslationOffset.Z = MeshRelativeLocation.Z;
	}
	else
	{
		BaseTranslationOffset.Z = DefaultChar->GetBaseTranslationOffset().Z + HalfHeightAdjust + GCBaseCharacterMovementComponent->CrouchedHalfHeight;
	}

	K2_OnStartProne(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

void AGCBaseCharacter::OnEndProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	RecalculateBaseEyeHeight();

	const ACharacter* DefaultChar = GetDefault<ACharacter>(GetClass());
	const float HeightDifference = GCBaseCharacterMovementComponent->bIsFullHeight ? 0.0f : GCBaseCharacterMovementComponent->CrouchedHalfHeight - GCBaseCharacterMovementComponent->PronedHalfHeight;

	if (GetMesh() && DefaultChar->GetMesh())
	{
		FVector& MeshRelativeLocation = GetMesh()->GetRelativeLocation_DirectMutable();
		MeshRelativeLocation.Z = GetMesh()->GetRelativeLocation().Z - 26.0f;
		BaseTranslationOffset.Z = MeshRelativeLocation.Z;
	}
	else
	{
		BaseTranslationOffset.Z = GetBaseTranslationOffset().Z + HeightDifference;
	}

	K2_OnEndProne(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

void AGCBaseCharacter::Jump()
{
	if (bIsCrouched)
	{
		ChangeCrouchState();
	}
	else if (GCBaseCharacterMovementComponent->IsProning())
	{
		UnProne(true);
	}
	else
	{
		if (CanJumpInternal_Implementation())
		{
			Super::Jump();
			
		}
		
	}
}

bool AGCBaseCharacter::CanJumpInternal_Implementation() const
{
	return   !GCBaseCharacterMovementComponent->IsOutOfStamina()
									   && !bIsProned
									   && !GetBaseCharacterMovementComponent()->IsMantling()
									   && Super::CanJumpInternal_Implementation();


}
void AGCBaseCharacter::OnJumped_Implementation()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	if (GCBaseCharacterMovementComponent->IsProning())
	{
		UnProne(true);
	}
	
}

void AGCBaseCharacter::Falling()
{
	Super::Falling();
	GetBaseCharacterMovementComponent()->bNotifyApex = true;

}

void AGCBaseCharacter::NotifyJumpApex()
{
	Super::NotifyJumpApex();
	CurrentFallApex = GetActorLocation();

}

void AGCBaseCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	float FallHeight = (CurrentFallApex - GetActorLocation()).Z * 0.01f;
	if (IsValid(FallDamageCurve))
	{
		float DamageAmount = FallDamageCurve->GetFloatValue(FallHeight);
		TakeDamage(DamageAmount, FDamageEvent(), GetController(), Hit.Actor.Get());
	}

	bool bIsWater = false;
	APhysicsVolume* Valume = Cast<APhysicsVolume>(Hit.Actor);
	if (Valume != nullptr)
	{
		bIsWater = Valume->bWaterVolume;
	}
	
	if (FallHeight >= HardLandingHeight && !bIsWater)
	{
		HardLanding();
	}

}

void AGCBaseCharacter::HardLanding()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!IsValid(AnimInstance))
	{
		return;
	}

	if (IsValid(HardLandingAnimMontage))
	{
		float Duration = AnimInstance->Montage_Play(HardLandingAnimMontage, 1.0f, EMontagePlayReturnType::Duration);
		LimitControl();
		GetWorld()->GetTimerManager().SetTimer(HardLandingTimer, this, &AGCBaseCharacter::UnLimitControl, Duration, false);

	}
}

void AGCBaseCharacter::LimitControl()
{
	GetController()->SetIgnoreMoveInput(true);
	
}

void AGCBaseCharacter::UnLimitControl()
{
	GetController()->SetIgnoreMoveInput(false);
	
}

void AGCBaseCharacter::StartSprint()
{
	if (IsStartFireMeleeWeapon())
	{
		return;
	}
	
	float GetSpeed = GetBaseCharacterMovementComponent()->Velocity.Size();

	if(CanJumpInternal_Implementation() && GetSpeed > 0.0f)
	{
		bIsSprintRequested = true;
		
	}

	if (bIsProned)
	{
		UnProne(false);
		 
	}
}

void AGCBaseCharacter::StopSprint()
{
	bIsSprintRequested = false;

}

void AGCBaseCharacter::Mantle(bool bForce /*= false*/)
{
	CharacterBaseMoveComponent->Mantle(bForce);

}

void AGCBaseCharacter::LadderUp(float Value)
{
	CharacterBaseMoveComponent->ClimbLadderUp(Value);
}

void AGCBaseCharacter::Ladder()
{
	CharacterBaseMoveComponent->InteractWithLadder();
}


void AGCBaseCharacter::Zipline()
{
	CharacterBaseMoveComponent->InteractWithZipline();
}

void AGCBaseCharacter::WallRun()
{
	CharacterBaseMoveComponent->WallRun();
}

void AGCBaseCharacter::Slide()
{
	CharacterBaseMoveComponent->Slide();
}

void AGCBaseCharacter::Stamina(bool bStamina)
{
	GetBaseCharacterMovementComponent()->SetIsOutOfStamina(bStamina);
}

bool AGCBaseCharacter::IsSwimmingUnderWater() const
{
	if (GetBaseCharacterMovementComponent()->IsSwimming())
	{	
		FVector HeadPosition = GetMesh()->GetSocketLocation(SocketHead);

		APhysicsVolume* Volume = GetBaseCharacterMovementComponent()->GetPhysicsVolume();
		float VolumeTopPlane = Volume->GetActorLocation().Z + Volume->GetBounds().BoxExtent.Z * Volume->GetActorScale3D().Z;
		if (VolumeTopPlane < HeadPosition.Z)
		{
			return true;
		}

	}
	return false;
}

void AGCBaseCharacter::StartFire()
{
	if (!CharacterAttributeComponent->IsAlive() && CharacterEquipmentComponent->IsSelectingWeapon() && CharacterEquipmentComponent->IsEquipping())
	{
		return;
	}

	ARangeWeaponItem* CurrentRangeWeapon = CharacterEquipmentComponent->GetCurrentRangeWeapon();
	if (IsValid(CurrentRangeWeapon))
	{
		
		CurrentRangeWeapon->StartFire();

	}
}


void AGCBaseCharacter::StopFire()
{
	ARangeWeaponItem* CurrentRangeWeapon = CharacterEquipmentComponent->GetCurrentRangeWeapon();
	if (IsValid(CurrentRangeWeapon))
	{
		CurrentRangeWeapon->StopFire();

	}
}

void AGCBaseCharacter::PrimaryMeleeAttack()
{
	AMeleeWeaponItem* CurrentMeleeWeapon = CharacterEquipmentComponent->GetCurrentMeleeWeapon();
	if (IsValid(CurrentMeleeWeapon))
	{
		CurrentMeleeWeapon->StartAttack(EMeleeAttackType::PrimeryAttack);
	}

}

void AGCBaseCharacter::SecondaryMeleeAttack()
{
	if (AMeleeWeaponItem* CurrentMeleeWeapon = CharacterEquipmentComponent->GetCurrentMeleeWeapon())
	{
		CurrentMeleeWeapon->StartAttack(EMeleeAttackType::SecondaryAttack);
	}
}

bool AGCBaseCharacter::IsStartFireMeleeWeapon()
{
	if (IsValid(GetCharacterEquipmentComponent_Muteble()->GetCurrentMeleeWeapon()))
	{
		AMeleeWeaponItem* MeleeWeapon = GetCharacterEquipmentComponent_Muteble()->GetCurrentMeleeWeapon();
		if (MeleeWeapon->GetStartAttack())
		{
			return true;
		}
	}

	return  false;
}

FRotator AGCBaseCharacter::GetAimOffset()
{
	FVector AimDirectionWorld = GetBaseAimRotation().Vector();
	FVector AimDirectionLocal = GetTransform().InverseTransformVectorNoScale(AimDirectionWorld);
	FRotator Result = AimDirectionLocal.ToOrientationRotator();
	
	return Result;
}

void AGCBaseCharacter::StartAiming()
{
	ARangeWeaponItem* CurrentRangeWeapon = GetCharacterEquipmentComponent()->GetCurrentRangeWeapon();
	if (!IsValid(CurrentRangeWeapon))
	{
		return;
	}

	bIsAiming = true;
	CurrentAimingMovementSpeed = CurrentRangeWeapon->GetAimMovementMaxSpeed();
	CurrentRangeWeapon->StartAim();
	OnStartAiming();
}

void AGCBaseCharacter::StopAiming()
{
	if (!bIsAiming)
	{
		return;
	}

	ARangeWeaponItem* CurrentRangeWeapon = GetCharacterEquipmentComponent()->GetCurrentRangeWeapon();
	if (IsValid(CurrentRangeWeapon))
	{
		CurrentRangeWeapon->StopAim();

	}

	bIsAiming = false;
	CurrentAimingMovementSpeed = 0.0f;
	OnStopAiming();
}

bool AGCBaseCharacter::IsAiming() const
{
	return bIsAiming;
}

float AGCBaseCharacter::GetAimingMovementSpeed() const
{
	return CurrentAimingMovementSpeed;
}

void AGCBaseCharacter::OnStartAiming_Implementation()
{
	OnStartAimingInternal();

}

void AGCBaseCharacter::OnStopAiming_Implementation()
{
	OnStopAimingInternal();
}

void AGCBaseCharacter::Reload()
{
	if (IsValid(CharacterEquipmentComponent->GetCurrentRangeWeapon()))
	{
		CharacterEquipmentComponent->ReloadCurrentWeapon();
	}

}

void AGCBaseCharacter::NextItem()
{
	CharacterEquipmentComponent->EquipNextItem();
}

void AGCBaseCharacter::PreviousItem()
{
	CharacterEquipmentComponent->EquipPreviousItem();

}

void AGCBaseCharacter::EquipPrimaryItem()
{
	CharacterEquipmentComponent->EquipItemInSlot(EEquipmentSlots::PrimaryItemSlot);
}

void AGCBaseCharacter::AdditionalShootingMode()
{
	ARangeWeaponItem* CurrentRangeWeapon = CharacterEquipmentComponent->GetCurrentRangeWeapon();
	if (IsValid(CurrentRangeWeapon))
	{
		CharacterEquipmentComponent->SwitchAmmoSlot();
		
	}
}

void AGCBaseCharacter::RegisterInteractiveActor(AInteractiveActor* InteractiveActor)
{
	AvaibleInteractiveActors.AddUnique(InteractiveActor);

}

void AGCBaseCharacter::UnregisterInteractiveActor(AInteractiveActor* InteractiveActor)
{
	AvaibleInteractiveActors.RemoveSingleSwap(InteractiveActor);

}

TArray<TWeakObjectPtr<AInteractiveActor>, TInlineAllocator<10>> AGCBaseCharacter::GetAvaibleInteractiveActors()
{
	return AvaibleInteractiveActors;
}

void AGCBaseCharacter::Interact()
{
	if (LineOfSightObject.GetInterface())
	{
		LineOfSightObject->Interact(this);
		LineOfSightObject = nullptr;
		OnInteractableObjectFound.ExecuteIfBound(NAME_None);
	}
}

bool AGCBaseCharacter::PickupItem(TWeakObjectPtr<UInventoryItem> ItemToPickup, int32 Count)
{
	bool Result = false;
	if (CharacterInventoryComponent->HasFreeSlot())
	{
		CharacterInventoryComponent->AddItem(ItemToPickup, Count);
		Result = true;
	}
	return Result;

}

void AGCBaseCharacter::UseInventory(APlayerController* PlayerController)
{
	if (!IsValid(PlayerController))
	{
		return;
	}

	if (!CharacterInventoryComponent->IsViewVisible())
	{
		CharacterInventoryComponent->OpenViewInventory(PlayerController);
		CharacterEquipmentComponent->OpenViewEquipment(PlayerController);
		PlayerController->SetInputMode(FInputModeGameAndUI{});
		PlayerController->bShowMouseCursor = true;
	}
	else
	{
		CharacterInventoryComponent->CloseViewInventory();
		CharacterEquipmentComponent->CloseViewEquipment();
		PlayerController->SetInputMode(FInputModeGameOnly{});
		PlayerController->bShowMouseCursor = false;
	}

}

void AGCBaseCharacter::Drone()
{
	//TODO Spawn Drone, Use key F

	if (CurrentSpawnDrone && ActiveDrones.Num() >= CharacterEquipmentComponent->GetMaxNumberDrones())
	{
		return;
	}
	
	FInventorySlot* Slot = CharacterInventoryComponent->FindItemSlot(FName("Drone"));
	if (Slot != nullptr)
	{
		UDrone* Item = Cast<UDrone>(Slot->Item);
		if (!Item && !Item->GetDrone())
		{
			return;
		}

		FTransform ThisTransform = GetTransform();
		ThisTransform.SetLocation(FVector(0.0f, 0.0f, -100.0f));
		
		AGCBaseDrone* Drone = GetWorld()->SpawnActorDeferred<AGCBaseDrone>(Item->GetDrone(), ThisTransform, GetOwner(), nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		CurrentSpawnDrone = Drone;

		Drone->SetOwnerCharacter(this);
		ActiveDrones.Add(Drone);
		
		UAnimMontage* EquipMontage = Item->GetCharacterEquipAnimMontage();
		if (IsValid(EquipMontage))
		{
			float EquipDuretion = PlayAnimMontage(EquipMontage);
		}

		Slot->ClearSlot();
	}
	
}

void AGCBaseCharacter::ActiveDrone()
{
	PossessDrone(ActiveDrones.Num() - 1);
}

void AGCBaseCharacter::SpawnDrone()
{
	if(CurrentSpawnDrone == nullptr)
	{
		return;
	}
	
	FTransform Transform = GetActorTransform();
	Transform.SetLocation(GetMesh()->GetSocketLocation(CurrentSpawnDrone->EquippedSocketName) + FVector(0.0f, 0.0f, 100.0f));
	
	CurrentSpawnDrone->FinishSpawning(Transform);
	CurrentSpawnDrone = nullptr;
	
	PossessDrone(ActiveDrones.Num() - 1);
}

void AGCBaseCharacter::PossessDrone(int8 Index)
{
	if (ActiveDrones.Num() == 0)
	{
		return;
	}

	CurrentIndexActiveDrone = Index;
	AGCBaseDrone* Drone = ActiveDrones[Index];

	GetController()->Possess(Drone);
	
}

int8 AGCBaseCharacter::GetCurrentIndexActiveDrone() const
{
	return CurrentIndexActiveDrone;
}

void AGCBaseCharacter::SetCurrentIndexActiveDrone(int8 NewIndex)
{
	CurrentIndexActiveDrone = NewIndex;
}

void AGCBaseCharacter::RemoveActiveDrones(AGCBaseDrone* Drone)
{
	if (!Drone)
	{
		return;
	}

	ActiveDrones.Remove(Drone);
}

void AGCBaseCharacter::InitializeHealthProgress()
{
	UGCAttributeProgressBar* Widget = Cast<UGCAttributeProgressBar>(HealthBarProgressComponent->GetUserWidgetObject());
	if (!IsValid(Widget))
	{
		HealthBarProgressComponent->SetVisibility(false);
		return;
	}

	if (IsPlayerControlled() && IsLocallyControlled())
	{
		HealthBarProgressComponent->SetVisibility(false);
	}

	//Widget->SetAttibuteSet(AttributeSet);

	
}

void AGCBaseCharacter::ConfirmWeaponSelection()
{
	if (CharacterEquipmentComponent->IsSelectingWeapon())
	{
		CharacterEquipmentComponent->ConfirmWeaponSelection();
	}
}

// void AGCBaseCharacter::OnLevelDeserialized_Implementation()
// {
//
// }

FGenericTeamId AGCBaseCharacter::GetGenericTeamId() const
{
	return FGenericTeamId((uint8)Team);
}

void AGCBaseCharacter::OnDeath()
{
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionProfileName(FName("NoCollision"));
	GetMesh()->SetCollisionProfileName(FName("NoCollision"));
	
	float Duration = PlayAnimMontage(OnDeathAnimMontage);
	if (FMath::IsNearlyZero(Duration))
	{
		EnableRagdoll();
	}

}

void AGCBaseCharacter::OnSprintStart_Implementation()
{

}

void AGCBaseCharacter::OnSprintEnd_Implementation()
{
}

bool AGCBaseCharacter::CanSprint()
{
	
	 return !GetBaseCharacterMovementComponent()->IsOutOfStamina() && FMath::IsNearlyEqual(GetVelocity().Rotation().Yaw, GetActorRotation().Yaw, 30.0f) && !IsStartFireMeleeWeapon();
	
}

void AGCBaseCharacter::OnStartAimingInternal()
{
	if (OnAimingStateChanged.IsBound())
	{
		OnAimingStateChanged.Broadcast(true);
	}
}

void AGCBaseCharacter::OnStopAimingInternal()
{
	if (OnAimingStateChanged.IsBound())
	{
		OnAimingStateChanged.Broadcast(false);
	}
}

void AGCBaseCharacter::TraceLineOfSight()
{
	if (!IsPlayerControlled())
	{
		return;
	}

	FVector ViewLocation;
	FRotator ViewRotation;

	if(APlayerController* PlayerController = GetController<APlayerController>())
	{
		PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
	}

	FVector ViewDirection = ViewRotation.Vector();

	FVector TraceEnd = ViewLocation + ViewDirection * LineOfSightDistance;

	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(HitResult, ViewLocation, TraceEnd, ECC_Visibility);
	
	if (LineOfSightObject.GetObject() != HitResult.Actor)
	{
		LineOfSightObject = HitResult.Actor.Get();
			
		FName ActionName;
		if (LineOfSightObject.GetInterface())
		{
			ActionName = LineOfSightObject->GetActionEventName();
			
		}
		else
		{
			ActionName = NAME_None;
		}

		OnInteractableObjectFound.ExecuteIfBound(ActionName);

	}
}

bool AGCBaseCharacter::CanMove()
{
	return !CharacterHitReactionComponent->IsBlockingReactionActive();
}

void AGCBaseCharacter::InitGameplayAbilitySystem(AController* NewController)
{
	AbilitySystemComponent->InitAbilityActorInfo(NewController, this);

	if (!bAreAbilityAdded)
	{
		for (TSubclassOf<UGameplayAbility>& AbilityClass : Abilities)
		{
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass));
		}
		
		bAreAbilityAdded = true;
	}

	AbilitySystemComponent->TryActivateAbilitiesByTag(InitialActiveAbilities);
	
}

float AGCBaseCharacter::SingnificanceFunction(USignificanceManager::FManagedObjectInfo* ObjectInfo, const FTransform& ViewPoint)
{
	if (ObjectInfo->GetTag() == SignificanceTagCharacter)
	{
		AGCBaseCharacter* Character = StaticCast<AGCBaseCharacter*>(ObjectInfo->GetObject());
		if (!IsValid(Character))
		{
			return SignificanceValueVeryHith;

		}

		if (Character->IsPlayerControlled() && Character->IsLocallyControlled())
		{
			return SignificanceValueVeryHith;

		}
		
		float DistSquared = FVector::DistSquared(Character->GetActorLocation(), ViewPoint.GetLocation());
		
		if (DistSquared <= FMath::Square(VeryHighSignificanceDistance))
		{
			return SignificanceValueVeryHith;
		}
		else if(DistSquared <= FMath::Square(HighSignificanceDistance))
		{
			return SignificanceValueHith;
		}
		else if(DistSquared <= FMath::Square(MediumSignificanceDistance))
		{
			return SignificanceValueMedium;
		}
		else if (DistSquared <= FMath::Square(LowSignificanceDistance))
		{
			return SignificanceValueLow;
		}
		else
		{
			return SignificanceValueVeryLow;
		}
	}

	return SignificanceValueVeryHith;
}

void AGCBaseCharacter::PostSignificanceFunction(USignificanceManager::FManagedObjectInfo* ObjectInfo, float OldSignificance, float Significance, bool bFinal)
{
	if (OldSignificance == Significance)
	{
		return;
	}

	if (ObjectInfo->GetTag() != SignificanceTagCharacter)
	{
		return;
	}

	AGCBaseCharacter* Character = StaticCast<AGCBaseCharacter*>(ObjectInfo->GetObject());
	if (!IsValid(Character))
	{
		return;
	}
	
	UCharacterMovementComponent* MovementComponent = Character->GetBaseCharacterMovementComponent();
	AAIController* AIController = Character->GetController<AAIController>();
	UWidgetComponent* Widget =  Character->HealthBarProgressComponent;

	if (Significance == SignificanceValueVeryHith)
	{
		MovementComponent->SetComponentTickInterval(0.0f);
		Widget->SetVisibility(true);
		Character->GetMesh()->SetComponentTickEnabled(true);
		Character->GetMesh()->SetComponentTickInterval(0.0f);

		if (IsValid(AIController))
		{
			AIController->SetActorTickInterval(0.0f);

		}

	}
	else if (Significance == SignificanceValueHith)
	{
		MovementComponent->SetComponentTickInterval(0.0f);
		Widget->SetVisibility(true);
		Character->GetMesh()->SetComponentTickEnabled(true);
		Character->GetMesh()->SetComponentTickInterval(0.05f);

		if (IsValid(AIController))
		{
			AIController->SetActorTickInterval(0.0f);

		}

	}
	else if (Significance == SignificanceValueMedium)
	{
		MovementComponent->SetComponentTickInterval(0.1f);
		Widget->SetVisibility(false);
		Character->GetMesh()->SetComponentTickEnabled(true);
		Character->GetMesh()->SetComponentTickInterval(0.1f);

		if (IsValid(AIController))
		{
			AIController->SetActorTickInterval(0.1f);

		}
	}
	else if (Significance == SignificanceValueLow)
	{
		MovementComponent->SetComponentTickInterval(1.0f);
		Widget->SetVisibility(false);
		Character->GetMesh()->SetComponentTickEnabled(true);
		Character->GetMesh()->SetComponentTickInterval(1.0f);

		if (IsValid(AIController))
		{
			AIController->SetActorTickInterval(1.0f);
			AIController->SetActorTickInterval(1.0f);

		}
	}
	else if (Significance == SignificanceValueVeryLow)
	{
		MovementComponent->SetComponentTickInterval(5.0f);
		Widget->SetVisibility(false);
		Character->GetMesh()->SetComponentTickEnabled(false);

		if (IsValid(AIController))
		{
			AIController->SetActorTickInterval(10.0f);

		}
	}

}

void AGCBaseCharacter::UpdateIkSetting(float DeltaSeconds)
{

	IKLeftFootSocketOffset = FMath::FInterpTo(IKLeftFootSocketOffset, GetIKOffsetForSocket(LeftFootSocketName), DeltaSeconds, IKInterpSpeed);
	IKRightFootSocketOffset = FMath::FInterpTo(IKRightFootSocketOffset, GetIKOffsetForSocket(RightFootSocketName), DeltaSeconds, IKInterpSpeed);
	IKPelvisOffset = FMath::FInterpTo(IKPelvisOffset, CalculateIKPelvisOffset(), DeltaSeconds, IKInterpSpeed);

}


float AGCBaseCharacter::GetIKOffsetForSocket(const FName& SocketName) const
{
	float Result = 0.0f;

	float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	FVector SocketLocation = GetMesh()->GetSocketLocation(SocketName);
	FVector TraceStart(SocketLocation.X, SocketLocation.Y, GetActorLocation().Z);
	FVector TraceEnd = TraceStart - (CapsuleHalfHeight + IKTraceDistance) * FVector::UpVector;

	FHitResult HitResult;

	ETraceTypeQuery TraceType = UEngineTypes::ConvertToTraceType(ECC_Visibility);
	if (UKismetSystemLibrary::LineTraceSingle(GetWorld(), TraceStart, TraceEnd, TraceType, true, TArray<AActor*>(), EDrawDebugTrace::ForOneFrame, HitResult, true))
	{	
		float CharacterBottom = TraceStart.Z - CapsuleHalfHeight;
		Result = CharacterBottom - HitResult.Location.Z;	
	}

	return Result;
}

float AGCBaseCharacter::CalculateIKPelvisOffset()
{
	return -FMath::Abs(IKRightFootSocketOffset - IKLeftFootSocketOffset);
}

void AGCBaseCharacter::TryChangeSprintState(float DeltaTime)
{
	
	bool bIsSprintActive = AbilitySystemComponent->IsAbilityActive(SprintAbilityTag);
	if (bIsSprintRequested && !bIsSprintActive  && CanSprint() /*|| !IsStartFireMeleeWeapon()*/)
	{
		if (AbilitySystemComponent->TryActivateAbilityWithTag(SprintAbilityTag))
		{		
			OnSprintStart(); 
		}
	
	}
	if ( bIsSprintActive && !(bIsSprintRequested && CanSprint()) && GCBaseCharacterMovementComponent->IsSprinting() || IsStartFireMeleeWeapon())
	{
		if (AbilitySystemComponent->TryCancelAbilityWithTag(SprintAbilityTag))
		{
			OnSprintEnd();
		}
	}
	
}

void AGCBaseCharacter::EnableRagdoll()
{
	GetMesh()->SetCollisionProfileName(CollisionProfileRagdoll);
	GetMesh()->SetSimulatePhysics(true);
}



