// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Equipment/WeaponWheelWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Inventory/Items/InventoryItem.h"
#include "Actors/Equipment/EquipableItem.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include <Utils/GCDataTableUtils.h>
#include "GameCodeTypes.h"
#include "Blueprint/WidgetTree.h"

void UWeaponWheelWidget::InitializeWeaponWheelWidget(UCharacterEquipmentComponent* EquipmentComponent)
{
	LinckedEqupmentComponent = EquipmentComponent;
}

void UWeaponWheelWidget::NextSegment()
{
	CurrentsSegmentIndex++;
	if (CurrentsSegmentIndex == EquipmentSlotSegments.Num())
	{
		CurrentsSegmentIndex = 0;
	}
	
	SelectSegment();

}

void UWeaponWheelWidget::PreviousSegment()
{
	CurrentsSegmentIndex--;
	if (CurrentsSegmentIndex < 0)
	{
		CurrentsSegmentIndex = EquipmentSlotSegments.Num() - 1;
	}

	SelectSegment();

}

void UWeaponWheelWidget::ConfirmSelection()
{
	EEquipmentSlots SelectedSlot = EquipmentSlotSegments[CurrentsSegmentIndex];
	LinckedEqupmentComponent->EquipItemInSlot(SelectedSlot);
	RemoveFromParent();
}

void UWeaponWheelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(RadialBackground) && !IsValid(BackgroundMaterial))
	{
		BackgroundMaterial = RadialBackground->GetDynamicMaterial();
		BackgroundMaterial->SetScalarParameterValue(FName("Segments"), EquipmentSlotSegments.Num());
	
	}

	for (int i = 0; i < EquipmentSlotSegments.Num(); ++i)
	{
		FName WidgetName = FName(FString::Printf(TEXT("ImageSegment%i"), i));
		UImage* WeaponImage = WidgetTree->FindWidget<UImage>(WidgetName);
		if (!IsValid(WeaponImage))
		{
			continue;
		}
		FWeaponTableRow* WeaponData = GetTableRowForSegment(i);
		
		if (WeaponData == nullptr)
		{
			WeaponImage->SetOpacity(0.0f);

		}
		else
		{
			WeaponImage->SetOpacity(1.0f);
			WeaponImage->SetBrushFromTexture(WeaponData->WeaponItemDescription.Icon);

		}
	}

}

void UWeaponWheelWidget::SelectSegment()
{
	BackgroundMaterial->SetScalarParameterValue(FName("Index"), CurrentsSegmentIndex);
	FWeaponTableRow* WeaponData = GetTableRowForSegment(CurrentsSegmentIndex);
	if (WeaponData == nullptr)
	{
		WeaponNameText->SetVisibility(ESlateVisibility::Hidden);

	}
	else
	{
		WeaponNameText->SetVisibility(ESlateVisibility::Visible);
		WeaponNameText->SetText(WeaponData->WeaponItemDescription.Name);

	}
}

FWeaponTableRow* UWeaponWheelWidget::GetTableRowForSegment(int32 SegmentIndex) const
{
	const EEquipmentSlots& SegmentSlot = EquipmentSlotSegments[SegmentIndex];
	AEquipableItem* EquipableItem = LinckedEqupmentComponent->GetItems()[(int32)SegmentSlot];
	if (!IsValid(EquipableItem))
	{
		return nullptr;

	}
	return GCDataTableUtils::FindWeaponData(EquipableItem->GetDataTableID());
}
