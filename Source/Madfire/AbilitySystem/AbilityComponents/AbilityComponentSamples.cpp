#include "AbilityComponentSamples.h"
#include "Character/CharacterBase.h"

#if _MSC_VER 
#pragma warning( disable : 4706 )
#endif

void UAbilitySelfSelection::ActivateComponent_Implementation()
{
    ACharacterBase *Owner = nullptr;
    if(CombatComponent && (Owner = Cast<ACharacterBase>(CombatComponent->GetOwner())))
        OnTargetChoosed.Broadcast(Owner);
    else
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Ability Self Component: Component or Owner Is Null"));
    }
}

void UAbilityProjectileSpawn::ActivateComponent_Implementation()
{
    if(!ProjectileClass)
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("UAbilityProjectileSpawn: No Projectile Class is Set"));
        return;
    }

    if(!CombatComponent)
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("UAbilityProjectileSpawn: No Ability System Component is Set"));
        return;
    }

    auto Owner = Cast<ACharacterBase>(CombatComponent->GetOwner());
    auto Controller = Owner->GetController();

    if(!(Owner && Controller))
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("UAbilityProjectileSpawn: No Owner or Controller"));
        return;
    }

    FVector Location;
    FRotator CameraRotation;

    Controller->GetPlayerViewPoint(Location,CameraRotation);
    Location = Owner->GetMesh()->GetSocketLocation(SocketName);
    Location += CameraRotation.Vector() * SpawnOffset;
    //Location.ForwardVector*=SpawnOffset;

    auto Actor = Owner->GetWorld()->SpawnActor(ProjectileClass, &Location, &CameraRotation);
    AAbstractProjectile *projectile = Cast<AAbstractProjectile>(Actor);
    projectile->OnTargetCoosed = OnTargetChoosed;

}
