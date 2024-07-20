// Fill out your copyright notice in the Description page of Project Settings.


#include "Zipline.h"
#include "../../../GameCodeTypes.h"
#include <Components/SceneComponent.h>
#include <Components/StaticMeshComponent.h>
#include <Components/BoxComponent.h>
#include <Kismet/KismetMathLibrary.h>
#include "DrawDebugHelpers.h"


AZipline::AZipline()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ZiplineRoot"));

	StartRailMechComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TopRail"));
	StartRailMechComponent->SetupAttachment(RootComponent);

	EndRailMechComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BottomRail"));
	EndRailMechComponent->SetupAttachment(RootComponent);

	CableMechComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cable"));
	CableMechComponent->SetupAttachment(RootComponent);

	InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionValue"));
	InteractionVolume->SetupAttachment(RootComponent);
	InteractionVolume->SetCollisionProfileName(CollisionProfilePawnInteractionVolume);
	InteractionVolume->SetGenerateOverlapEvents(true);

	StartBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("StartBox"));
	StartBoxComponent->SetupAttachment(RootComponent);

	EndBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("EndTestBox"));
	EndBoxComponent->SetupAttachment(RootComponent);

}

/*
void AZipline::OnConstruction(const FTransform& Transform)
{	
	StartRailMechComponent->SetRelativeLocation(FVector(-ZiplineWigth * 0.5f, 0.0f, ZiplineHeight * 0.5f));
	EndRailMechComponent->SetRelativeLocation(FVector(ZiplineWigth * 0.5f, 0.0f, ZiplineHeight * 0.5f));
	CableMechComponent->SetRelativeLocation(FVector(0.0f, 0.0f, ZiplineHeight));

	UStaticMesh* TopRailMesh = StartRailMechComponent->GetStaticMesh();
	if (IsValid(TopRailMesh))
	{
		float MeshHeight = TopRailMesh->GetBoundingBox().GetSize().Z;
		if (!FMath::IsNearlyZero(MeshHeight))
		{
			StartRailMechComponent->SetRelativeScale3D(FVector(1.0f, 1.0f, ZiplineHeight /MeshHeight));
		}
	}

	UStaticMesh* BottomRailMesh = EndRailMechComponent->GetStaticMesh();
	if (IsValid(BottomRailMesh))
	{
		float MeshHeight = BottomRailMesh->GetBoundingBox().GetSize().Z;
		if (!FMath::IsNearlyZero(MeshHeight))
		{
			EndRailMechComponent->SetRelativeScale3D(FVector(1.0f, 1.0f, ZiplineHeight / MeshHeight));
		}
	}

	UStaticMesh* CableMesh = CableMechComponent->GetStaticMesh();
	if (IsValid(CableMesh))
	{
		float MeshWigth = CableMesh->GetBoundingBox().GetSize().X;
		if (!FMath::IsNearlyZero(MeshWigth))
		{
			CableMechComponent->SetRelativeScale3D(FVector(ZiplineWigth / MeshWigth, 1.0f , 1.0f));
		}
	}

	FRotator OritationRotation = UKismetMathLibrary::FindLookAtRotation(StartRailMechComponent->GetComponentLocation(), EndRailMechComponent->GetRelativeLocation());

	//DrawDebugSphere(World, OutHit.Location, Radius, 16, HitColor, false, DrawTime);
	
	FVector TopLocation = StartRailMechComponent->GetRelativeLocation();
	
	TestBoxComponent->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));
	TestBoxComponent->SetRelativeLocation(FVector(TopLocation.X, TopLocation.Y, TopLocation.Z * 2.0f));

	float BoxDepthExtent = GetZiplineInteractionBox()->GetScaledBoxExtent().X;
	GetZiplineInteractionBox()->SetBoxExtent(FVector(BoxDepthExtent, ZiplineWigth * 0.5f, ZiplineInteractionBoxZ));
	GetZiplineInteractionBox()->SetRelativeLocation(FVector(0.0f, 0.0f, ZiplineHeight - ZiplineInteractionBoxZ));	
	GetZiplineInteractionBox()->SetRelativeRotation(OritationRotation);

}

*/

void AZipline::OnConstruction(const FTransform& Transform)
{
	StartRailMechComponent->SetRelativeLocation(PositionStartRailMech);
	EndRailMechComponent->SetRelativeLocation(PositionEndRailMesh);
	
	UStaticMesh* TopRailMesh = StartRailMechComponent->GetStaticMesh();
	if (IsValid(TopRailMesh))
	{
		float MeshHeight = TopRailMesh->GetBoundingBox().GetSize().Z;
		if (!FMath::IsNearlyZero(MeshHeight))
		{
			StartRailMechComponent->SetRelativeScale3D(FVector(1.0f, 1.0f, StartRailHeight / MeshHeight));
			
		}
	}

	UStaticMesh* BottomRailMesh = EndRailMechComponent->GetStaticMesh();
	if (IsValid(BottomRailMesh))
	{
		float MeshHeight = BottomRailMesh->GetBoundingBox().GetSize().Z;
		if (!FMath::IsNearlyZero(MeshHeight))
		{
			EndRailMechComponent->SetRelativeScale3D(FVector(1.0f, 1.0f, EndRailHeight / MeshHeight));

		}
	}

	FVector StartLocation = StartRailMechComponent->GetRelativeLocation();
	FVector EndLocation = EndRailMechComponent->GetRelativeLocation();

	FVector StartRailUpPoint = FVector(StartLocation.X, StartLocation.Y, StartRailHeight * 0.5f + StartLocation.Z);
	FVector EndRailUpPoint = FVector(EndLocation.X, EndLocation.Y, EndRailHeight * 0.5f + EndLocation.Z);

	StartBoxComponent->SetRelativeLocation(StartRailUpPoint);
	EndBoxComponent->SetRelativeLocation(EndRailUpPoint);

	UStaticMesh* CableMesh = CableMechComponent->GetStaticMesh();
	if (IsValid(CableMesh))
	{
		float MeshWigth = CableMesh->GetBoundingBox().GetSize().X;
		if (!FMath::IsNearlyZero(MeshWigth))
		{
			
			float DistanceLocation = UKismetMathLibrary::Vector_Distance(StartRailUpPoint, EndRailUpPoint);

			CableMechComponent->SetRelativeScale3D(FVector(DistanceLocation / MeshWigth, 0.25f, 0.25f));
			
			FVector DirectionVector = FVector((StartRailUpPoint.X + EndRailUpPoint.X) / 2.0f, (StartRailUpPoint.Y + EndRailUpPoint.Y) / 2.0f, (StartRailUpPoint.Z + EndRailUpPoint.Z) / 2.0f);
			CableMechComponent->SetRelativeLocation(DirectionVector);

			FRotator FindRotation = UKismetMathLibrary::FindLookAtRotation(StartRailUpPoint, EndRailUpPoint);
			CableMechComponent->SetRelativeRotation(FindRotation);	
		}
	}

	float DistanceLocation = UKismetMathLibrary::Vector_Distance(StartRailMechComponent->GetRelativeLocation(), EndRailMechComponent->GetRelativeLocation());
	
	GetZiplineInteractionBox()->SetBoxExtent(FVector(DistanceLocation * 0.5f, BoxDepthExtentY , BoxDepthExtentZ ));
	GetZiplineInteractionBox()->SetRelativeLocation(CableMechComponent->GetRelativeLocation());
	GetZiplineInteractionBox()->SetRelativeRotation(CableMechComponent->GetRelativeRotation());

}

void AZipline::BeginPlay()
{
	Super::BeginPlay();


}

float AZipline::GetZiplineHeight() const
{
	//return ZiplineHeight;
	return StartRailHeight;

}

UAnimMontage* AZipline::GetAttachFromTopAnimMontage() const
{
	return AttachFromAnimMontage;
}

UBoxComponent* AZipline::GetZiplineInteractionBox() const
{
	return StaticCast<UBoxComponent*>(InteractionVolume);
}

void AZipline::OnInteractionVolueOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnInteractionVolueOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

}

void AZipline::OnInteractionVolueOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnInteractionVolueOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);

}
