#pragma once

#include "CharacterCreature.h"

#include "CharacterNPC.generated.h"


class UCharacterSensingComponent;


UENUM(BlueprintType)
enum class EAlertState : uint8
{
    Calm = 0,
    Suspicious,
    Alerted
};

/**
 * TODO
 */
UCLASS()
class MADFIRE_API ACharacterNPC : public ACharacterCreature
{
    GENERATED_BODY()

public:
    ACharacterNPC(const FObjectInitializer &ObjectInitializer);

protected:
    void SetGuardState(EAlertState NewState);

    UFUNCTION()
    void OnStateChanged(EAlertState NewState);

    UPROPERTY(VisibleAnywhere, Category = "Character Components")
    UCharacterSensingComponent* SensingComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    EAlertState AlertState;

};
