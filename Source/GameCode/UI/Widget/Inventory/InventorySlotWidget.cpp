// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Inventory/InventorySlotWidget.h"

#include "Actors/Interactive/Pickables/PickableAmmo.h"
#include "Actors/Interactive/Pickables/PickableWeapon.h"
#include "Components/CharacterComponents/CharacterInventoryComponent.h"
#include "Inventory/Items/InventoryItem.h"
#include "Pawns/Character/GCBaseCharacter.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "Inventory/Items/Equipables/InventoryAmmoItem.h"
#include "Inventory/Items/Equipables/WeaponInventoryItem.h"
#include "Utils/GCDataTableUtils.h"

void UInventorySlotWidget::InitializeItemSlot(FInventorySlot& InventorySlot)
{
	LinkedSlot = &InventorySlot;

	FInventorySlot::FInventorySlotUpdate OnInventorySlotUpdate;
	OnInventorySlotUpdate.BindUObject(this, &UInventorySlotWidget::UpdateView);
	LinkedSlot->BindOnInventorySlotUpdate(OnInventorySlotUpdate);

	FInventorySlot::FInventoryCountUpdate OnInventoryCountUpdate;
	OnInventoryCountUpdate.BindUObject(this, &UInventorySlotWidget::UpdateCount);
	LinkedSlot->BindOnInventoryCountUpdate(OnInventoryCountUpdate);
}

void UInventorySlotWidget::UpdateView()
{
	if (LinkedSlot == nullptr)
	{
		ImageItemIcon->SetBrushFromTexture(nullptr);
		return;
	}

	if (LinkedSlot->Item.IsValid())
	{
		const FInventoryItemDescription& Description = LinkedSlot->Item->GetDescription();
		ImageItemIcon->SetBrushFromTexture(Description.Icon);
		AmmoCount = LinkedSlot->Count;
		bIsVisibilityAmmo = LinkedSlot->bIsVisibility;
	}
	else
	{
		ImageItemIcon->SetBrushFromTexture(nullptr);
	}

}

void UInventorySlotWidget::SetItemIcon(UTexture2D* Icon)
{
	ImageItemIcon->SetBrushFromTexture(Icon);
}

void UInventorySlotWidget::UpdateCount(int32 NewCount)
{
	AmmoCount = NewCount;
}

FReply UInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (LinkedSlot == nullptr)
	{
		return FReply::Handled();
	}

	if (!LinkedSlot->Item.IsValid())
	{
		return FReply::Handled();
	}

	FKey MouseBtn = InMouseEvent.GetEffectingButton();
	if (MouseBtn == EKeys::RightMouseButton)
	{
		/* Some simplification, so as not to complicate the architecture
		 * - on instancing item, we use the current pawn as an outer one.
		 * In real practice we need use callback for inform item holder what action was do in UI */

		TWeakObjectPtr<UInventoryItem> LinkedSlotItem = LinkedSlot->Item;
		AGCBaseCharacter* ItemOwner = Cast<AGCBaseCharacter>(LinkedSlotItem->GetOuter());

		if (LinkedSlotItem->Consume(ItemOwner))
		{
			LinkedSlot->ClearSlot();
		}
		return FReply::Handled();
	}

	FEventReply Reply = UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton);
	return Reply.NativeReply;

}

void UInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	UDragDropOperation* DragOperation = Cast<UDragDropOperation>(UWidgetBlueprintLibrary::CreateDragDropOperation(UDragDropOperation::StaticClass()));

	/* Some simplification for not define new widget for drag and drop operation  */
	UInventorySlotWidget* DragWidget = CreateWidget<UInventorySlotWidget>(GetOwningPlayer(), GetClass());
	DragWidget->ImageItemIcon->SetBrushFromTexture(LinkedSlot->Item->GetDescription().Icon);
	DragWidget->AmmoCount = LinkedSlot->Count;
	DragWidget->bIsVisibilityAmmo = LinkedSlot->bIsVisibility;
	
	DragOperation->DefaultDragVisual = DragWidget;
	DragOperation->Pivot = EDragPivot::MouseDown;
	DragOperation->Payload = LinkedSlot->Item.Get();
	OutOperation = DragOperation;

	LinkedSlot->ClearSlot();

}

bool UInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (!LinkedSlot->Item.IsValid())
	{
		UInventorySlotWidget* OldWidget = Cast<UInventorySlotWidget>(InOperation->DefaultDragVisual);
		if (OldWidget)
		{
			LinkedSlot->bIsVisibility = OldWidget->bIsVisibilityAmmo;
			LinkedSlot->Count = OldWidget->AmmoCount;
			
		}
		LinkedSlot->Item = TWeakObjectPtr<UInventoryItem>(Cast<UInventoryItem>(InOperation->Payload));
		LinkedSlot->UpdateSlotState();
		return true;
	}

	return false;

}

void UInventorySlotWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UInventoryItem* Item = Cast<UInventoryItem>(InOperation->Payload);
	if (!Item)
	{
		LinkedSlot->Item = TWeakObjectPtr<UInventoryItem>(Cast<UInventoryItem>(InOperation->Payload));
		LinkedSlot->UpdateSlotState();
	}

	TSubclassOf<APickableItem> DefaultPickableItem = nullptr;
	
	FWeaponTableRow* WeaponDataRow = GCDataTableUtils::FindWeaponData(Item->GetDataTableID());
	if (WeaponDataRow)
	{
		DefaultPickableItem = WeaponDataRow->PickableActor;
	}

	FItemTableRow* ItemTableRow = GCDataTableUtils::FindInventoryItemData(Item->GetDataTableID());
	if (ItemTableRow)
	{
		DefaultPickableItem = ItemTableRow->PickableActorClass;
	}

	bool bIsAmmo = false;
	FAmmoTableRow* AmmoTableRow = GCDataTableUtils::FindAmmoItemData(Item->GetDataTableID());
	if (AmmoTableRow)
	{
		DefaultPickableItem = AmmoTableRow->PickableActorClass;

		UInventorySlotWidget* OldWidget = Cast<UInventorySlotWidget>(InOperation->DefaultDragVisual);
		if (OldWidget)
		{
			UInventoryAmmoItem* AmmoItem = Cast<UInventoryAmmoItem>(InOperation->Payload);
			if (AmmoItem)
			{
				AGCBaseCharacter* Character = Cast<AGCBaseCharacter>(GetOwningPlayerPawn());
				AmmoItem->SetAmmoCount(OldWidget->AmmoCount);
				AmmoItem->RemoveAmmunition(Character);

				FVector ForwardLocation = GetOwningPlayerPawn()->GetActorLocation() + GetOwningPlayerPawn()->GetActorForwardVector() * 100;
				APickableAmmo* PickableAmmo = GetWorld()->SpawnActor<APickableAmmo>(DefaultPickableItem, ForwardLocation, FRotator::ZeroRotator);
				if (PickableAmmo)
				{
					PickableAmmo->SetAmmoCount(AmmoItem->GetAmmoCount());
				}
				bIsAmmo = true;
			}
		}
	}

	if (DefaultPickableItem == nullptr)
	{
		LinkedSlot->Item = TWeakObjectPtr<UInventoryItem>(Cast<UInventoryItem>(InOperation->Payload));
		LinkedSlot->UpdateSlotState();	
	}
	else if (DefaultPickableItem && !bIsAmmo)
	{
		FVector ForwardLocation = GetOwningPlayerPawn()->GetActorLocation() + GetOwningPlayerPawn()->GetActorForwardVector() * 100;
		APickableItem* PickableItem = GetWorld()->SpawnActor<APickableItem>(DefaultPickableItem, ForwardLocation, FRotator::ZeroRotator);
	}


}
