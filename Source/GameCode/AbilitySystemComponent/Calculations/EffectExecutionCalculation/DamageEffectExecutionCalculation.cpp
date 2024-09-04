// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageEffectExecutionCalculation.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemComponent/AttributeSets/GCCharacterAttributeSet.h"


struct GCDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Health);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Defence);

	GCDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UGCCharacterAttributeSet, Health, Target, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UGCCharacterAttributeSet, Defence, Target, true);
		
	}
	
};

static const GCDamageStatics& DamageStatics()
{
	static GCDamageStatics Statics;
	return Statics;
}

UDamageEffectExecutionCalculation::UDamageEffectExecutionCalculation()
{
	RelevantAttributesToCapture.Add(GCDamageStatics().HealthDef);
	RelevantAttributesToCapture.Add(GCDamageStatics().DefenceDef);
	
}

void UDamageEffectExecutionCalculation::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	Super::Execute_Implementation(ExecutionParams, OutExecutionOutput);
	
	// UAbilitySystemComponent* TargetAbilitySystemComponent = ExecutionParams.GetTargetAbilitySystemComponent();
	//UAbilitySystemComponent* SourceAbilitySystemComponent = ExecutionParams.GetSourceAbilitySystemComponent();

	//AActor* SourceActor = SourceAbilitySystemComponent ? SourceAbilitySystemComponent->GetAvatarActor() : nullptr;
	//AActor* TargetActor = TargetAbilitySystemComponent ? TargetAbilitySystemComponent->GetAvatarActor() : nullptr;

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	FGameplayTagContainer AssetTags;
	Spec.GetAllAssetTags(AssetTags);

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	float Defence = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DefenceDef, EvaluationParameters, Defence);

	float Damage = Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Abilities.Attributes.Health")), false, 0.0f);
	
	float TotalDamage = FMath::Max<float>(0.0f, Damage - Defence);
	if (TotalDamage > 0.0f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(DamageStatics().HealthProperty, EGameplayModOp::Additive, -Damage));
		
	}

	
}
