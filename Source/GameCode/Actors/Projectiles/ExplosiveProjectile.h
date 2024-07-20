// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GCProjectile.h"
#include "ExplosiveProjectile.generated.h"
	  
class UExplosionComponent;

UENUM(BlueprintType)
enum class EProjectileType : uint8
{
	Default,
	Additional
};

UCLASS()
class GAMECODE_API AExplosiveProjectile : public AGCProjectile
{
	GENERATED_BODY()
	
public:
	
	AExplosiveProjectile();

protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UExplosionComponent* ExlosionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
	EProjectileType ProjectileType = EProjectileType::Default;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion", meta = (EditCondition = "ProjectileType == EProjectileType::Default"))
	float DetonationTime = 2.0f;

	virtual void OnProjectileLaunched() override;

	virtual void OnCollisionHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
private:
	
	void OnDetonationTimerElapsed();
	
	AController* GetController();
	
	FTimerHandle DitonationTimer;

};
