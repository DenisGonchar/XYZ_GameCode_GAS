// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbility_CharacterSprint.h"
#include "Pawns/Character/GCBaseCharacter.h"
#include "Pawns/Character/CharacterMovementComponent/GCBaseCharacterMovementComponent.h"

bool UGameplayAbility_CharacterSprint::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                          const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
                                                          const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;	
	}
	
	return ActorInfo->AvatarActor->IsA<AGCBaseCharacter>();
	
}

void UGameplayAbility_CharacterSprint::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AGCBaseCharacter* Character = StaticCast<AGCBaseCharacter*>(ActorInfo->AvatarActor.Get());
	Character->GetBaseCharacterMovementComponent()->StartSprint();
	
}

void UGameplayAbility_CharacterSprint::CancelAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);

	AGCBaseCharacter* Character = StaticCast<AGCBaseCharacter*>(ActorInfo->AvatarActor.Get());
	Character->GetBaseCharacterMovementComponent()->StopSprint();
	
	
}
