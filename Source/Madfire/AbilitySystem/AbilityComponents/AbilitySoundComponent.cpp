#include "AbilitySoundComponent.h"
#include "Character/Components/AbilitySystemComponent.h"

void UAbilitySoundComponent::ActivateComponent_Implementation()
{

}

void UAbilitySoundComponent::UpdateComponent_Implementation()
{

}

void UAbilitySoundComponent::DeactivateComponent_Implementation()
{

}

bool UAbilitySoundComponent::ComponentSetup(UAbilitySystemComponent *Component)
{
    Super::ComponentSetup(Component);

    if(!IsValid())
        return false;
    auto Owner = Cast<ACharacter>(AbilitySystemComponent->GetOwner());
    const FAttachmentTransformRules TransformRules = FAttachmentTransformRules(LocationRule, RotationRule, ScaleRule, bWeldSimulatedBodies);
    if(Owner)
        AudioComponent->AttachToComponent(Owner->GetMesh(), TransformRules, SocketName);
    else
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Ability Sound Component: Owner Is Not Character"));
        return false;
    }
    return true;
}

void UAbilitySoundComponent::PlaySound(USoundCue *AbilitySound, bool Loop)
{
    if(!AudioComponent->IsValidLowLevelFast())
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Ability Sound Component: Audio Component invaid"));
        return;
    }
    AudioComponent->SetSound(AbilitySound);
    AudioComponent->SetActive(true);
}

void UAbilitySoundComponent::StopSound()
{
    if(!AudioComponent->IsValidLowLevelFast())
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Ability Sound Component: Audio Component invaid"));
        return;
    }
    AudioComponent->SetActive(false);
}

void UAbilitySoundComponent::ComponentInit(UAbstractAbility *parent)
{
    AudioComponent = NewObject<UAudioComponent>(this, UAudioComponent::StaticClass(),NAME_None);
    Super::ComponentInit(parent);
}

bool UAbilitySoundComponent::IsValid()
{
    if(!(AbilitySystemComponent->IsValidLowLevelFast()))
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Ability Sound Component: Component or Owner Is Null"));
        return false;
    }
    return true;
}
