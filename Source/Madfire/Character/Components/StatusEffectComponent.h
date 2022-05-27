// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AbilitySystem/StatusEffect.h"

#include "StatusEffectComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEffectQueueEvent, UStatusEffect *, EventEffect);

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UStatusEffectQueueComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
    UStatusEffectQueueComponent();

    UFUNCTION(BlueprintCallable, Category = "Status Effect Component")
    void Append(UStatusEffect *effect);

    UFUNCTION(BlueprintCallable, Category = "Status Effect Component")
    void Remove(UStatusEffect *effect);

    UFUNCTION(BlueprintPure, Category = "Status Effect Component")
    UStatusEffect *Find(FName effectName);

    UPROPERTY(BlueprintReadOnly, Category = "Status Effect Component")
    TArray<UStatusEffect*> EffectQueue;

    UPROPERTY(BlueprintAssignable)
    FEffectQueueEvent OnAppend;

    UPROPERTY(BlueprintAssignable)
    FEffectQueueEvent OnRemove;

    //TODO: Proxt Parameters for Effects
    //Proxy Parameters


protected:
	// Called when the game starts
	virtual void BeginPlay() override;


public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    void Update(float DeltaTime);

/*
    void AddParticle(UParticleSystem *particle, FName AttachPoint);
    void RemoveParticle(UParticleSystem *particle);
    */
};
