// Fill out your copyright notice in the Description page of Project Settings.


#include "GameCode/UI/Widget/DamageIndicatorView.h"
#include "DamageIndicator.h"

void UDamageIndicatorView::NativeConstruct()
{
	Super::NativeConstruct();
	if (IndicatorWidgets.Num() > 0)
	{
		return;
	}

	check(IsValid(DamageIndicatorClass));
	IndicatorWidgets.Reserve(MaxDamageIndicators);
	for (int i = 0; i < MaxDamageIndicators; ++i)
	{
		UDamageIndicator* Indicator = CreateWidget<UDamageIndicator>(this, DamageIndicatorClass);
		IndicatorWidgets.Add(Indicator);

	}
}

void UDamageIndicatorView::OnPointDamageReceived(AActor* DamagedActor, float Damage, AController* InstigatedBy,
	FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection,
	const UDamageType* DamageType, AActor* DamageCauser)
{
	CurrentWidgetIndex = GetNextWidgetIndex();
	UDamageIndicator* Indicator = IndicatorWidgets[CurrentWidgetIndex];
	if (!Indicator->IsVisible())
	{
		Indicator->AddToViewport();
	}
	Indicator->ActivateWidget(HitLocation, DamagedActor);
}

int32 UDamageIndicatorView::GetNextWidgetIndex()
{
	int32 Result = CurrentWidgetIndex + 1;
	if (Result == IndicatorWidgets.Num())
	{
		Result = 0;
	}
	return  Result;
}
