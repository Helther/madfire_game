#include "CharacterNPC.h"
#include "Components/CharacterSensingComponent.h"


ACharacterNPC::ACharacterNPC(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
    SensingComponent = CreateDefaultSubobject<UCharacterSensingComponent>(TEXT("SensingComponent"));

    AlertState = EAlertState::Calm;
}

void ACharacterNPC::SetGuardState(EAlertState NewState)
{
    // TODO
}

void ACharacterNPC::OnStateChanged(EAlertState NewState)
{
    //TODO
}
