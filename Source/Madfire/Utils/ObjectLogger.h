#pragma once

#include "Kismet/KismetSystemLibrary.h"

#include "ObjectLogger.generated.h"

/*
 This exists for managing of logging debug info inside UObject hierarchy 
(like AActor with it's components) and enabling/disabling logging for indivdiual children
in one place, while also providing possibility to display the name of object instance which
is doing logging, to separate all the different actors from one another
*/

USTRUCT(BlueprintType)
struct FLoggerInfo
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FString LoggerName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool Enabled;

    const UObject *Object;
};

USTRUCT(BlueprintType)
struct FObjectLogger
{
    GENERATED_BODY()

    // Call this when main object is initiated by the engine and pass this pointer
    void Init(const UObject* Object)
    {
        check(Object != nullptr);
        ObjectName = UKismetSystemLibrary::GetObjectName(Object);
        LogObject = Object;
    }

    void RegisterSubLogger(const UObject *Object, bool isEnabled) 
    {
        check(Object != nullptr);
        const auto Name = UKismetSystemLibrary::GetObjectName(Object);
        check(!Name.IsEmpty());
        for (const auto& i : Loggers)
        {
            check((i.Object != Object) && *FString::Printf(TEXT("FObjectLogger: %s already registered"), *Name));
        }
        Loggers.Add({ Name, isEnabled, Object });
    }

    bool IsEnabled(const UObject* Object)
    {
        if (!Enabled)
            return false;
        else if (LogObject == Object)
            return true;
        for (const auto& i : Loggers)
        {
            if (i.Object == Object)
                return i.Enabled;
        }
        // object not registered
        return false;
    }

    FString GetLogPrefix(const UObject *Object)
    {
        if (LogObject == Object)
            return ObjectName + ": ";
        for (const auto& i : Loggers)
        {
            if (i.Object == Object)
                return ObjectName + ": " + i.LoggerName + ": ";
        }
        check(false && "FObjectLogger: GetLogPrefix: logger is not registered");
        return FString();
    }
    /* 
      for constructing string literal Message analogous with UE_LOG
      use *FString::Printf(TEXT("Your text format"),...Args)
      like so: Log(this, *FString::Printf(TEXT("skill val %d"), number));
    */
    void Log(const UObject *Object, const FString& Message)
    {
        if (IsEnabled(Object))
            UE_LOG(LogTemp, Warning, TEXT("%s%s"), *GetLogPrefix(Object), *Message);
    }

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FString ObjectName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool Enabled;
    UPROPERTY(EditAnywhere, EditFixedSize, BlueprintReadWrite)
    TArray<FLoggerInfo> Loggers;

private:
    const UObject *LogObject;
};
