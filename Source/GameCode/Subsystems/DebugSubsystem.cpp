// Fill out your copyright notice in the Description page of Project Settings.
#include "DebugSubsystem.h"
#include "Kismet/GameplayStatics.h"

bool UDebugSubsystem::IsCategoryEnabled(const FName& CategoryName) const
{
	const bool* bIsEnabled = EnabledDebugCategories.Find(CategoryName);

	return bIsEnabled != nullptr && *bIsEnabled;
}

void UDebugSubsystem::IncreaseTimeDilation()
{
	if (CurrentDilationStep == (DilationSteps.Num() - 1))
	{
		return;
	}

	++CurrentDilationStep;
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(),DilationSteps[CurrentDilationStep]);
}

void UDebugSubsystem::DecreaseTimeDilation()
{
	if (CurrentDilationStep == 0)
	{
		return;
	}

	--CurrentDilationStep;
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), DilationSteps[CurrentDilationStep]);

}

void UDebugSubsystem::EnableDebugCategory(const FName& CategoryName, bool bIsEnabled)
{
	EnabledDebugCategories.FindOrAdd(CategoryName);
	EnabledDebugCategories[CategoryName] = bIsEnabled;

}

