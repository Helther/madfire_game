#pragma once

#include "Animation/AnimMontage.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/AttributesComponent.h"
#include "Components/CombatComponent.h"
#include "Components/EquipmentComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/MovementComponentBase.h"
#include "Components/ExperienceComponent.h"
#include "Components/EffectComponent.h"
#include "Components/AbilitySystemComponent.h"


#include "CharacterBase.generated.h"


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

    float TakeDamage(float DamageAmount, const struct FDamageEvent &DamageEvent, class AController *EventInstigator, AActor *DamageCauser) override;
    virtual void Update();

    UFUNCTION(BlueprintCallable, Category = "Character Base")
    void OnUpdate(); //!< Blueprint interface  for Blueprints

    UFUNCTION(BlueprintCallable, Category = "Character Base")
    virtual void DoLevelUp(const FLevelInfo &levelInfo);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats",
              meta = (ClampMin = "0.0", UIMin = "0.0"))
    float MaxWeightMod;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Stats")
    float MaxWeight;


    UPROPERTY(BlueprintReadWrite, Category = "Character Components", VisibleAnywhere)
    UAttributesComponent *AttributesComponent;
    UPROPERTY(BlueprintReadWrite, Category = "Character Components", VisibleAnywhere)
    UExperienceComponent *ExperienceComponent;
    UPROPERTY(BlueprintReadWrite, Category = "Character Components", VisibleAnywhere)
    UEquipmentComponent *EquipmentComponent;
    UPROPERTY(BlueprintReadWrite, Category = "Character Components", VisibleAnywhere)
    UInventoryComponent *InventoryComponent;
    UPROPERTY(BlueprintReadWrite, Category = "Character Components", VisibleAnywhere)
    UEffectComponent *EffectComponent;
    UPROPERTY(BlueprintReadWrite, Category = "Character Components", VisibleAnywhere)
    UAbilitySystemComponent *AbilitySystemComponent;


protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;
    // Called every frame
    virtual void Tick(float DeltaTime) override;

        // clang-format on
    void CalculateMaxWeight();
};
