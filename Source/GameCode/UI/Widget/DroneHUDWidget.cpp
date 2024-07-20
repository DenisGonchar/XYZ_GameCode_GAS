// Fill out your copyright notice in the Description page of Project Settings.


#include "DroneHUDWidget.h"

#include "Blueprint/WidgetTree.h"
#include "ReticleWidget.h"

UReticleWidget* UDroneHUDWidget::GetReticalWidget()
{
	return WidgetTree->FindWidget<UReticleWidget>(ReticleWidgetName);
}
