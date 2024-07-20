// Fill out your copyright notice in the Description page of Project Settings.


#include "GCCharacterAttributeSet.h"

#include "GenericPlatform/GenericPlatformCrashContext.h"

void UGCCharacterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute.AttributeName == FString("String"))
	{
		NewValue = FMath:: Clamp(NewValue, 0.0f, MaxStamina.GetCurrentValue());
	}

	if (Attribute.AttributeName == FString("Health"))
	{
		NewValue = FMath:: Clamp(NewValue, 0.0f, MaxHealth.GetCurrentValue());
	}

}

void UGCCharacterAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	if (Attribute.AttributeName == FString("String"))
	{
		NewValue = FMath:: Clamp(NewValue, 0.0f, MaxStamina.GetBaseValue());
	}

	if (Attribute.AttributeName == FString("Haelth"))
	{
		NewValue = FMath:: Clamp(NewValue, 0.0f, MaxHealth.GetBaseValue());
	}

	
}

float UGCCharacterAttributeSet::GetStaminaPercent() const
{
	return Stamina.GetCurrentValue() / MaxStamina.GetCurrentValue();
}

float UGCCharacterAttributeSet::GetHealthPercent() const
{
	return Health.GetCurrentValue() / MaxHealth.GetCurrentValue();
}
