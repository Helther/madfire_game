// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile.h"

#include "AbilitySystemLog.h"

AAbstractProjectile::AAbstractProjectile()
{
    PrimaryActorTick.bCanEverTick = true;
    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Component"));
    CollisionComp->OnComponentHit.AddDynamic(this, &AAbstractProjectile::Hit);
    CollisionComp->OnComponentBeginOverlap.AddDynamic(this,
                                                      &AAbstractProjectile::Overlap);
    CollisionComp->OnComponentEndOverlap.AddDynamic(this,
                                                    &AAbstractProjectile::EndOverlap);
    SetRootComponent(CollisionComp);

    MovementComponent =
      CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
    MovementComponent->InitialSpeed             = 1000.0f;
    MovementComponent->MaxSpeed                 = 3000.0f;
    MovementComponent->bRotationFollowsVelocity = true;
    MovementComponent->bShouldBounce            = true;
    MovementComponent->UpdatedComponent         = RootComponent;
    MovementComponent->ProjectileGravityScale   = 0.00001f;

    //OnDestroyed.AddDynamic(this, &AProjectile::Destroy);
}

void AAbstractProjectile::BeginPlay() { Super::BeginPlay(); }

void AAbstractProjectile::Tick(float DeltaTime) { Super::Tick(DeltaTime); }

void AAbstractProjectile::Hit(UPrimitiveComponent *HitComponent,
                              AActor *OtherActor,
                              UPrimitiveComponent *OtherComp,
                              FVector NormalImpulse,
                              const FHitResult &Hit)
{
    if (AAbilityComponent::Execute(
          OtherActor, ActionList, ActionEmpowerment, Caster, this))
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Component: yay it is a hit"));
    }
    else
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Component: target is not found"));
    }
}


void AAbstractProjectile::Overlap(UPrimitiveComponent *OverlappedComponent,
                                  AActor *OtherActor,
                                  UPrimitiveComponent *OtherComp,
                                  int32 OtherBodyIndex,
                                  bool bFromSweep,
                                  const FHitResult &SweepResult)
{
    if (AAbilityComponent::Execute(
          OtherActor, ActionList, ActionEmpowerment, Caster, this))
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Component: yay it is an overlap"));
    }
    else
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Component: target is not found"));
    }
}


void AAbstractProjectile::EndOverlap(UPrimitiveComponent *OverlappedComponent,
                                     AActor *OtherActor,
                                     UPrimitiveComponent *OtherComp,
                                     int32 OtherBodyIndex)
{
    UE_LOG(LogTemp, Warning, TEXT("EndOverlap( )"));
}