// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUDWidget.generated.h"

class UDamageIndicatorView;
class UHighlightInteractable;

UCLASS()
class GAMECODE_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	class UReticleWidget* GetReticalWidget();
	class UAmmoWidget* GetAmmoWidget();

	class UGrenadeWidget* GetGrenadeWidget();

	class UCharacterAttributesWidget* GetCharacterAttributesWidget();

	void SetHighlightInteractebleVisibility(bool bIsVisible);
	void SetHightInteractableActionText(FName KeyName);

	UFUNCTION(BlueprintImplementableEvent)
	void OnTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION()
	void OnPointDamageReceived( AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser );

protected:
	UFUNCTION(BlueprintCallable)
	float GetHealthPercent() const;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget names")
	FName ReticleWidgetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget names")
	FName AmmoWidgetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget names")
	FName AttributesWidgetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget names")
	FName GrenadeWidgetName;

	UPROPERTY(meta = (BindWidget))
	UHighlightInteractable* InteractableKey;

	UPROPERTY(meta=(BindWidget))
	UDamageIndicatorView* DamageIndicatorView;
};
