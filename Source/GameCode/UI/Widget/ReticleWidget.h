// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameCodeTypes.h"
#include "Actors/Equipment/Weapons/BowWeaponItem.h"
#include "ReticleWidget.generated.h"

class AEquipableItem;
class UBowstringWidget;

UCLASS()
class GAMECODE_API UReticleWidget : public UUserWidget
{
	GENERATED_BODY()
public:

	UBowstringWidget* GetBowstringWidget();
	
protected:
	UFUNCTION(BlueprintNativeEvent)
	void OnAimingStateChanged(bool bIsAiming);

	UFUNCTION(BlueprintNativeEvent)
	void OnEquippedItemChanged(AEquipableItem* EquippedItem);

	UFUNCTION(BlueprintImplementableEvent)
	void OnTargetHit(bool bIsKilled);

	UFUNCTION(BlueprintNativeEvent)
	void OnRotationAngleChanged(FRotator NewRotatorAngle);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget names")
	FName BowstringWidgetName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Reticle")
	EReticleType CurrentReticle = EReticleType::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FRotator RotationAngle = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Reticle")
	EEquipableItemType CurrentTypeEquipableItem = EEquipableItemType::None;
	
private:
	
	TWeakObjectPtr<const AEquipableItem> CurrentEquippedItem;

	ABowWeaponItem* BowWeaponItem;
	
	void SetupCurrentReticle();
};
