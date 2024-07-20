// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include <Subsystems/SaveSubsystem/SaveSubsystemInterface.h>
#include "Components/LimbComponet/GCCharacterLimbComponent.h"
#include "CharacterAttributeComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnDeathEventSignature);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnHealthChanged, float);

DECLARE_MULTICAST_DELEGATE_OneParam(FOutOfStaminaEventSignature, bool);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnStaminaCharacter, bool);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnOxygenCharacter, bool);



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMECODE_API UCharacterAttributeComponent : public UActorComponent /*, public ISaveSubsystemInterface*/
{
	GENERATED_BODY()

public:	
	UCharacterAttributeComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	FOnDeathEventSignature OnDeathEvent;
	FOnHealthChanged OnHealthChangedEvent;

	FOutOfStaminaEventSignature OutOfStaminaEventSignature;
	
	FOnStaminaCharacter OnStaminaCharacter;
	FOnOxygenCharacter OnOxygenCharacter;

	bool IsAlive() { return Health > 0.0f; }

	float GetHealthPercet() const;
	float GetStaminaPercet() const;
	float GetOxygenPercet() const;
	void UpdateStamina(float DeltaTime);
	void UpdateOxygenVolue(float DeltaTime);

	void AddHealth(float HealthToAdd);
	void RestoreFullStamina();

	//virtual void OnLevelDeserialized_Implementation() override;

	ETypesCharacter GetTypeCharacter() const;


	UFUNCTION()
	void TakeDamage(float Damage, AController* InstigatedBy, AActor* DamageCauser, FName BoneName);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health", meta = (UIMin = 0.0f))
	ETypesCharacter TypeCharacter = ETypesCharacter::Default;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health", meta = (UIMin = 0.0f))
	float MaxHealth = 100.0f;
	
	//Stamina
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Stamina", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MaxStamina = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Stamina", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float StaminaRestoveVelocity = 0.0f; //скорость восстановление выносливасти 

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Stamina", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float SprintStaminaConsumptionVelocity = 0.0f;// Скорость потребления выносливости игрока
	
	void OnStartStaminingInternal();
	void OnStopStaminingInternal();
	//

	//Oxygen
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Oxygen", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MaxOxygen = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Oxygen", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float OxygenRestoreVelocity = 15.0f;	//скорость восстановление

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Oxygen", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float SwimOxygenConsumptionVelocity = 2.0f; // Скорость потребления

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Oxygen", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float OxygenDamage = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Oxygen", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float OxygenIntervalDamage = 2.0f;
	
	void OnStartOxygeningInternal();
	void OnStopOxygeningInternal();

	//
private:
	UPROPERTY(ReplicatedUsing = OnRep_Health, SaveGame)
	float Health = 0.0f;
	
	UFUNCTION()
	void OnRep_Health();

	void OnHealthChanged();

	float CurrentStamina = 0.0f;

	float Oxygen = 0.0f;
	float OxygenIntervalTick = 0.0f;

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	void DebugDrawAttributes();

#endif

	UFUNCTION()
	void OnTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);


	
	TWeakObjectPtr<class AGCBaseCharacter> CachedBaseCharacterOwner;
	TWeakObjectPtr<UGCCharacterLimbComponent> LimbsComponent;

};
