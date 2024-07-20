// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RangeWeaponItem.h"
#include "BowWeaponItem.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnBowstringChanged, bool, float);

class UBowstringWidget;

UENUM()
enum class EBowstringType : uint8
{
	Tension,
	Hold
};

UCLASS()
class GAMECODE_API ABowWeaponItem : public ARangeWeaponItem
{
	GENERATED_BODY()

public:
	ABowWeaponItem();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	void UpdateBowstring(float DeltaTime);
	
	virtual void StartFire() override;
	virtual void StopFire() override;
	
	virtual void StartAim() override;
	virtual void StopAim() override;

	void StartBowstring();
	void StopBowstring();
	
	float GetTension() const { return Tension; };

	void StartTimers();
	void ClearTimers();
	float GetTimeTimerReadyToFire();
	
	void ShakeAim();
	void ReadyToFire();

	void StartRecoil();
	
	bool bBowstring() { return bIsBowstring; }

	EBowstringType GetBowstringType() const { return BowstringType; };
	
public:
	FOnBowstringChanged OnBowstringChangedEvent;
	
protected:
	void Shot();
	void OnAimTimerElapsed();
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Time", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float AfterTimeCanShoot = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Time", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float TimeTension = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Time", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float TimeReadyToFire = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Time", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float TimeShakeAim = 8.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Time", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float TimeReload = 8.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Anim")
	UAnimMontage* StartTension;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Arrow")
	UStaticMeshComponent* ArrowMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Arrow")
	FName NameArrowSocket = NAME_None;

private:
	float Tension = 0.0f;
	bool bIsBowstring = false;

	float TimeShotFire = 0.0f;

	EBowstringType BowstringType = EBowstringType::Tension;
	
	FTimerHandle FireTimer;
	FTimerHandle TimerBowstring; // 1.0 cek
	FTimerHandle TimerReadyToFire; // 10.0 cek
	FTimerHandle TimerShakeAim; //8.0 cek
	FTimerHandle TimerRecoil; // 1.0 cek
};
