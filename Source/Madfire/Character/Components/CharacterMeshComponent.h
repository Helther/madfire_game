// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "SkeletalMeshMerge.h"
#include "CharacterMeshComponent.generated.h"


USTRUCT(BlueprintType)
struct FSkeletalMeshMergeParams
{
    GENERATED_BODY()

        FSkeletalMeshMergeParams() :
        StripTopLODS(0),
        bNeedsCpuAccess(0),
        bSkeletonBefore(0),
        Skeleton(nullptr)
    {}

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TArray<FSkelMeshMergeSectionMapping> MeshSectionMappings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TArray<FSkelMeshMergeUVTransformMapping> UVTransformsPerMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TArray<USkeletalMesh*> MeshesToMerge;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int32 StripTopLODS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        uint32 bNeedsCpuAccess:1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        uint32 bSkeletonBefore:1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
        class USkeleton* Skeleton;
};

UCLASS()
class UMeshMergeFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category = "Mesh Merge", meta = (UnsafeDuringActorConstruction = "true"))
        static class USkeletalMesh* MergeMeshes(const FSkeletalMeshMergeParams& Params);
};


/*
 * Manages character mesh asset, updates it when equipment changes.
 * Merges equipment with BaseMesh and sets BaseSkeleton to the resulting mesh.
 * Don't forget to set Base assets!
 */
UCLASS()
class MADFIRE_API UCharacterMeshComponent : public USkeletalMeshComponent
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Component Update")
        void Update();
    UFUNCTION(BlueprintCallable, Category = "Component Update")
        void SetUpdate(bool Update) { IsDirty = Update; }
    UFUNCTION(BlueprintCallable, Category = "Component Update")
        bool IsUpdateNeeded() { return IsDirty; }

    void BeginPlay() override;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USkeletalMesh* BaseMesh;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USkeleton* BaseSkeleton;

    bool IsDirty = false;
};
