#pragma once

#include "Attributes/Attributes.h"
#include "Attributes/ConsumableAttributes.h"
#include "Attributes/DamageMap.h"
#include "Attributes/Damage.h"
#include "CoreMinimal.h"
#include "Barrier.generated.h"


UCLASS(BlueprintType, Blueprintable, Abstract)
class UBarrier : public UObject
{
    GENERATED_BODY()

public:
    float TakeDamage(float DamageAmount, UDamageType *DamageType, AActor *DamageCauser);

    UPROPERTY(EditAnywhere)
    FDamageAbsorb DamageAbsorb;

    UPROPERTY(EditAnywhere)
    FDamageResistance DamageResistance;

    //Type of Taken Damage
    UPROPERTY(EditAnywhere)
    TSubclassOf<UDamageBase> DamageClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Health; //Effects or some stuff can update Health

    UFUNCTION(BlueprintImplementableEvent)
    void OnBarrierEnd();

    //TODO: Вспомнить, зачем этот эвент?
    //UFUNCTION(BlueprintImplementableEvent)
    //float DamageCalculate(float DamageAmount, UDamageType *DamageType, AActor *DamageCauser);

    UFUNCTION(BlueprintImplementableEvent)
    void OnHit(float DamageAmount, AActor *Source);
};
