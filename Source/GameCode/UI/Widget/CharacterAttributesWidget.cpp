// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/CharacterAttributesWidget.h"
#include "Pawns/Character/GCBaseCharacter.h"
#include "Components/CharacterComponents/CharacterAttributeComponent.h"

void UCharacterAttributesWidget::NativeConstruct()
{
	Super::NativeConstruct();

	APawn* Pawn = GetOwningPlayerPawn();
	AGCBaseCharacter* Character = Cast<AGCBaseCharacter>(Pawn);
	if (Character)
	{
		UCharacterAttributeComponent* CharacterAttribute = Character->GetCharacterAttributeComponent_Muteble();

		CharacterAttribute->OnStaminaCharacter.AddUFunction(this, FName("OnStaminingStateChanged"));
		CharacterAttribute->OnOxygenCharacter.AddUFunction(this, FName("OnOxygeningStateChanged"));
	}
	
}

float UCharacterAttributesWidget::GetHealthPercent() const
{
	float Result = 1.0f;

	APawn* Pawn = GetOwningPlayerPawn();
	AGCBaseCharacter* Character = Cast<AGCBaseCharacter>(Pawn);
	if (IsValid(Character))
	{
		const UCharacterAttributeComponent* CharacterAttributes = Character->GetCharacterAttributeComponent();

		Result = CharacterAttributes->GetHealthPercet();

	}

	return Result;
		
}

float UCharacterAttributesWidget::GetStaminaPercent() const
{
	float Result = 1.0f;

	APawn* Pawn = GetOwningPlayerPawn();
	AGCBaseCharacter* Character = Cast<AGCBaseCharacter>(Pawn);
	if (IsValid(Character))
	{
		const UCharacterAttributeComponent* CharacterAttributes = Character->GetCharacterAttributeComponent();
		
		Result = CharacterAttributes->GetStaminaPercet();

	}

	return Result;
}

float UCharacterAttributesWidget::GetOxygenPercent() const
{
	float Result = 1.0f;

	APawn* Pawn = GetOwningPlayerPawn();
	AGCBaseCharacter* Character = Cast<AGCBaseCharacter>(Pawn);
	if (IsValid(Character))
	{
		const UCharacterAttributeComponent* CharacterAttributes = Character->GetCharacterAttributeComponent();

		Result = CharacterAttributes->GetOxygenPercet();

	}

	return Result;
}
