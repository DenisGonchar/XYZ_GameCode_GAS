// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeWeaponItem.h"
#include "GameCodeTypes.h"
#include "Pawns/Character/GCBaseCharacter.h"
#include "Components/Weapon/MeleeHitRegistrator.h"
#include <Pawns/Character/CharacterMovementComponent/GCBaseCharacterMovementComponent.h>

#include "DataAsset/ImpactFXData.h"

AMeleeWeaponItem::AMeleeWeaponItem()
{
	EquippedSocketName = SocketCharacterWeapon;
}

void AMeleeWeaponItem::StartAttack(EMeleeAttackType AttackType)
{
	AGCBaseCharacter* CharacterOwner = GetCharacterOwner();
	if (!IsValid(CharacterOwner))
	{
		return;
	}
	if (bIsStartAttack)
	{
		return;
	}
	HitActors.Empty();

	CurrentAttack = Attacks.Find(AttackType);
	if (CurrentAttack && IsValid(CurrentAttack->AttackMontage))
	{
		UAnimInstance* CharacterAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
		if (IsValid(CharacterAnimInstance))
		{
			float Duration = CharacterAnimInstance->Montage_Play(CurrentAttack->AttackMontage, 1.0f, EMontagePlayReturnType::Duration);
			GetWorld()->GetTimerManager().SetTimer(AttackTimer, this, &AMeleeWeaponItem::OnAttackTimerElapsed, Duration, false);

			bIsStartAttack = true;
			
		}
		else
		{
			OnAttackTimerElapsed();
		}

	}


}

void AMeleeWeaponItem::SetIsHitRegistrationEnabled(bool bIsregistrationEnanled)
{
	HitActors.Empty();
	for (UMeleeHitRegistrator* HitRegistrator : HitRegistrators)
	{
		HitRegistrator->SetIsHitRegistrationEnabled(bIsregistrationEnanled);
	}
}

bool AMeleeWeaponItem::GetStartAttack() const
{
	return bIsStartAttack;
}

void AMeleeWeaponItem::BeginPlay()
{
	Super::BeginPlay();

	GetComponents<UMeleeHitRegistrator>(HitRegistrators);
	for (UMeleeHitRegistrator* HitRegistrator: HitRegistrators)
	{
		HitRegistrator->OnMeleeHitRegistred.AddUFunction(this, FName("ProcessHit"));
	}


}

void AMeleeWeaponItem::ProcessHit(const FHitResult& HitResult, const FVector& HitDirection)
{
	if (CurrentAttack == nullptr)
	{
		return;
	}
	

	AActor* HitActor = HitResult.GetActor();
	if (!IsValid(HitActor))
	{
		return;
	}

	if (HitActors.Contains(HitActor))
	{
		return;
	}

	FPointDamageEvent DamageEvent;
	DamageEvent.HitInfo = HitResult;
	DamageEvent.ShotDirection = HitDirection;
	DamageEvent.DamageTypeClass = CurrentAttack->DamageTypeClass;

	AGCBaseCharacter* CharacterOwner = GetCharacterOwner();
	AController* Controller = IsValid(CharacterOwner) ? CharacterOwner->GetController<AController>() : nullptr;

	HitActor->TakeDamage(CurrentAttack->DamageAmount, DamageEvent,	Controller, GetOwner());

	HitActors.Add(HitActor);

	if (IsValid(ImpactFXData))
	{
		ImpactFXData->PlayImpactFX(HitResult, 1.0f);
	}

}

void AMeleeWeaponItem::OnAttackTimerElapsed()
{
	bIsStartAttack = false;
	
	AGCBaseCharacter* CharacterOwner = GetCharacterOwner();
	CharacterOwner->GetBaseCharacterMovementComponent()->SetMovementMode(MOVE_Walking);

	CurrentAttack = nullptr;
	SetIsHitRegistrationEnabled(false);
}
