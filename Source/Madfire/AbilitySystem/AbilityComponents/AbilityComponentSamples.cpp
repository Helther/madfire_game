#include "AbilityComponentSamples.h"
#include "Character/CharacterBase.h"

void UAbilitySelfSelection::ActivateComponent_Implementation()
{
    ACharacterBase *Owner = nullptr;
    if(AbilitySystemComponent && (Owner = Cast<ACharacterBase>(AbilitySystemComponent->GetOwner())))
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

    if(!AbilitySystemComponent)
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("UAbilityProjectileSpawn: No Ability System Component is Set"));
        return;
    }

    auto Owner = Cast<ACharacterBase>(AbilitySystemComponent->GetOwner());
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
