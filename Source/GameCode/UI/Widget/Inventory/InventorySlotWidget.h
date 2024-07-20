// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventorySlotWidget.generated.h"

struct FInventorySlot;
class UImage;

UCLASS()
class GAMECODE_API UInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeItemSlot(FInventorySlot& InventorySlot);
	void UpdateView();
	void SetItemIcon(UTexture2D* Icon);

	void UpdateCount(int32 NewCount);
	
protected:
	UPROPERTY(meta = (BindWidget))
	UImage* ImageItemIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 AmmoCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsVisibilityAmmo = false;
	
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

private:
	FInventorySlot* LinkedSlot;

};
