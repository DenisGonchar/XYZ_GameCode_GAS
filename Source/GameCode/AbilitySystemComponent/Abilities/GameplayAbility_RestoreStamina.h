// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayAbility_RestoreStamina.generated.h"

class AGCBaseCharacter;
class UGCBaseCharacterMovementComponent;
/**
 * 
 */
UCLASS()
class GAMECODE_API UGameplayAbility_RestoreStamina : public UGameplayAbility
{
	GENERATED_BODY()

public:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag OutOfStaminaTag;

	UPROPERTY(EditDefaultsOnly)
	FGameplayAttribute StaminaAttribute;

private:
	UFUNCTION()
	void OnStaminaChanged();
	
	TWeakObjectPtr<UGCBaseCharacterMovementComponent> MovementComponent;
	TWeakObjectPtr<AGCBaseCharacter> Character;

	
};