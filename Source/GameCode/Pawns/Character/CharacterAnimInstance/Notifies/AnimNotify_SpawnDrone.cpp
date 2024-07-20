// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_SpawnDrone.h"

#include "Pawns/Character/GCBaseCharacter.h"
#include "Widgets/Text/ISlateEditableTextWidget.h"

void UAnimNotify_SpawnDrone::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	AGCBaseCharacter* CharacterOwner = Cast<AGCBaseCharacter>(MeshComp->GetOwner());
	if (!IsValid(CharacterOwner))
	{
		return;
	}

	CharacterOwner->SpawnDrone();
	
	
}
