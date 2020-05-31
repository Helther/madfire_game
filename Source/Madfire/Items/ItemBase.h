// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "../Interactable/Interactable.h"
#include "ItemBase.generated.h"

UENUM(BlueprintType, meta = (Bitflags))
enum class EItemFlags : uint8     //!< Флаги предметов.
{
	NONE = 0										,	
	Usable = 1			UMETA(DisplayName = "Usable"),
	Consumable = 2		UMETA(DisplayName = "Consumable"),
	Magical = 4			UMETA(DisplayName = "Magical"),
	Cursed = 8			UMETA(DisplayName = "Cursed"),
	Stolen = 16			UMETA(DisplayName = "Stolen"),
	Unbreakable = 32	UMETA(DisplayName = "Unbreakable"),
	Unsellable = 64		UMETA(DisplayName = "Unsellable"),
	Unstolable = 128	UMETA(DisplayName = "Unstolable")
};
ENUM_CLASS_FLAGS(EItemFlags)



/**
 * 
 */
UCLASS(config = Game, EditInlineNew, BlueprintType, Blueprintable)
class UItemBase : public UObject, public IInteractableInterface
{
	GENERATED_BODY()
	
public:
	UItemBase(const class FObjectInitializer &ObjectInititalizer);

	//Interaction Interface
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactable Interface")
		bool OnInteract(EInteractType EInteractType, ACharacter *controller);
	bool OnInteract_Implementation(EInteractType EInteractType, ACharacter *controller);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactable Interface")
		uint8 GetInteractableFlags() const;
	uint8 GetInteractableFlags_Implementation() const;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactable Interface")
		bool OnBeginFocus();
	bool OnBeginFocus_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interactable Interface")
		bool OnEndFocus();
	bool OnEndFocus_Implementation();

protected:
	UPROPERTY(BlueprintReadWrite, meta = (Bitmask, BitmaskEnum = "EItemFlags"), Category = "Interactable Interface")
		uint8 InteractFlags;
	
	
	//Item
public:
	virtual void TakeDamage(float, const struct FDamageEvent &, class AActor *EventInstigator);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Bitmask, BitmaskEnum = "EInteractType"), Category = "Item Info")
		TSubclassOf<class AItemActor> ItemActor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Info")
		USkeletalMesh *ItemMesh = nullptr;

	//Item Info
	//UPROPERTY(BlueprintReadOnly, Category = "Item Info")
		//EItemType

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Bitmask, BitmaskEnum = "EItemFlags"), Category = "Item Info")
		uint8 ItemFlags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Info")
		float MaxDurability = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Info")
		float CurrentDurability = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Info")
		float Weight = 0;

	UPROPERTY(EditAnywhere,  BlueprintReadWrite, Category = "Item Info")
		uint8 MaxCount = 1;
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Info")
//		uint8 CurrentCount = 1;

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

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite, Category = "Item Info")
		UItemBase *ItemBase = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Info")
		USkeletalMeshComponent *ItemMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Info")
		uint8 StackCount;
};
