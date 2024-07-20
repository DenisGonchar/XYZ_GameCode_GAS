// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DamageIndicator.generated.h"

class UImage;
/**
 * 
 */
UCLASS()
class GAMECODE_API UDamageIndicator : public UUserWidget
{
	GENERATED_BODY()

public:
	void ActivateWidget(FVector HitLocation_In, AActor* HitActor_In);

 protected:
	void virtual NativeConstruct() override;

	void virtual NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	UPROPERTY(EditDefaultsOnly)
	float LifeTime = 5.0f;

	UPROPERTY(meta=(BindWidget))
	UImage* IndicatorImage;

	UFUNCTION(BlueprintNativeEvent)
	void OnIndicatorActivated();
	
	UFUNCTION(BlueprintNativeEvent)
	void OnLifeTimeElapsed();
private:
	FVector HitLocation;
	TWeakObjectPtr<AActor> HitActor;

	UMaterialInstanceDynamic* IndicatorMaterial;
	FTimerHandle LifeTimeTimer;
};
