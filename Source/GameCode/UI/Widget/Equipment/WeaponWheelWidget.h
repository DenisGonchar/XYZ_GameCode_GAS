// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameCodeTypes.h"
#include "Inventory/Items/InventoryItem.h"
#include "WeaponWheelWidget.generated.h"

class UImage;
class UTextBlock;
class UMaterialInstanceDynamic;
class UCharacterEquipmentComponent;

UCLASS()
class GAMECODE_API UWeaponWheelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeWeaponWheelWidget(UCharacterEquipmentComponent* EquipmentComponent);

	void NextSegment();
	void PreviousSegment();

	void ConfirmSelection();

protected:
	
	virtual void NativeConstruct() override;
	void SelectSegment();

	UPROPERTY(meta = (BindWidget))
	UImage* RadialBackground;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponNameText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon whell settings")
	TArray<EEquipmentSlots> EquipmentSlotSegments;

private:
	FWeaponTableRow* GetTableRowForSegment(int32 SegmentIndex) const;

	int32 CurrentsSegmentIndex;

	UMaterialInstanceDynamic* BackgroundMaterial;

	TWeakObjectPtr<UCharacterEquipmentComponent> LinckedEqupmentComponent;
};
