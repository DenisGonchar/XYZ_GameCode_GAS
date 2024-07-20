// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Characters/Modular/GCAIModularCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

#include <Components/LimbComponet/GCCombinedSkeletalMeshComponent.h>
#include "Components/LimbComponet/GCCharacterLimbComponent.h"
#include "Components/CharacterComponents/CharacterAttributeComponent.h"


AGCAIModularCharacter::AGCAIModularCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UGCCombinedSkeletalMeshComponent>(ACharacter::MeshComponentName))
{
	CombinedSkeletalMeshComponent = CastChecked<UGCCombinedSkeletalMeshComponent>(GetMesh());

	CharacterLimbsComponent = CreateDefaultSubobject<UGCCharacterLimbComponent>("CharacterLimdsComponent");
	
}

void AGCAIModularCharacter::BeginPlay()
{
	Super::BeginPlay();


}

UGCCombinedSkeletalMeshComponent* AGCAIModularCharacter::GetCombinedSkeletalMeshComponent() const
{
	return CombinedSkeletalMeshComponent;
}

float AGCAIModularCharacter::InternalTakePointDamage(float Damage, FPointDamageEvent const& PointDamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (CharacterAttributeComponent->GetTypeCharacter() == ETypesCharacter::Modular)
	{
		CharacterLimbsComponent->TakeDamage(Damage, EventInstigator, DamageCauser, PointDamageEvent.HitInfo.GetComponent(), PointDamageEvent.HitInfo.BoneName);

	}	

	return Super::InternalTakePointDamage(Damage, PointDamageEvent, EventInstigator, DamageCauser);

}
