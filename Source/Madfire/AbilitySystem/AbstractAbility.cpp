#include "AbstractAbility.h"
#include "Character/Components/CombatComponent.h"


DEFINE_LOG_CATEGORY(AbilitySystemLog);


UAbstractAbility::UAbstractAbility(const FObjectInitializer &f) :
    Super(f)
{}

void UAbstractAbility::AbilitySetup(UCombatComponent *Component)
{
    CombatComponent = Component;
    for(auto &component : AbilityComponents)
    {
        component->ComponentSetup(Component);
    }
}

void UAbstractAbility::ActivateAbility()
{
    if(CurrentCharges <= 0)
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Ability: Cooldown"));
        return;
    }
    Activate();
}

void UAbstractAbility::ReleaseAbility()
{
    if(CurrentCharges <= 0)
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Ability: Cooldown"));
        return;
    }
    Release();
}

void UAbstractAbility::CancelAbility()
{
    Cancel();
}

void UAbstractAbility::Tick(float DeltaTime)
{
    if(bIsCooldown)
    {
        if(CurrentCharges < MaxCharges)
        {
            CurrentCooldownTime+=DeltaTime;
            if(CurrentCooldownTime >= CooldownTime)
            {
                CurrentCharges+=ChargePerCooldown;
                if(CurrentCharges > MaxCharges)
                    CurrentCharges = MaxCharges;
            }
        }
        else
        {
            bIsCooldown = false;
        }
    }
}

bool UAbstractAbility::IsTickable() const
{
    return true;
}

bool UAbstractAbility::IsTickableInEditor() const
{
    return false;
}

bool UAbstractAbility::IsTickableWhenPaused() const
{
    return false;
}

TStatId UAbstractAbility::GetStatId() const
{
    return TStatId();
}

float UAbstractAbility::GetCooldownTimeRemaining() const
{
    return CooldownTime - CurrentCooldownTime;
}

UWorld* UAbstractAbility::GetWorld() const
{
    return GetOuter()->GetWorld();
}

void UAbstractAbility::AbilityInit()
{
    OnAbilityInit();
}

void UAbstractAbility::Activate()
{
    OnAbilityStart();
}

void UAbstractAbility::Release()
{
    OnAbilityEnd();
}

void UAbstractAbility::Cancel()
{
    bIsActive = false;
    OnAbilityCancel();
    check(CombatComponent && "Cancel: Bad component!");
    CombatComponent->UpdateCombatFSM();
}

void UAbstractAbility::Interrupt()
{
    bIsActive = false;
    OnAbilityInterrupt();
    check(CombatComponent && "Interrupt: Bad component!");
    CombatComponent->UpdateCombatFSM();
}

bool UAbstractAbility::StopAnimation()
{
    if(!CombatComponent)
        return false;

    auto Owner = Cast<ACharacter>(CombatComponent->GetOwner());
    if(!Owner)
        return false;

    Owner->StopAnimMontage(CastAnimMontage);
    return true;
}

bool UAbstractAbility::StartAnimation(FName SectionName, FOnMontageBlendingOutStarted Delegate)
{
    if(!CastAnimMontage)
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Ability: No CastAnimMontage is not set"));
        return false;
    }

    if(!CombatComponent)
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Ability: AbilitySystemComponent is not set"));
        return false;
    }

    if(!CombatComponent->GetOwner())
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Ability: AbilitySystemComponent Owner is not set"));
        return false;
    }


    auto Owner = Cast<ACharacter>(CombatComponent->GetOwner());

    if(!Owner)
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Ability: Owner is not CharacterBase"));
        return false;
    }

    Owner->PlayAnimMontage(CastAnimMontage,1.f,SectionName);//PrecastSectionName);

    auto animInstance    = Owner->GetMesh()->GetAnimInstance();
    auto montageInstance = animInstance->GetActiveInstanceForMontage(CastAnimMontage);
    montageInstance->OnMontageBlendingOutStarted = Delegate;//OnPrecastFinish;

    return true;
}


UAbstractAbility *UAbstractAbility::SpawnAbility(UObject *Parent, TSubclassOf<UAbstractAbility> AbilityClass, FName Name)
{
    check((AbilityClass) && "Ability Class is Invalid");
    auto NewAbility = NewObject<UAbstractAbility>(Parent, AbilityClass, Name);
    if(!NewAbility)
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Ability Functional: Can't create ability"));
        return nullptr;
    }
    NewAbility->AbilityInit();
    return NewAbility;
}

UAbilityComponent *UAbstractAbility::CreateComponent(UAbstractAbility *Parent, TSubclassOf<UAbilityComponent> ComponentClass, FName Name)
{
    check((ComponentClass) && "Ability Component is Invalid");
    auto Comp = NewObject<UAbilityComponent>(Parent, ComponentClass, Name);
    if(!Comp)
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Ability Functional: Can't create Target Component"));
        return nullptr;
    }
    Comp->ComponentInit(Parent);

    return Comp;
}

AActor *UAbstractAbility::SpawnAbilityActor(AActor *Owner, TSubclassOf<AActor> ActorClass, FAbilityActorSpawnParameters param, FName Name)
{
    check((ActorClass) && "Actor Class is Invalid");

    constexpr bool NoCollisionFail = false; //A bool that determines whether a collision test will be performed when spawning the Actor. If true, no collision test will be performed when spawning the Actor regardless of the collision settings of the root component or template Actor.
    constexpr bool RemoteOwned = false; //Is the actor remotely owned. This should only be set true by the package map when it is creating an actor on a client that was replicated from the server.
    constexpr bool DeferConstruction = false; //Only applicable if the Actor is being spawned from a Blueprint.

    if(Owner)
    {
        auto Actor = Owner->GetWorld()->SpawnActor(ActorClass, &param.Location, &param.Rotation);
        Actor->SetOwner(Owner);
        return Actor;
    }
    else
        return nullptr;
}
