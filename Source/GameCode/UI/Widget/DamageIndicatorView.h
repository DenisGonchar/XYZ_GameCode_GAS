// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DamageIndicatorView.generated.h"

class UDamageIndicator;
/**
 * 
 */
UCLASS()
class GAMECODE_API UDamageIndicatorView : public UUserWidget
{
	GENERATED_BODY()

public:
	void OnPointDamageReceived( AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser );

protected:
	virtual void NativeConstruct() override;


	UPROPERTY(EditDefaultsOnly, meta=(ClampMin = 1, UIMin = 1))
	int32 MaxDamageIndicators = 5;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDamageIndicator> DamageIndicatorClass;

private:
	int32 GetNextWidgetIndex();
	
	UPROPERTY(Transient)
	TArray<UDamageIndicator*> IndicatorWidgets;

	int32 CurrentWidgetIndex = -1;

};
