// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameCodeTypes.h"
#include "GenericTeamAgentInterface.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "GCBaseDrone.generated.h"

class UBoxComponent;
class UCameraComponent;
class UStaticMeshComponent;
class USpringArmComponent;
class UExplosionComponent;

class AGCBaseCharacter;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnRotationAngleChanged, FRotator)

UCLASS()
class GAMECODE_API AGCBaseDrone : public APawn, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	AGCBaseDrone();
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

#pragma region Move
	UFUNCTION(BlueprintCallable)
	void MoveForward(float Value);
	UFUNCTION(BlueprintCallable)
	void MoveRight(float Value);
	UFUNCTION(BlueprintCallable)
	void MoveUp(float Value);

	UFUNCTION(BlueprintCallable)
	void Turn(float Value);
	UFUNCTION(BlueprintCallable)
	void LookUp(float Value);
#pragma endregion Move

	void OnRotatorAngleBind(FRotator Rotation);
	
	void Detonation();
	void BackToCharacter();

	void PreviousDrone();
	void NextDrone();
	
	AGCBaseCharacter* GetOwnerCharacter() const {return OwnerCharacter; };
	void SetOwnerCharacter(AGCBaseCharacter* Character);

	void RemoveDrone();
	
	void SwitchCameara();

	void OnViewFirstCamera();
	void OnViewThirdCamera();
	void DefaultViewCamera();
	
	void SetSeeMesh(bool IsSee);
	
#pragma region IGenericTeamAgentInterface 
	virtual FGenericTeamId GetGenericTeamId() const override;

#pragma endregion IGenericTeamAgentInterface

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser);

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FName EquippedSocketName = NAME_Name;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TSubclassOf<class AGCDroneController> DroneController;

	FOnRotationAngleChanged OnRotationAngleChanged;
	
protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UBoxComponent* MoverComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* StaticMeshComponent;

	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UCameraComponent* ThirdCameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UCameraComponent* FirstCameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USpringArmComponent* ThirdSpringArmComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USpringArmComponent* FirstSpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UExplosionComponent* ExlosionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Parameters|Speed")
	float ForwardSpeed = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Parameters|Speed")
	float RightSpeed = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Parameters|Speed")
	float UpSpeed = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Parameters|Speed")
	float BaseTurnRate = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Parameters|Speed")
	float BaseLookUpRate = 45.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Parameters|Angle")
	float RotationRollAngle = 45.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Parameters|Angle")
	float RotationPitchAngle = 45.0f; 

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Parameters|Angle")
	float RotationRollAngleSpeed = 10.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Parameters|Angle")
	float RotationPitchAngleSpeed = 10.0f; 

#pragma region Team
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Team")
	ETeams Team = ETeams::Enemy;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Team")
	float LineOfSightDistance = 500.0f;
#pragma endregion Team
	
private:
	float CurrentSpeed;
	FRotator RotationAngle = FRotator::ZeroRotator;
	
	UPROPERTY()
	AGCBaseCharacter* OwnerCharacter;
};
