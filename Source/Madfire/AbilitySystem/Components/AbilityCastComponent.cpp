#include "AbilityCastComponent.h"
#include "../AbstractAbility.h"

 UAbilityCastComponent::UAbilityCastComponent(const FObjectInitializer &f) :
	 UAbilityComponent(f)
 {}


void UAbilityCastComponent::ComponentInit(UAbstractAbility *parent)
{}

bool UAbilityCastComponent::ComponentSetup(ACharacter *Component) 
{
	return false;
}

void UAbilityCastComponent::Interrupt()
{
    auto Ability = GetOwner();
    Ability->Interrupt();
}

void UAbilityCastComponent::Activate(int32 Stage) 
{
    CurrentStage = Stage;
    check(CurrentStage < CastStages.Num() && "Invalid Cast Stage");

    StartAnimation(CastStages[CurrentStage]);
}

void UAbilityCastComponent::Deactivate() 
{
    check(CurrentStage < CastStages.Num() && "Invalid Cast Stage");
    StopAnimation(CastStages[CurrentStage]);
}

void UAbilityCastComponent::PlayNext() 
{
    if(CurrentStage < CastStages.Num())
        Activate(CurrentStage + 1);
}

bool UAbilityCastComponent::StopAnimation(const FAbilityCastStage &Stage)
{
    auto Ability = GetOwner();
    check(Ability != nullptr && "No Ability Outer");
    check(Stage.AnimMontage != nullptr && "No CastAnimMontage Is Set");

    auto Character = Cast<ACharacter>(Ability->GetOwner());
    
    if(!Character)
        return false;

    Character->StopAnimMontage(Stage.AnimMontage);
    return true;
}

bool UAbilityCastComponent::StartAnimation(const FAbilityCastStage &Stage)
{
    auto Ability = GetOwner();
    check(Ability != nullptr && "No Ability Outer");
    check(Stage.AnimMontage != nullptr && "No CastAnimMontage Is Set");

    auto Character = Cast<ACharacter>(Ability->GetOwner());
    
    if(!Character)
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Ability: Character Owner is not set or not a Character"));
        return false;
    }

    Character->PlayAnimMontage(Stage.AnimMontage,1.f,Stage.SectionName);//PrecastSectionName);

    auto animInstance    = Character->GetMesh()->GetAnimInstance();
    auto montageInstance = animInstance->GetActiveInstanceForMontage(Stage.AnimMontage);
    montageInstance->OnMontageBlendingOutStarted.BindUObject(this,&UAbilityCastComponent::OnAnimBlendoutStart);
    //montageInstance->OnMontageEnded.BindUObject(this,&UAbilityCastComponent::AnimEnd);

    UFunction *Fun = Stage.OnStartTest.ResolveMember<UFunction>(GetOwner()->GetClass(),false);
    if(Fun) GetOwner()->ProcessEvent(Fun,nullptr);

    return true;
}

void UAbilityCastComponent::OnAnimBlendoutStart(UAnimMontage *animMontage, bool bInterrupted)
{
    auto Ability = GetOwner();
    check(Ability != nullptr && "No Ability Outer");
    check(animMontage != nullptr && "No CastAnimMontage Is Set");

    if(bInterrupted)
    {
        Interrupt();
        return;
    }
        
    UFunction *Fun = CastStages[CurrentStage].OnEnd.ResolveMember<UFunction>(GetOwner()->GetClass(),false);
    if(Fun) GetOwner()->ProcessEvent(Fun,nullptr);

    PlayNext();
}

/*
void UAbilityCastComponent::AnimEnd(UAnimMontage *animMontage, bool bInterrupted)
{
    auto Ability = GetOwner();
    check(Ability != nullptr && "No Ability Outer");
    check(animMontage != nullptr && "No CastAnimMontage Is Set");

    if(bInterrupted)
        Interrupt();   
    
}*/