// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../Interactable/Interactable.h"
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Rules/GameRules.h"
#include "ItemBase.generated.h"

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
    UPROPERTY(EditAnywhere, meta = (Bitmask, BitmaskEnum = "EInteractType"), Category = "Interactable Interface")
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
    TSubclassOf<class AItemActor> ItemActor;
};


class UBoxComponent;

//TODO: Basic Interract

UCLASS(BlueprintType, Blueprintable)
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

    virtual void OnConstruction(const FTransform &Transform) override;
    float TakeDamage(float DamageAmount, const FDamageEvent &DamageEvent, AController *EventInstigator, AActor *DamageCauser) override;
protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;
    // Called every frame
    virtual void Tick(float DeltaTime) override;

public:
    UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite, Category = "Item Info")
    UItem *ItemBase = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Info")
    UMeshComponent *ItemMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Info")
    UBoxComponent *ItemCollision;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Info")
    uint8 StackCount;
};
