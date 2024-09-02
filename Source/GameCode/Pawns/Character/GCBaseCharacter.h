// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameCodeTypes.h"
#include "GenericTeamAgentInterface.h"
#include <UObject/ScriptInterface.h>
#include <Subsystems/SaveSubsystem/SaveSubsystemInterface.h>

#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "SignificanceManager.h"
#include "Components/CharacterComponents/CharacterMoveComponent.h"
#include "GCBaseCharacter.generated.h"

class UGCCharacterAttributeSet;
class IInteractable;
class UInventoryItem;
class UAnimMontage;

class AInteractiveActor;
class UGCBaseCharacterMovementComponent;
class UCharacterEquipmentComponent;
class UCharacterAttributeComponent;
class UCharacterInventoryComponent;
class UCharacterHitReactionComponent;
class ULedgeDetectorComponent;
class UGCAbilitySystemComponent;
class UWidgetComponent;

class AGCBaseDrone;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnAimingStateChanged, bool)
DECLARE_DELEGATE_OneParam(FOnInteractableObjectFound, FName)

UCLASS(Abstract,NotBlueprintable)
class GAMECODE_API AGCBaseCharacter : public ACharacter, public IGenericTeamAgentInterface, public ISaveSubsystemInterface, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AGCBaseCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void Tick(float DeltaTime) override;

	virtual float InternalTakePointDamage(float Damage, struct FPointDamageEvent const& PointDamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

#pragma region Component
	UGCBaseCharacterMovementComponent* GetBaseCharacterMovementComponent() const {return GCBaseCharacterMovementComponent; }

	const UCharacterEquipmentComponent* GetCharacterEquipmentComponent() const { return CharacterEquipmentComponent; } 
	UCharacterEquipmentComponent* GetCharacterEquipmentComponent_Muteble() const { return CharacterEquipmentComponent; }

	const UCharacterAttributeComponent* GetCharacterAttributeComponent() const { return CharacterAttributeComponent; }
	UCharacterAttributeComponent* GetCharacterAttributeComponent_Muteble() const { return CharacterAttributeComponent; }

	const UCharacterInventoryComponent* GetCharacterInventoryComponent() const { return CharacterInventoryComponent; }
	UCharacterInventoryComponent* GetCharacterInventoryComponent_Muteble() const { return CharacterInventoryComponent; }

	ULedgeDetectorComponent* GetLedgeDetectorComponent() const { return LedgeDetertorComponent; }
	UCharacterMoveComponent* GetCharacterMoveComponent() const { return CharacterBaseMoveComponent; }

	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override; 
	const UGCCharacterAttributeSet* GetGCCharacterAttributeSet() const { return  AttributeSet; }

#pragma endregion Component
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PossessedBy(AController* NewController) override;

#pragma region BaseMove
	virtual void MoveForward(float Value) {ForwardAxis = Value;};
	virtual void MoveRight(float Value) { RightAxis = Value; };
	virtual void Turn(float Value) {};
	virtual void LookUp(float Value) {};
#pragma endregion BaseMove

#pragma region  Crouch
	virtual void ChangeCrouchState();
#pragma endregion Crounch
	
#pragma region Prone
	virtual void ChangeProneState();
	
	virtual void Prone(bool bClientSimulation = false);

	virtual void UnProne(bool bIsFullHeight, bool bClientSimulation = false);

	virtual bool CanProne() const;

	virtual void OnStartProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust);
	virtual void OnEndProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnStartProne", ScriptName = "OnStartProne"))
	void K2_OnStartProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnEndProne", ScriptName = "OnEndProne"))
	void K2_OnEndProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust);
#pragma endregion Prone
	
#pragma region Jump
	virtual void Jump() override;
	virtual bool CanJumpInternal_Implementation() const override;
	virtual void OnJumped_Implementation() override;
#pragma endregion Jump
	
#pragma region Fall
	virtual void Falling() override;
	virtual void NotifyJumpApex() override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void HardLanding();

	virtual void LimitControl();
	virtual void UnLimitControl();
#pragma endregion Fall
	
#pragma region Sprint
	virtual void StartSprint();
	virtual void StopSprint();
#pragma endregion Sprint
	
#pragma region Swim
	virtual void SwimForward(float Value) {};
	virtual void SwimRight(float Value) {};
	virtual void SwimUp(float Value) {};
#pragma endregion Swim
	
#pragma region Mantle
	UFUNCTION(BlueprintCallable)
	void Mantle(bool bForce = false);
	virtual void OnMantle(const FMantlingSetting& MantlingSetting, float MantlingAnimationStartTime) { };
#pragma endregion Montle

#pragma region Attribute
	void LadderUp(float Value);
	void Ladder();

	void Zipline();
	
	void WallRun();

	void Slide();
	
	void Stamina(bool bStamina);

	UFUNCTION(BlueprintCallable)
	bool IsSwimmingUnderWater() const;
#pragma endregion Attribute
	
#pragma region Fire
	void StartFire();
	void StopFire();
#pragma endregion Fire

#pragma region FireMelee
	void PrimaryMeleeAttack();
	void SecondaryMeleeAttack();

	virtual bool IsStartFireMeleeWeapon();
#pragma endregion FireMelee
	
#pragma region FireAndAim
	FRotator GetAimOffset();
	void StartAiming();
	void StopAiming();
	bool IsAiming() const;
	float GetAimingMovementSpeed() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Character")
	void OnStartAiming();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Character")
	void OnStopAiming();
#pragma endregion FireAndAim
	
#pragma region Reload
	void Reload();
#pragma endregion Reload

#pragma region Switch
	void NextItem();
	void PreviousItem();

	void EquipPrimaryItem();

	void AdditionalShootingMode();
#pragma endregion Switch
	
#pragma region IKSocket
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetIKRightFootSocketOffset () const {return IKRightFootSocketOffset;}
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetIKLeftFootSocketOffset() const { return IKLeftFootSocketOffset; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetIKPelvisSocketOffset() const { return IKPelvisOffset; }
#pragma endregion IKSocket

#pragma region Interactive
	void RegisterInteractiveActor(AInteractiveActor* InteractiveActor);
	void UnregisterInteractiveActor(AInteractiveActor* InteractiveActor);

	TArray<TWeakObjectPtr<AInteractiveActor>, TInlineAllocator<10>> GetAvaibleInteractiveActors();

	void TraceLineOfSight();

	void Interact();
	
	bool PickupItem(TWeakObjectPtr<UInventoryItem> ItemToPickup, int32 Count = 1);
	void UseInventory(APlayerController* PlayerController);
#pragma endregion Interactive

#pragma region Drone
	void Drone();
	void ActiveDrone(); 
	
	void SpawnDrone();
	void PossessDrone(int8 Index);

	int8 GetCurrentIndexActiveDrone() const;
	void SetCurrentIndexActiveDrone(int8 NewIndex);
	
	TArray<AGCBaseDrone*> GetActiveDrones() const { return ActiveDrones; };
	void RemoveActiveDrones(AGCBaseDrone* Drone);
#pragma endregion Drone
	
#pragma region Widget
	void InitializeHealthProgress();

	void ConfirmWeaponSelection();
#pragma endregion Widget	
	
#pragma region ISaveSubsystemInterface
	//virtual void OnLevelDeserialized_Implementation() override;
#pragma endregion ISaveSubsystemInterface

#pragma region IGenericTeamAgentInterface 
	virtual FGenericTeamId GetGenericTeamId() const override;

#pragma endregion IGenericTeamAgentInterface

public:
#pragma region BaseMove
	float ForwardAxis = 0.0f;
	float RightAxis = 0.0f;
#pragma endregion BaseMove
	
#pragma region Prone
	UPROPERTY(BlueprintReadOnly, Category = "Character | Prone")
	bool bIsProned = false;
#pragma endregion Prone

#pragma region FireAndAim
	FOnAimingStateChanged OnAimingStateChanged;
#pragma endregion FireAndAim
	
#pragma region INteractive
	FOnInteractableObjectFound OnInteractableObjectFound;
#pragma endregion INteractive
	
#pragma region IKSocket
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | IK Setting", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float IKTraceExtendDistance = 30.0f;
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Character | IK Setting", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float IKInterpSpeed = 20.0f;
#pragma endregion IKSocket

protected:
	virtual void OnDeath();

#pragma region Sprint
	UFUNCTION(BlueprintNativeEvent, Category = "Character | Movement")
	void OnSprintStart();
	virtual void OnSprintStart_Implementation();
	
	UFUNCTION(BlueprintNativeEvent, Category = "Character | Movement")
	void OnSprintEnd();
	virtual void OnSprintEnd_Implementation();

	virtual bool CanSprint();
#pragma endregion Sprint
	
#pragma region Aiming
	virtual void OnStartAimingInternal();
	virtual void OnStopAimingInternal();
#pragma endregion Aiming
	
#pragma region Move
	virtual bool CanMove();
#pragma endregion Move
	
protected:
#pragma region Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Components")
	UGCBaseCharacterMovementComponent* GCBaseCharacterMovementComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Components")
	UCharacterMoveComponent* CharacterBaseMoveComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Components")
	ULedgeDetectorComponent* LedgeDetertorComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Components")
	UCharacterAttributeComponent* CharacterAttributeComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Components")
	UCharacterEquipmentComponent* CharacterEquipmentComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Components")
	UCharacterInventoryComponent* CharacterInventoryComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Components")
	UCharacterHitReactionComponent* CharacterHitReactionComponent;
	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly,Category = "Character | Components")
	UWidgetComponent* HealthBarProgressComponent;

#pragma endregion Component

#pragma region IKNameSocket
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | IK Socket name")
	FName RightFootSocketName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | IK Socket name")
	FName LeftFootSocketName;
#pragma endregion IKNameSocket

#pragma region Death
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,  Category = "Character | Animations")
	UAnimMontage* OnDeathAnimMontage;
#pragma endregion Death
	
#pragma region Fall
	//Damage depending from fall height (in meters)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Attributes")
	class UCurveFloat* FallDamageCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Attributes")
	float HardLandingHeight = 8.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Attributes")
	UAnimMontage* HardLandingAnimMontage;
#pragma endregion Fall
	
#pragma region Widget
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Oxygen")
	FName SocketHead;
#pragma endregion Widget
	
#pragma region Team
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Team")
	ETeams Team = ETeams::Enemy;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Team")
	float LineOfSightDistance = 500.0f;
#pragma endregion Team

#pragma region Interactabel
	UPROPERTY()
	TScriptInterface<IInteractable> LineOfSightObject;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Significance")
	bool bIsSignificanceEnabled = true; 

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Significance")
	float VeryHighSignificanceDistance = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Significance")
	float HighSignificanceDistance = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Significance")
	float MediumSignificanceDistance = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Significance")
	float LowSignificanceDistance = 6000.0f;
#pragma endregion Interactabel

#pragma region Drone
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Drone",  meta = (ClampMin = 1, UIMin = 1))
	int MaxNumberDrones = 3;

#pragma endregion Drone
	
#pragma region GameplayAbilities
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category= "Abilities")
	UGCAbilitySystemComponent* AbilitySystemComponent;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category= "Abilities")
	UGCCharacterAttributeSet* AttributeSet;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Abilities")
	TArray<TSubclassOf<class UGameplayAbility>> Abilities;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Abilities")
	FGameplayTag SprintAbilityTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Abilities")
	FGameplayTag CrouchAbilityTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Abilities")
	FGameplayTagContainer InitialActiveAbilities;
	
	bool bAreAbilityAdded;
#pragma endregion GameplayAbilities

private:
#pragma region Drone
	TArray<AGCBaseDrone*> ActiveDrones;
	AGCBaseDrone* CurrentSpawnDrone;
	
	int8 CurrentIndexActiveDrone = 0;
#pragma endregion Drone

#pragma region GameplayAbilities
	void InitGameplayAbilitySystem(AController* NewController);
	
#pragma endregion GameplayAbilities
	
	float SingnificanceFunction(USignificanceManager::FManagedObjectInfo* ObjectInfo, const FTransform& ViewPoint);
	void PostSignificanceFunction(USignificanceManager::FManagedObjectInfo* ObjectInfo, float OldSignificance, float Significance, bool bFinal);
	
#pragma region IK
	void UpdateIkSetting(float DeltaSeconds);
	float GetIKOffsetForSocket(const FName& SocketName) const;
	float CalculateIKPelvisOffset();
#pragma endregion IK

#pragma region Sprint
	void TryChangeSprintState(float DeltaTime);
	void EnableRagdoll();
#pragma endregion Sprint
	
private:
	
#pragma region Fall
	FVector CurrentFallApex;
	FTimerHandle HardLandingTimer;
#pragma endregion Fall
	
#pragma region IK
	float IKRightFootSocketOffset = 0.0f;
	float IKLeftFootSocketOffset = 0.0f;
	float IKPelvisOffset = 0.0f;

	float IKTraceDistance = 0.0f;
	float IKScale = 0.0f;
#pragma endregion IK

#pragma region Sprint
	bool bIsSprintRequested = false;
#pragma endregion Sprint

	
#pragma region Aim
	bool bIsAiming;
	float CurrentAimingMovementSpeed;
#pragma endregion Aim
	
	TArray<TWeakObjectPtr<AInteractiveActor>, TInlineAllocator<10>> AvaibleInteractiveActors;
};
