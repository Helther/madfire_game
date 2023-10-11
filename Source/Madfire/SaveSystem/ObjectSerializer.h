// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Containers/Array.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Containers/UnrealString.h"
#include "Misc/DateTime.h"
#include "GameFramework/SaveGame.h"
#include "Engine.h"
#include "ObjectSerializer.generated.h"

/**
 * “ут расположена система сериализации Object -> Record и обратно
 */


struct FSerializedObject
{
    UClass *Class;
    FString OuterPath;
    TArray<uint8> Data;    
};  

struct FObjectSnapshot
{
    FSerializedObject Object;
    TMap<FString, FSerializedObject> Subobjects;
    TSet<FString> Classes;
    FTransform ActorTransform;
    FName UniqueName;
};

struct FSharedArchiveData
{
    UObject *RootObj;
    AActor *OuterActor;

    FObjectSnapshot Snapshot;
    FString RootPath;

    TMap<UObject*, FString> VisitedObjects;
    TMap<FString, UObject *> DeserializedObjects;
};

static FArchive &operator<<(FArchive &Ar, FSerializedObject &Obj);

static FArchive &operator<<(FArchive &Ar, FObjectSnapshot &Snapshot);

struct FObjectSnapshotProxyArchive : public FNameAsStringProxyArchive
{
    FObjectSnapshotProxyArchive(FArchive &InnerAr, FSharedArchiveData &Shared);

    UObject *DeserializeObjectPath(const FString &SerializedPath);
    FString GetSerializeObjectPath(UObject *Obj);
    virtual FArchive &operator<<(UObject *&Obj) override;
    static void DeserializeObject(UObject *&Obj, FSharedArchiveData &Shared, const FString &SerializedPath, FSerializedObject &Data, UObject *Outer, bool bIsRoot);

    FSharedArchiveData &Shared;
};

USTRUCT(BlueprintType)
struct FSavingRecord
{
    GENERATED_BODY()

    FSavingRecord() {}

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SaveName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString LastTimePlayedStr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastTimePlayed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastTimePlayed_UTC;
};

class FSaveGameArchive : public FObjectAndNameAsStringProxyArchive
{
public:
    FSaveGameArchive(FArchive &InInnerArchive) :
        FObjectAndNameAsStringProxyArchive(InInnerArchive, true)
    {
        ArIsSaveGame = true;
    }
};

USTRUCT(BlueprintType)
struct FMFObjectRecord
{
    GENERATED_BODY()

    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
    TArray<uint8> Data;

    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
    FString ObjectName;
};

struct FWCSaveGameArchive : public FObjectAndNameAsStringProxyArchive
{
    FWCSaveGameArchive(FArchive &InInnerArchive)
      : FObjectAndNameAsStringProxyArchive(InInnerArchive, true)
    {
        ArIsSaveGame = true;
        ArNoDelta    = true;
    }
};

USTRUCT(BlueprintType)
struct FObjectRecord
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite)
    UClass *Class = nullptr; //Class that this object is

    UPROPERTY(BlueprintReadWrite)
    UObject *Outer = nullptr; //save the outer for object so they get loaded back in the corrent

    UPROPERTY(BlueprintReadWrite)
    int32 OuterID; //save the outer used for object they get loaded back in the correct

    UPROPERTY(BlueprintReadWrite)
    bool bActor; //if the outer is an actor otherwise will be UObject

    UPROPERTY(BlueprintReadWrite)
    FName Name;

    UPROPERTY(BlueprintReadWrite)
    TArray<uint8> Data; // serialized data for all UProperties that are 'SaveGame' enabled

    UPROPERTY(BlueprintReadWrite)
    FTransform Transform; // Spawn location if it's an actor

    UPROPERTY(BlueprintReadWrite)
    UObject *Self = nullptr; //this is for loading only, store a pointer for the loaded object here
};

USTRUCT(BlueprintType)
struct FComponentRecord
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite)
    UClass *Class;

    UPROPERTY(BlueprintReadWrite)
    FName Name;

    UPROPERTY(BlueprintReadWrite)
    TArray<uint8> Data;

    UPROPERTY(BlueprintReadWrite)
    FTransform Transform;

    UPROPERTY(BlueprintReadWrite)
    FVector Velocity;

    UPROPERTY(BlueprintReadWrite)
    FVector AngularVelocity;
};

USTRUCT(BlueprintType)
struct FActorRecord
{
    GENERATED_BODY()

public:
    AActor *SpawnedActor;

    UPROPERTY(BlueprintReadWrite)
    UClass *Class = nullptr; //Class that this object is

    UPROPERTY(BlueprintReadWrite)
    FName Name;

    UPROPERTY(BlueprintReadWrite)
    TArray<uint8> Data; // serialized data for all UProperties that are 'SaveGame' enabled

    UPROPERTY(BlueprintReadWrite)
    FTransform Transform; // Spawn location if it's an actor

    UPROPERTY(BlueprintReadWrite)
    TArray<FComponentRecord> Components;

    UPROPERTY(BlueprintReadWrite)
    float LifeSpan;
};


UCLASS()
class UMadfireSerializer : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    static void SerializeObjectToByteArray(UObject *Obj, TArray<uint8> &FinalBytes);

    UFUNCTION(BlueprintCallable)
    static void SerializeUObject(UObject *Obj, FString FileName);

    UFUNCTION(BlueprintCallable)
    static UObject *DeserilizeUObject(FString FileName, UObject *InOuter);

    UFUNCTION(BlueprintCallable)
    static UObject *DeserializeUObjectFromByteArray(TArray<uint8> &AllBytes, UObject *InOuter);

    UFUNCTION(BlueprintCallable)
    static void SerializeActor(AActor *actor, FActorRecord &record);

    UFUNCTION(BlueprintCallable)
    static void DeserializeActor(AActor *actor, const FActorRecord &record);

    UFUNCTION(BlueprintCallable)
    static AActor *LoadActorBegin(UWorld *world, FActorRecord &rec);

    UFUNCTION(BlueprintCallable)
    static void LoadActorFinish(AActor *a, const FActorRecord &rec);

};