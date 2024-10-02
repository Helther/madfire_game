#include "AbilityComponent.h"

#include "AbilitySystem/AbstractAbility.h"
#include "Classes/Engine/World.h"


UAbilityComponent::UAbilityComponent(const FObjectInitializer &f) :
    Super(f)
{
    UE_LOG(AbilitySystemLog, Warning, TEXT("AbilityComponent: Construction"));
    
    auto Outer = GetOuter();  
    auto AbilityOuter = Cast<UAbstractAbility>(Outer);    
    if(!AbilityOuter)
        return;  

    if(!AbilityOuter->AbilityComponents.Contains(this))
        AbilityOuter->AbilityComponents.Add(this);
}

void UAbilityComponent::ComponentInit(UAbstractAbility *parent)
{
    parent->AddComponent(this);
    OnComponentInit();
}

bool UAbilityComponent::ComponentSetup(ACharacter *Component)
{
    UE_LOG(AbilitySystemLog, Warning, TEXT("AbilityComponent: Setup"));    
    
    OnComponentSetup();
    return true;
}

UAbstractAbility *UAbilityComponent::GetOwner() const
{
    return StaticCast<UAbstractAbility *>(GetOuter());    
}

AActor *UAbilityComponent::GetActorOwner() const
{
    auto Ability = GetOwner();
    check(Ability);
        
    return Ability->GetOwner();    
}

