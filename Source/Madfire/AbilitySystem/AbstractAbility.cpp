#include "AbstractAbility.h"
#include "Character/Components/CombatComponent.h"
#include "Character/CharacterBase.h"

DEFINE_LOG_CATEGORY(AbilitySystemLog);


UAbstractAbility::UAbstractAbility(const FObjectInitializer &f) :
    Super(f)
{
}

void UAbstractAbility::AbilitySetup(ACharacter *Character)
{    
    CharacterOwner = Character;
    /*for(auto &component : AbilityComponents)
    {
        component->ComponentSetup(Component);
    }*/
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

UAbilityComponent *UAbstractAbility::GetComponentByName(FName Name) const
{
    return nullptr;
}

float UAbstractAbility::GetCooldownTimeRemaining() const
{
    return CooldownTime - CurrentCooldownTime;
}

const FAbilityInfo &UAbstractAbility::GetInfo() const
{
    return GetAbilityInfo(InfoDataTable, AbilityName);
}

ACharacterBase *UAbstractAbility::GetOwner() const
{
    check(CharacterOwner && "No Combat Component owner");
    auto Character = Cast<ACharacterBase>(CharacterOwner);
    check(Character && "No owner");
    return Character;
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
    OnAbilityCancel();
}

void UAbstractAbility::Interrupt()
{
    OnAbilityInterrupt();
}

bool UAbstractAbility::StopAnimation()
{
    // auto Owner = Cast<ACharacter>(CombatComponent->GetOwner());
    if(!CharacterOwner)
        return false;

    //Owner->StopAnimMontage(CastAnimMontage);
    return true;
}

bool UAbstractAbility::StartAnimation(FName SectionName, FOnMontageBlendingOutStarted Delegate)
{
    if(!CastAnimMontage)
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Ability: No CastAnimMontage is not set"));
        return false;
    }

    if(!CharacterOwner)
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Ability: AbilitySystemComponent Owner is not set"));
        return false;
    }

    // Owner->PlayAnimMontage(CastAnimMontage,1.f,SectionName);//PrecastSectionName);

    //auto animInstance    = Owner->GetMesh()->GetAnimInstance();
    //auto montageInstance = animInstance->GetActiveInstanceForMontage(CastAnimMontage);
    //montageInstance->OnMontageBlendingOutStarted = Delegate;//OnPrecastFinish;

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

const FAbilityInfo &UAbstractAbility::GetAbilityInfo(UDataTable *AbilityInfoDT, FName AblName)
{
    static FAbilityInfo s;
    FString ContextString;

    if(!IsValid(AbilityInfoDT))
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Ability: Info Data Table invalid"));
        return s;
    }

    auto info = AbilityInfoDT->FindRow<FAbilityInfo>(AblName,ContextString);

    if(info)
    {
        return *info;
    }
    else
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Ability: Info not found"));
    }

    return s;
}


