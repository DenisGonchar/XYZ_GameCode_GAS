// Fill out your copyright notice in the Description page of Project Settings.


#include "GameCode/DataAsset/ImpactFXData.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "PhysicalMaterials/PhysicalMaterial.h"


void UImpactFXData::PlayImpactFX(const FHitResult& Hit, float Scale /*= 1.0f*/)
{
	AActor* HitActor = Hit.GetActor();
	if (!IsValid(HitActor))
	{
		return;
	}

	UNiagaraSystem* ImpactFX = DefaultImpactFX;
	UPhysicalMaterial* PhysicalMaterial = Hit.PhysMaterial.Get();
	if (IsValid(PhysicalMaterial))
	{
	 	UNiagaraSystem* const* FoundFX = PhysicalImpactFX.Find(PhysicalMaterial);
	    if (FoundFX != nullptr)
	    {
		    ImpactFX = *FoundFX;
	    }
	}
	
	UWorld* World = HitActor->GetWorld();
	FVector FXLocation = Hit.ImpactPoint;
	FRotator FXRotation = Hit.ImpactNormal.ToOrientationRotator();
	
	UNiagaraComponent* ImpactFCComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, ImpactFX, FXLocation, FXRotation);
	if (IsValid(ImpactFCComponent))
	{
		ImpactFCComponent->SetFloatParameter(FName("Scale"), Scale);
				
	}
	
}
