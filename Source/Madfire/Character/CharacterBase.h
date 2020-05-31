
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LevelList.h"
#include "MovementComponentBase.h"
#include "CombatComponent.h"
#include "AttributesComponent.h"
#include "CharacterBase.generated.h"


UENUM(BluePrintType)
// состояния контроля персонажа
enum class ECCState : uint8
{
	NONE = 0,
	Stun = 1,
	KnockDown = 2,
	LossOfBalance = 4,
	Stasis = 8,
	Sleep = 16,
	MAX = 32
};

/**
 * 
 */
UCLASS()
class MADFIRE_API ACharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACharacterBase(const FObjectInitializer& ObjectInitializer);

	float TakeDamage(float DamageAmount, 
                    const struct FDamageEvent &DamageEvent,
					class AController *EventInstigator,
					AActor *DamageCauser) override;

	virtual void update();

	UFUNCTION(BlueprintCallable, Category = "Character Base")
		void OnUpdate(); //!< Blueprint interface  for Blueprints

	UBaseMovementComponent* GetMovementComponent() const override;

    UCombatComponent* GetCombatComponent() const;

	UAttributesComponent* GetAttributesComponent() const;

	ULevelComponent* GetLevelComponent() const;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
		ECCState CurrentCCState;											   
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(BlueprintReadWrite, Category = "Character Components", 
		VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
		UCombatComponent *CombatComponent;
	UPROPERTY(BlueprintReadWrite, Category = "Character Components",
		VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
		UAttributesComponent *AttributesComponent;
	UPROPERTY(BlueprintReadWrite, Category = "Character Components", 
		VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
		ULevelComponent *LevelComponent;		

};
