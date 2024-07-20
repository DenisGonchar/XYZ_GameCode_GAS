// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GCAttributeProgressBar.generated.h"

class UGCCharacterAttributeSet;
class UAttributeSet;
class UProgressBar;

UCLASS()
class GAMECODE_API UGCAttributeProgressBar : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetProgressPercantage(float Percentage);

	void SetAttibuteSet(UGCCharacterAttributeSet* AttributeSet_In);

	UFUNCTION(BlueprintCallable)
	float GetHealthPercent() const;
	
protected:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthProgressBar;

	TWeakObjectPtr<UGCCharacterAttributeSet> AttributeSet;
};
