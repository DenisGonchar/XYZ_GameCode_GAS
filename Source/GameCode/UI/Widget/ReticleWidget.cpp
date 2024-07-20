// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/ReticleWidget.h"
#include "Actors/Equipment/EquipableItem.h"
#include "Blueprint/WidgetTree.h"
#include "BowstringWidget.h"

void UReticleWidget::OnAimingStateChanged_Implementation(bool bIsAiming)
{
	SetupCurrentReticle();

}

void UReticleWidget::OnEquippedItemChanged_Implementation(AEquipableItem* EquippedItem)
{
	CurrentEquippedItem = EquippedItem;
	if (CurrentEquippedItem.IsValid())
	{
		CurrentTypeEquipableItem = CurrentEquippedItem->GetItemType();
	}
	
	SetupCurrentReticle();

	BowWeaponItem = Cast<ABowWeaponItem>(EquippedItem);
	if (BowWeaponItem)
	{
		UBowstringWidget* BowstringWidget = GetBowstringWidget();
		if (BowstringWidget)
		{
			BowWeaponItem->OnBowstringChangedEvent.AddUObject(BowstringWidget, &UBowstringWidget::SetBowstring);
		}
	}
}

void UReticleWidget::OnRotationAngleChanged_Implementation(FRotator NewRotatorAngle)
{
	RotationAngle = NewRotatorAngle;
}

UBowstringWidget* UReticleWidget::GetBowstringWidget()
{
	return WidgetTree->FindWidget<UBowstringWidget>(BowstringWidgetName);
	 
}

void UReticleWidget::SetupCurrentReticle()
{
	CurrentReticle = CurrentEquippedItem.IsValid() ? CurrentEquippedItem->GetReticleType() : EReticleType::None;


}
