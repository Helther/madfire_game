// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Rules/GameRules.h"
#include "Runtime/Engine/Classes/Particles/ParticleSystemComponent.h"
#include "AbilitySystem/AbstractAbility.h"
#include "StatusEffect.generated.h"

UENUM()
enum class EStatusEffectType
{
    Undefinded,
    Physical,
    Magic,
    Poison,
    Disease,
    Curse,
};


UENUM(Meta = (Bitflags))
enum class EStatusEffectParameters
{
    Infinity,
    Negative,
};
ENUM_CLASS_FLAGS(EStatusEffectParameters);
/**
 * 
 */
UCLASS(BlueprintType, Blueprintable, MinimalAPI, Abstract)
class UStatusEffect : public UObject
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Status Effect Parameters")
    void EffectSetup(class ACharacterBase *causer);

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Status Effect Parameters")
    void OnApplying(class ACharacterBase *Target);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Status Effect Parameters")
    void OnUpdate(class ACharacterBase *Target, float DeltaTime);

    void OnUpdate_Implementation(class ACharacterBase *Target, float DeltaTime);

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Status Effect Parameters")
    void OnEnding(class ACharacterBase *Target);

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Status Effect Parameters")
    bool Combine(UStatusEffect *anotherOne);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(Bitmask, BitmaskEnum = "EStatusEffectParameters"), Category = "Status Effect Parameters")
    int32 EffectParameters;

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Meta = (ExposeOnSpawn="true"), Category = "Status Effect Parameters")
    class ACharacterBase *Causer;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status Effect Parameters")
    EStatusEffectType EffectType;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status Effect Parameters")
    EMagicSchool MagicSchool;

    //UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effect Parameters")
    //int32 EffectID;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Status Effect Parameters")
    FName EffectID;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Status Effect Parameters")
    float CurrentDuration;

    //UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect Parameters")
    //int32 MaxDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effect Parameters")
    uint8 CurrentStack;

    //UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect Parameters")
    //uint8 MaxStack;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Status Effect Parameters")
    UTexture2D *EffectIcon;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Status Effect Parameters")
    FText Description;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Status Effect Parameters")
    FText EffectName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effect Parameters")
    bool bShownInWidget;

    UFUNCTION(BlueprintCallable, meta = (DeterminesOutputType = "ActorClass"), Category = "Status Effect System")
    static AActor *SpawnStatusEffectActor(AActor *Owner, TSubclassOf<AActor> ActorClass, FAbilityActorSpawnParameters param, FName Name);
};




