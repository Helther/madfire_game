#include "AbilityManagerComponent.h"
#include "Character/CharacterCreature.h"




UAbilityManagerComponent::UAbilityManagerComponent(const FObjectInitializer &ObjectInitializer) 
{

}


void UAbilityManagerComponent::BeginPlay() 
{    
    Super::BeginPlay();
    Owner = Cast<ACharacterCreature>(GetOwner());
    check(Owner != nullptr && "UCombatComponent: No pawn");
}

inline void UAbilityManagerComponent::OnAbilityEnter()
{
    if (!Owner->GetCombatComponent()->WeaponDrawn()) // not in combat mode
    {
        Owner->GetMovementComponent()->maxSpeedBlock                 = true;
        Owner->GetMovementComponent()->bUseControllerDesiredRotation = true;
        Owner->GetMovementComponent()->bOrientRotationToMovement     = false;
    }
    CalculateMaxCastingSpeed(Owner->GetMovementComponent()->GetSpeedModifier());
    Owner->GetAttributeComponent()->Stamina->Consume(Owner->GetAbilityManager()->CurrentAbility->StaminaCost);
    Owner->GetAttributeComponent()->Stamina->BlockRegen();
    Owner->GetMovementComponent()->MoveAbilitiesCurrent = CurrentAbility->MoveAbilities;
}

inline void UAbilityManagerComponent::OnAbilityLeave()
{
    if (Owner->GetCombatComponent()->WeaponDrawn()) // in combat mode
        Owner->GetCombatComponent()->CalculateMaxCombatSpeeds();
    else
    {
        Owner->GetMovementComponent()->maxSpeedBlock = false;
        Owner->GetMovementComponent()->CalculateMaxSpeeds(Owner->GetMovementComponent()->GetSpeedModifier());
        Owner->GetMovementComponent()->bUseControllerDesiredRotation = false;
        Owner->GetMovementComponent()->bOrientRotationToMovement     = true;
    }
    Owner->GetAttributeComponent()->Stamina->UnblockRegen();
    Owner->GetMovementComponent()->MoveAbilitiesCurrent = Owner->GetMovementComponent()->MoveAbilitiesDefault; // TODO think how to restore it to last state
    SetGlobalCooldown();
    Owner->ClearAbilityCameraMode();
}

void UAbilityManagerComponent::Update()
{
    auto character = Cast<ACharacterBase>(GetOwner());
    auto attributes = character->GetAttributeComponent();
    auto intelect = attributes->Attributes.AttributeMap[(int32)EAttributes::Int];
    auto wisdom = attributes->Attributes.AttributeMap[(int32)EAttributes::Wis];

    LevelSpellList.SetNum(wisdom,true);
    for(auto &list : LevelSpellList)
        list.SpellList.SetNum(intelect,true);

    AbilityList.Empty();

    AbilityList.Append(ExtraAbilityList);
    AbilityList.Append(SpellAbilityList);
    AbilityList.Append(EquipAbilityList);
    AbilityList.Append(TalentAbilityList);
}

UAbstractAbility *UAbilityManagerComponent::GetAbility(FName AbilityName)
{
    if(!AbilityList.Contains(AbilityName))
        return nullptr;
    return AbilityList[AbilityName];
} 

void UAbilityManagerComponent::UpdateSpellBook()
{
    SpellAbilityList.Empty();

    auto owner = GetOwner();
 
    for(const auto &spellLevel : LevelSpellList)
    {
       
        for(const auto &spell : spellLevel.SpellList)
        {
            //if(!spell) continue;

            const auto &info = spell.GetAbilityInfo();

            if(!info.AbilityClass)
                continue;
            auto abl = UAbstractAbility::SpawnAbility(owner,info.AbilityClass);
            if(abl)
            {
                const auto name = abl->AbilityName;
                if (SpellAbilityList.Contains(name))
                {
                    SpellAbilityList[name]->MaxCharges += abl->GetInfo().MaxCharges;
                    SpellAbilityList[name]->CurrentCharges = SpellAbilityList[name]->MaxCharges;
                      
                }
                else
                {
                    SpellAbilityList.Add(abl->AbilityName, abl);
                }
            }
        }
    }

    Update();
}


void UAbilityManagerComponent::OnUseAbility(bool Started)
{
    AbilityPressed = Started;

    if (Owner->GetССComponent()->HasCC())
        return;

    if (CurrentAbility != nullptr)
    {
        if (AbilityPressed)
        {
            ActivateAbility();
        }
        else
            ReleaseAbility();
    }
}

void UAbilityManagerComponent::CalculateMaxCastingSpeed(float SpeedModifier)
{
    Owner->GetMovementComponent()->MaxRunSpeed = MaxSpeedWhileCasting * SpeedModifier;
}

void UAbilityManagerComponent::ActivateAbility()
{
    if(CurrentAbility->IsValidLowLevelFast())
    {
        CurrentAbility->ActivateAbility();
        CurrentAbility->Activate();
        Owner->SetAbilityCameraMode(CurrentAbility->AbilityCameraMode);
        OnAbilityEnter();
    }
}

void UAbilityManagerComponent::ReleaseAbility()
{
    if(CurrentAbility->IsValidLowLevelFast())
    {
        CurrentAbility->ReleaseAbility();
        CurrentAbility->Release();
        OnAbilityLeave();
    }
    
}

void UAbilityManagerComponent::SetCurrentAbility(UAbstractAbility *Ability)
{
    if(CurrentAbility)
    {
        CurrentAbility->CancelAbility();
    }

    CurrentAbility = Ability;
    //if(CurrentAbility)
        //CurrentAbility->AbilitySetup(this);
}

bool UAbilityManagerComponent::IsAbilityPressed() const 
{
    return AbilityPressed; 
}

bool UAbilityManagerComponent::IsOnGlobalCooldown() const 
{
    return GetWorld()->GetTimerManager().IsTimerActive(GlobalCooldownTimer); 
}

void UAbilityManagerComponent::SetGlobalCooldown() 
{
    GetWorld()->GetTimerManager().SetTimer(GlobalCooldownTimer, GlobalCooldownTime, false); 
}

bool UAbilityManagerComponent::IsUsingAbility() const
{
    return CurrentAbility && CurrentAbility->bIsActive;
}
