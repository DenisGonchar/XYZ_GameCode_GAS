// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharacterHitReactionComponent.generated.h"

class AGCBaseCharacter;
class UAnimMontage;

USTRUCT(BlueprintType)
struct FHitReaction
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* ReactionAnimation;

	UPROPERTY(EditDefaultsOnly)
	bool bIsActionsBlocking;
	
};

USTRUCT(BlueprintType)
struct FPointDamageHitReaction
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	FHitReaction FrontHitReaction;
	
	UPROPERTY(EditDefaultsOnly)
	FHitReaction BackHitReaction;
		
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMECODE_API UCharacterHitReactionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	bool IsBlockingReactionActive() const;
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly)
	FPointDamageHitReaction PointDamageHitReaction;

	UPROPERTY(EditDefaultsOnly)
	FHitReaction RadialDamageHitReaction;
		
	UPROPERTY(EditDefaultsOnly)
	FHitReaction DefaultHitReaction;

private:
	UFUNCTION()
	void OnTakePointDamage(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser );

	UFUNCTION()
	void OnTakeRadialDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, FVector Origin, FHitResult HitInfo, class AController* InstigatedBy, AActor* DamageCauser );
	
	UFUNCTION()
	void OnTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	void PlayerHitReaction(const FHitReaction* HitReaction);
	
	TWeakObjectPtr<AGCBaseCharacter> CachedBaseCharacter;

	FTimerHandle BlockingReactionTimer;
	
	bool bIsDamageProcessed = false;
};
