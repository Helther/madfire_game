
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
	uint32 IsInCombat : 1;
	uint32 IsWeaponDrawn : 1;
	uint32 IsAttacking : 1;
	uint32 IsParrying : 1;
	uint32 IsBlocking : 1;
	uint32 IsCasting : 1;
	float CastStaminaCost;

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
	void On_StanceChanged();
	void On_Weapon_OnOff();
	void On_Blocking();
	void On_Casting();

	UCombatStance* GetCurrentCombatStance();
protected:

	UItemWeapon *DefaultWeapon;
	UCombatStance *CurrentCombatStance;
    TSet<AActor> CharactersHit;
	class ACharacterBase *Character;
    bool HasAlreadyHitTarget(AActor *Target);
	
};
