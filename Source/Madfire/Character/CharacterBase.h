
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LevelList.h"
#include "MovementComponentBase.h"
#include "CombatComponent.h"
#include "AttributesComponent.h"
#include "CharacterBase.generated.h"


UENUM(BlueprintType, meta = (Bitflags))
// состояния контроля персонажа
enum ECCState
{
	NoCC = 0					UMETA(DisplayName = "Stunned"),
	Stun = 1					UMETA(DisplayName = "KnockedDown"),
	KnockDown = 2			UMETA(DisplayName = "LostAllBalance"),
	LossOfBalance = 4	UMETA(DisplayName = "UnderStasis"),
	Stasis = 8				UMETA(DisplayName = "Sleeping"),
	Sleep	= 16				
};
ENUM_CLASS_FLAGS(ECCState)
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

	virtual void Update();
	void OnJumped_Implementation() override;
	UFUNCTION(BlueprintCallable, Category = "Character Base")
		void OnUpdate(); //!< Blueprint interface  for Blueprints

	UBaseMovementComponent* GetMovementComponent() const override;
  UCombatComponent* GetCombatComponent() const;
	UAttributesComponent* GetAttributesComponent() const;
	ULevelComponent* GetLevelComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Character Base")
		virtual void DoLevelUp(const FLevelInfo &levelInfo);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Bitmask, BitmaskEnum = "ECCState"),
    Category = "Character Stats")
		int CurrentCCState;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Called every frame
	virtual void Tick(float DeltaTime) override;

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
