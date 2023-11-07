// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/UserDefinedStruct.h"
#include "GameFramework/DamageType.h"
#include "../Rules/GameRules.h"
#include "Engine/EngineTypes.h"

#include "Damage.generated.h"

//TODO: Heal type and Mechanix

/**
 * 
 */

UCLASS(BlueprintType)
class UDamageBase : public UDamageType
{
    GENERATED_BODY()
public:
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
    EDamageType Type;
};


UCLASS(BlueprintType)
class UAbsoluteDamage : public UDamageBase
{
    GENERATED_BODY()
public:

};


UCLASS(BlueprintType, Blueprintable)
class UPhysicDamage : public UDamageBase
{
    GENERATED_BODY()
public:

};

USTRUCT(BlueprintType)
struct FForceRadialDamageParams : public FRadialDamageParams
{
    GENERATED_BODY()

    FForceRadialDamageParams() :
        FRadialDamageParams(),
        BaseForce(0.f),
        MinimumForce(0.f),
        ForceFalloff(0.f)
    {}
    FForceRadialDamageParams(float InBaseForce, float InBaseDamage, float InInnerRadius, float InOuterRadius, float InForceFalloff, float InDamageFalloff) :
        FRadialDamageParams(InBaseDamage, InInnerRadius, InOuterRadius, InDamageFalloff),
        BaseForce(InBaseForce),
        MinimumForce(0.f),
        ForceFalloff(InForceFalloff)
    {}
    FForceRadialDamageParams(float InBaseForce, float InBaseDamage, float InMinimumForce, float InMinimumDamage, float InInnerRadius, float InOuterRadius, float InForceFalloff, float InDamageFalloff) :
        FRadialDamageParams(InBaseDamage, InMinimumDamage, InInnerRadius, InOuterRadius, InDamageFalloff),
        BaseForce(InBaseForce),
        MinimumForce(InMinimumForce),
        ForceFalloff(InForceFalloff)
    {}
    FForceRadialDamageParams(float InBaseDamage, float InBaseForce, float InRadius) :
        FRadialDamageParams(InBaseDamage, InRadius),
        BaseForce(InBaseForce),
        MinimumForce(0.f),
        ForceFalloff(0.f)
    {}

    /** Max damage done */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=RadialDamageParams)
	float BaseForce;

	/** Damage will not fall below this if within range */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=RadialDamageParams)
	float MinimumForce;

    /** Describes amount of exponential damage falloff */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=RadialDamageParams)
	float ForceFalloff;
};

UCLASS(BlueprintType, Blueprintable)
class UMagicDamage : public UDamageBase
{
    GENERATED_BODY()
public:
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
    EMagicSchool MagicSchool;
};


struct FForceDamageEvent : public FDamageEvent
{
    explicit FForceDamageEvent(TSubclassOf<class UDamageType> InDamageTypeClass, float force)
        : FDamageEvent(InDamageTypeClass), 
        Force(force) 
    {}

    /** ID for this class. NOTE this must be unique for all damage events. */
	static const int32 ClassID = 10;
    
    float Force;
};

struct FForcePointDamageEvent : public FPointDamageEvent
{
    explicit FForcePointDamageEvent() : 
        FPointDamageEvent(),
        Force(0.0f) 
    {}
    explicit FForcePointDamageEvent(float InDamage, const FHitResult &InHitInfo, const FVector &InShotDir, TSubclassOf<class UDamageType> InDamageTypeClass, float force) : 
        FPointDamageEvent(InDamage, InHitInfo, InShotDir, InDamageTypeClass), 
        Force(force) 
    {}

    /** ID for this class. NOTE this must be unique for all damage events. */
	static const int32 ClassID = 11;
    
    float Force;
};

struct FForceRadialDamageEvent : public FRadialDamageEvent
{
    explicit FForceRadialDamageEvent() : 
        FRadialDamageEvent(), 
        Force(0.0f) 
    {}

    /** ID for this class. NOTE this must be unique for all damage events. */
	static const int32 ClassID = 12;
    
    float Force;
};


UCLASS(MinimalAPI)
class UForceDamageHelper : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Game|Damage", meta=(WorldContext="WorldContextObject", AutoCreateRefTerm="IgnoreActors"))
	static bool ApplyForceRadialDamage(const UObject* WorldContextObject, float BaseForce, float BaseDamage, const FVector& Origin, float DamageRadius, TSubclassOf<class UDamageType> DamageTypeClass, const TArray<AActor*>& IgnoreActors, AActor* DamageCauser = NULL, AController* InstigatedByController = NULL, bool bDoFullDamage = false, ECollisionChannel DamagePreventionChannel = ECC_Visibility);
	
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Game|Damage", meta=(WorldContext="WorldContextObject", AutoCreateRefTerm="IgnoreActors"))
	static bool ApplyForceRadialDamageWithFalloff(const UObject* WorldContextObject, float BaseForce,  float MinimumForce, float BaseDamage, float MinimumDamage, const FVector& Origin, float DamageInnerRadius, float DamageOuterRadius, float ForceFalloff, float DamageFalloff, TSubclassOf<class UDamageType> DamageTypeClass, const TArray<AActor*>& IgnoreActors, AActor* DamageCauser = NULL, AController* InstigatedByController = NULL, ECollisionChannel DamagePreventionChannel = ECC_Visibility);
	
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Game|Damage")
	static float ApplyForcePointDamage(AActor* DamagedActor, float BaseDamage, const FVector& HitFromDirection, const FHitResult& HitInfo, AController* EventInstigator, AActor* DamageCauser, TSubclassOf<class UDamageType> DamageTypeClass);

    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Game|Damage")
	static float ApplyForceDamage(AActor* DamagedActor, float BaseForce, float BaseDamage, AController* EventInstigator, AActor* DamageCauser, TSubclassOf<class UDamageType> DamageTypeClass);
};
