// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbility_CharacterCrouch.h"

#include "Pawns/Character/GCBaseCharacter.h"

bool UGameplayAbility_CharacterCrouch::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                          const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
                                                          const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;	
	}

	return ActorInfo->AvatarActor->IsA<AGCBaseCharacter>();
	
}

void UGameplayAbility_CharacterCrouch::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AGCBaseCharacter* Character = StaticCast<AGCBaseCharacter*>(ActorInfo->AvatarActor.Get());
	Character->Crouch();
	
}

void UGameplayAbility_CharacterCrouch::CancelAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
	
	AGCBaseCharacter* Character = StaticCast<AGCBaseCharacter*>(ActorInfo->AvatarActor.Get());
	Character->UnCrouch();
	
}
