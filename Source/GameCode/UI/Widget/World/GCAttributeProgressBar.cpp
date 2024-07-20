// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/World/GCAttributeProgressBar.h"

#include "AbilitySystemComponent/AttributeSets/GCCharacterAttributeSet.h"
#include "Components/ProgressBar.h"

void UGCAttributeProgressBar::SetProgressPercantage(float Percentage)
{
	HealthProgressBar->SetPercent(Percentage);
}

void UGCAttributeProgressBar::SetAttibuteSet(UGCCharacterAttributeSet* AttributeSet_In)
{
	AttributeSet = AttributeSet_In;
}

float UGCAttributeProgressBar::GetHealthPercent() const
{
	float Result = 0.0f;
	if (AttributeSet.IsValid())
	{
	 Result = AttributeSet->GetHealthPercent();
		
	}
	return Result;
}
