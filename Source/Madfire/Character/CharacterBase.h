#pragma once

#include "Animation/AnimMontage.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/AttributesComponent.h"
#include "Components/StatusEffectComponent.h"
#include "Items/ItemBase.h"
#include "Utils/ObjectLogger.h"

#include "CharacterBase.generated.h"


class UPawnNoiseEmitterComponent;

// container of character cheats that might be applied to it
struct FCheats
{
    unsigned int Invulnerable : 1;
    unsigned int InvulnerableToHit : 1;
};

UENUM(BlueprintType)
enum class ESpawnPivot : uint8
{
    Root = 0
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

    UFUNCTION(BlueprintCallable, Category = "Character Function")
    void DropItem(UItem *item, int32 amount, FTransform transform);

    UFUNCTION(BlueprintCallable, Category = "Character Function")
    int32 AddMaterial();

    UFUNCTION(BlueprintCallable, Category = "Character Function")
    static void SetupItemActorCollision(AItemActor* InActor, bool Enable);

    virtual void UseAimComponent(class UAbilityAimComponent *) {}
    // Offset is in character's local space (X-axis is forward)
    UFUNCTION(BlueprintCallable, Category = "Character Function")
    FVector GetProjectileSpawnLocation(ESpawnPivot Pivot, FVector Offset) const;

    FCheats Cheats;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FObjectLogger Logger;

    UFUNCTION(BlueprintCallable, meta = (Tooltip = "Return AttributeComponent"), Category = "Character Components")
    FORCEINLINE UAttributeComponent *GetAttributeComponent() const {return AttributeComponent;}
    UFUNCTION(BlueprintCallable, meta = (Tooltip = "Return AttributeComponent"), Category = "Character Components")
    FORCEINLINE UStatusEffectQueueComponent *GetStatusEffectComponent() const {return StatusEffectComponent;}
    UFUNCTION(BlueprintCallable, meta = (Tooltip = "Return AttributeComponent"), Category = "Character Components")
    FORCEINLINE UPawnNoiseEmitterComponent *GetNoiseEmitterComponent() const {return NoiseEmitterComponent;}


private:
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Character Components", meta=(AllowPrivateAccess = "true"))
    TObjectPtr<UAttributeComponent> AttributeComponent;
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Character Components", meta=(AllowPrivateAccess = "true"))
    TObjectPtr<UStatusEffectQueueComponent> StatusEffectComponent;
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere,Category = "Character Components", meta=(AllowPrivateAccess = "true"))
    TObjectPtr<UPawnNoiseEmitterComponent> NoiseEmitterComponent;
};
