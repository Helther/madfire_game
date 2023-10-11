// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveSystem/SaveSystem.h"
#include <FileManagerGeneric.h>


void UMadfireSaveSystem::Initialize(FSubsystemCollectionBase &Collection)
{
    Super::Initialize(Collection);

    // Save Settings
}

void UMadfireSaveSystem::HandleStartingNewPlayer(AController *NewPlayer) {}

bool UMadfireSaveSystem::OverrideSpawnTransfrom(AController *NewPlayer) { return true; }

TArray<FString> UMadfireSaveSystem::GetSaveFiles() const
{
    TArray<FString> result;
    auto path = GetSavePath();
    //path.Append(TEXT("//save//"));

    auto isExist      = FPaths::DirectoryExists(path);
    auto &FileManager = FPlatformFileManager::Get().GetPlatformFile();


    if (!isExist)
    {
        FileManager.CreateDirectory(*path);
    }

    FileManager.FindFiles(result, *path, TEXT(SAVEGAME_POSTFIX));

    for (auto &name : result)
    {
        name = FPaths::GetBaseFilename(name);
    }

    if (result.IsEmpty())
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("No Saves Found"));
    }

    return result;
}

FString UMadfireSaveSystem::GetSavePath() const
{
    auto path = FPaths::ProjectSavedDir();
    path.Append(TEXT("SaveGames/"));
    return path;
}


void UMadfireSaveSystem::RemoveSave(FString InSlotName)
{
    auto path = GetSavePath(); 

    auto isExist      = FPaths::DirectoryExists(path);
    auto &FileManager = FPlatformFileManager::Get().GetPlatformFile();

    FString fullname = path + "/" + InSlotName + ".sav";

    FileManager.DeleteFile(*fullname);
    return;
}

void UMadfireSaveSystem::WriteSaveGame(FString InSlotName)
{
    auto save = Cast<UMFGameSave>(UGameplayStatics::CreateSaveGameObject(UMFGameSave::StaticClass()));

    TArray<AActor *> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AItemActor::StaticClass(), FoundActors);

    //save(FoundActors);
    for (auto item : FoundActors)
    {
        FMFObjectRecord objrec;
        objrec.ObjectName = item->GetName();
        UMadfireSerializer::SerializeObjectToByteArray(item, objrec.Data);

        save->ObjectDB.Add(std::move(objrec));
    }

    FoundActors.Empty();

    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacterPlayer::StaticClass(), FoundActors);

    for (auto item : FoundActors)
    {
        save->Player = Cast<ACharacterPlayer>(item);
    }

    UGameplayStatics::SaveGameToSlot(save, *InSlotName, 0);
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Game Saved."));
}

void UMadfireSaveSystem::LoadSaveGame(UObject *outer, FString InSlotName)
{
    auto save = Cast<UMFGameSave>(UGameplayStatics::LoadGameFromSlot(*InSlotName, 0));

    if (!save)
    {
        FString errorstr = "No Slot found " + InSlotName;
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, *errorstr);
        return;
    }

    auto &db = save->ObjectDB;
    TArray<AActor *> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);

    for (auto rec : db)
    {
        auto &name = rec.ObjectName;

        auto actor = FoundActors.FindByPredicate([name](AActor *a) { return a && a->GetName() == name; });

        if (actor) //Actor already exist
        {
            //(*actor)->K2_DestroyActor();
        }
        UMadfireSerializer::DeserializeUObjectFromByteArray(rec.Data, outer);
    }

    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Game Loaded."));
}

void UMadfireSaveSystem::WriteSaveGameN(USaveHeader *save, FString InSlotName)
{
    check(save);

    save->version = SaveVersion;

    UGameplayStatics::SaveGameToSlot(save, *InSlotName, 0);
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Game Saved."));
}

USaveHeader *UMadfireSaveSystem::LoadSaveGameN(FString InSlotName)
{
    auto save = Cast<USaveHeader>(UGameplayStatics::LoadGameFromSlot(*InSlotName, 0));

    auto sv = save->version;

    if (SaveVersion != sv)
    {
        //TODO: Do something
    }

    if (!save)
    {
        FString errorstr = "No Slot found " + InSlotName;
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, *errorstr);
        return nullptr;
    }

    return save;
}
