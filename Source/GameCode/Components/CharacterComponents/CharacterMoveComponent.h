// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharacterMoveComponent.generated.h"

class ALedgePlatform;
class UGCBaseCharacterMovementComponent;
class AGCBaseCharacter;
class ULedgeDetectorComponent;

USTRUCT(BlueprintType)
struct FMantlingSetting
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UAnimMontage* MantlingMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UAnimMontage* FPMantlingMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UCurveVector* MantlingCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float AnimationCorrectionXY = 65.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float AnimationCorrectionZ = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MaxHeight = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MinHeight = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MaxHeightStartTime = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MinHeightStartTime = 0.5f;

};

UENUM(BlueprintType)
enum class EMoveRockClimbing : uint8
{
	Nome,
	Up,
	Down
	
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAMECODE_API UCharacterMoveComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCharacterMoveComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnPlayrCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);


	//Slide
	void Slide();

	void StartSlide(float HalfHeightAbjust);
	void EndSlide(float HalfHeightAbjust);

	bool CanSlide();

	UFUNCTION()
	void OnRep_InSlide(bool bWasSliding);

	//WallRun
	void WallRun();
	bool CanWallRun();

	//Zipline
	void ClimbZipline(float Value);
	void InteractWithZipline();
	const class AZipline* GetAvailableZipline() const;

	//Ladder
	void ClimbLadderUp(float Value);
	void InteractWithLadder();
	const class ALadder* GetAvailableLadder() const;

	//Mantle
	 UFUNCTION(BlueprintCallable)
	 void Mantle(bool bForce = false);
	
	 bool CanMantle() const;
	 
	 UFUNCTION()
	 void OnRep_IsMantlong(bool bWasMantling);

	//RockClimbing
	void RockClimbing(EMoveRockClimbing Move);
	void StartRockClimbing();
	
	bool CanRockClimbing() const;
	
public:
	FVector BaseTranslationOffset = FVector::ZeroVector;

	 //Mantle
	 UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Movement | Mantling")
	 FMantlingSetting HighMantleSettings;
	
	 UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Movement | Mantling")
	 FMantlingSetting LowMantleSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Movement | Mantling", meta = (ClampMix = 0.0f, UIMin = 0.0f))
	float LowMantleMaxHeight = 125.0f;
	
	 UPROPERTY(ReplicatedUsing = OnRep_IsMantlong)
	 bool bIsMantling;

	//Slide
	UPROPERTY(ReplicatedUsing= OnRep_InSlide)
	bool bIsSliding;

	//RockClimbing
	UPROPERTY()
	bool bIsRockClimbing;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Character | Movement | Rock Climbing")
	class UAnimMontage* StartClimbingMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Character | Movement | Rock Climbing")
	class UAnimMontage* UpClimbingMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Character | Movement | Rock Climbing")
	class UAnimMontage* DownClimbingMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Character | Movement | Rock Climbing")
	float UpClimbingMontageTime = 2.0f;	

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Character | Movement | Rock Climbing")
	float DownClimbingMontageTime = 2.0f;	

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Character | Movement | Rock Climbing")
	bool bIsDrawDebugUp = false;	

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Character | Movement | Rock Climbing")
	bool bIsDrawDebugDonw = false;	
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Character | Movement | Rock Climbing")
	float UpClimbingOffset = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Character | Movement | Rock Climbing")
	float DownClimbingOffset = 50.0f;
	
	UPROPERTY()
	ALedgePlatform* CurrentLedgeActor;
	
protected:
	TSoftObjectPtr<class AGCBaseCharacter> CachedBaseCharacter;

	UPROPERTY()
	UGCBaseCharacterMovementComponent* CurrentBaseCharacterMovementComponent;

	UPROPERTY()
	ULedgeDetectorComponent* CurrentLedgeDetectorComponent;
	
	//Mantle
	const FMantlingSetting& GetMantlingSetting(float LedgeHeight) const;

	FTimerHandle ClimbingTimer;
};
