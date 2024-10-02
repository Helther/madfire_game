#pragma once

#include "CoreMinimal.h"
#include "Engine.h"
#include "AbilityComponent.h"
#include "GameFramework/Actor.h"
#include "TraceFunctional/TraceFunctional.h"
#include "AbilityActionComponent.h"
#include "AbilityTargetSelectionComponent.generated.h"

/*
Нужные компонентов:
	Проджектай (Хоминг Проджектайл)
	Область (Компонент Область)
	СамПерсонаж - тут просто прослойка которая будет выполнять все экшоны на персонаже,
				  который создал этот компонент.
	Луч до первого таргета (Луч на определённое растояние)
	
*/


DECLARE_DELEGATE_ThreeParams(FAbilityExecuteEvent, const TArray<AActor*> &, FVector, FRotator);

USTRUCT(BlueprintType)
struct FAbilityTargetInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability System")
    FVector Position;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability System")
    FVector End;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability System")
    FQuat Rotation;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability System")
    TArray<UAbilityAction*> AbilityActions;
};

UCLASS(EditInlineNew, Blueprintable, BlueprintType)
class UAbilityTargetSelection : public UAbilityComponent
{
    GENERATED_BODY()

public:
    // UAbilityComponent interface
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Ability System")
    void Execute(FAbilityTargetInfo info);
    void Execute_Implementation(FAbilityTargetInfo info) {}        
    
public:
    void ComponentInit(UAbstractAbility *parent) override;
    bool ComponentSetup(ACharacter *Component) override;
};


UCLASS(Abstract, EditInlineNew, Blueprintable, BlueprintType)
class UAbilityAreaSelection : public UAbilityTargetSelection
{

    GENERATED_BODY()

public:
    void TraceActors(FAbilityTargetInfo info, float Radius, FCollisionShape CollisionShape);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability System")
    bool bIgnoreOwner = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability System")
    bool bIgnoreBlocks = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability System")
    bool bIgnoreTouches = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability System")
    FName OwnerTag = "NullOwnerTag";
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability System")
    FName TraceTag = "NullTraceTag";
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability System")
    TArray<AActor*> ActorsToIgnore;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability System")
    FCollisionResponseContainer CollisionResponse;
};


UCLASS(EditInlineNew, Blueprintable, BlueprintType)
class UAbilitySphereSelection : public UAbilityAreaSelection
{
    GENERATED_BODY()

public:
    void Execute_Implementation(FAbilityTargetInfo info);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability System")
    float Radius = 1.f;
};

UCLASS(EditInlineNew, Blueprintable, BlueprintType)
class UAbilityBoxSelection : public UAbilityAreaSelection
{
    GENERATED_BODY()

public:
    void Execute_Implementation(FAbilityTargetInfo info);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability System")
    FVector BoxHalfExtent;
};

UCLASS(EditInlineNew, Blueprintable, BlueprintType)
class UAbilityCapsuleSelection : public UAbilityAreaSelection
{
    GENERATED_BODY()

public:
    void Execute_Implementation(FAbilityTargetInfo info);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability System")
    float CapsuleRadius;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability System")
    float CapsuleHalfHeight;
};

UCLASS(Abstract, Blueprintable, BlueprintType)
class AAbilityActor : public AActor
{
    GENERATED_BODY()

public:
    AAbilityActor();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(BlueprintReadOnly, Category = "Ability System")    
    FAbilityTargetInfo TargetInfo;

    UPROPERTY(BlueprintReadOnly, Category = "Ability System")
    class UAbilityActorSpawner *UAbilityActorSpawner;
};

UCLASS(EditInlineNew, Blueprintable, BlueprintType)
class UAbilityActorSpawner : public UAbilityTargetSelection
{
    GENERATED_BODY()

public:
    void Execute_Implementation(FAbilityTargetInfo info);
            
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability System")
    FTransform SpawnPoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability System")
    TSubclassOf<AAbilityActor> ActorClass;
};