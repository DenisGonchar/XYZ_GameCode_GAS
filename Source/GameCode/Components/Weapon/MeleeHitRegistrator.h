// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "MeleeHitRegistrator.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnMeleeHitRegistred, const FHitResult&,  const FVector&);

UCLASS(meta = (BlueprintSpawnableComponent))
class GAMECODE_API UMeleeHitRegistrator : public USphereComponent
{
	GENERATED_BODY()
	
public:
	UMeleeHitRegistrator();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void ProcessHitRegistration();

	void SetIsHitRegistrationEnabled(bool bIsEnabled_In);

	FOnMeleeHitRegistred OnMeleeHitRegistred;
protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Melee hit registration")
	bool bIsHitRegistrationEnabled = false;

private:
	
	FVector PreviousComponentLocation = FVector::ZeroVector;

};
