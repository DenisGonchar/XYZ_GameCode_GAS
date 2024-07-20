// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_AttachEquippedItem.h"
#include "../../GCBaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"

void UAnimNotify_AttachEquippedItem::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	AGCBaseCharacter* CharacterOwner = Cast<AGCBaseCharacter>(MeshComp->GetOwner());
	if (!IsValid(CharacterOwner))
	{
		return;
	}

	CharacterOwner-> GetCharacterEquipmentComponent_Muteble()->AttachCurrentItemToEquippedSocket();

}
