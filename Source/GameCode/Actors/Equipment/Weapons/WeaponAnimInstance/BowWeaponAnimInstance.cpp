// Fill out your copyright notice in the Description page of Project Settings.


#include "BowWeaponAnimInstance.h"

#include "Actors/Equipment/Weapons/BowWeaponItem.h"


void UBowWeaponAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	CachedBowWeapon = Cast<ABowWeaponItem>(GetOwningActor());

	
}

void UBowWeaponAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	
	if (!CachedBowWeapon.IsValid())
	{
		return;
	}
	
	Tension = CachedBowWeapon->GetTension();
}
