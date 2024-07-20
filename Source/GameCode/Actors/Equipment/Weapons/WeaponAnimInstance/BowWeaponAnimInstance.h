// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BowWeaponAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class GAMECODE_API UBowWeaponAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
		
	virtual void NativeBeginPlay() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;


protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon animation")
	float Tension = 0.f;

private:
	TWeakObjectPtr<class ABowWeaponItem> CachedBowWeapon;

};
