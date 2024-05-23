#pragma once

#include "CharacterBase.h"
#include "Animation/AnimMontage.h"
#include "Components/CombatComponent.h"
#include "Components/EquipmentComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/MovementComponentBase.h"
#include "Components/Camera/CameraMode.h"
#include "Components/AbilityManagerComponent.h"
#include "Components/CCComponent.h"
#include "CharacterCreature.generated.h"


class UCharacterMeshComponent;
UCLASS()
class MADFIRE_API ACharacterCreature : public ACharacterBase
{
    GENERATED_BODY()

    friend struct FCCStateManager;

public:
    // Sets default values for this character's properties
    ACharacterCreature(const FObjectInitializer &ObjectInitializer);

    float TakeDamage(float DamageAmount, const struct FDamageEvent &DamageEvent, class AController *EventInstigator, AActor *DamageCauser) override;
    virtual void Update(const FCharacterLevelInfo &levelInfo);
    UFUNCTION(BlueprintCallable, Category = "Camera")
    virtual void SetAbilityCameraMode(TSubclassOf<UCameraMode> CameraMode) {}
    UFUNCTION(BlueprintCallable, Category = "Camera")
    virtual void ClearAbilityCameraMode() {}
    virtual void FireLoadedProjectile();

    void StartAnimation(UAnimMontage *AnimMontage,
                        const FOnMontageBlendingOutStarted &EndCallBack, float InPlayRate,
                        FName StartSectionName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "Character Animation")
    void InterruptAnimation(ECCState Source, int SubMode = 0);

    UHumanoidMovementComponent* GetMovementComponent() const override;
    UCharacterMeshComponent* GetMeshComponent() const;
    UCCComponent* GetССComponent() const { return CCComponent; }

    UFUNCTION(BlueprintCallable, Category = "Character")
    void DoLevelUp(const FCharacterLevelInfo &levelInfo);

    UFUNCTION(BlueprintCallable, Category = "Character")
    bool PickupItem(AItemActor *ItemActor);

    // TODO уточнить при балансировке
    FORCEINLINE float CalculateMaxWeight() const
    { return MaxWeightBase + MaxWeightModBase * UAttributeSetHelper::GetModifier(GetAttributeComponent()->Attributes, EAttributes::Str); }
    // TODO уточнить при балансировке
    float CalculateKnockDownTime() const;
    void UpdateCCLOBAnims();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Stats")
    float MaxWeight;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Stats")
    float KnockDownTime;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
    float KnockDownBaseTime;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
    float KnockDownModBase;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
    float KnockDownWeightModBase;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
    float MaxWeightBase;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats")
    float MaxWeightModBase;

    UFUNCTION(BlueprintCallable, meta = (Tooltip = "Return AttributeComponent"), Category = "Character Components")
    FORCEINLINE UInventoryComponent *GetInventoryComponent() const{return InventoryComponent;}
    UFUNCTION(BlueprintCallable, meta = (Tooltip = "Return AttributeComponent"), Category = "Character Components")
    FORCEINLINE UEquipmentComponent *GetEquipmentComponent() const{return EquipmentComponent;}
    UFUNCTION(BlueprintCallable, meta = (Tooltip = "Return AttributeComponent"), Category = "Character Components")
    FORCEINLINE UCombatComponent *GetCombatComponent() const { return CombatComponent; }
    UFUNCTION(BlueprintCallable, meta = (Tooltip = "Return AttributeComponent"), Category = "Character Components")
    FORCEINLINE UAbilityManagerComponent *GetAbilityManager() const{ return AbilityManager; }

    UCharacterAnimInstance* AnimInstance;

private:
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Character Components", meta=(AllowPrivateAccess = "true"))
    TObjectPtr<UInventoryComponent>     InventoryComponent;
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Character Components", meta=(AllowPrivateAccess = "true"))
    TObjectPtr<UEquipmentComponent>     EquipmentComponent;
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere,Category = "Character Components", meta=(AllowPrivateAccess = "true"))
    TObjectPtr<UCombatComponent>        CombatComponent;
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere,Category = "Character Components", meta=(AllowPrivateAccess = "true"))
    TObjectPtr<UAbilityManagerComponent> AbilityManager;
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Character Components", meta=(AllowPrivateAccess = "true"))
    TObjectPtr<UCCComponent>     CCComponent;

protected:
    void PostInitializeComponents() override;
    void BeginPlay() override;

    // clang-format off
    UFUNCTION(BlueprintCallable, Category = "Character Animation")
    void CCStunEnded(UAnimMontage *animMontage, bool bInterrupted);
    UFUNCTION(BlueprintCallable, Category = "Character Animation")
    void CCLOBEnded(UAnimMontage *animMontage, bool bInterrupted); 
    UFUNCTION(BlueprintCallable, Category = "Character Animation")
    void CCKnockDownEnded(UAnimMontage *animMontage, bool bInterrupted);

    TMap<ECCState, void (ACharacterCreature::*)(UAnimMontage *, bool)> CCInterruptDelegates;
};
