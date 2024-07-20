// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawns/Character/CharacterAnimInstance/Notifies/AnimNotify_LaunchThrowable.h"
#include "../../GCBaseCharacter.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"

void UAnimNotify_LaunchThrowable::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	AGCBaseCharacter* CharacterOwner = Cast<AGCBaseCharacter>(MeshComp->GetOwner());
	if (!IsValid(CharacterOwner))
	{
		return;
	}

	CharacterOwner->GetCharacterEquipmentComponent_Muteble()->LaunchCurrentThrowableItem();

}
