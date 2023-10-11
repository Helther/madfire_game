// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ObjectSerializer.h"
#include "MadfireSaveGame.h"
#include "SaveSystem.generated.h"

/**
 * Система сохранений, тут вся работа с файлами сохранений
 */


#define SAVEGAME_POSTFIX "sav"


UCLASS(meta = (DisplayName = "SaveGame System"))
class UMadfireSaveSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    void Initialize(FSubsystemCollectionBase &Collection) override;

    // Resotre serialized data from PlayerState into player
    void HandleStartingNewPlayer(AController *NewPlayer);

    //Restore spawn transform using stored data per PlayerState after being fully initialized
    UFUNCTION(BlueprintCallable)
    bool OverrideSpawnTransfrom(AController *NewPlayer);

    //If something change then ++
    UPROPERTY()
    uint32 SaveVersion;

    UFUNCTION(BlueprintCallable, Category = "Save System")
    void WriteSaveGame(FString InSlotName = "testsave");
    // Load from disk
    UFUNCTION(BlueprintCallable, Category = "Save System")
    void LoadSaveGame(UObject *outer, FString InSlotName = "testsave");

    UFUNCTION(BlueprintPure, Category = "Save System")
    TArray<FString> GetSaveFiles() const;

    UFUNCTION(BlueprintPure, Category = "Save System")
    FString GetSavePath() const;

    UFUNCTION(BlueprintCallable, Category = "Save System")
    void RemoveSave(FString InSlotName = "testsave");

    // Новый интерфейс 
    UFUNCTION(BlueprintCallable, Category = "Save System")
    void WriteSaveGameN(USaveHeader *save, FString InSlotName = "testsave");
    // Load from disk
    UFUNCTION(BlueprintCallable, Category = "Save System")
    USaveHeader *LoadSaveGameN(FString InSlotName = "testsave");
};