// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawns/Character/CharacterAnimInstance/Notifies/AnimNotify_StopSlide.h"
#include "../../GCBaseCharacter.h"
#include "../../CharacterMovementComponent/GCBaseCharacterMovementComponent.h"

void UAnimNotify_StopSlide::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	AGCBaseCharacter* CharacterOwner = Cast<AGCBaseCharacter>(MeshComp->GetOwner());
	if (!IsValid(CharacterOwner))
	{
		return;
	}

	CharacterOwner->GetBaseCharacterMovementComponent()->StopSlide();
}
