#include "AbilityComponent.h"

#include "AbilitySystem/AbilitySystemLog.h"
#include "AbilitySystem/AbstractAbility.h"
#include "Classes/Engine/World.h"


/*
bool AAbilityComponent::Execute(AActor *Target,
                                const TArray<TSubclassOf<UAbilityAction>> &List,
                                int Empowerment, ACharacter *Caster, AActor *Causer)
{
    check(Target != nullptr && "AbilityComponent: target not found")

      for (auto &action : List)
    {
        auto obj = action.GetDefaultObject();
        check(obj != nullptr && "AbilityComponent: action is not valid");
        obj->Caster = Caster;
        obj->Causer = Causer;
        obj->Execute(Target, Empowerment);
        obj->ConditionalBeginDestroy();
    }
    return true;
}
*/
/*
bool USpawnAction::Execute_Implementation(AActor *Target, int32 Empowerment)
{
    check(SkillComponent != nullptr && "Spawn actor: invalid actor");
    check(Target->GetWorld() != nullptr && "Spawn actor: invalid world");

    FActorSpawnParameters SpawnInfo;

    FVector TargetLocation  = Target->GetActorLocation();
    FRotator TargetRotation = Target->GetActorRotation();

    ACharacter *character = Cast<ACharacter>(Caster);

    check(character != nullptr && "Spawn actor: invalid caster");
    TargetRotation        = character->GetController()->GetControlRotation();
    FVector ForwardVector = character->GetActorForwardVector();

    AAbilityComponent *SklCmpt = Target->GetWorld()->SpawnActor<AAbilityComponent>(
      SkillComponent, TargetLocation + PositionOffset, TargetRotation + RotationOffset,
      SpawnInfo);
    check(SklCmpt != nullptr && "Spawn actor: invalid skill component");
    SklCmpt->ComponentSetup(character, Empowerment);
    return true;
}*/

/*
AAbilityComponent *FAbilityComponentSpawnParameters::Spawn(AActor *Caster, int param)
{
    check(SkillComponent != nullptr && "Spawn actor: invalid actor");
    check(Caster->GetWorld() != nullptr && "Spawn actor: invalid world");

    FActorSpawnParameters SpawnInfo;

    FVector TargetLocation  = Caster->GetActorLocation();
    FRotator TargetRotation = Caster->GetActorRotation();

    ACharacter *character = Cast<ACharacter>(Caster);
    if (character)
    {
        TargetRotation        = character->GetController()->GetControlRotation();
        FVector ForwardVector = character->GetActorForwardVector();


        TargetLocation.Z += LocationOffset.Z;
        TargetLocation.X += ForwardVector.X * LocationOffset.X;
        TargetLocation.Y += ForwardVector.Y * LocationOffset.Y;
    }
    else
    {
        TargetLocation.X += 90 - TargetRotation.Yaw;
    }

    FVector Location   = TargetLocation;
    FRotator Roatation = TargetRotation + RotationOffset;

    AAbilityComponent *SklCmpt = Caster->GetWorld()->SpawnActor<AAbilityComponent>(
      SkillComponent, Location, Roatation, SpawnInfo);
    check(SklCmpt != nullptr && "Spawn actor: invalid skill component");

    SklCmpt->ComponentSetup(character, param);
    return SklCmpt;
}*/

UAbilityComponent::UAbilityComponent(const FObjectInitializer &f) :
    Super(f)
{
    UE_LOG(AbilitySystemLog, Error, TEXT("AbilityComponent: Construction"));
}

void UAbilityComponent::ComponentInit(UAbstractAbility *parent)
{
    parent->AddComponent(this);
    OnComponentInit();
}


bool UAbilityComponent::ComponentSetup(UCombatComponent *Component)
{
    UE_LOG(AbilitySystemLog, Error, TEXT("AbilityComponent: Setup"));

    CombatComponent = Component;
    OnComponentSetup();
    return true;
}
