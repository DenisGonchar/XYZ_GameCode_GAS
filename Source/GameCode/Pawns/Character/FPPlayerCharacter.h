// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerCharacter.h"
#include "Components/CharacterComponents/CharacterMoveComponent.h"
#include "FPPlayerCharacter.generated.h"

class UAnimMontage;

UCLASS()
class GAMECODE_API AFPPlayerCharacter : public APlayerCharacter
{
	GENERATED_BODY()
	
public:
	AFPPlayerCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void PossessedBy(AController* NewController) override;

	virtual void Tick(float DeltaTime) override;

	virtual void OnStartCrouch(float HalfHeightAbjust, float ScaledHalfHeightAnjust) override;
	virtual void OnEndCrouch(float HalfHeightAbjust, float ScaledHalfHeightAnjust) override;

	virtual void OnSprintStart_Implementation() override;
	virtual void OnSprintEnd_Implementation() override;

	virtual void HardLanding() override;

	virtual void LimitControl() override;
	virtual void UnLimitControl() override;

	virtual FRotator GetViewRotation() const override;

	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode = 0) override;

	virtual void OnMantle(const FMantlingSetting& MantlingSetting, float MantlingAnimationStartTime) override;
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | First person")
	class USkeletalMeshComponent*  FirstPersonMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | First person")
	class UCameraComponent* FirstPersonCameraComponent;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | First person | Camera | Ladder | Pitch", meta = (UIMin = -89.0f, UIMax = 89.0f) )
	float LadderCameraMinPitch = -60.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | First person | Camera | Ladder | Pitch", meta = (UIMin = -89.0f, UIMax = 89.0f))
	float LadderCameraMaxPitch = 80.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | First person | Camera | Ladder | Yaw", meta = (UIMin = 0.0f, UIMax = 359.0f))
	float LadderCameraMinYaw = 5.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | First person | Camera | Ladder | Yaw", meta = (UIMin = 0.0f, UIMax = 359.0f))
	float LadderCameraMaxYaw = 175.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | First person | Camera | Zipline | Pitch", meta = (UIMin = -89.0f, UIMax = 89.0f))
	float ZiplineCameraMinPitch = -60.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | First person | Camera | Zipline | Pitch", meta = (UIMin = -89.0f, UIMax = 89.0f))
	float ZiplineCameraMaxPitch = 80.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | First person | Camera | Zipline | Yaw", meta = (UIMin = 0.0f, UIMax = 359.0f))
	float ZiplineCameraMinYaw = 5.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | First person | Camera | Zipline | Yaw", meta = (UIMin = 0.0f, UIMax = 359.0f))
	float ZiplineCameraMaxYaw = 175.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Animation")
	UAnimMontage* FPHardLandingAnimMontage;

private:
	
	void OnLadderStarted();
	void OnLadderStopped();
	
	void OnZiplineStarted();
	void OnZiplineStoped();

	FTimerHandle FPMontageTimer;
	
	bool IsFPMontagePlaying() const;

	void OnFPMontageTimerElapsed();

	TWeakObjectPtr<class AGCPlayerController> GCPlayerController;

};
