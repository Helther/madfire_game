// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GameFramework/GameState.h"
#include "Character/CharacterPlayer.h"
#include "ObjectSerializer.h"
#include "MadfireSaveGame.generated.h"

/**
 * Тут анриаловский SaveGame в котором хранятся все рекорды объектов
 */


UCLASS(Blueprintable, BlueprintType)
class UMFGameSave : public USaveGame
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMFObjectRecord> ObjectDB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FSavingRecord MasterSaveRecordCopy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class ACharacterPlayer *Player;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<uint8> SerialisedDataMasterObject;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSet<FString> ExternallySerialisedUObjectNames;
};

UCLASS(Blueprintable)
class USaveHeader : public USaveGame
{
    GENERATED_BODY()

public:
    UPROPERTY()
    int version = 0;
};

UCLASS(Blueprintable, BlueprintType)
class USmGameSave : public USaveHeader
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FActorRecord PlayerRecord;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator PlayerControllerRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FObjectRecord> ObjectsToSave;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FActorRecord> ActorsToSave;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UGameInstance *CurrentState;

    UFUNCTION(BlueprintCallable, Category = "SaveSystem")
    void SaveWorld(UWorld *WorldToSave, AActor *Player);

    UFUNCTION(BlueprintCallable, Category = "SaveSystem")
    void LoadWorld(UWorld *WorldToLoad);

private:
    AActor *PopActor(TArray<AActor *> ActorList, FName ActorName);
};