// Fill out your copyright notice in the Description page of Project Settings.


#include "GameCode/UI/Widget/DamageIndicator.h"

#include "Components/Image.h"

void UDamageIndicator::ActivateWidget(FVector HitLocation_In, AActor* HitActor_In)
{
	HitLocation = HitLocation_In;
	HitActor = HitActor_In;

	GetWorld()->GetTimerManager().SetTimer(LifeTimeTimer, this, &UDamageIndicator::OnLifeTimeElapsed, LifeTime, false);
	OnIndicatorActivated();
	
}

void UDamageIndicator::NativeConstruct()
{
	Super::NativeConstruct();

	if (!IsValid(IndicatorMaterial))
	{
		IndicatorMaterial  =  IndicatorImage->GetDynamicMaterial();
	}
	
}

void UDamageIndicator::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!HitActor.IsValid())
	{
		return;
	}

	FVector HitLocationLocal = HitActor->GetActorTransform().InverseTransformPosition(HitLocation);
	FVector HitDirection = HitLocationLocal.GetSafeNormal2D();
	float HitAngleRadians = FMath::Atan2(HitDirection.Y, HitDirection.X);
	float Angle = FMath::GetMappedRangeValueClamped(FVector2D(-PI, PI), FVector2D(1.0f, 0.0f), HitAngleRadians);
	
	IndicatorMaterial->SetScalarParameterValue(FName("Angle"), Angle);
}

void UDamageIndicator::OnIndicatorActivated_Implementation()
{
	
}

void UDamageIndicator::OnLifeTimeElapsed_Implementation()
{
	HitActor = nullptr;
}
