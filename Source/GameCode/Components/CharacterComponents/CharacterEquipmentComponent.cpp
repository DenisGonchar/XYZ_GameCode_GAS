// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include <Pawns/Character/GCBaseCharacter.h>
#include <Actors/Equipment/Weapons/RangeWeaponItem.h>
#include "CharacterInventoryComponent.h"
#include "Actors/Equipment/Throwables/ThrowableItem.h"
#include "Actors/Equipment/Weapons/MeleeWeaponItem.h"
#include "GameCodeTypes.h"
#include "Inventory/Items/Equipables/InventoryAmmoItem.h"
#include "Net/UnrealNetwork.h"
#include "UI/Widget/Equipment/EquipmentViewWidget.h"
#include "UI/Widget/Equipment/WeaponWheelWidget.h"

UCharacterEquipmentComponent::UCharacterEquipmentComponent()
{
	SetIsReplicatedByDefault(true);
}

void UCharacterEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCharacterEquipmentComponent, CurrentEquippedSlot);
	DOREPLIFETIME(UCharacterEquipmentComponent, AmunitionArray);
	DOREPLIFETIME(UCharacterEquipmentComponent, ItemsArray);
	
}

void UCharacterEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();

	checkf(GetOwner()->IsA<AGCBaseCharacter>(), TEXT("UCharacterEquipmentComponent::BeginPlay() CharacterEquipmentComponent can be used only with a baseCharacter"));
	CachedBaseCharacter = StaticCast<AGCBaseCharacter*>(GetOwner());

	CreateLoadout();
	AutoEquip();

}
EEquipableItemType UCharacterEquipmentComponent::GetCurrentEquipperItemType() const
{
	EEquipableItemType Result = EEquipableItemType::None;
	if (IsValid(CurrentEquippedItem))
	{
		Result = CurrentEquippedItem->GetItemType();
	}


	return Result;
}

ARangeWeaponItem* UCharacterEquipmentComponent::GetCurrentRangeWeapon() const
{
	return CurrentEquippendWeapon;
}

AThrowableItem* UCharacterEquipmentComponent::GetCurrentThrowableWeapon() const
{
	return CurrentThrowableItem;
}

AMeleeWeaponItem* UCharacterEquipmentComponent::GetCurrentMeleeWeapon() const
{
	return CurrentMeleeWeapon;
}

bool UCharacterEquipmentComponent::IsEquipping() const
{
	return bIsEquipping;
}

void UCharacterEquipmentComponent::ReloadCurrentWeapon()
{
	check(IsValid(CurrentEquippendWeapon))
	int32 AvailableAmunition = GetAvailadleAmunitionForCurrentWeapon();
	if (AvailableAmunition <= 0)
	{
		return;
	}

	CurrentEquippendWeapon->StartReload();

}

void UCharacterEquipmentComponent::EquipItemInSlot(EEquipmentSlots Slot)
{
	if (bIsEquipping)
	{
		return;
	}

	UnEquipCurrentItem();
	
	CurrentEquippedItem = ItemsArray[(uint32)Slot];
	CurrentEquippendWeapon = Cast<ARangeWeaponItem>(CurrentEquippedItem);
	CurrentThrowableItem = Cast<AThrowableItem>(CurrentEquippedItem);
	CurrentMeleeWeapon = Cast<AMeleeWeaponItem>(CurrentEquippedItem);

	if (IsValid(CurrentEquippedItem))
	{
		if (!IsValid(CurrentThrowableItem))
		{
		
			UAnimMontage* EquipMontage = CurrentEquippedItem->GetCharacterEquipAnimMontage();
			if (IsValid(EquipMontage))
			{
				bIsEquipping = true;
				float EquipDuretion = CachedBaseCharacter->PlayAnimMontage(EquipMontage);
				GetWorld()->GetTimerManager().SetTimer(EquipTimer, this, &UCharacterEquipmentComponent::EquipAnimationFinished, EquipDuretion, false);

			}
			else
			{
				AttachCurrentItemToEquippedSocket();

			}
		}
		CurrentEquippedItem->Equip();
		
	}
	
	if (IsValid(CurrentThrowableItem) && CurrentThrowableItem->CanThrow())
	{
		UAnimMontage* EquipMontage = CurrentEquippedItem->GetCharacterEquipAnimMontage();
		if (IsValid(EquipMontage))
		{
			bIsEquipping = true;
			float EquipDuretion = CachedBaseCharacter->PlayAnimMontage(EquipMontage);
			GetWorld()->GetTimerManager().SetTimer(EquipTimer, this, &UCharacterEquipmentComponent::EquipAnimationFinished, EquipDuretion, false);

		}
		else
		{
			AttachCurrentItemToEquippedSocket();

		}

		CurrentThrowableItem->OnThrowAmmoChanged.AddUFunction(this, FName("OnCurrentThrowItemAmmoChanged"));
		OnCurrentThrowItemAmmoChanged(CurrentThrowableItem->GetThrowAmmo());


	}

	if (IsValid(CurrentEquippendWeapon))
	{
		OnCurrentWeaponAmmoChangedHandled = CurrentEquippendWeapon->OnAmmoChanged.AddUFunction(this, FName("OnCurrentWeaponAmmoChanged"));
		OnCurrentWeaponReloadHandled = CurrentEquippendWeapon->OnReloadComplete.AddUFunction(this, FName("OnWeaponReloadComplete"));
		OnCurrentWeaponAmmoChanged(CurrentEquippendWeapon->GetAmmo());
	}

	if (OnEquippedItemChanged.IsBound())
	{
		OnEquippedItemChanged.Broadcast(CurrentEquippedItem);
	}

	CurrentEquippedSlot = Slot;

	if (GetOwner()->GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_EquipItemInSlot(CurrentEquippedSlot);
	}

}

EEquipmentSlots UCharacterEquipmentComponent::GetCurrentEquippedSlot()
{
	return CurrentEquippedSlot;
}

void UCharacterEquipmentComponent::SwitchAmmoSlot()
{
	
	if (bIsEquipping)
	{
		return;
	}
	
	if (IsValid(CurrentEquippendWeapon))
	{
		CurrentEquippendWeapon->StopFire();
		CurrentEquippendWeapon->EndReload(false);
	
	}
	
	CurrentEquippendWeapon->SwitchAmmoType();
	

	if (OnEquippedItemChanged.IsBound())
	{
		OnEquippedItemChanged.Broadcast(CurrentEquippedItem);
	}
	
}

void UCharacterEquipmentComponent::AttachCurrentItemToEquippedSocket()
{
	if (IsValid(CurrentEquippedItem))
	{

		CurrentEquippedItem->AttachToComponent(CachedBaseCharacter->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, CurrentEquippedItem->GetEquippedSocketName());

	}

}

void UCharacterEquipmentComponent::LaunchCurrentThrowableItem()
{
	if (GetOwner()->HasAuthority() || GetOwner()->GetLocalRole() == ROLE_AutonomousProxy)
	{
		if (CurrentThrowableItem)
		{
			CurrentThrowableItem->Throw();

			bIsEquipping = false;
			EquipItemInSlot(PreviousEquippedSlot);
		}
	}

}

void UCharacterEquipmentComponent::UnEquipCurrentItem()
{
	if (IsValid(CurrentEquippedItem))
	{
		CurrentEquippedItem->AttachToComponent(CachedBaseCharacter->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, CurrentEquippedItem->GetUnEquippedSocketName());
		CurrentEquippedItem->UnEquip();

	}
	if (IsValid(CurrentEquippendWeapon))
	{
		CurrentEquippendWeapon->StopFire();
		CurrentEquippendWeapon->EndReload(false);
		CurrentEquippendWeapon->OnAmmoChanged.Remove(OnCurrentWeaponAmmoChangedHandled);
		CurrentEquippendWeapon->OnReloadComplete.Remove(OnCurrentWeaponReloadHandled);

	}

	PreviousEquippedSlot = CurrentEquippedSlot;

	CurrentEquippedSlot = EEquipmentSlots::None;

}

void UCharacterEquipmentComponent::EquipNextItem()
{
	//TODO Weapon wheel widget
	/*if (CachedBaseCharacter->IsPlayerControlled())
	{	
		if (IsSelectingWeapon())
		{
			WeaponWheelWidget->NextSegment();
		}
		else
		{
			APlayerController* PlayerController = CachedBaseCharacter->GetController<APlayerController>();
			OpenWeaponWheel(PlayerController);
		}
		return;
	}*/

	uint32 CurrentSlotIndex = (uint32)CurrentEquippedSlot;
	uint32 NextSlotIndex = NextItemsArraySlotIndex(CurrentSlotIndex);
	
	while (CurrentSlotIndex != NextSlotIndex && (IgnoreSlotsWhileSwitching.Contains((EEquipmentSlots)NextSlotIndex) || !IsValid(ItemsArray[NextSlotIndex])))
	{
		NextSlotIndex = NextItemsArraySlotIndex(NextSlotIndex);

	}
	if (CurrentSlotIndex != NextSlotIndex)
	{
		EquipItemInSlot((EEquipmentSlots) NextSlotIndex);

	}

}

void UCharacterEquipmentComponent::EquipPreviousItem()
{
	/*if (CachedBaseCharacter->IsPlayerControlled())
	{
		if (IsSelectingWeapon())
		{
			WeaponWheelWidget->PreviousSegment();
		}
		else
		{
			APlayerController* PlayerController = CachedBaseCharacter->GetController<APlayerController>();
			OpenWeaponWheel(PlayerController);
		}
		return;
	}*/


	uint32 CurrentSlotIndex = (uint32)CurrentEquippedSlot;
	uint32 PreviousSlotIndex = PreviousItemsArraySlotIndex(CurrentSlotIndex);

	while (CurrentSlotIndex != PreviousSlotIndex && (IgnoreSlotsWhileSwitching.Contains((EEquipmentSlots)PreviousSlotIndex) || !IsValid(ItemsArray[PreviousSlotIndex])))
	{
		PreviousSlotIndex = PreviousItemsArraySlotIndex(PreviousSlotIndex);

	}
	if (CurrentSlotIndex != PreviousSlotIndex)
	{
		EquipItemInSlot((EEquipmentSlots)PreviousSlotIndex);

	}
}

bool UCharacterEquipmentComponent::AddEquipmentItemToSlot(const TSubclassOf<AEquipableItem> EquipableItemClass, int32 SlotIndex)
{
	if (!IsValid(EquipableItemClass))
	{
		return false;
	}

	AEquipableItem* DefaultItemObject = EquipableItemClass->GetDefaultObject<AEquipableItem>();
	if (!DefaultItemObject->IsSlotCompatible((EEquipmentSlots)SlotIndex))
	{
		return false;
	}

	if (!IsValid(ItemsArray[SlotIndex]))
	{
		AEquipableItem* Item = GetWorld()->SpawnActor<AEquipableItem>(EquipableItemClass);
		Item->AttachToComponent(CachedBaseCharacter->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, Item->GetUnEquippedSocketName());
		Item->SetOwner(CachedBaseCharacter.Get());
		Item->UnEquip();
		ItemsArray[SlotIndex] = Item;

		if (Item->GetItemType() == EEquipableItemType::Throwable)
		{
			AThrowableItem* ThrowItem = Cast<AThrowableItem>(Item);
			OnCurrentThrowItemAmmoChanged(ThrowItem->GetMaxThrowAmmo());
		}

	}
	else if (DefaultItemObject->IsA<ARangeWeaponItem>())
	{
		ARangeWeaponItem* RangeWeaponObject = StaticCast<ARangeWeaponItem*>(DefaultItemObject);
		int32 AmmoSlotIndex = (int32)RangeWeaponObject->GetDefaultAmmoType();
		int32 MaxAmmo = RangeWeaponObject->GetDefaultMaxAmmo();
		AmunitionArray[AmmoSlotIndex] += MaxAmmo;
		OnCurrentWeaponAmmoChanged(CurrentEquippendWeapon->GetAmmo());
	}

	

	return true;
}

void UCharacterEquipmentComponent::AddAmmunitionAmmo(EAmmunitionType AmmunitionType, int32 Count)
{
	int32 AmmoSlotIndex = (int32)AmmunitionType;
	AmunitionArray[AmmoSlotIndex] += Count;
	OnCurrentWeaponAmmoChanged(CurrentEquippendWeapon->GetAmmo());
	
}

void UCharacterEquipmentComponent::RemoveAmmunitionAmmo(EAmmunitionType AmmunitionType, int32 Count)
{
	int32 AmmoSlotIndex = (int32)AmmunitionType;
	AmunitionArray[AmmoSlotIndex] -= Count;
	OnCurrentWeaponAmmoChanged(CurrentEquippendWeapon->GetAmmo());
	
}

void UCharacterEquipmentComponent::RemoveItemFromSlot(int32 SlotIndex)
{
	if ((uint32)CurrentEquippedSlot == SlotIndex)
	{
		UnEquipCurrentItem();
	}
	ItemsArray[SlotIndex]->Destroy();
	ItemsArray[SlotIndex] = nullptr;

}

void UCharacterEquipmentComponent::OpenViewEquipment(APlayerController* PlayerController)
{
	if (!IsValid(ViewWidget))
	{
		CreateEquipmentWidgets(PlayerController);
	}
	
	if (!ViewWidget->IsVisible())
	{
		ViewWidget->AddToViewport();
	}
}

void UCharacterEquipmentComponent::CloseViewEquipment()
{
	if (ViewWidget->IsVisible())
	{
		ViewWidget->RemoveFromParent();
	}
}

bool UCharacterEquipmentComponent::IsViewVisible() const
{
	bool Result = false;
	if (IsValid(ViewWidget))
	{
		Result = ViewWidget->IsVisible();
	}
	return Result;

}

void UCharacterEquipmentComponent::OpenWeaponWheel(APlayerController* PlayerController)
{
	if (!IsValid(WeaponWheelWidget))
	{
		CreateEquipmentWidgets(PlayerController);
	}

	if (!WeaponWheelWidget->IsVisible())
	{
		WeaponWheelWidget->AddToViewport();
	}
}

bool UCharacterEquipmentComponent::IsSelectingWeapon() const
{
	if (IsValid(WeaponWheelWidget))
	{
		return WeaponWheelWidget->IsVisible();
	}
	return false;

}

void UCharacterEquipmentComponent::ConfirmWeaponSelection() const
{
	WeaponWheelWidget->ConfirmSelection();
}

// void UCharacterEquipmentComponent::OnLevelDeserialized_Implementation()
// {
// 	EquipItemInSlot(CurrentEquippedSlot);
// }

const TArray<AEquipableItem*>& UCharacterEquipmentComponent::GetItems() const
{
	return ItemsArray;
}

void UCharacterEquipmentComponent::CreateEquipmentWidgets(APlayerController* PlayerController)
{
	checkf(IsValid(ViewWidgetClass), TEXT("UCharacterEquipmentComponent::CreateViewWidget view widget class is not defined"));

	if (!IsValid(PlayerController))
	{
		return;
	}

	ViewWidget = CreateWidget<UEquipmentViewWidget>(PlayerController, ViewWidgetClass);
	ViewWidget->InitializeEquipmentWidget(this);

	WeaponWheelWidget = CreateWidget<UWeaponWheelWidget>(PlayerController, WeaponWheelClass);
	WeaponWheelWidget->InitializeWeaponWheelWidget(this);
}


void UCharacterEquipmentComponent::Server_EquipItemInSlot_Implementation(EEquipmentSlots Slot)
{
	EquipItemInSlot(Slot);
}

void UCharacterEquipmentComponent::CreateLoadout()
{	

	if (GetOwner()->GetLocalRole() < ROLE_Authority)
	{
		return;
	}

	AmunitionArray.AddZeroed((uint32)EAmmunitionType::MAX);
	for (const TPair<EAmmunitionType, int32>& AmmoPair : MaxAmunitionAmount)
	{
		AmunitionArray[(uint32)AmmoPair.Key] = FMath::Max(AmmoPair.Value, 0);

	}

	ItemsArray.AddZeroed((uint32)EEquipmentSlots::MAX);
	for (const TPair<EEquipmentSlots, TSubclassOf<AEquipableItem>>& ItemPair : ItemsLoadout)
	{
		if (!IsValid(ItemPair.Value))
		{
			continue;
		}

		AddEquipmentItemToSlot(ItemPair.Value, (int32)ItemPair.Key);
		
	}

}

void UCharacterEquipmentComponent::AutoEquip()
{
	if (AutoItemInSlot != EEquipmentSlots::None)
	{
		EquipItemInSlot(AutoItemInSlot);
	}
}

void UCharacterEquipmentComponent::EquipAnimationFinished()
{
	bIsEquipping = false;
	AttachCurrentItemToEquippedSocket();
	
}

int32 UCharacterEquipmentComponent::NextItemsArraySlotIndex(uint32 CurrentSlotIndex)
{
	if (CurrentSlotIndex == ItemsArray.Num() - 1)
	{
		return 0;
	}
	else
	{
		return CurrentSlotIndex + 1;
	}
}

int32 UCharacterEquipmentComponent::PreviousItemsArraySlotIndex(uint32 CurrentSlotIndex)
{
	if (CurrentSlotIndex == 0)
	{
		return ItemsArray.Num() - 1;
	}
	else
	{
		return CurrentSlotIndex - 1;
	}
}

int32 UCharacterEquipmentComponent::GetAvailadleAmunitionForCurrentWeapon()
{
	check(GetCurrentRangeWeapon());

	return AmunitionArray[(uint32)GetCurrentRangeWeapon()->GetAmmoType()];

}

void UCharacterEquipmentComponent::OnWeaponReloadComplete()
{
	ReloadAmmoInCurrentWeapon();
}

void UCharacterEquipmentComponent::ReloadAmmoInCurrentWeapon(int32 NumberOfAmmo /*= 0*/, bool bChecksFull /*= false*/)
{
	int32 AvailableAmunition = GetAvailadleAmunitionForCurrentWeapon();
	int32 CurrentAmmo = CurrentEquippendWeapon->GetAmmo();
	int32 AmmoToReload = CurrentEquippendWeapon->GetCurrentMaxAmmo() - CurrentAmmo;
	int32 ReloadedAmmo = FMath::Min(AvailableAmunition, AmmoToReload);

	if (NumberOfAmmo > 0)
	{

		ReloadedAmmo = FMath::Min(ReloadedAmmo, NumberOfAmmo);

	}

	AmunitionArray[(uint32)CurrentEquippendWeapon->GetAmmoType()] -= ReloadedAmmo;
	CurrentEquippendWeapon->SetAmmo(ReloadedAmmo + CurrentAmmo);

	UCharacterInventoryComponent* Inventrory = CachedBaseCharacter->GetCharacterInventoryComponent_Muteble();
	if (Inventrory)
	{
		float LeftReloadAmmo = 0.0f;
		do
		{
			FInventorySlot* Item = Inventrory->FindItemSlot(CurrentEquippendWeapon->GetDataTableAmmoID());
			if (!Item)
			{
				break;
			}

			UInventoryAmmoItem* Ammo = Cast<UInventoryAmmoItem>(Item->Item);
			if (Ammo && Ammo->GetAmmunitionType() == CurrentEquippendWeapon->GetAmmoType())
			{
				Item->Count -= ReloadedAmmo;
				ReloadedAmmo = Item->Count;
				LeftReloadAmmo = ReloadedAmmo;
				
				Item->UpdateCount();
				
				if (ReloadedAmmo <= 0)
				{
					Item->ClearSlot();
				}
				ReloadedAmmo = FMath::Abs(ReloadedAmmo);
			}
			else
			{
				break;
			}
			
		}
		while (LeftReloadAmmo < 0);
		
		
	}

	if (bChecksFull)
	{
		AvailableAmunition = AmunitionArray[(uint32)CurrentEquippendWeapon->GetAmmoType()];
		bool bIsFullyReoaded = CurrentEquippendWeapon->GetAmmo() == CurrentEquippendWeapon->GetCurrentMaxAmmo();
		if (AvailableAmunition == 0 || bIsFullyReoaded)
		{
			CurrentEquippendWeapon->EndReload(true);

		}

	}
}

void UCharacterEquipmentComponent::OnCurrentWeaponAmmoChanged(int32 Ammo)
{
	if (OnCurrentWeaponAmmoChangedEvent.IsBound())
	{
		OnCurrentWeaponAmmoChangedEvent.Broadcast(Ammo, GetAvailadleAmunitionForCurrentWeapon());
	}
}

void UCharacterEquipmentComponent::OnCurrentThrowItemAmmoChanged(int32 Ammo)
{
	if (OnCurrentThrowItemAmmoChangedEvent.IsBound())
	{
		OnCurrentThrowItemAmmoChangedEvent.Broadcast(Ammo);
	}
}

void UCharacterEquipmentComponent::OnRep_CurrentEquippedSlot(EEquipmentSlots CurrentEquipSlot_Old)
{
	EquipItemInSlot(CurrentEquippedSlot);
}

void UCharacterEquipmentComponent::OnRep_ItemsArray()
{
	for (AEquipableItem* Item : ItemsArray)
	{
		if (IsValid(Item))
		{
			Item->UnEquip();
		}

	}
}

