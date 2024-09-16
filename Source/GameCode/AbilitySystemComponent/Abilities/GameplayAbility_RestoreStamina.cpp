// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystemComponent/Abilities/GameplayAbility_RestoreStamina.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitAttributeChange.h"
#include "AbilitySystemComponent/AttributeSets/GCCharacterAttributeSet.h"
#include "Pawns/Character/CharacterMovementComponent/GCBaseCharacterMovementComponent.h"
#include "Pawns/Character/GCBaseCharacter.h"

bool UGameplayAbility_RestoreStamina::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                         const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
                                                         const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo->MovementComponent.IsValid() || !ActorInfo->MovementComponent->IsA<UGCBaseCharacterMovementComponent>())
	{
		return false;
	}
	
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);

}

void UGameplayAbility_RestoreStamina::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	Character = StaticCast<AGCBaseCharacter*>(ActorInfo->AvatarActor.Get());
	MovementComponent = StaticCast<UGCBaseCharacterMovementComponent*>(ActorInfo->MovementComponent.Get());

	UAbilityTask_WaitAttributeChange* AttributeChangeTask = UAbilityTask_WaitAttributeChange::WaitForAttributeChange(this, StaminaAttribute, FGameplayTag(), FGameplayTag(), false);
	AttributeChangeTask->OnChange.AddDynamic(this, &UGameplayAbility_RestoreStamina::OnStaminaChanged);
	AttributeChangeTask->Activate();
}

void UGameplayAbility_RestoreStamina::OnStaminaChanged()
{
	const UGCCharacterAttributeSet* AttributeSet = Character->GetGCCharacterAttributeSet();
	float Stamina = AttributeSet->Stamina.GetCurrentValue();
	float MaxStamina = AttributeSet->MaxStamina.GetCurrentValue();

    UE_LOG(LogTemp, Warning, TEXT("Stamina: %f"), Stamina);

	if (Stamina == 0.0f)
	{
		Character->GetAbilitySystemComponent()->AddLooseGameplayTag(OutOfStaminaTag);
		MovementComponent->SetIsOutOfStamina(true);
	}
	else if (Stamina == MaxStamina && MovementComponent->IsOutOfStamina())
	{
		Character->GetAbilitySystemComponent()->RemoveLooseGameplayTag(OutOfStaminaTag);
		MovementComponent->SetIsOutOfStamina(false);
	}
}
