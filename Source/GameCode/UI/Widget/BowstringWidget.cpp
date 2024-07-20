// Fill out your copyright notice in the Description page of Project Settings.


#include "BowstringWidget.h"


void UBowstringWidget::SetBowstring(bool NewBowstring, float NewTension)
{
	bIsBowstring = 	NewBowstring;
	Tension = NewTension;
	
}
