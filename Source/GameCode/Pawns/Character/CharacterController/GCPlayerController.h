// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GCPlayerController.generated.h"

class UPlayerHUDWidget;
class UDroneHUDWidget;

DECLARE_DELEGATE_OneParam(FOnTargetHit, bool)

UCLASS()
class GAMECODE_API AGCPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	
	virtual void SetPawn(APawn* InPawn) override;

	virtual void Tick(float DeltaSeconds) override;
	
	bool GetIgnoreCameraPitch() const;

	void SetIgnoreCameraPitch(bool bIgnoreCameraPitch_In);

	virtual void InstigatedAnyDamage(float Damage, const UDamageType* DamageType, AActor* DamagedActor, AActor* DamageCauser) override;

	void SetVisibilityCharacterWidgets(ESlateVisibility SlateVisibility);
	void SetVisibilityDroneWidgets(ESlateVisibility SlateVisibility);
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UPlayerHUDWidget> PlayerHUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UDroneHUDWidget> DroneHUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<class UUserWidget> MainMenuWidgetClass;

	virtual void SetupInputComponent() override;

	FOnTargetHit OnTargetHit;
private:

	void MoveUp(float Value);
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);

	//Drone
	void Detonation();
	void BackToCharacter();

	void NextActiveDrone();
	void PreviousActiveDrone();

	void SwitchCamera();
	
	//Crouch
	void ChangeCrouchState();

	//Prone
	void ChangeProneState();

	//RockClimbing
	void RockClimbing();
	void UpMoveClimbing();
	void DownMoveClimbing();
	
	//Mantle
	void Mantle();
	
	//Jump
	void Jump();
	
	//Sprint
	void StartSprint();
	void StopSprint();

	//Swim
	void SwimForward(float Value);
	void SwimRight(float Value);
	void SwimUp(float Value);

	//Ladder
	void ClimbLadder(float Value);
	void InteractWithLadder();

	//Zipline
	void InteractWithZipline();

	//WallRun
	void WallRun();

	//Slide
	void Slide();
		
	//Fire
	void PlayerStartFire();
	void PlayerStopFire();

	//Aim
	void StartAim();
	void StopAim();

	//Reload
	void Reload();

	//Item
	void NextItem();
	void PreviousItem();

	void EquipPrimaryItem();

	//Melee attack
	void PrimaryMeleeAttack();
	void SecondaryMeleeAttack();

	//AdditionalShootingMode
	void AdditionalShootingMode();
	
	//Menu
	void ToggleMainMenu();

	//Inventory
	void Interact();
	void UseInventory();

	void ConfirmWeaponWheelSelection();

	//SpawnDrone
	void Drone();
	void ActiveDrone();
	
	void QuickSaveGame();
	void QuickLoadGame();

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	void Debug_IncreaseGlobalDilation();
	void Debug_DecreaseGlobalDilation();
#endif

	TSoftObjectPtr<class AGCBaseCharacter> CachedBaseCharacter;
	TSoftObjectPtr<class AGCBaseDrone> CachedBaseDrone;

private:

	bool bIgnoreCameraPitch = false;

	void OnInteractableObjectFound(FName ActionName);
	void CreateAndInitializeWidget();
	
	UPlayerHUDWidget* PlayerHUDWidget = nullptr;
	UDroneHUDWidget* DroneHUDWidget = nullptr;
	UUserWidget* MainMenuWidget = nullptr;


};
