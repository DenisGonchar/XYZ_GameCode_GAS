// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GCProjectile.h"
#include "ArrowProjectile.generated.h"

UCLASS()
class GAMECODE_API AArrowProjectile : public AGCProjectile
{
	GENERATED_BODY()

public:
	AArrowProjectile();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Pickable")
	TSubclassOf<AActor> PickableItem;

	virtual void OnCollisionHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector  NormalImpulse, const FHitResult& Hit) override;

};
