// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GCCharacterAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class GAMECODE_API UGCCharacterAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;

	float GetStaminaPercent() const;

	float GetHealthPercent() const;
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Attributes | Stamina")
	FGameplayAttributeData Stamina = 100.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Attributes | Stamina")
	FGameplayAttributeData MaxStamina = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Attributes | Health")
	FGameplayAttributeData Health = 100.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Attributes | Health")
	FGameplayAttributeData MaxHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Attributes | Damage")
	FGameplayAttributeData Defence = 10.0f;
	
	
	
	
};
