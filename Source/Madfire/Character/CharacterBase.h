#pragma once

#include "Animation/AnimMontage.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/AttributesComponent.h"
#include "Components/EffectComponent.h"
#include "Utils/ObjectLogger.h"

#include "CharacterBase.generated.h"


class UPawnNoiseEmitterComponent;

// container of character cheats that might be applied to it
struct FCheats
{
    unsigned int Invulnerable : 1;
    unsigned int InvulnerableToHit : 1;
};

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
    // Returns remaining HP
    float TakeDamage(float DamageAmount, const struct FDamageEvent &DamageEvent, class AController *EventInstigator, AActor *DamageCauser) override;
    virtual void Update(const FCharacterLevelInfo &levelInfo);
    void BeginPlay() override;

    UPROPERTY(BlueprintReadOnly, Category = "Character Components", VisibleAnywhere)
    UAttributeComponent *AttributeComponent;
    UPROPERTY(BlueprintReadOnly, Category = "Character Components", VisibleAnywhere)
    UEffectComponent *EffectComponent;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Components")
	UPawnNoiseEmitterComponent* NoiseEmitterComponent;

    virtual void UseAimComponent(class UAbilityAimComponent *) {}

    FCheats Cheats;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FObjectLogger Logger;
};
