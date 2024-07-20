#include "Actors/Equipment/EquipableItem.h"
#include "Pawns/Character/GCBaseCharacter.h"


AEquipableItem::AEquipableItem()
{
	SetReplicates(true);
}

void AEquipableItem::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);

	if (IsValid(NewOwner))
	{
		checkf(GetOwner()->IsA<AGCBaseCharacter>(), TEXT("AEquipableItem::SetOwner() only character can be an owner of an equipable item"));
		CachedCharacterOwner = StaticCast<AGCBaseCharacter*>(GetOwner());
		if (GetLocalRole() == ROLE_Authority)
		{
			SetAutonomousProxy(true);
		}

	}
	else
	{
		CachedCharacterOwner = nullptr;
	}
}

EEquipableItemType AEquipableItem::GetItemType() const
{
	return ItemType;
}

UAnimMontage* AEquipableItem::GetCharacterEquipAnimMontage() const
{
	return CharacterEquipAnimMontage;
}

FName AEquipableItem::GetUnEquippedSocketName() const
{
	return UnEquippedSocketName;
}

FName AEquipableItem::GetEquippedSocketName() const
{
	return EquippedSocketName;
}

void AEquipableItem::Equip()
{
	if (OnEquipementStateChanged.IsBound())
	{
		OnEquipementStateChanged.Broadcast(true);
	}


}

void AEquipableItem::UnEquip()
{
	if (OnEquipementStateChanged.IsBound())
	{
		OnEquipementStateChanged.Broadcast(false);
	}
}

EReticleType AEquipableItem::GetReticleType() const
{
	return ReticleType;
}

FName AEquipableItem::GetDataTableID() const
{
	return DataTableID;
}

bool AEquipableItem::IsSlotCompatible(EEquipmentSlots Slot)
{
	return CompatableEquipmentSlots.Contains(Slot);
}

AGCBaseCharacter* AEquipableItem::GetCharacterOwner() const
{
	return CachedCharacterOwner.IsValid() ? CachedCharacterOwner.Get() : nullptr;
}
