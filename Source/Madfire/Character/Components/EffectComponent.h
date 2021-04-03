// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AbilitySystem/EffectBase.h"
#include "EffectComponent.generated.h"


UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MADFIRE_API UEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEffectComponent();

    UFUNCTION(BlueprintCallable, Category = "Effect Component")
    void Append(UEffectBase *effect);

    UFUNCTION(BlueprintCallable, Category = "Effect Component")
    void Remove(UEffectBase *effect);

    UFUNCTION(BlueprintPure, Category = "Effect Component")
    UEffectBase *Find(FName effectName);

    UPROPERTY(BlueprintReadOnly, Category = "Effect Component")
    TArray<UEffectBase*> _effectQueue;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;


public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    void Update(float DeltaTime);
	
};
