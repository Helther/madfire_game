#pragma once

#include "AbilitySystem/AbstractAbility.h"
#include "GameFramework/Actor.h"


#include "AbilityFunctional.generated.h"

USTRUCT(BlueprintType)
struct FAbilityActorSpawnParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    FVector Location;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    FRotator Rotation;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    FVector Scale;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    class APawn *Instigator = nullptr; //
};


UCLASS(MinimalAPI)
class UAbilityFunctional : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Meta=(DefaultToSelf="Parent"), Category = "AbilitySystem")
    static UAbstractAbility *SpawnAbility(UObject *Parent, TSubclassOf<UAbstractAbility> AbilityClass, FName Name = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "AbilitySystem")
    static AActor *SpawnAbilityActor(AActor *Owner, TSubclassOf<AActor> ActorClass, FAbilityActorSpawnParameters param, FName Name = NAME_None);
};
