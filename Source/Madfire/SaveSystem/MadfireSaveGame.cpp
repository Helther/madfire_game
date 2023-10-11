// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveSystem/MadfireSaveGame.h"
#include "Items/ItemBase.h"
#include "SaveInterface.h"
#include "Character/PlayerCharController.h"

void USmGameSave::SaveWorld(UWorld* WorldToSave, AActor *Player)
{
    check(WorldToSave);
    UE_LOG(LogTemp, Display, TEXT("Start Saving World"));

    ObjectsToSave.Empty();
    ActorsToSave.Empty();
    TArray<AActor *> FoundActors;
    //UGameplayStatics::GetAllActorsOfClass(WorldToSave, AActor::StaticClass(), FoundActors);
    UGameplayStatics::GetAllActorsWithInterface(WorldToSave, USaveable::StaticClass(), FoundActors);

    PlayerControllerRotator = Cast<APawn>(Player)->GetController()->GetControlRotation();
    UMadfireSerializer::SerializeActor(Player, PlayerRecord);

    for (auto actor : FoundActors)
    {
        FActorRecord record;
        //ActorToRecord
        if (actor == Player)
            continue;
        UMadfireSerializer::SerializeActor(actor, record);
        ActorsToSave.Add(record);
    }
}

void USmGameSave::LoadWorld(UWorld* WorldToLoad)
{ 
    check(WorldToLoad);
    UE_LOG(LogTemp, Display, TEXT("Start Loading World"));


    //auto oldplayer = UGameplayStatics::GetPlayerCharacter(WorldToLoad, 0);
    //APlayerController *oldctrl = UGameplayStatics::GetPlayerController(WorldToLoad, 0);
    //oldctrl->Pause();    
    //oldplayer->K2_DestroyActor();
    TArray<AActor *> FoundActors;
    //UGameplayStatics::GetAllActorsOfClass(WorldToLoad, AActor::StaticClass(), FoundActors);
    UGameplayStatics::GetAllActorsWithInterface(WorldToLoad, USaveable::StaticClass(), FoundActors);        

    for (auto actor : FoundActors)
    {
        UE_LOG(LogTemp, Log, TEXT("Destroying actor %s"), actor->GetFName());
        //actor->Destroy();                
        actor->K2_DestroyActor();
    }
    GEngine->ForceGarbageCollection();

    //auto player = UMadfireSerializer::LoadActorBegin(WorldToLoad, PlayerRecord);

    for (auto record : ActorsToSave)
    {
        UMadfireSerializer::LoadActorBegin(WorldToLoad, record);
    }

    //APlayerController *ctrl = UGameplayStatics::GetPlayerController(WorldToLoad, 0);
    //UMadfireSerializer::LoadActorFinish(PlayerRecord.SpawnedActor, PlayerRecord);

    //Cast<APlayerCharController>(oldctrl)->OnPossess(Cast<APawn>(player));
    //oldctrl->SetControlRotation(PlayerControllerRotator);
    //oldctrl->UpdateRotation(123456);        

    for (auto record : ActorsToSave)
        if (record.SpawnedActor)
            UMadfireSerializer::LoadActorFinish(record.SpawnedActor, record);

    //oldctrl->UnFreeze();
}

AActor* USmGameSave::PopActor(TArray<AActor*> ActorList, FName ActorName)
{
    for (int i = 0; i < ActorList.Num(); ++i)
    {
        auto actor = ActorList[i];
        if (actor && actor->GetFName().Compare(ActorName) == 0) 
        {
            ActorList[i] = nullptr;
            return actor;
        }
    }

    return nullptr;
}