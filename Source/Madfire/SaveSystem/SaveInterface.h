// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "SaveInterface.generated.h"




UINTERFACE(Blueprintable, MinimalAPI)
class USaveable : public UInterface
{
    GENERATED_BODY()
};


class ISaveable
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintImplementableEvent, Category = "SaveSystem")
    void Postload();

    UFUNCTION(BlueprintImplementableEvent, Category = "SaveSystem")
    void Presave();

    UFUNCTION(BlueprintImplementableEvent, Category = "SaveSystem")
    bool DoNotSave();
};
