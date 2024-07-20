// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DebugSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class GAMECODE_API UDebugSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	bool IsCategoryEnabled(const FName& CategoryName) const;
	
	UFUNCTION(Exec)
	void IncreaseTimeDilation();
	
	
	void DecreaseTimeDilation();


private:
	UFUNCTION(Exec)
	void EnableDebugCategory(const FName& CategoryName, bool bIsEnabled);

	TMap<FName, bool> EnabledDebugCategories;
	
	TArray<float> DilationSteps = {0.1f, 0.25f, 0.5f, 1.0f, 2.0f, 4.0f, 10.0f};

	int CurrentDilationStep = 3;

};
