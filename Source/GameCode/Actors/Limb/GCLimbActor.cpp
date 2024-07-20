// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Limb/GCLimbActor.h"

// Sets default values
AGCLimbActor::AGCLimbActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TransformComponent = CreateDefaultSubobject<USceneComponent>("TransformComponent");
	SetRootComponent(TransformComponent);

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>("MeshComponent");
	MeshComponent->SetupAttachment(TransformComponent);

}

// Called when the game starts or when spawned
void AGCLimbActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGCLimbActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

