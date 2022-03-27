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
    UItem(const class FObjectInitializer &ObjectInititalizer);

    //Interaction Interface
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactable Interface")
    bool OnInteract(EInteractType EInteractType, ACharacter *controller);
    bool OnInteract_Implementation(EInteractType EInteractType, ACharacter *controller);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactable Interface")
    uint8 GetInteractableFlags() const;
    uint8 GetInteractableFlags_Implementation() const;

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactable Interface")
    FText GetInteractableName() const;
    FText GetInteractableName_Implementation() const;

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactable Interface")
    bool OnBeginFocus();
    bool OnBeginFocus_Implementation();

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactable Interface")
    bool OnEndFocus();
    bool OnEndFocus_Implementation();

protected:
    UPROPERTY(EditAnywhere, meta = (Bitmask, BitmaskEnum = "EInteractType"), Category = "Interactable Interface")
    uint8 InteractFlags;


    //Item
public:
    virtual void TakeDamage(float, const struct FDamageEvent &, class AActor *EventInstigator);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Info")
    TSubclassOf<class AItemActor> ItemActor;


    //Item Info
    //UPROPERTY(BlueprintReadOnly, Category = "Item Info")
    //EItemType

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Info")
    UTexture2D *ItemIcon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Bitmask, BitmaskEnum = "EItemFlags"), Category = "Item Info")
    uint8 ItemFlags;

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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Info")
    FText ItemName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Info")
    FText ItemDescription;

    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemStructure")
    //TArray<Effect> bla bla bla... ///Stupid comment about item's effects

private:
};


class UBoxComponent;

UCLASS(BlueprintType, Blueprintable)
class AItemActor : public AActor, public IInteractableInterface
{
    GENERATED_BODY()

public:
    //Interaction Interface
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactable Interface")
    bool OnInteract(EInteractType EInteractType, ACharacter *controller);
    bool OnInteract_Implementation(EInteractType EInteractType, ACharacter *controller);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactable Interface")
    uint8 GetInteractableFlags() const;
    uint8 GetInteractableFlags_Implementation() const;

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
