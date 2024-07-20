// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Equipment/EquipableItem.h"
#include "ThrowableItem.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnThrowAmmoChanged, int32);

class AGCProjectile;

UCLASS(Blueprintable)
class GAMECODE_API AThrowableItem : public AEquipableItem
{
	GENERATED_BODY()
	
public:
		
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void Throw();

	bool CanThrow() const {return ThrowAmmo > 0; }

	EAmmunitionType GetThrowAmmoType() const { return ThrowAmmoType; }

	int32 GetThrowAmmo() const { return ThrowAmmo; }
	int32 GetMaxThrowAmmo() const { return MaxThrowAmmo; }

	void SetThrowAmmo(int32 NewAmmo);

	FOnThrowAmmoChanged OnThrowAmmoChanged;

	UFUNCTION()
	void OnRep_ThrowAmmo();
protected:
	
	UPROPERTY(ReplicatedUsing= OnRep_ThrowAmmo)
	int32 ThrowAmmo = 2;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwables | Ammo", meta = (ClampMin = 0, UIMin = 0))
	int32 MaxThrowAmmo = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwables | Ammo")
	EAmmunitionType ThrowAmmoType = EAmmunitionType::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwables")
	TSubclassOf<AGCProjectile> ProjectileClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwables", meta = (UIMin = 1.0f, ClampMax = 1.0f, EditCondition = "HitRegistration == EHitRegistrationType::Projectile"))
	int32 ProjectilePoolSize = 10;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Throwables", meta = (UIMin = -90.f, UIMax = 90.0f, ClampMin = -90.0f, ClampMax = 90.0f))
	float ThrowAngle = 0.0f;

private:
	UFUNCTION()
	void ProcessProjectileHit(AGCProjectile* Projectile, const FHitResult& HitResult, const FVector& Direction);

	UPROPERTY(Replicated)
	TArray<AGCProjectile*> ProjectilePool;

	UPROPERTY(Replicated)
	int32 CurrentProjectileIndex;
	
	const FVector ProjectilePoolLocation = FVector(0.0f, 0.0f, -100.0f);

};
