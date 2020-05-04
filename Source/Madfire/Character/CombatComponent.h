
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "./Items/ItemWeapon.h"
#include "CombatStance.h"
#include "CombatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MADFIRE_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombatComponent(const class FObjectInitializer &ObjectInitilizer);

protected:


	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	int32 IsInCombat : 1;
	int32 IsWeaponDrawn : 1;
	int32 IsAttacking : 1;
	int32 IsParrying : 1;
	int32 IsBlocking : 1;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void Update();

	void DoAttack();
	void DoBlock();
	void DrawWeapon();
	void SheathWeapon();
	void EnterCombatMode();
	void ExitCombatMode();

    float CalculateDamage(AActor *Target, UItemWeapon *Weapon);
    float CalculatePower(AActor *Target, UItemWeapon *Weapon);

	
private:

	UItemWeapon *DefaultWeapon;
	UCombatStance *CurrentCombatStance;
    TSet<AActor> CharactersHit;

    bool HasAlreadyHitTarget(AActor *Target);
};
