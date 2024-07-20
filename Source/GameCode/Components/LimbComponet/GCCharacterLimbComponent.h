// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameCodeTypes.h"
#include "GCCharacterLimbComponent.generated.h"

class USkeletalMeshComponent;
class AController;
class UPrimitiveComponent;
class UDamageType;
class AGCLimbActor;
class UGCCombinedSkeletalMeshComponent;

USTRUCT(BlueprintType)
struct FLimdData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Health = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CharacterDamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBodyPart BodyPart;

};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMECODE_API UGCCharacterLimbComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	virtual void OnRegister() override;
	virtual void BeginPlay() override;

	void TakeDamage(float Damage, AController* InstigatedBy, AActor* DamageCauser, UPrimitiveComponent* FHitComponent, FName BoneName);

	const FLimdData* FindLimbDataByBone(FName BoneName) const;

protected:
	void TakeLimbDamage(FLimdData& Limb, float Damage);

	void SpawnLimbActor(const FLimdData& Limb, const FTransform& Transform);

	FLimdData* FindLimbData(USkeletalMeshComponent* MeshComponent, FName BoneName);

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FLimdData> Limbs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<AGCLimbActor> LimbActorClass;

	TWeakObjectPtr<UGCCombinedSkeletalMeshComponent> CombinedSkeletalMesh;

	
		
};
