// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/LimbComponet/GCCharacterLimbComponent.h"
#include "GCCombinedSkeletalMeshComponent.h"
#include "Actors/Limb/GCLimbActor.h"

void UGCCharacterLimbComponent::OnRegister()
{
	Super::OnRegister();

	CombinedSkeletalMesh = GetOwner()->FindComponentByClass<UGCCombinedSkeletalMeshComponent>();
	if (CombinedSkeletalMesh.IsValid())
	{
		for (const auto& limb : Limbs)
		{
			CombinedSkeletalMesh->SetBodyPartBoneName(limb.BodyPart, limb.BoneName);
		}

	}
}

void UGCCharacterLimbComponent::BeginPlay()
{
	Super::BeginPlay();

	if (CombinedSkeletalMesh == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] CombinedSkeletalMesh is null"), *GetNameSafe(this));

	}
}

void UGCCharacterLimbComponent::TakeDamage(float Damage, AController* InstigatedBy, AActor* DamageCauser, UPrimitiveComponent* FHitComponent, FName BoneName)
{
	if (auto SkeletalMeshComponent = Cast<USkeletalMeshComponent>(FHitComponent))
	{
		if (auto limb = FindLimbData(SkeletalMeshComponent, BoneName))
		{
			if (limb->Health > 0.0f)
			{
				TakeLimbDamage(*limb, Damage);
				if (limb->Health <= 0.0f && CombinedSkeletalMesh.IsValid())
				{
					CombinedSkeletalMesh->RemoveBodyParts({ limb->BodyPart });

					SpawnLimbActor(*limb, SkeletalMeshComponent->GetComponentTransform());
				}
			}
		}
	}
}

const FLimdData* UGCCharacterLimbComponent::FindLimbDataByBone(FName BoneName) const
{
	if (CombinedSkeletalMesh.IsValid())
	{
		return const_cast<UGCCharacterLimbComponent*>(this)->FindLimbData(CombinedSkeletalMesh.Get(), BoneName);

	}

	return nullptr;
}

void UGCCharacterLimbComponent::TakeLimbDamage(FLimdData& Limb, float Damage)
{
	if (Limb.Health > 0.0f)
	{
		Limb.Health = FMath::Max(0.f, Limb.Health - Damage);

	}
}

void UGCCharacterLimbComponent::SpawnLimbActor(const FLimdData& Limb, const FTransform& Transform)
{
	if (const auto bodyPartPtr = CombinedSkeletalMesh->GetBodyParts().Find(Limb.BodyPart))
	{
		if (LimbActorClass != nullptr && bodyPartPtr->Mesh != nullptr)
		{
			const auto LimbActor = GetWorld()->SpawnActorDeferred<AGCLimbActor>(LimbActorClass, Transform, GetOwner(), nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

			LimbActor->PrototyActor = GetOwner();
			LimbActor->GetMeshComponent()->SetSkeletalMesh(bodyPartPtr->Mesh);
			LimbActor->FinishSpawning(Transform);

		}
	}
}

FLimdData* UGCCharacterLimbComponent::FindLimbData(USkeletalMeshComponent* MeshComponent, FName BoneName)
{
	do
	{
		for (auto& limb : Limbs)
		{
			if (limb.BoneName == BoneName)
			{
				return &limb;
			}
		}
		BoneName = MeshComponent->GetParentBone(BoneName);

	} while (BoneName != NAME_None);

	return nullptr;
}
