
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "./Items/ItemWeapon.h"
#include "../../CombatSystem/CombatStance.h"
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "CombatComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
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
    /// todo replace with SM
    bool IsInCombat    = false;
    bool IsWeaponDrawn = false;
    bool IsAttacking   = false;
    bool IsParrying    = false;
    bool IsBlocking    = false;
    bool IsCasting     = false;
    float CastStaminaCost;

    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction *ThisTickFunction) override;
    UFUNCTION(BlueprintCallable, Category = "Combat Component")
    void Update();
    UFUNCTION(BlueprintCallable, Category = "Component Update")    // todo
    void SetUpdate() { IsDirty = true; }
    UFUNCTION(BlueprintCallable, Category = "Component Update")
    void UnSetUpdate() { IsDirty = false; }
    UFUNCTION(BlueprintCallable, Category = "Component Update")
    bool IsUpdateNeeded() { return IsDirty; }
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

    UCombatStance *GetCurrentCombatStance();

protected:
    UItemWeapon *DefaultWeapon;
    UCombatStance *CurrentCombatStance;
    TSet<AActor *> CharactersHit;
    class ACharacterPlayer *Character;
    bool HasAlreadyHitTarget(AActor *Target);

private:
    bool IsDirty = false;
};
