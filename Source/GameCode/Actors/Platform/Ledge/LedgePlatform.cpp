// Fill out your copyright notice in the Description page of Project Settings.


#include "LedgePlatform.h"


// Sets default values
ALedgePlatform::ALedgePlatform()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	
	PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
	PlatformMesh->SetupAttachment(RootComponent);

	PositionForCharacter = CreateDefaultSubobject<USceneComponent>(TEXT("PositionForCharacter"));
	PositionForCharacter->SetupAttachment(Root);
}
