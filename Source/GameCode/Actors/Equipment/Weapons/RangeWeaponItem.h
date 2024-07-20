// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Equipment/EquipableItem.h"
#include <Subsystems/SaveSubsystem/SaveSubsystemInterface.h>
#include "RangeWeaponItem.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnReloadComplete);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAmmoChanged, int32);

UENUM(BlueprintType)
enum class EWeaponFireMode : uint8
{
	Single, 
	FullAuto

};

UENUM(BlueprintType)
enum class EReloadType : uint8
{
	FullClip,
	ByBullet


};

USTRUCT(BlueprintType)
struct FRecoilParameters
{
	GENERATED_BODY()

	//������ � ������� 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin = -2.0f, UIMin = -2.0f, ClampMax = 2.0f, UIMax = 2.0f))
	float RecoilYaw = -0.1f;

	//������ �����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin = -2.0f, UIMin = -2.0f, ClampMax = 2.0f, UIMax = 2.0f))
	float RecoilPitch = 0.2f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin = 0.0f, UIMin = 0.0f))
	float RecoilTime = 0.05f;

	//Recoil rollback speed in shots per minute
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin = 0.0f, UIMin = 0.0f))
	float RecoilRollbackSpeed = 1200.0f;
};

USTRUCT(BlueprintType)
struct FWeaponParameters
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations|Weapon")
	UAnimMontage* WeaponFireMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations|Weapon")
	UAnimMontage* WeaponReloadMontage;

	//FullClip reload type adds ammo only when the whole reload animation is successfully played
	//ByBullet reload type requires section "ReloadEnd" in character reload animation
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations|Weapon")
	EReloadType ReloadType = EReloadType::FullClip;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations|Character")
	UAnimMontage* CharacterFireMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations|Character")
	UAnimMontage* CharacterReloadMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Parameters")
	EWeaponFireMode WeaponFireMode = EWeaponFireMode::Single;
	
	//Rate of fire in round per minute
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Parameters", meta = (ClampMin = 1.0f, UIMin = 1.0f))
	float RateOfFire = 600.0f;

	//Bullet spread half angle in degrees
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parameters ", meta = (ClampMin = 0.0f, UIMin = 0.0f, ClampMax = 2.0f, UIMax = 2.0f))
	float SpreadAngle = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Parameters|Ammo", meta = (ClampMin = 1.0f, UIMin = 1.0f))
	EAmmunitionType AmmoType = EAmmunitionType::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parameters | Ammo", meta = (ClampMin = 1.0f, UIMin = 1.0f))
	int32 MaxAmmo = 30.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Parameters | Ammo")
	bool bAutoReload = true;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Parameters|Aimimg", meta = (ClampMin = 0.0f, UIMin = 0.0f, ClampMax = 2.0f, UIMax = 2.0f))
	float AimSpreadAngle = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Parameters|Aimimg", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float AimMovementMaxSpeed = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Parameters|Aimimg", meta = (ClampMin = 0.0f, UIMin = 0.0f, ClampMax = 120.0f, UIMax = 120.0f))
	float AimFOV = 60.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Parameters|Aimimg", meta = (ClampMin = 0.0f, UIMin = 0.0f, ClampMax = 1.0f, UIMax = 1.0f))
	float AimTurnModifier = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Parameters|Aimimg", meta = (ClampMin = 0.0f, UIMin = 0.0f, ClampMax = 1.0f, UIMax = 1.0f))
	float AimLookUpModifier = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Parameters|Aimimg")
	EReticleType AimReticleType = EReticleType::Default;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Parameters|Recoil")
	FRecoilParameters RecoilParameters;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Parameters|Recoil")
	UCurveFloat* RecoilAngleCurve;
		
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Parameters|Recoil")
	TSubclassOf<class UMatineeCameraShake> ShotCameraShakeClass;
	
	int32 WeaponAmmo;
	int32 WeaponMaxAmmo;
	
};

class UAnimMontage;
class UMatineeCameraShake;

UCLASS(Blueprintable)
class GAMECODE_API ARangeWeaponItem : public AEquipableItem /*, public ISaveSubsystemInterface*/
{
	GENERATED_BODY()
	
public:
	ARangeWeaponItem();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void Tick(float DeltaSeconds) override;
	
	virtual void StartFire();
	virtual void StopFire();
	
	bool IsFiring() const { return bIsFiring; }

	virtual void StartAim() { bIsAiming = true; }
	virtual void StopAim() { bIsAiming = false; }
	virtual bool bAiming();
	
	float GetAimFOV() const { return CurrentWeaponParameters.AimFOV; }
	float GetAimMovementMaxSpeed() const { return CurrentWeaponParameters.AimMovementMaxSpeed; }
	float GetAimTurnModifier() const { return CurrentWeaponParameters.AimTurnModifier; }
	float GetAimLookUpModifier() const { return CurrentWeaponParameters.AimLookUpModifier; }

	FTransform GetForeGripTransform() { return WeaponMesh->GetSocketTransform(SocketWeaponForeGrip); }

	//Ammo
	int32 GetAmmo() const { return CurrentWeaponParameters.WeaponAmmo; }
	int32 GetCurrentMaxAmmo() const { return CurrentWeaponParameters.WeaponMaxAmmo; }
	int32 GetDefaultMaxAmmo() const { return WeaponDefaultParameters.MaxAmmo; }
	
	void SetAmmo(int32 NewAmmo);
	bool CanShoot() const;

	FOnAmmoChanged OnAmmoChanged;

	EAmmunitionType GetAmmoType() const { return CurrentWeaponParameters.AmmoType;}
	EAmmunitionType GetDefaultAmmoType() const { return WeaponDefaultParameters.AmmoType;}
	
	virtual EReticleType GetReticleType() const;
	FWeaponParameters GetWeaponParameters() {return CurrentWeaponParameters;}

	//Reload
	void StartReload();
	void EndReload(bool bIsSuccess);

	bool IsReloadnig() const;

	FOnReloadComplete OnReloadComplete;


	//virtual void OnLevelDeserialized_Implementation() override;

	void StopRecoilRollback();

	bool SwitchAmmoType();

	UFUNCTION(Server, Reliable)
	void Server_ReloadWeapon();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Reload();

	
	void SetAllAmmo();

	FName GetDataTableAmmoID() const {return DataTableAmmoID;}
	
protected:
	
	virtual void BeginPlay() override;

	
	float GetCurrentBulletSpreadAngle() const;
protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UWeaponBarellComponent* WeaponBarell;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FWeaponParameters WeaponDefaultParameters;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FWeaponParameters WeaponAdditionalParameters;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FName DataTableAmmoID = NAME_None;

	bool bIsAiming = false;
private:

	UPROPERTY(Replicated)
	FWeaponParameters CurrentWeaponParameters;

	bool bIsAdditional = false;
	

	UPROPERTY(Replicated)
	bool bIsReloading = false;
	
	bool bIsFiring = false;

	void ProcessRecoil(float DeltaTime);
	void ProcessRecoilback(float DeltaTime);
	
	void MakeShot();

	void OnShotTimerElapsed();

	float GetShotTimerInterval() const;
	float GetRecoilTimeInterval() const;
	
	float PlayAnimMontage(UAnimMontage* AnimMontage);
	void StopAnimMontage(UAnimMontage* AnimMontage, float BlendOutTime = 0.0f);

	float AccumulatedRecoilPitch = 0.0f;
	float AccumulatedRecoilYaw = 0.0f;
	int32 AccumulatedShots = 0;
	float RecoilRollbackTime = 0.0f;

	
	FTimerHandle ShotTimer;
	FTimerHandle ReloadTimer;
	FTimerHandle RecoilTimer;
	FTimerHandle RecoilRollbackTimer;
	
};
