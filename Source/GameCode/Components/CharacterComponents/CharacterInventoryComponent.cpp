// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterInventoryComponent.h"
#include "UI/Widget/Inventory/InventoryViewWidget.h"
#include "Inventory/Items/InventoryItem.h"

void UCharacterInventoryComponent::OpenViewInventory(APlayerController* PlayerController)
{
	if (!IsValid(InventoryViewWidget))
	{
		CreateViewWidget(PlayerController);
		
	}

	if (!InventoryViewWidget->IsVisible())
	{
		InventoryViewWidget->AddToViewport();
	}
}

void UCharacterInventoryComponent::CloseViewInventory()
{
	if (InventoryViewWidget->IsVisible())
	{
		InventoryViewWidget->RemoveFromParent();
	}
}

bool UCharacterInventoryComponent::IsViewVisible() const
{
	bool Result = false;
	if (IsValid(InventoryViewWidget))
	{
		Result = InventoryViewWidget->IsVisible();
	}
	return Result;
}

int32 UCharacterInventoryComponent::GetCapacity() const
{
	return Capacity;
}

bool UCharacterInventoryComponent::HasFreeSlot() const
{
	return ItemsInInventory < Capacity;

}

bool UCharacterInventoryComponent::AddItem(TWeakObjectPtr<UInventoryItem> ItemToAdd, int32 Count)
{
	if (!ItemToAdd.IsValid() || Count < 0)
	{
		return false;
	}

	bool Result = false;
	FInventorySlot* FreeSlot = FindFreeSlot();

	if (FreeSlot != nullptr)
	{
		FreeSlot->Item = ItemToAdd;
		FreeSlot->Count = Count;
		FreeSlot->bIsVisibility = ItemToAdd->GetDescription().bIsAmmoVisibility;
		
		ItemsInInventory++;
		Result = true;
		FreeSlot->UpdateSlotState();

	}

	return Result;

}

bool UCharacterInventoryComponent::RemoveItem(FName ItemID)
{
	FInventorySlot* ItemSlot = FindItemSlot(ItemID);

	if (ItemSlot != nullptr)
	{
		InventorySlots.RemoveAll([=](const FInventorySlot& Slot) {return Slot.Item->GetDataTableID() == ItemID; });
		return true;
	}

	return false;

}

TArray<FInventorySlot> UCharacterInventoryComponent::GetAllItemsCopy() const
{
	return InventorySlots;
}

TArray<FText> UCharacterInventoryComponent::GetAllItemsName() const
{
	TArray<FText> Result;

	for (const FInventorySlot& Slot : InventorySlots)
	{
		if (Slot.Item.IsValid())
		{
			Result.Add(Slot.Item->GetDescription().Name);
		}
	}
	return Result;
}

void UCharacterInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	InventorySlots.AddDefaulted(Capacity);
		
}

void UCharacterInventoryComponent::CreateViewWidget(APlayerController* PlayerController)
{
	if (IsValid(InventoryViewWidget))
	{
		return;
	}

	if (!IsValid(PlayerController) || !IsValid(InventoryViewWidgetClass))
	{
		return;
	}

	InventoryViewWidget	= CreateWidget<UInventoryViewWidget>(PlayerController, InventoryViewWidgetClass);
	InventoryViewWidget->InitializeViewWidget(InventorySlots);

}

FInventorySlot* UCharacterInventoryComponent::FindItemSlot(FName ItemID)
{
	return InventorySlots.FindByPredicate([=](const FInventorySlot& Slot) {return  Slot.Item.IsValid() && Slot.Item->GetDataTableID() == ItemID;});
}

FInventorySlot* UCharacterInventoryComponent::FindFreeSlot()
{
	return InventorySlots.FindByPredicate([=](const FInventorySlot& Slot) {return !Slot.Item.IsValid();});

}

void FInventorySlot::BindOnInventorySlotUpdate(const FInventorySlotUpdate& Callback) const
{
	OnInventorySlotUpdate = Callback;

}

void FInventorySlot::UnbindOnInventorySlotUpdate()
{
	OnInventorySlotUpdate.Unbind();
	
}

void FInventorySlot::UpdateSlotState()
{
	OnInventorySlotUpdate.ExecuteIfBound();
}

void FInventorySlot::BindOnInventoryCountUpdate(const FInventoryCountUpdate& Callback) const
{
	OnInventoryCountUpdate = Callback;
}

void FInventorySlot::UnbindOnInventoryCountUpdate()
{
	OnInventoryCountUpdate.Unbind();
}

void FInventorySlot::UpdateCount()
{
	OnInventoryCountUpdate.ExecuteIfBound(Count);
}


void FInventorySlot::ClearSlot()
{
	Item = nullptr;
	Count = 0;
	bIsVisibility = false;
	UpdateSlotState();

}
