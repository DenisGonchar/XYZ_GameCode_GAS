// Fill out your copyright notice in the Description page of Project Settings.


#include "GameCode/Components/CharacterComponents/CharacterHitReactionComponent.h"

#include "CharacterAttributeComponent.h"
#include "Pawns/Character/GCBaseCharacter.h"

bool UCharacterHitReactionComponent::IsBlockingReactionActive() const
{
	return GetWorld()->GetTimerManager().IsTimerActive(BlockingReactionTimer);
}

void UCharacterHitReactionComponent::BeginPlay()
{
	Super::BeginPlay();

	check(GetOwner()->IsA<AGCBaseCharacter>());
	CachedBaseCharacter = StaticCast<AGCBaseCharacter*>(GetOwner());

	CachedBaseCharacter->OnTakeAnyDamage.AddDynamic(this, &UCharacterHitReactionComponent::OnTakeAnyDamage);
	CachedBaseCharacter->OnTakePointDamage.AddDynamic(this, &UCharacterHitReactionComponent::OnTakePointDamage);
	CachedBaseCharacter->OnTakeRadialDamage.AddDynamic(this, &UCharacterHitReactionComponent::OnTakeRadialDamage);
	
}

void UCharacterHitReactionComponent::OnTakePointDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy,
	FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection,
	const UDamageType* DamageType, AActor* DamageCauser)
{
	bIsDamageProcessed = false;
	if (IsBlockingReactionActive())
	{
		return;
	}
	
	FHitReaction* Reaction = nullptr;
	
	//скалярное произведения 
	if (FVector::DotProduct(GetOwner()->GetActorForwardVector(), ShotFromDirection) < 0)
	{
		// front reaction
		Reaction = &PointDamageHitReaction.FrontHitReaction;
		
	}
	else
	{
		Reaction = &PointDamageHitReaction.BackHitReaction;
		
	}

	if (Reaction != nullptr && IsValid(Reaction->ReactionAnimation))
	{
		PlayerHitReaction(Reaction);
		bIsDamageProcessed = true;
	}
}

void UCharacterHitReactionComponent::OnTakeRadialDamage(AActor* DamagedActor, float Damage,
	const UDamageType* DamageType, FVector Origin, FHitResult HitInfo, AController* InstigatedBy, AActor* DamageCauser)
{
	
	bIsDamageProcessed = false;
	if (IsBlockingReactionActive())
	{
		return;
	}
	
	if (IsValid(RadialDamageHitReaction.ReactionAnimation))
	{
		FVector LookAtOrigin = (Origin - CachedBaseCharacter->GetActorLocation()).GetSafeNormal2D();
		FRotator LookAtRotation = LookAtOrigin.ToOrientationRotator();
		CachedBaseCharacter->SetActorRotation(LookAtRotation);

		PlayerHitReaction(&RadialDamageHitReaction);
		
		bIsDamageProcessed = true;
	}
	
}

void UCharacterHitReactionComponent::OnTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
                                                     AController* InstigatedBy, AActor* DamageCauser)
{
	if (!bIsDamageProcessed)
	{
		PlayerHitReaction(&DefaultHitReaction);
	}
	bIsDamageProcessed = false;
	
}

void UCharacterHitReactionComponent::PlayerHitReaction(const FHitReaction* HitReaction)
{
	if (!CachedBaseCharacter->GetCharacterAttributeComponent_Muteble()->IsAlive())
	{
		return;
	}
	
	//TODO for getting actual direction make sure that we do it from AnimInstance
	float ReactionDurection = CachedBaseCharacter->PlayAnimMontage(HitReaction->ReactionAnimation);
	if (HitReaction->bIsActionsBlocking)
	{
		GetWorld()->GetTimerManager().SetTimer(BlockingReactionTimer, ReactionDurection, false);
	}
}


