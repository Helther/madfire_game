// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Rules/GameRules.h"
#include "Runtime/Engine/Classes/Particles/ParticleSystemComponent.h"
#include "EffectBase.generated.h"

UENUM()
enum class EEffectType
{
    Undefinded		UMETA(DisplayName = "Undefinded"),
    Physical		UMETA(DisplayName = "Physical"),
    Magic           UMETA(DisplayName = "Magic"),
    Poison  		UMETA(DisplayName = "Poison"),
    Disease 		UMETA(DisplayName = "Disease"),
    Curse   		UMETA(DisplayName = "Curse"),
};


UENUM(Meta = (Bitflags))
enum class EEffectParameters
{
    Infinity    UMETA(DisplayName = "Infinity"),
    Negative    UMETA(DisplayName = "Negative"),
};
ENUM_CLASS_FLAGS(EEffectParameters);
/**
 * 
 */
UCLASS(BlueprintType, Blueprintable, MinimalAPI, Abstract)
class UEffectBase : public UObject
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Effect Parameters")
    void EffectSetup(class ACharacterBase *causer);

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Effect Parameters")
    void OnApplying(class ACharacterBase *Target);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Effect Parameters")
    void OnUpdate(class ACharacterBase *Target, float DeltaTime);

    void OnUpdate_Implementation(class ACharacterBase *Target, float DeltaTime);

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Effect Parameters")
    void OnEnding(class ACharacterBase *Target);

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Effect Parameters")
    bool Combine(UEffectBase *anotherOne);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(Bitmask, BitmaskEnum = "EEffectParameters"), Category = "Effect Parameters")
    int32 EffectParameters;

    UPROPERTY(BlueprintReadWrite, Category = "Effect Paramaeters")
    ACharacterBase *Causer;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect Paramaeters")
    EEffectType EffectType;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect Paramaeters")
    EMagicSchool MagicSchool;

    //UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effect Parameters")
    //int32 EffectID;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effect Parameter")
    FName EffectID;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effect Parameters")
    float CurrentDuration;

    //UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect Parameters")
    //int32 MaxDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Parameters")
    uint8 CurrentStack;

    //UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect Parameters")
    //uint8 MaxStack;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effect Parameters")
    UTexture2D *EffectIcon;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effect Parameters")
    FText Description;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effect Parameters")
    FText EffectName;

};
