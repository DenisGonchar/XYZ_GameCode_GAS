// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ImpactFXData.generated.h"

class UNiagaraSystem;
class UPhysicalMaterial;

USTRUCT(BlueprintType)
struct FImpactDescription
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UNiagaraSystem* ImpactFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DefaultScale;
	
};

UCLASS()
class GAMECODE_API UImpactFXData : public UDataAsset
{
	GENERATED_BODY()

public:
	void PlayImpactFX(const FHitResult& Hit, float Scale = 1.0f);
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraSystem* DefaultImpactFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<UPhysicalMaterial*, UNiagaraSystem*> PhysicalImpactFX;

	//как пример
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	//TMap<UPhysicalMaterial*, FImpactDescription> DescriptionImpactFX;

};
