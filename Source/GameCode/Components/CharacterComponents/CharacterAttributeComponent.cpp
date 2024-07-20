// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAttributeComponent.h"
#include "Pawns/Character/GCBaseCharacter.h"
#include "Subsystems/DebugSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameCodeTypes.h"
#include <DrawDebugHelpers.h>
#include <Components/CapsuleComponent.h>
#include "Pawns/Character/CharacterMovementComponent/GCBaseCharacterMovementComponent.h"
#include <Net/UnrealNetwork.h>


UCharacterAttributeComponent::UCharacterAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);
}

void UCharacterAttributeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCharacterAttributeComponent, Health);
	
}

void UCharacterAttributeComponent::BeginPlay()
{
	Super::BeginPlay();
	
	checkf(MaxHealth > 0.0f,TEXT("UCharacterAttributeComponent::BeginPlay max health cannot be equal to 0"));
	checkf(GetOwner()->IsA<AGCBaseCharacter>(), TEXT("UCharacterAttributeComponent::BeginPlay UCharacterAttributeComponent can be used only with AGCBaseCharacter"));

	CachedBaseCharacterOwner = StaticCast<AGCBaseCharacter*>(GetOwner());
	LimbsComponent = GetOwner()->FindComponentByClass<UGCCharacterLimbComponent>();

	Health = MaxHealth;

	CurrentStamina = MaxStamina;

	Oxygen = MaxOxygen;
	OxygenIntervalTick = OxygenIntervalDamage;

	/*if (GetOwner()->HasAuthority())
	{
		CachedBaseCharacterOwner->OnTakeAnyDamage.AddDynamic(this, &UCharacterAttributeComponent::OnTakeAnyDamage);

	}*/

	if (TypeCharacter == ETypesCharacter::Default)
	{
		if (GetOwner()->HasAuthority())
		{
			CachedBaseCharacterOwner->OnTakeAnyDamage.AddDynamic(this, &UCharacterAttributeComponent::OnTakeAnyDamage);

		}
	}
}

void UCharacterAttributeComponent::OnStartStaminingInternal()
{
	if(OnStaminaCharacter.IsBound())
	{
		OnStaminaCharacter.Broadcast(true);
	}
}

void UCharacterAttributeComponent::OnStopStaminingInternal()
{
	if (OnStaminaCharacter.IsBound())
	{
		OnStaminaCharacter.Broadcast(false);
	}
}

void UCharacterAttributeComponent::OnStartOxygeningInternal()
{
	if (OnOxygenCharacter.IsBound())
	{
		OnOxygenCharacter.Broadcast(true);
	}
}

void UCharacterAttributeComponent::OnStopOxygeningInternal()
{
	if (OnOxygenCharacter.IsBound())
	{
		OnOxygenCharacter.Broadcast(false);
	}
}

void UCharacterAttributeComponent::OnRep_Health()
{
	OnHealthChanged();
}

void UCharacterAttributeComponent::OnHealthChanged()
{	
	if (OnHealthChangedEvent.IsBound())
	{
		OnHealthChangedEvent.Broadcast(GetHealthPercet());
	}

	if (Health <= 0.0f)
	{

		if (OnDeathEvent.IsBound())
		{
			OnDeathEvent.Broadcast();

		}
	}
}

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT

void UCharacterAttributeComponent::DebugDrawAttributes()
{
	UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();

	if (!DebugSubsystem->IsCategoryEnabled(DebugCategoryCharacterAttributes))
	{
		return;
	}
	
	FVector TextLocation = CachedBaseCharacterOwner->GetActorLocation() + (CachedBaseCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 5.0f) * FVector::UpVector;
	DrawDebugString(GetWorld(), TextLocation, FString::Printf(TEXT("Health: %.2f"), Health), nullptr, FColor::Green, 0.0f, true);
	
	FVector StaminaTextLocation = CachedBaseCharacterOwner->GetActorLocation() + (CachedBaseCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()) * FVector::UpVector;
	DrawDebugString(GetWorld(), StaminaTextLocation, FString::Printf(TEXT("Stamina: %.2f"), CurrentStamina), nullptr, FColor::Yellow, 0.0f, true);

	FVector OxygenTextLocation = CachedBaseCharacterOwner->GetActorLocation() + (CachedBaseCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - 5.0f) * FVector::UpVector;
	DrawDebugString(GetWorld(), OxygenTextLocation, FString::Printf(TEXT("Oxygen: %.2f"), Oxygen), nullptr, FColor::Blue, 0.0f, true);

}
#endif

void UCharacterAttributeComponent::OnTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (!IsAlive())
	{
		return;
	}

	//UE_LOG(LogDamage, Warning, TEXT("UCharacterAttributeComponent::OnTakeAnyDamage %s recevied %.2f amount of damage from %s"), *CachedBaseCharacterOwner->GetName(), Damage, *DamageCauser->GetName());

	Health = FMath::Clamp(Health - Damage, 0.0f, MaxHealth);

	OnHealthChanged();
}
void UCharacterAttributeComponent::TakeDamage(float Damage, AController* InstigatedBy, AActor* DamageCauser, FName BoneName)
{
	switch (TypeCharacter)
	{
	case ETypesCharacter::Default:
	{
		if (!IsAlive())
		{
			return;
		}

		//UE_LOG(LogDamage, Warning, TEXT("UCharacterAttributeComponent::OnTakeAnyDamage %s recevied %.2f amount of damage from %s"), *CachedBaseCharacterOwner->GetName(), Damage, *DamageCauser->GetName());

		Health = FMath::Clamp(Health - Damage, 0.0f, MaxHealth);

		OnHealthChanged();

		break;
	}
	case ETypesCharacter::Modular:
	{
		float damageMultipler = 1.0f;

		if (LimbsComponent.IsValid())
		{
			if (auto limbDataPtr = LimbsComponent->FindLimbDataByBone(BoneName))
			{
				damageMultipler = limbDataPtr->CharacterDamageMultiplier;
			}
		}

		const float finalDamage = Damage * damageMultipler;

		Health = FMath::Max(Health - finalDamage, 0.0f);

		OnHealthChanged();

		break;
	}
	default:
		break;
	}

	
}

void UCharacterAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	DebugDrawAttributes();
#endif

	UpdateStamina(DeltaTime);
	UpdateOxygenVolue(DeltaTime);

}

float UCharacterAttributeComponent::GetHealthPercet() const
{
	return Health / MaxHealth;
}

float UCharacterAttributeComponent::GetStaminaPercet() const
{
	return CurrentStamina / MaxStamina;
}

float UCharacterAttributeComponent::GetOxygenPercet() const
{
	return Oxygen / MaxOxygen;
}

void UCharacterAttributeComponent::UpdateStamina(float DeltaTime)
{
	if (CurrentStamina <= 0)
	{
		//GetBaseCharacterMovementComponent()->SetIsOutOfStamina(true);
		OutOfStaminaEventSignature.Broadcast(true);
		OnStartStaminingInternal();
	}
	if (CurrentStamina < MaxStamina && !CachedBaseCharacterOwner->GetBaseCharacterMovementComponent()->IsSprinting())
	{
		OnStartStaminingInternal();
		CurrentStamina += StaminaRestoveVelocity * DeltaTime;
		CurrentStamina = FMath::Clamp(CurrentStamina, 0.0f, MaxStamina);
		GEngine->AddOnScreenDebugMessage(-1, 0.001f, FColor::Green, FString::Printf(TEXT("Stamina %.2f"), CurrentStamina));

	}
	if (CachedBaseCharacterOwner->GetBaseCharacterMovementComponent()->IsSprinting() && !CachedBaseCharacterOwner->GetBaseCharacterMovementComponent()->IsOutOfStamina())
	{
		OnStartStaminingInternal();
		CurrentStamina -= SprintStaminaConsumptionVelocity * DeltaTime;
		CurrentStamina = FMath::Clamp(CurrentStamina, 0.0f, MaxStamina);
		GEngine->AddOnScreenDebugMessage(-1, 0.001f, FColor::Red, FString::Printf(TEXT("Stamina %.2f"), CurrentStamina));

	}
	if (FMath::IsNearlyEqual(CurrentStamina, MaxStamina))
	{
		//GetBaseCharacterMovementComponent()->SetIsOutOfStamina(false);
		OutOfStaminaEventSignature.Broadcast(false);
		OnStopStaminingInternal();
	}
}

void UCharacterAttributeComponent::UpdateOxygenVolue(float DeltaTime)
{
	if (Oxygen <= 0.0f)
	{
		//Multicast Oxygen
		OnStartOxygeningInternal();

		if (IsAlive())
		{
			if (OxygenIntervalTick <= 0.0f)
			{
				Health = FMath::Clamp(Health - OxygenDamage, 0.0f, MaxHealth);

				if (Health <= 0.0f)
				{
					if (OnDeathEvent.IsBound())
					{
						OnDeathEvent.Broadcast();

					}
				}

				OxygenIntervalTick = OxygenIntervalDamage;
			}
			else
			{
				OxygenIntervalTick -= DeltaTime;
			}
			
		}
		

	}
	if (CachedBaseCharacterOwner->IsSwimmingUnderWater() || (Oxygen < MaxOxygen && !CachedBaseCharacterOwner->GetBaseCharacterMovementComponent()->IsSwimming()))
	{
		OnStartOxygeningInternal();
		Oxygen += OxygenRestoreVelocity * DeltaTime;
		Oxygen = FMath::Clamp(Oxygen, 0.0f, MaxOxygen);
		GEngine->AddOnScreenDebugMessage(-1, 0.001f, FColor::Yellow, FString::Printf(TEXT("Oxygen: %.2f"), Oxygen));

	}
	if (!CachedBaseCharacterOwner->IsSwimmingUnderWater() && CachedBaseCharacterOwner->GetBaseCharacterMovementComponent()->IsSwimming())
	{
		OnStartOxygeningInternal();
		Oxygen -= SwimOxygenConsumptionVelocity * DeltaTime;
		Oxygen = FMath::Clamp(Oxygen, 0.0f, MaxOxygen);
		GEngine->AddOnScreenDebugMessage(-1, 0.001f, FColor::Red, FString::Printf(TEXT("Oxygen: %.2f"), Oxygen));

	}
	if (FMath::IsNearlyEqual(Oxygen, MaxOxygen))
	{
		//Multicast
		//Oxygen: False Damage
		//GEngine->AddOnScreenDebugMessage(-1, 0.001f, FColor::White, FString::Printf(TEXT("Oxygen: False Damage")));
		OnStopOxygeningInternal();
	}

}

void UCharacterAttributeComponent::AddHealth(float HealthToAdd)
{
	Health = FMath::Clamp(Health + HealthToAdd, 0.0f, MaxHealth);
	OnHealthChanged();

}

void UCharacterAttributeComponent::RestoreFullStamina()
{
	CurrentStamina = MaxStamina;
}

// void UCharacterAttributeComponent::OnLevelDeserialized_Implementation()
// {
// 	OnHealthChanged();
// }

ETypesCharacter UCharacterAttributeComponent::GetTypeCharacter() const
{
	return TypeCharacter;
}

