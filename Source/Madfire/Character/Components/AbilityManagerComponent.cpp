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

    if (Owner->CCStateManager.HasCC())
        return;

    if (CurrentAbility != nullptr)
    {
        if (AbilityPressed)
        {
            auto CombatComp = Owner->GetCombatComponent();
            if (!CurrentAbility->bIsActive)
                CombatComp->UpdateCombatFSM();
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
    }
}

void UAbilityManagerComponent::ReleaseAbility()
{
    if(CurrentAbility->IsValidLowLevelFast())
    {
        CurrentAbility->ReleaseAbility();
        CurrentAbility->Release();
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
    if(!Owner) return false;
    check(Owner != nullptr && "Shit ");
    auto CombatComp = Owner->GetCombatComponent();
    return CombatComp->HasCombatStance() && CombatComp->GetCombatFSM().isInState(ECombatState::Ability);
}
