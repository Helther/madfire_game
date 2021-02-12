#pragma once

#include "Animation/AnimMontage.h"
#include "AttributesComponent.h"
#include "CombatComponent.h"
#include "CoreMinimal.h"
#include "EquipmentComponent.h"
#include "GameFramework/Character.h"
#include "InventoryComponent.h"
#include "LevelList.h"
#include "MovementComponentBase.h"

#include "CharacterBase.generated.h"


UENUM(BlueprintType, meta = (Bitflags))
// Character Control Bit mask with values as priorities (the more the better)
enum ECCState
{
    NoCC          = 0,
    Sleep         = 1 UMETA(DisplayName = "Sleeping"),
    LossOfBalance = 2 UMETA(DisplayName = "LostAllBalance"),
    Stun          = 4 UMETA(DisplayName = "Stunned"),
    KnockDown     = 8 UMETA(DisplayName = "KnockedDown"),
    Stasis        = 16 UMETA(DisplayName = "UnderStasis")
};
ENUM_CLASS_FLAGS(ECCState)

/**
 * Obvious Class Decription
 */
UCLASS()
class MADFIRE_API ACharacterBase : public ACharacter
{
    GENERATED_BODY()

public:
    // Sets default values for this character's properties
    ACharacterBase(const FObjectInitializer &ObjectInitializer);

    float TakeDamage(float DamageAmount,
                     const struct FDamageEvent &DamageEvent,
                     class AController *EventInstigator,
                     AActor *DamageCauser) override;

    virtual void Update();
    void OnJumped_Implementation() override;
    UFUNCTION(BlueprintCallable, Category = "Character Base")
    void OnUpdate(); //!< Blueprint interface  for Blueprints

    UFUNCTION(BlueprintCallable, Category = "Character Base") // todo debug
    void StartAnimTest(float InPlayRate);

    void StartAnimation(UAnimMontage *AnimMontage,
                        const FOnMontageBlendingOutStarted &EndCallBack,
                        float InPlayRate,
                        FName StartSectionName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "Character Base")
    void InterruptAnimation(ECCState Source, int OldState);

    UBaseMovementComponent *GetMovementComponent() const override;
    UCombatComponent *GetCombatComponent() const;
    UAttributesComponent *GetAttributesComponent() const;
    ULevelComponent *GetLevelComponent() const;
    UEquipmentComponent *GetEquipmentComponent() const;
    UInventoryComponent *GetInventoryComponent() const;

    UFUNCTION(BlueprintCallable, Category = "Character Base")
    virtual void DoLevelUp(const FLevelInfo &levelInfo);

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
              meta = (Bitmask, BitmaskEnum = "ECCState"), Category = "Character Stats")
    int CurrentCCState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats",
              meta = (ClampMin = "0.0", UIMin = "0.0"))
    float MaxWeightMod;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Stats")
    float MaxWeight;


protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    void PostInitializeComponents() override;

    // clang-format off
    UFUNCTION(BlueprintCallable, Category = "Animation")
        void CCStunEnded(UAnimMontage *animMontage, bool bInterrupted);
    UFUNCTION(BlueprintCallable, Category = "Animation")
        void CCKnockDownEnded(UAnimMontage *animMontage, bool bInterrupted);
    UFUNCTION(BlueprintCallable, Category = "Animation")
        void CCStatisEnded(UAnimMontage *animMontage, bool bInterrupted);
    // todo add other

	UPROPERTY(BlueprintReadWrite, Category = "Character Components", 
		VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
		UCombatComponent *CombatComponent;
	UPROPERTY(BlueprintReadWrite, Category = "Character Components",
		VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
		UAttributesComponent *AttributesComponent;
	UPROPERTY(BlueprintReadWrite, Category = "Character Components", 
		VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
		ULevelComponent *LevelComponent;
	UPROPERTY(BlueprintReadWrite, Category = "Character Components", 
		VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
		UEquipmentComponent *EquipmentComponent;
	UPROPERTY(BlueprintReadWrite, Category = "Character Components", 
		VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
		UInventoryComponent *InventoryComponent;
    // clang-format on
    void CalculateMaxWeight();

    int GetHighestPriorityCC(int bitmask);

    TMap<ECCState, void (ACharacterBase::*)(UAnimMontage *, bool)> CCInterruptDelegates;
    TMap<ECCState, UAnimMontage *> CCAnims;
};
