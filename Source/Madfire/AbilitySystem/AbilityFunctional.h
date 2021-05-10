#pragma once

#include "AbilitySystem/AbstractAbility.h"
#include "AbilitySystem/AbilityComponents/AbilityLocationSelectionComponent.h"
#include "AbilitySystem/AbilityComponents/AbilityAimComponent.h"
#include "AbilitySystem/AbilityComponents/AbilityActorSelectionComponent.h"
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


UCLASS(MinimalAPI, Abstract)
class UAbilityFunctional : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Meta=(DefaultToSelf="Parent", DeterminesOutputType="AbilityClass"), Category = "AbilitySystem")
    static UAbstractAbility *SpawnAbility(UObject *Parent, TSubclassOf<UAbstractAbility> AbilityClass, FName Name = NAME_None);

    UFUNCTION(BlueprintCallable, Meta=(DefaultToSelf="Parent", DeterminesOutputType= "TargetSelectionClass"), Category = "AbilitySystem")
    static UAbilityComponent *CreateComponent(UAbstractAbility *Parent, TSubclassOf<UAbilityComponent> ComponentClass, FName Name = NAME_None);

    UFUNCTION(BlueprintCallable, meta = (DeterminesOutputType = "ActorClass"), Category = "AbilitySystem")
    static AActor *SpawnAbilityActor(AActor *Owner, TSubclassOf<AActor> ActorClass, FAbilityActorSpawnParameters param, FName Name = NAME_None);
};
