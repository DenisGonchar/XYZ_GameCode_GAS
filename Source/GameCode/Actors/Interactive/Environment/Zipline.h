// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../InteractiveActor.h"
#include "Zipline.generated.h"

class UStaticMeshComponent;
class UAnimMontage;
class UBoxComponent;

UCLASS(Blueprintable)
class GAMECODE_API AZipline : public AInteractiveActor
{
	GENERATED_BODY()
	
public:
	AZipline();

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void BeginPlay() override;

	float GetZiplineHeight() const;

	UAnimMontage* GetAttachFromTopAnimMontage() const;


public:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* StartRailMechComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* EndRailMechComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* CableMechComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UBoxComponent* StartBoxComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UBoxComponent* EndBoxComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline Parameters")
	UAnimMontage* AttachFromAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline Parameters")
	float BoxDepthExtentY  = 32.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline Parameters")
	float BoxDepthExtentZ = 32.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline parameters")
	float StartRailHeight = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline parameters")
	float EndRailHeight = 40.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline parameters")
	FVector PositionStartRailMech = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline parameters")
	FVector PositionEndRailMesh = FVector::ZeroVector;

	UBoxComponent* GetZiplineInteractionBox() const;

	virtual void OnInteractionVolueOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	virtual void OnInteractionVolueOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

	
private:


};
