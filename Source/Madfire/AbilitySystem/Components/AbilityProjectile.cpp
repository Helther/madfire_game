// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilityProjectile.h"
#include "AbilitySystem/AbstractAbility.h"

AAbstractProjectile::AAbstractProjectile()
{
    PrimaryActorTick.bCanEverTick = true;
    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Component"));
    SetRootComponent(CollisionComp);

    MovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
    MovementComponent->InitialSpeed             = 1000.0f;
    MovementComponent->MaxSpeed                 = 3000.0f;
    MovementComponent->bRotationFollowsVelocity = true;
    MovementComponent->bShouldBounce            = true;
    MovementComponent->UpdatedComponent         = RootComponent;
    MovementComponent->ProjectileGravityScale   = 0.00001f;

}

void AAbstractProjectile::BeginPlay()
{
    Super::BeginPlay();
}

void AAbstractProjectile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AAbstractProjectile::Execute(const TArray<AActor *> &Actors, FVector Location, FRotator Rotation)
{
    if(!OnExecuteEvent.IsBound())
    {
        UE_LOG(AbilitySystemLog, Error, TEXT("Projectile: None been taken"));
    }
    OnExecuteEvent.ExecuteIfBound(Actors,Location,Rotation);
}
