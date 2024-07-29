// Fill out your copyright notice in the Description page of Project Settings.


#include "GCPlayerController.h"
#include "../GCBaseCharacter.h"
#include "Blueprint/UserWidget.h"
#include "UI/Widget/PlayerHUDWidget.h"
#include "UI/Widget/ReticleWidget.h"
#include "UI/Widget/AmmoWidget.h"
#include "UI/Widget/CharacterAttributesWidget.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "Components/CharacterComponents/CharacterAttributeComponent.h"
#include "GameCodeTypes.h"
#include "GameFramework/PlayerInput.h"
#include "Subsystems/SaveSubsystem/SaveSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "SignificanceManager.h"
#include "Pawns/Drone/GCBaseDrone.h"
#include "Subsystems/DebugSubsystem.h"
#include "UI/Widget/DroneHUDWidget.h"

void AGCPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	CachedBaseCharacter = Cast<AGCBaseCharacter>(InPawn);
	
	if (CachedBaseCharacter.IsValid() && IsLocalController())
	{
		CreateAndInitializeWidget();
		CachedBaseCharacter->OnInteractableObjectFound.BindUObject(this, &AGCPlayerController::OnInteractableObjectFound);

		SetVisibilityCharacterWidgets(ESlateVisibility::Visible);
		SetVisibilityDroneWidgets(ESlateVisibility::Hidden);
		
	}

	CachedBaseDrone = Cast<AGCBaseDrone>(InPawn);
	if (CachedBaseDrone.IsValid())
	{
		CreateAndInitializeWidget();	
		SetVisibilityCharacterWidgets(ESlateVisibility::Hidden);
		SetVisibilityDroneWidgets(ESlateVisibility::Visible);
	}
}

void AGCPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	USignificanceManager* SignificanceManager = FSignificanceManagerModule::Get(GetWorld());
	if (IsValid(SignificanceManager))
	{
		FVector ViewLocation;
		FRotator ViewRotation;
		GetPlayerViewPoint(ViewLocation, ViewRotation);
		FTransform ViewTransform(ViewRotation, ViewLocation);
		TArray<FTransform> Viewpoints = { ViewTransform };
		SignificanceManager->Update(Viewpoints);
		 
	}
}

bool AGCPlayerController::GetIgnoreCameraPitch() const
{
	return bIgnoreCameraPitch;
}

void AGCPlayerController::SetIgnoreCameraPitch(bool bIgnoreCameraPitch_In)
{
	bIgnoreCameraPitch = bIgnoreCameraPitch_In;
}

void AGCPlayerController::InstigatedAnyDamage(float Damage, const UDamageType* DamageType, AActor* DamagedActor,
	AActor* DamageCauser)
{
	Super::InstigatedAnyDamage(Damage, DamageType, DamagedActor, DamageCauser);

	UCharacterAttributeComponent* AttributeComponent = DamagedActor->FindComponentByClass<UCharacterAttributeComponent>();
	if (IsValid(AttributeComponent))
	{
		OnTargetHit.ExecuteIfBound(!AttributeComponent->IsAlive());
	}
	
}

void AGCPlayerController::SetVisibilityCharacterWidgets(ESlateVisibility SlateVisibility)
{
	if (IsValid(PlayerHUDWidget))
	{
		PlayerHUDWidget->SetVisibility(SlateVisibility);
	}
	//if (IsValid(MainMenuWidget) && MainMenuWidget != nullptr)
	//{
	//	MainMenuWidget->SetVisibility(SlateVisibility);
	//}
}

void AGCPlayerController::SetVisibilityDroneWidgets(ESlateVisibility SlateVisibility)
{
	if (IsValid(DroneHUDWidget))
	{
		DroneHUDWidget->SetVisibility(SlateVisibility);
	}
}

void AGCPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAxis("MoveForward", this, &AGCPlayerController::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AGCPlayerController::MoveRight);
	InputComponent->BindAxis("Turn", this, &AGCPlayerController::Turn);
	InputComponent->BindAxis("LookUp", this, &AGCPlayerController::LookUp);
	InputComponent->BindAxis("SwimForward", this, &AGCPlayerController::SwimForward);
	InputComponent->BindAxis("SwimRight", this, &AGCPlayerController::SwimRight);
	InputComponent->BindAxis("SwimUp", this, &AGCPlayerController::SwimUp);
	InputComponent->BindAxis("ClimbLadderUp", this, &AGCPlayerController::ClimbLadder);
	InputComponent->BindAction("InteractWithLadder", EInputEvent::IE_Pressed, this, &AGCPlayerController::InteractWithLadder);
	InputComponent->BindAction("InteractWithZipline", EInputEvent::IE_Pressed, this, &AGCPlayerController::InteractWithZipline);
	InputComponent->BindAction("WallRun", EInputEvent::IE_Pressed, this, &AGCPlayerController::WallRun);
	InputComponent->BindAction("Slide", EInputEvent::IE_Pressed, this, &AGCPlayerController::Slide);
	InputComponent->BindAction("Mantle", EInputEvent::IE_Pressed, this, &AGCPlayerController::Mantle);
	InputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &AGCPlayerController::Jump);
	InputComponent->BindAction("Crouch",EInputEvent::IE_Pressed, this, &AGCPlayerController::ChangeCrouchState);
	InputComponent->BindAction("Sprint", EInputEvent::IE_Pressed, this, &AGCPlayerController::StartSprint);
	InputComponent->BindAction("Sprint", EInputEvent::IE_Released, this, &AGCPlayerController::StopSprint);
	InputComponent->BindAction("Prone", EInputEvent::IE_Pressed, this, &AGCPlayerController::ChangeProneState);
	InputComponent->BindAction("Fire", EInputEvent::IE_Pressed, this, &AGCPlayerController::PlayerStartFire);
	InputComponent->BindAction("Fire", EInputEvent::IE_Released, this, &AGCPlayerController::PlayerStopFire);
	InputComponent->BindAction("Aim", EInputEvent::IE_Pressed, this, &AGCPlayerController::StartAim);
	InputComponent->BindAction("Aim", EInputEvent::IE_Released, this, &AGCPlayerController::StopAim);
	InputComponent->BindAction("Reload", EInputEvent::IE_Pressed, this, &AGCPlayerController::Reload);
	InputComponent->BindAction("NextItem", EInputEvent::IE_Pressed, this, &AGCPlayerController::NextItem);
	InputComponent->BindAction("PreviousItem", EInputEvent::IE_Pressed, this, &AGCPlayerController::PreviousItem);
	InputComponent->BindAction("EquipPrimaryItem", EInputEvent::IE_Pressed, this, &AGCPlayerController::EquipPrimaryItem);
	InputComponent->BindAction("PrimaryMeleeAttack", EInputEvent::IE_Pressed, this, &AGCPlayerController::PrimaryMeleeAttack);
	InputComponent->BindAction("SecondaryMeleeAttack", EInputEvent::IE_Pressed, this, &AGCPlayerController::SecondaryMeleeAttack);
	InputComponent->BindAction(ActionInteract, EInputEvent::IE_Pressed, this, &AGCPlayerController::Interact);
	InputComponent->BindAction("UseInventory", EInputEvent::IE_Pressed, this, &AGCPlayerController::UseInventory);
	InputComponent->BindAction("ConfirmWeaponWheelSelection", EInputEvent::IE_Pressed, this, &AGCPlayerController::ConfirmWeaponWheelSelection);
	InputComponent->BindAction("QuickSaveGame", EInputEvent::IE_Pressed, this, &AGCPlayerController::QuickSaveGame);
	InputComponent->BindAction("QuickLoadGame", EInputEvent::IE_Pressed, this, &AGCPlayerController::QuickLoadGame);
	InputComponent->BindAction("AdditionalShootingMode", EInputEvent::IE_Pressed, this, &AGCPlayerController::AdditionalShootingMode);
	InputComponent->BindAction("SpawnDrone", EInputEvent::IE_Pressed, this, &AGCPlayerController::Drone);
	InputComponent->BindAction("ActiveDrone", EInputEvent::IE_Pressed, this, &AGCPlayerController::ActiveDrone);
	
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	InputComponent->BindAction("Debug_IncreaseGlobalDilation", EInputEvent::IE_Pressed, this, &AGCPlayerController::Debug_IncreaseGlobalDilation);
	InputComponent->BindAction("Debug_DecreaseGlobalDilation", EInputEvent::IE_Pressed, this, &AGCPlayerController::Debug_DecreaseGlobalDilation);
#endif

	FInputActionBinding& ToggleMenuBinding = InputComponent->BindAction("ToggleMainMenu", EInputEvent::IE_Pressed, this, &AGCPlayerController::ToggleMainMenu);
	ToggleMenuBinding.bExecuteWhenPaused = true;

	InputComponent->BindAxis("Drone_MoveForward", this, &AGCPlayerController::MoveForward);
	InputComponent->BindAxis("Drone_MoveRight", this, &AGCPlayerController::MoveRight);
	InputComponent->BindAxis("Drone_MoveUp", this, &AGCPlayerController::MoveUp);
	
	InputComponent->BindAction("Drone_Detonation", IE_Pressed, this, &AGCPlayerController::Detonation);
	InputComponent->BindAction("Drone_BackToCharacter", IE_Pressed, this, &AGCPlayerController::BackToCharacter);
	InputComponent->BindAction("Drone_Next", IE_Pressed, this, &AGCPlayerController::NextActiveDrone);
	InputComponent->BindAction("Drone_Previous", IE_Pressed, this, &AGCPlayerController::PreviousActiveDrone);
	InputComponent->BindAction("Drone_SwitchCamera", IE_Pressed, this, &AGCPlayerController::SwitchCamera);
}

void AGCPlayerController::MoveUp(float Value)
{
	if (CachedBaseDrone.IsValid())
	{
		CachedBaseDrone->MoveUp(Value);	
	}
}

void AGCPlayerController::MoveForward(float Value)
{
	if ( CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->MoveForward(Value);

	}
	
	if (CachedBaseDrone.IsValid())
	{
		CachedBaseDrone->MoveForward(Value);
	}
}

void AGCPlayerController::MoveRight(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->MoveRight(Value);

	}

	if (CachedBaseDrone.IsValid())
	{
		CachedBaseDrone->MoveRight(Value);
	}
}

void AGCPlayerController::Turn(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->Turn(Value);

	}
	if (CachedBaseDrone.IsValid())
	{
		CachedBaseDrone->Turn(Value);

	}
}

void AGCPlayerController::LookUp(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->LookUp(Value);

	}

	if (CachedBaseDrone.IsValid())
	{
		CachedBaseDrone->LookUp(Value);

	}
}

void AGCPlayerController::Detonation()
{
	if (CachedBaseDrone.IsValid())
	{
		CachedBaseDrone->Detonation();
	}
}

void AGCPlayerController::BackToCharacter()
{
	if (CachedBaseDrone.IsValid())
	{
		CachedBaseDrone->BackToCharacter();
	}
}

void AGCPlayerController::NextActiveDrone()
{
	if (CachedBaseDrone.IsValid())
	{
		CachedBaseDrone->NextDrone();
	}
}

void AGCPlayerController::PreviousActiveDrone()
{
	if (CachedBaseDrone.IsValid())
	{
		CachedBaseDrone->PreviousDrone();
	}
}

void AGCPlayerController::SwitchCamera()
{
	if (CachedBaseDrone.IsValid())
	{
		CachedBaseDrone->SwitchCameara();
	}
}

void AGCPlayerController::ChangeCrouchState()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->ChangeCrouchState();

	}

}

void AGCPlayerController::ChangeProneState()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->ChangeProneState();
	
	}
}

void AGCPlayerController::Mantle()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->Mantle();

	}
}

void AGCPlayerController::Jump()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->Jump();

	}
}

void AGCPlayerController::StartSprint()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StartSprint();

	}

}

void AGCPlayerController::StopSprint()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StopSprint();

	}
}

void AGCPlayerController::SwimForward(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->SwimForward(Value);

	}

}

void AGCPlayerController::SwimRight(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->SwimRight(Value);

	}
}

void AGCPlayerController::SwimUp(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->SwimUp(Value);
	}
}

void AGCPlayerController::ClimbLadder(float Value)
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->LadderUp(Value);
	}
}

void AGCPlayerController::InteractWithLadder()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->Ladder();
	}
}

void AGCPlayerController::InteractWithZipline()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->Zipline();
	}
}

void AGCPlayerController::WallRun()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->WallRun();
	}
}

void AGCPlayerController::Slide()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->Slide();
	}
	
}

void AGCPlayerController::PlayerStartFire()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StartFire();
	}

}

void AGCPlayerController::PlayerStopFire()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StopFire();
	}

}

void AGCPlayerController::StartAim()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StartAiming();
	}

}

void AGCPlayerController::StopAim()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->StopAiming();
	}

}

void AGCPlayerController::Reload()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->Reload();
	}

}

void AGCPlayerController::NextItem()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->NextItem();
	}

}

void AGCPlayerController::PreviousItem()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->PreviousItem();
	}

}

void AGCPlayerController::EquipPrimaryItem()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->EquipPrimaryItem();
	}

}

void AGCPlayerController::PrimaryMeleeAttack()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->PrimaryMeleeAttack();
	}

}

void AGCPlayerController::SecondaryMeleeAttack()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->SecondaryMeleeAttack();
	}

}

void AGCPlayerController::AdditionalShootingMode()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->AdditionalShootingMode();
	}
}

void AGCPlayerController::ToggleMainMenu()
{
	if (!IsValid(MainMenuWidget) || !IsValid(PlayerHUDWidget))
	{
		return;
	}
	if (MainMenuWidget->IsVisible())
	{
		MainMenuWidget->RemoveFromParent();
		PlayerHUDWidget->AddToViewport();
		SetInputMode(FInputModeGameOnly {});
		SetPause(false);
		bShowMouseCursor = false;

	}
	else
	{
		MainMenuWidget->AddToViewport();
		PlayerHUDWidget->RemoveFromParent();
		SetInputMode(FInputModeGameAndUI {});
		SetPause(true);
		bShowMouseCursor = true;

	}


}

void AGCPlayerController::Interact()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->Interact();
	}
}

void AGCPlayerController::UseInventory()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->UseInventory(this);
	}
}

void AGCPlayerController::ConfirmWeaponWheelSelection()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->ConfirmWeaponSelection();
	}
}

void AGCPlayerController::Drone()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->Drone();
	}
}

void AGCPlayerController::ActiveDrone()
{
	if (CachedBaseCharacter.IsValid())
	{
		CachedBaseCharacter->ActiveDrone();
	}
}

void AGCPlayerController::QuickSaveGame()
{
	USaveSubsystem* SaveSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<USaveSubsystem>();
	SaveSubsystem->SaveGame();

}

void AGCPlayerController::QuickLoadGame()
{
	USaveSubsystem* SaveSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<USaveSubsystem>();
	SaveSubsystem->LoadLastGame();

}

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
void AGCPlayerController::Debug_IncreaseGlobalDilation()
{
	UDebugSubsystem* DebugSybsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
	DebugSybsystem->IncreaseTimeDilation();

}

void AGCPlayerController::Debug_DecreaseGlobalDilation()
{
	UDebugSubsystem* DebugSybsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
	DebugSybsystem->DecreaseTimeDilation();
}

#endif

void AGCPlayerController::OnInteractableObjectFound(FName ActionName)
{
	if (!IsValid(PlayerHUDWidget))
	{
		return;
	}
	
	TArray<FInputActionKeyMapping> ActionKeys = PlayerInput->GetKeysForAction(ActionName);
	const bool HasAnyKeys = ActionKeys.Num() != 0;

	if (HasAnyKeys)
	{
		FName ActionKey = ActionKeys[0].Key.GetFName();
		PlayerHUDWidget->SetHightInteractableActionText(ActionKey);

	}

	PlayerHUDWidget->SetHighlightInteractebleVisibility(HasAnyKeys);

}

void AGCPlayerController::CreateAndInitializeWidget()
{
	if (!IsValid(PlayerHUDWidget))
	{
		PlayerHUDWidget = CreateWidget<UPlayerHUDWidget>(GetWorld(), PlayerHUDWidgetClass);
		if (IsValid(PlayerHUDWidget))
		{
			PlayerHUDWidget->AddToViewport();
		}
	}

	if (!IsValid(DroneHUDWidget))
	{
		DroneHUDWidget = CreateWidget<UDroneHUDWidget>(GetWorld(), DroneHUDWidgetClass);
		if (IsValid(DroneHUDWidget))
		{
			DroneHUDWidget->AddToViewport();
			if (CachedBaseDrone.IsValid())
			{
				SetVisibilityDroneWidgets(ESlateVisibility::Hidden);
			}
		}
	}
	
	// if (!IsValid(MainMenuWidget))
	// {
	// 	MainMenuWidget = CreateWidget<UUserWidget>(GetWorld(), MainMenuWidgetClass);
	// }

	if (CachedBaseCharacter.IsValid() && IsValid(PlayerHUDWidget))
	{
		CachedBaseCharacter->OnTakeAnyDamage.AddUniqueDynamic(PlayerHUDWidget, &UPlayerHUDWidget::OnTakeAnyDamage);
		CachedBaseCharacter->OnTakePointDamage.AddUniqueDynamic(PlayerHUDWidget, &UPlayerHUDWidget::OnPointDamageReceived);
		
		UReticleWidget* ReticleWidget = PlayerHUDWidget->GetReticalWidget();
		if (IsValid(ReticleWidget))
		{
			CachedBaseCharacter->OnAimingStateChanged.AddUFunction(ReticleWidget, FName("OnAimingStateChanged"));
			UCharacterEquipmentComponent* CharacterEquipment = CachedBaseCharacter->GetCharacterEquipmentComponent_Muteble();
			CharacterEquipment->OnEquippedItemChanged.AddUFunction(ReticleWidget, FName("OnEquippedItemChanged"));

			OnTargetHit.BindUFunction(ReticleWidget, FName("OnTargetHit"));
			
		}

		UAmmoWidget* AmmoWidget = PlayerHUDWidget->GetAmmoWidget();
		if (IsValid(AmmoWidget))
		{
			UCharacterEquipmentComponent* CharacterEquipment = CachedBaseCharacter->GetCharacterEquipmentComponent_Muteble();
			CharacterEquipment->OnCurrentWeaponAmmoChangedEvent.AddUFunction(AmmoWidget, FName("UpdateAmmoCount"));
			CharacterEquipment->OnCurrentThrowItemAmmoChangedEvent.AddUFunction(AmmoWidget,FName("UpdateThrowAmmoCount"));
		}

	}

	if (CachedBaseDrone.IsValid() && IsValid(DroneHUDWidget))
	{
		UReticleWidget* ReticleWidget = DroneHUDWidget->GetReticalWidget();
		if (IsValid(ReticleWidget))
		{
			CachedBaseDrone->OnRotationAngleChanged.AddUFunction(ReticleWidget, FName("OnRotationAngleChanged"));
		}
	}

	SetInputMode(FInputModeGameOnly{});
	bShowMouseCursor = false;
}
