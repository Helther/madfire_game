
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LevelList.h"
#include "PlayerMovementComponent.h"
#include "CombatComponent.h"
#include "AttributesComponent.h"
#include "CharacterBase.generated.h"

UENUM(BluePrintType)
enum class ECCState : uint8
{
	NONE = 1,
	Stun = 2,
	KnockDown = 4,
	LossOfBalance = 8,
	Stasis = 16,
	Sleep = 32,
	MAX = 64
}; // состояния контроля персонажа

UCLASS()
class MADFIRE_API ACharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACharacterBase(const FObjectInitializer& ObjectInitializer);

	float TakeDamage(float DamageAmount, const struct FDamageEvent &DamageEvent,
					class AController *EventInstigator,
					AActor *DamageCauser) override;

	virtual void update();

	UFUNCTION(BlueprintCallable, Category = "Character Base")
		void OnUpdate(); //!< Blueprint interface  for Blueprints

	class UBaseMovementComponent* GetMovementComponent() const override;

    UCombatComponent* GetCombatComponent() const;

	UAttributesComponent* GetAttributesComponent() const;

	ULevelComponent* GetLevelComponent() const;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input 
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    virtual void OnWalkingOffLedge_Implementation( const FVector & PreviousFloorImpactNormal,
                                                   const FVector & PreviousFloorContactNormal,
                                                   const FVector & PreviousLocation,
                                                   float TimeDelta) override;

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
