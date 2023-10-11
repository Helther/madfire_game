// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../Interactable/Interactable.h"
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Rules/GameRules.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/DataTable.h"
#include "SaveSystem/SaveInterface.h"
#include "ItemBase.generated.h"

struct FItemActorEquipmentSlotNames
{
    static const FName WeaponLoadedAmmo;  // Specifies the place for attaching loaded ammo to a weapon
    static const FName EquipSocket;  // Place for attaching equipped items
    static const FName WeaponEquipSocket;  // Place for attaching a held weapon actor
};

USTRUCT(BlueprintType)
struct FInventoryItemInfo : public FTableRowBase
{

    GENERATED_USTRUCT_BODY()

    FInventoryItemInfo() :
        ItemIcon(nullptr),
        Description(FText::FromString("Undefined")),
        Name(FText::FromString("Undefined"))
    {}

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    UTexture2D *ItemIcon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    FText Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    int32 MaxCharges = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    TSubclassOf<class UItem> AbilityClass;
};


UCLASS(config = Game, EditInlineNew, BlueprintType, Blueprintable)
class UItem : public UObject, public IInteractableInterface
{
    GENERATED_BODY()

public:
    //Interaction Interface
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactable Interface")
    bool OnInteract(EInteractType InteractType, ACharacter *controller);
    bool OnInteract_Implementation(EInteractType InteractType, ACharacter *controller);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactable Interface")
    UPARAM(meta=(Bitmask, BitmaskEnum = EInteractType)) int32 GetInteractFlags() const;
    UPARAM(meta=(Bitmask, BitmaskEnum = EInteractType)) int32 GetInteractFlags_Implementation() const;

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactable Interface")
    FText GetInteractableName() const;
    FText GetInteractableName_Implementation() const;

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactable Interface")
    bool OnBeginFocus();
    bool OnBeginFocus_Implementation();

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactable Interface")
    bool OnEndFocus();
    bool OnEndFocus_Implementation();

    //Item Interaction
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Bitmask, BitmaskEnum = "EInteractType"), Category = "Interactable Interface")
    int32 ItemInteractFlags;


public:
    UItem(const class FObjectInitializer &ObjectInititalizer);

    UFUNCTION(BlueprintImplementableEvent, Category = "Item interface")
    void TakeDamage(float DamageValue, const FDamageEvent &Event, AActor *DamageCauser);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Info")
    UTexture2D *ItemIcon;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Info")
    FText ItemName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Info")
    FText ItemDescription;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Info")
    float MaxDurability = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Info")
    float CurrentDurability = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Info")
    float Weight = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Info")
    uint8 MaxCount = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Info")
    int32 Cost;    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Bitmask, BitmaskEnum = "EItemFlags"), Category = "Item Info")
    uint8 ItemFlags;

    //Item spawn parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Info")
    TSubclassOf<class AItemActor> ItemActorInEquipment;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Info")
    TSubclassOf<class AItemActor> ItemActorInWorld;
};


class UBoxComponent;

// Actor containing skeletal mesh for animation and some sort of FSM
// item classes of actors: for equipment and world
UCLASS(BlueprintType, Blueprintable, Abstract)
class AItemActor : public AActor, public IInteractableInterface
{
    GENERATED_BODY()

public:
    //Interaction Interface
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactable Interface")
    bool OnInteract(EInteractType InteractType, ACharacter *controller);
    bool OnInteract_Implementation(EInteractType InteractType, ACharacter *controller);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactable Interface")
    UPARAM(meta=(Bitmask, BitmaskEnum = EInteractType)) int32 GetInteractFlags() const;
    UPARAM(meta=(Bitmask, BitmaskEnum = EInteractType)) int32 GetInteractFlags_Implementation() const;

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactable Interface")
    FText GetInteractableName() const;
    FText GetInteractableName_Implementation() const;

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactable Interface")
    bool OnBeginFocus();
    bool OnBeginFocus_Implementation();

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactable Interface")
    bool OnEndFocus();
    bool OnEndFocus_Implementation();

public:
    // Sets default values for this actor's properties
    AItemActor();
    AItemActor(const FObjectInitializer& ObjectInitializer);

    float TakeDamage(float DamageAmount, const FDamageEvent &DamageEvent, AController *EventInstigator, AActor *DamageCauser) override;

    UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite)
    UItem *ItemBase = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMeshComponent *ItemMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UBoxComponent *ItemCollision;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Info")
    uint8 StackCount;

protected:
    void InitializeActor();
    void InitMesh();
};


// Use if item has skeletal mesh when equipped
UCLASS(BlueprintType, Blueprintable)
class AItemActorEquipment : public AItemActor
{
    GENERATED_BODY()

public:
    AItemActorEquipment();
    AItemActorEquipment(const FObjectInitializer& ObjectInitializer);

protected:
    void InitSkeletalMesh();
};


UCLASS(BlueprintType, Blueprintable)
class AItemActorWorld : public AItemActor, public ISaveable
{
    GENERATED_BODY()

public:
    AItemActorWorld();
    AItemActorWorld(const FObjectInitializer& ObjectInitializer);

protected:
    void InitStaticMesh();
};


// Class for moving item actors, like arrows and bolts
UCLASS(BlueprintType, Blueprintable)
class AItemActorProjectile : public AItemActorWorld
{
    GENERATED_BODY()

public:
    AItemActorProjectile();

    void LaunchProjectile(FVector Direction, float Speed, float DamageCoefficient);
    UFUNCTION()
    void OnOverlap(UPrimitiveComponent* comp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
    UFUNCTION()
    void OnStop(const FHitResult& SweepResult);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UProjectileMovementComponent* MovementComp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LifeTime;

private:
    void OnTimeout();
    void AttachToHitActor(USceneComponent* ActorMesh, FVector HitLocation);

    FTimerHandle DestroyTimer;
    float InitialSpeed = 0.f;
    float WeaponDamageCoefficient = 0.f;
};
