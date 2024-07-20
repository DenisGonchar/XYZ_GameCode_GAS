// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/Characters/GCAICharacter.h"
#include "GCAIModularCharacter.generated.h"

class UGCCharacterLimbComponent;
class UGCCombinedSkeletalMeshComponent;


UCLASS()
class GAMECODE_API AGCAIModularCharacter : public AGCAICharacter
{
	GENERATED_BODY()
	
public:
	AGCAIModularCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	UGCCombinedSkeletalMeshComponent* GetCombinedSkeletalMeshComponent() const;

	virtual float InternalTakePointDamage(float Damage, struct FPointDamageEvent const& PointDamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UGCCharacterLimbComponent* CharacterLimbsComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UGCCombinedSkeletalMeshComponent* CombinedSkeletalMeshComponent;


};
