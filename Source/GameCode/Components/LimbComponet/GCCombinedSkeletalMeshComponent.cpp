#include "Components/LimbComponet/GCCombinedSkeletalMeshComponent.h"

#include "SkeletalMeshMerge.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/Skeleton.h"

#include "GameCodeTypes.h"



static void ToMergeParams(const TArray<FSkelMeshMergeSectionMapping_BP>& InSectionMappings, TArray<FSkelMeshMergeSectionMapping>& OutSectionMappings)
{
	if (InSectionMappings.Num() > 0)
	{
		OutSectionMappings.AddUninitialized(InSectionMappings.Num());
		for (int32 i = 0; i < InSectionMappings.Num(); ++i)
		{
			OutSectionMappings[i].SectionIDs = InSectionMappings[i].SectionIDs;
		}
	}
};
static void ToMergeParams(const TArray<FSkelMeshMergeUVTransformMapping>& InUVTransformsPerMesh, TArray<FSkelMeshMergeUVTransforms>& OutUVTransformsPerMesh)
{
	if (InUVTransformsPerMesh.Num() > 0)
	{
		OutUVTransformsPerMesh.Empty();
		OutUVTransformsPerMesh.AddUninitialized(InUVTransformsPerMesh.Num());
		for (int32 i = 0; i < InUVTransformsPerMesh.Num(); ++i)
		{
			TArray<TArray<FTransform>>& OutUVTransforms = OutUVTransformsPerMesh[i].UVTransformsPerMesh;
			const TArray<FSkelMeshMergeUVTransform>& InUVTransforms = InUVTransformsPerMesh[i].UVTransformsPerMesh;
			if (InUVTransforms.Num() > 0)
			{
				OutUVTransforms.Empty();
				OutUVTransforms.AddUninitialized(InUVTransforms.Num());
				for (int32 j = 0; j < InUVTransforms.Num(); j++)
				{
					OutUVTransforms[i] = InUVTransforms[i].UVTransforms;
				}
			}
		}
	}
};
USkeletalMesh* UMeshMergeFunctionLibrary::MergeMeshes(const FSkeletalMeshMergeParams& Params)
{
	TArray<USkeletalMesh*> MeshesToMergeCopy = Params.MeshesToMerge;
	MeshesToMergeCopy.RemoveAll([](USkeletalMesh* InMesh)
		{
			return InMesh == nullptr;
		});
	if (MeshesToMergeCopy.Num() < 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("Must provide multiple valid Skeletal Meshes in order to perform a merge."));
		return nullptr;
	}
	EMeshBufferAccess BufferAccess = Params.bNeedsCpuAccess ?
		EMeshBufferAccess::ForceCPUAndGPU :
		EMeshBufferAccess::Default;
	TArray<FSkelMeshMergeSectionMapping> SectionMappings;
	TArray<FSkelMeshMergeUVTransforms> UvTransforms;
	ToMergeParams(Params.MeshSectionMappings, SectionMappings);
	ToMergeParams(Params.UVTransformsPerMesh, UvTransforms);

	USkeletalMesh* BaseMesh = NewObject<USkeletalMesh>();
	if (Params.Skeleton && Params.bSkeletonBefore)
	{
		BaseMesh->SetSkeleton(Params.Skeleton);

	}

	FSkeletalMeshMerge Merger(BaseMesh, MeshesToMergeCopy, SectionMappings, Params.StripTopLODS, BufferAccess, UvTransforms.GetData());
	if (!Merger.DoMerge())
	{
		UE_LOG(LogTemp, Warning, TEXT("Merge failed!"));
		return nullptr;
	}
	if (Params.Skeleton && !Params.bSkeletonBefore)
	{
		BaseMesh->SetSkeleton(Params.Skeleton);
	}

	return BaseMesh;
}

void UGCCombinedSkeletalMeshComponent::OnRegister()
{
	Super::OnRegister();

	for (const auto& bodyPart : BodyParts)
	{
		if (bodyPart.Value.Mesh != nullptr)
		{
			CurrentBodyParts.Add(bodyPart.Key);
		}

	}

	TArray<FMeshBodyPart> meshBodyParts;
	BodyParts.GenerateValueArray(meshBodyParts);

	GenerateMesh(meshBodyParts);

}

void UGCCombinedSkeletalMeshComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UGCCombinedSkeletalMeshComponent::RemoveBodyParts(const TArray<EBodyPart>& PartsToRemove)
{
	if (PartsToRemove.Num() != 0)
	{
		TArray<FMeshBodyPart> meshParts;

		const int32 oldBodyPartsNumbers = CurrentBodyParts.Num();

		for (auto bodyPart : PartsToRemove)
		{
			CurrentBodyParts.Remove(bodyPart);

		}

		if (oldBodyPartsNumbers != CurrentBodyParts.Num())
		{

			for (const auto& bodyPart : CurrentBodyParts)
			{
				meshParts.Add(BodyParts.FindChecked(bodyPart));

			}

			GenerateMesh(meshParts);

		}

	}
}

const TMap<EBodyPart, FMeshBodyPart>& UGCCombinedSkeletalMeshComponent::GetBodyParts() const
{
	return BodyParts;
}

void UGCCombinedSkeletalMeshComponent::SetBodyPartBoneName(EBodyPart BodyPart, FName BoneName)
{
	if (const auto bodyPartPtr = BodyParts.Find(BodyPart))
	{
		bodyPartPtr->BoneName = BoneName;
	}
}

void UGCCombinedSkeletalMeshComponent::GenerateMesh(const TArray<FMeshBodyPart>& MeshParts)
{
	FSkeletalMeshMergeParams SkeletalMeshMergeParams;
	SkeletalMeshMergeParams.Skeleton = Skeleton;
	SkeletalMeshMergeParams.bSkeletonBefore = true;
	SkeletalMeshMergeParams.bNeedsCpuAccess = true;

	for (const auto bodyPart : MeshParts)
	{
		if (bodyPart.Mesh != nullptr)
		{
			SkeletalMeshMergeParams.MeshesToMerge.Add(bodyPart.Mesh);
		}
	}

	if (const auto skeletalMesh = UMeshMergeFunctionLibrary::MergeMeshes(SkeletalMeshMergeParams))
	{
		skeletalMesh->SetPhysicsAsset(PhysicsAsset);
		SetSkeletalMesh(skeletalMesh);
	}

	for (const auto& bodyPart : BodyParts)
	{
		if (!bodyPart.Value.BoneName.IsNone() && !CurrentBodyParts.Contains(bodyPart.Key))
		{
			TermBodiesBelow(bodyPart.Value.BoneName);
		}
	}
}
