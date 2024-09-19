// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystemComponent/Abilities/GameplayAbility_Life.h"

#include "Abilities/Tasks/AbilityTask_WaitAttributeChange.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Pawns/Character/GCBaseCharacter.h"
#include "Widgets/Text/ISlateEditableTextWidget.h"

bool UGameplayAbility_Life::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                               const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
                                               const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	// if (ActorInfo->AvatarActor->IsA<AGCBaseCharacter>())
	// {
	// 	return false;
	// }
	//
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UGameplayAbility_Life::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                            const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                            const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	UAbilityTask_WaitAttributeChange* OnHealthChangedTask = UAbilityTask_WaitAttributeChange::WaitForAttributeChangeWithComparison(this, HealthAttribute, FGameplayTag(), FGameplayTag(), EWaitAttributeChangeComparison::ExactlyEqualTo, 0.0f);
	OnHealthChangedTask->OnChange.AddDynamic(this, &UGameplayAbility_Life::OnHealthElapsed);
	OnHealthChangedTask->Activate();
	
}

void UGameplayAbility_Life::CancelAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateCancelAbility)
{
	UCharacterMovementComponent* CharacterMovement = Cast<UCharacterMovementComponent>(ActorInfo->MovementComponent.Get());
	if (IsValid(CharacterMovement))
	{
		CharacterMovement->DisableMovement();
	}

	float AnimDuration = 0.0f;
	if (ActorInfo->AnimInstance.IsValid())
	{
		AnimDuration = ActorInfo->AnimInstance->Montage_Play(OnDeathAnimMontage, 1.0f, EMontagePlayReturnType::Duration);
	}

	if (AnimDuration == 0.0f && ActorInfo->SkeletalMeshComponent.IsValid())
	{
		ActorInfo->SkeletalMeshComponent->SetCollisionProfileName(CollisionProfileRagdoll);
		ActorInfo->SkeletalMeshComponent->SetSimulatePhysics(true);
	}
	
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UGameplayAbility_Life::OnHealthElapsed()
{

	K2_CancelAbility();
}
