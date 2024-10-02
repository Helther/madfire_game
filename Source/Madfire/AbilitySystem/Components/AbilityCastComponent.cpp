#include "AbilityCastComponent.h"
#include "../AbstractAbility.h"

 UAbilityCastComponent::UAbilityCastComponent(const FObjectInitializer &f) :
	 UAbilityComponent(f)
 {
     bIsTickable = true;
 }


void UAbilityCastComponent::ComponentInit(UAbstractAbility *parent)
{}

bool UAbilityCastComponent::ComponentSetup(ACharacter *Component) 
{
	return false;
}

FName UAbilityCastComponent::GetCurrentStateName() const 
{
    return CastStages[CurrentStage].SectionName;
}

int32 UAbilityCastComponent::GetCurrentStateID() const 
{
    return CurrentStage; 
}

void UAbilityCastComponent::Interrupt()
{
    auto Ability = GetOwner();
    Ability->bIsActive = false;
    Ability->Interrupt();
}

void UAbilityCastComponent::Cancel()
{
    auto Ability = GetOwner();
    Ability->bIsActive = false;
    Ability->Cancel();
}

float UAbilityCastComponent::GetCurrentMaxCastTime() const
{
    check(CurrentStage < CastStages.Num() && "Invalid Cast Stage");
    return CastStages[CurrentStage].CastTime;
}

void UAbilityCastComponent::Activate(int32 Stage) 
{
    auto Ability = GetOwner();
    CurrentCastTime = 0;
    CurrentStage = Stage;
    check(CurrentStage < CastStages.Num() && "Invalid Cast Stage");

    StartAnimation(CastStages[CurrentStage]);
    Ability->bIsActive = true;
}

void UAbilityCastComponent::Deactivate() 
{
    auto Ability = GetOwner();
    check(CurrentStage < CastStages.Num() && "Invalid Cast Stage");
    
    bUserIterrupt = true;

    if(Ability->bIsActive)
        StopAnimation(CastStages[CurrentStage]);
}

void UAbilityCastComponent::AddCastStage(FAbilityCastStage Stage, int32 index) 
{
    CastStages.Insert(Stage, index);
    LastStage = CastStages.Num();
}

void UAbilityCastComponent::PlayNext() 
{
    StopAnimation(CastStages[CurrentStage]);
    const auto NewStage = CurrentStage + 1;
    if(NewStage < CastStages.Num() && NewStage < LastStage)
    {            
        Activate(NewStage);
    }
}

void UAbilityCastComponent::Continue() 
{
    bContinue = true;
    StopAnimation(CastStages[CurrentStage]);
    const auto NewStage = CurrentStage + 1;
    if(NewStage < CastStages.Num() && NewStage < LastStage)
    {            
        Activate(NewStage);
    }
}

bool UAbilityCastComponent::StopAnimation(const FAbilityCastStage &Stage)
{
    auto Ability = GetOwner();

    if(Stage.IsTimeBased && !Stage.AnimMontage)
        return false;

    check(Ability != nullptr && "No Ability Outer");
    check(Stage.AnimMontage != nullptr && "No CastAnimMontage Is Set");

    auto Character = Cast<ACharacter>(Ability->GetOwner());
    
    if(!Character)
        return false;

    if(!Character->GetMesh()->GetAnimInstance()->Montage_IsPlaying(Stage.AnimMontage))
    {
        Ability->bIsActive = false;
        return false;
    }

    Character->StopAnimMontage(Stage.AnimMontage);

    Ability->bIsActive = false;

    return true;                             
}

bool UAbilityCastComponent::StartAnimation(const FAbilityCastStage &Stage)
{
    auto Ability = GetOwner();

    if(Stage.IsTimeBased && !Stage.AnimMontage)
    {
        OnStart.ExecuteIfBound(CurrentStage, Stage.SectionName);
        return true;
    }   

    check(Ability != nullptr && "No Ability Outer");
    check(Stage.AnimMontage != nullptr && "No CastAnimMontage Is Set");

    auto Character = Cast<ACharacter>(Ability->GetOwner());
    
    if(!Character)
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Ability: Character Owner is not set or not a Character"));
        return false;
    }
    CurrentCastTime = 0.f;
    bUserIterrupt = bContinue = false;

    Character->PlayAnimMontage(Stage.AnimMontage,1.f,Stage.SectionName);//PrecastSectionName);

    auto animInstance    = Character->GetMesh()->GetAnimInstance();
    auto montageInstance = animInstance->GetActiveInstanceForMontage(Stage.AnimMontage);
    montageInstance->OnMontageBlendingOutStarted.BindUObject(this,&UAbilityCastComponent::OnAnimBlendoutStart);

    OnStart.ExecuteIfBound(CurrentStage, Stage.SectionName);

    return true;
}

void UAbilityCastComponent::Tick_Implementation(float Delta) 
{
    CurrentCastTime += Delta;
    auto Stage = CastStages[CurrentStage];

    if(Stage.ProceedStage && CurrentCastTime >= Stage.CastTime)
    {
        Continue();
    }
}

void UAbilityCastComponent::OnAnimBlendoutStart(UAnimMontage *animMontage, bool bInterrupted)
{
    auto Ability = GetOwner();
    check(Ability != nullptr && "No Ability Outer");
    check(animMontage != nullptr && "No CastAnimMontage Is Set");

    auto Stage = CastStages[CurrentStage];

    if(bInterrupted)
    {
        if(bContinue)
        {
            OnEnd.ExecuteIfBound(CurrentStage, Stage.SectionName);
            return;
        }

        if(bUserIterrupt)
        {
            if(Stage.IsLoop && CurrentCastTime >= Stage.CastTime)
            {
                OnEnd.ExecuteIfBound(CurrentStage, Stage.SectionName);
                return;
            }
            else
            {
                Cancel(); 
                return;
            }
        }
        else
        {
            Interrupt();
        }
    }

    OnEnd.ExecuteIfBound(CurrentStage, Stage.SectionName);
    return;

    if(bInterrupted)
    {
        if(!bUserIterrupt)
        {        
            //OnEnd.ExecuteIfBound(CurrentStage, CastStages[CurrentStage].SectionName, ECastAnimStatus::Interrupt);
            Interrupt();
            return;
        }
        else
        {
            //OnEnd.ExecuteIfBound(CurrentStage, CastStages[CurrentStage].SectionName, ECastAnimStatus::Cancel);
            return;
        }
    }
  
    //OnEnd.ExecuteIfBound(CurrentStage, CastStages[CurrentStage].SectionName, ECastAnimStatus::Proceed);
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

