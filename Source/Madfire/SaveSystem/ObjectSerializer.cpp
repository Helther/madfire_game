// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveSystem/ObjectSerializer.h"
#include "Items/ItemBase.h"
#include "Character/CharacterPlayer.h"


FArchive &operator<<(FArchive &Ar, FSerializedObject &Obj)
{
    Ar << Obj.Class;
    Ar << Obj.OuterPath;
    Ar << Obj.Data;
    return Ar;
}

FArchive &operator<<(FArchive &Ar, FObjectSnapshot &Snapshot)
{
    Ar << Snapshot.Object;
    Ar << Snapshot.Subobjects;
    Ar << Snapshot.Classes;
    Ar << Snapshot.ActorTransform;
    return Ar;
}

FObjectSnapshotProxyArchive::FObjectSnapshotProxyArchive(FArchive &InnerAr, FSharedArchiveData &Shared) : 
    FNameAsStringProxyArchive(InnerAr),
    Shared(Shared)
{
    ArNoDelta = true;
    ArIsSaveGame = true;
}

FString FObjectSnapshotProxyArchive::GetSerializeObjectPath(UObject *Obj)
{
    FString Path = Obj->GetPathName();
    if (Path.StartsWith(Shared.RootPath))
        Path = FString("$ROOT$") + Path.Mid(Shared.RootPath.Len());

    return Path;
}

FArchive &FObjectSnapshotProxyArchive::operator<<(UObject *&Obj)
{
    if (IsLoading())
    {
        FString SerializedPath;
        InnerArchive << SerializedPath;
        Obj = DeserializeObjectPath(SerializedPath);
    }
    else
    {
        //Check if we already visited this
        if (auto SerializedPath = Shared.VisitedObjects.Find(Obj))
        {
            InnerArchive << *SerializedPath;
        }
        else if (Obj)
        {
            FString Path = Obj->GetPathName();

            if (Obj->IsA<UClass>()) //class special case
            {
                Shared.Snapshot.Classes.Add(Path);
            }
            else if (Path.StartsWith(Shared.RootPath)) //not sure if we should check Outer chain or this
            {
                // Generic opt-in event
                if (UFunction *Func = Obj->FindFunction("OnSerialize"))
                    Obj->ProcessEvent(Func, nullptr);

                Path = GetSerializeObjectPath(Obj);
                Shared.VisitedObjects.Add(Obj, Path);

                FSerializedObject &Data = Shared.Snapshot.Subobjects.Emplace(Path, FSerializedObject());
                Data.Class = Obj->GetClass();
                Data.OuterPath = GetSerializeObjectPath(Obj->GetOuter());

                // Serialize sub object in its own archive
                FMemoryWriter SubobjectWriter(Data.Data, true);
                FObjectSnapshotProxyArchive SubobjectAr(SubobjectWriter, Shared);
                Obj->Serialize(SubobjectAr);
            }
            else
            {
                // Remove Level part from path
                // Not sure if this is a good idea, external refs should probaly be nulled if we change level...
                Path = Obj->GetPathName(Obj->GetTypedOuter<ULevel>());
            }

            InnerArchive << Path;

            Shared.VisitedObjects.Add(Obj, Path);
        }
        else
        {
            FString Path = TEXT("None");
            InnerArchive << Path;
            Shared.VisitedObjects.Add(Obj, Path);
        }
    }
    return *this;
}

UObject *FObjectSnapshotProxyArchive::DeserializeObjectPath(const FString &SerializedPath)
{
    // Special case for serialized class refs : use LoadObject instead of FindObject
    if (Shared.Snapshot.Classes.Contains(SerializedPath))
    {
        return LoadObject<UClass>(nullptr, *SerializedPath);
    }
    else if (auto KnownObj = Shared.DeserializedObjects.Find(SerializedPath)) // Already visited this
    {
        return *KnownObj;
    }
    else if (auto Data = Shared.Snapshot.Subobjects.Find(SerializedPath))
    {
        // Recursively ensure Outer is deserialized first
        UObject *Outer = DeserializeObjectPath(Data->OuterPath);
        if (!Outer)
        {
            UE_LOG(LogTemp, Warning,
                   TEXT("Could not find or deserialize Outer for %s (Outer path = %s)"),
                   *SerializedPath, *Data->OuterPath);
            return nullptr;
        }

        if (auto KnownObj2 = Shared.DeserializedObjects.Find(SerializedPath))
        {
            return *KnownObj2;
        }

        UObject *Obj = nullptr;

        //We might already exist as a default subobject (component)
        FString FullPath = SerializedPath.Replace(TEXT("$ROOT$"), *Shared.RootPath);
        if (FullPath.StartsWith(Outer->GetPathName()))
        {
            Obj = FindObject<UObject>(nullptr, *FullPath, false);
            //Class check
            if (Obj && Obj->GetClass() != Data->Class)
                Obj = nullptr;
        }
        DeserializeObject(Obj, Shared, SerializedPath, *Data, Outer, false);
        return Obj;
    }
    else if (SerializedPath.StartsWith("$ROOT$"))
    {
        UE_LOG(LogTemp, Warning,
               TEXT("Found $ROOT$ ref not in Subobjects nor in DeserializedObjects: %s"),
               *SerializedPath);
        return nullptr;
    }
    else
    {
        return FindObject<UObject>(nullptr, *SerializedPath, false);
    }
}

void FObjectSnapshotProxyArchive::DeserializeObject(UObject *&Obj,
                                                    FSharedArchiveData &Shared,
                                                    const FString &SerializedPath,
                                                    FSerializedObject &Data,
                                                    UObject *Outer, bool bIsRoot)
{
    UActorComponent *ComponentToRegister = nullptr;

    if (!Obj)
    {
        if (Shared.OuterActor && Data.Class->IsChildOf<UActorComponent>())
        {
            Obj = ComponentToRegister = Shared.OuterActor->AddComponentByClass(Data.Class, true, FTransform::Identity, true);
        }
        else
        {
            FStaticConstructObjectParameters Parms(Data.Class);
            Parms.Outer = Outer;
            Obj         = StaticConstructObject_Internal(Parms);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[DeserializeObject] Deserializing into %s"), *SerializedPath);

    if (bIsRoot)
        Shared.RootPath = Obj->GetPathName();

    // Don't deserialize this object again
    Shared.DeserializedObjects.Add(SerializedPath, Obj);

    //Deserialize into found/create object - this will recursively deserialize subobjects referenced by UProperties
    FMemoryReader ObjectReader(Data.Data, true);
    FObjectSnapshotProxyArchive ObjectAr(ObjectReader, Shared);

    Obj->Serialize(ObjectAr);

    UE_LOG(LogTemp, Log, TEXT("[DeserializeObject] Deserialized %s"), *SerializedPath);

    // Register new component if applicable
    if (ComponentToRegister)
    {
        if (auto SceneComp = Cast<USceneComponent>(ComponentToRegister))
            Shared.OuterActor->FinishAddComponent(ComponentToRegister, true, SceneComp->GetRelativeTransform());
        else
            Shared.OuterActor->FinishAddComponent(ComponentToRegister, true, FTransform::Identity);
    }
}

void UMadfireSerializer::SerializeObjectToByteArray(UObject *Obj, TArray<uint8> &FinalBytes)
{
    // Generic opt-in event
    if (UFunction *Func = Obj->FindFunction("OnSerialize"))
        Obj->ProcessEvent(Func, nullptr);

    FSharedArchiveData Shared;
    Shared.RootObj = Obj;
    Shared.Snapshot.Object.Class = Obj->GetClass();
    Shared.RootPath = Obj->GetPathName();
    Shared.VisitedObjects.Add(Obj, TEXT("$ROOT$")); //Add root obj ref here so we don't try to serialize it as a subobject

    // Reserve container to avoid reallocation
    TArray<UObject *> Subobjects;
    GetObjectsWithOuter(Obj, Subobjects);
    Shared.Snapshot.Subobjects.Reserve(Subobjects.Num());

    if (Obj->GetClass()->IsChildOf<AActor>())
        Shared.Snapshot.ActorTransform = Cast<AActor>(Obj)->GetActorTransform();
    

    // Serialize root object - subobjects will be recursively serialized into Snapshot.Subobjects
    FMemoryWriter ObjectWriter(Shared.Snapshot.Object.Data, true);
    FObjectSnapshotProxyArchive ObjectAr(ObjectWriter, Shared);
    Obj->Serialize(ObjectAr);

    // Serialize intermedite structure
    FMemoryWriter FinalWriter(FinalBytes, true);
    FObjectAndNameAsStringProxyArchive FinalAr(FinalWriter, true);
    FinalAr << Shared.Snapshot;
}

UObject *UMadfireSerializer::DeserializeUObjectFromByteArray(TArray<uint8> &AllBytes, UObject *InOuter)
{
    check(InOuter && "Invaid outer");

    // Deserialize structure
    FMemoryReader StructuralReader = FMemoryReader(AllBytes, true);
    FObjectAndNameAsStringProxyArchive StructuralAr(StructuralReader, true);
    FSharedArchiveData Shared;
    StructuralAr << Shared.Snapshot;

    // Spawn root object (and its defaults subobjects)
    if (Shared.Snapshot.Object.Class->IsChildOf<AActor>())
    {
        FActorSpawnParameters Params;
        Params.bNoFail = true;
        //Params.bDeferConstruction = true;
        Shared.RootObj = Shared.OuterActor = InOuter->GetWorld()->SpawnActor<AActor>(Shared.Snapshot.Object.Class, Params);        
        //Actor->FinishSpawning(Actor->GetTransform());
        Shared.OuterActor->SetActorTransform(Shared.Snapshot.ActorTransform);          
    }
    else 
    {
        // Non-Actor case is now factorized in FObjectSnapshotProxyArchive::DeserializeObject
        Shared.RootObj = nullptr;
        Shared.OuterActor = Cast<AActor>(InOuter);
    }

    if (!InOuter)
        InOuter = GetTransientPackage();

    // Deserialize root object - subobjects will be recusively deserialized from snapshot.Subobjects
    FObjectSnapshotProxyArchive::DeserializeObject(Shared.RootObj, Shared, "$ROOT$", Shared.Snapshot.Object, InOuter, true);

    // Delete subobjects that were not in the snapshot (ie. default components removed at runtime)
    TArray<UObject *> Subobjects;
    GetObjectsWithOuter(Shared.RootObj, Subobjects);
    TArray<UObject *> VisitedObjects; 
    Shared.DeserializedObjects.GenerateValueArray(VisitedObjects);

    for (auto Obj : Subobjects)
    {
        if (Obj && !VisitedObjects.Contains(Obj))
        {
            if (auto Comp = Cast<UActorComponent>(Obj))
                Comp->DestroyComponent();
            else
                Obj->MarkAsGarbage();
        }
    }

    //Fix up actor components
    if (Shared.RootObj == Shared.OuterActor)
        Shared.OuterActor->UpdateComponentTransforms();

    // Generic opt-in event
    for (auto& Pair : Shared.DeserializedObjects)
    {
        if (UFunction *Func = Pair.Value->FindFunction("OnDeserialize"))
            Pair.Value->ProcessEvent(Func, nullptr);
    }

    return Shared.RootObj;
}


void UMadfireSerializer::SerializeUObject(UObject *Obj, FString FileName) 
{
    TArray<uint8> FinalBytes;
    SerializeObjectToByteArray(Obj, FinalBytes);
    // Save to file
    FString FilePath = FPaths::ProjectSavedDir() / FileName;
    bool retval      = FFileHelper::SaveArrayToFile(FinalBytes, *FilePath);

    if (retval)
    {
        UE_LOG(LogTemp, Log, TEXT("Saved to %s"), *FileName);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Failed to save to %s"), *FileName);
    }

}

UObject *UMadfireSerializer::DeserilizeUObject(FString FileName, UObject *InOuter)
{
    // Read from file
    TArray<uint8> AllBytes;
    FString FilePath = FPaths::ProjectSavedDir() / FileName;

    if (!FFileHelper::LoadFileToArray(AllBytes, *FilePath))
    {
        UE_LOG(LogTemp, Log, TEXT("Failed to open file %s"), *FileName);
        return nullptr;
    }

    return DeserializeUObjectFromByteArray(AllBytes, InOuter);
}



void UMadfireSerializer::SerializeActor(AActor *actor, FActorRecord &record) 
{
    UE_LOG(LogTemp, Display, TEXT("Saving Actor: %s"), *actor->GetName());    

    FMemoryWriter MemoryWriter = FMemoryWriter(record.Data, true);
    FSaveGameArchive arch      = FSaveGameArchive(MemoryWriter);

    //Saving actor
    record.Class           = actor->GetClass();
    record.Name            = actor->GetFName();
    record.Transform       = actor->GetTransform();
    record.LifeSpan        = actor->GetLifeSpan();

    actor->Serialize(arch);


    //saving components
    TArray<UActorComponent *> comps = TArray<UActorComponent *>();
    actor->GetComponents(comps);

    for (auto comp : comps)
    {
        int idx = record.Components.Emplace();
        FComponentRecord &compRec = record.Components[idx];

        compRec.Class = comp->GetClass();
        compRec.Name  = comp->GetFName();

        UE_LOG(LogTemp, Display, TEXT("Saving Actor Component: %s"), *comp->GetName());    

        auto sc = Cast<USceneComponent>(comp);

        if (sc != nullptr)
        {
            compRec.Transform = sc->GetRelativeTransform();
            auto pc           = Cast<UPrimitiveComponent>(comp);
            if (pc != nullptr)
            {
                compRec.Velocity = pc->GetPhysicsLinearVelocity();
                compRec.AngularVelocity = pc->GetPhysicsAngularVelocityInDegrees();
            }
        }   

        auto mc = Cast<UMovementComponent>(comp);
        if (mc != nullptr)
            compRec.Velocity = mc->Velocity;

        FMemoryWriter MemoryWriter = FMemoryWriter(compRec.Data, true);
        FSaveGameArchive compArch  = FSaveGameArchive(MemoryWriter);
        comp->Serialize(compArch);
    }
}

void UMadfireSerializer::DeserializeActor(AActor *actor, const FActorRecord &record) 
{
    FMemoryReader Reader  = FMemoryReader(record.Data, true);
    FSaveGameArchive arch = FSaveGameArchive(Reader);
    actor->Serialize(arch);

    LoadActorFinish(actor, record);
}

AActor *UMadfireSerializer::LoadActorBegin(UWorld *world, FActorRecord &rec) 
{
    UE_LOG(LogTemp, Display, TEXT("Loading Actor: %s"), *rec.Name.ToString());

    FActorSpawnParameters SpawnParam;

    //SpawnParam.Name = rec.Name;
    SpawnParam.bDeferConstruction = true;
    SpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    //FActorSpawnUtils::MakeUniqueActorName()

    auto a = world->SpawnActor<AActor>(rec.Class, rec.Transform, SpawnParam);
    if (!a)
    {
        UE_LOG(LogTemp, Display, TEXT("Couldn't Load the Actor: %s"), *rec.Name.ToString());
        rec.SpawnedActor = nullptr;
        return nullptr;
    }

    a->SetLifeSpan(rec.LifeSpan);

    FMemoryReader Reader = FMemoryReader(rec.Data, true);
    FSaveGameArchive arch = FSaveGameArchive(Reader);
    a->Serialize(arch);

    rec.SpawnedActor = a;
    return a;
}

void UMadfireSerializer::LoadActorFinish(AActor *a, const FActorRecord &rec)
{
    TArray<UActorComponent *> comp = TArray<UActorComponent *>();

    a->GetComponents(comp);

    for (auto& compRec : rec.Components)
    {
        for (auto c : comp)
        {
            if (c->GetFName() == compRec.Name)
            {
                FMemoryReader CompReader = FMemoryReader(compRec.Data, true);
                FSaveGameArchive compArch = FSaveGameArchive(CompReader);

                c->Serialize(compArch);

                auto sc = Cast<USceneComponent>(c);
                if (sc != nullptr)
                {
                    sc->SetRelativeTransform(compRec.Transform, false, nullptr, ETeleportType::TeleportPhysics);

                    auto pc = Cast<UPrimitiveComponent>(c);
                    if (pc != nullptr && pc->IsAnySimulatingPhysics())
                    {
                        pc->SetPhysicsLinearVelocity(compRec.Velocity);
                        pc->SetPhysicsAngularVelocityInDegrees(compRec.AngularVelocity);
                        pc->GetBodyInstance()->UpdateMassProperties();
                    }
                }

                auto cc = Cast<UCombatComponent>(c);
                if (cc)
                {
                    auto ch = Cast<ACharacterCreature>(a);
                    if (ch)
                    {
                        ch->CombatComponent = cc;
                    }
                }

                auto mc = Cast<UMovementComponent>(c);
                if (mc != nullptr)
                {
                    mc->Velocity = compRec.Velocity;
                    mc->UpdateComponentVelocity();
                }
                break;
            }
        }
    }

    a->FinishSpawning(rec.Transform);
}
