#include "AbilityTargetSelectionComponent.h"
#include "AbilitySystem/AbstractAbility.h"


void UAbilityTargetSelection::ComponentInit(UAbstractAbility *parent)
{
    Super::ComponentInit(parent);
}

bool UAbilityTargetSelection::ComponentSetup(ACharacter *Component)
{
    return Super::ComponentSetup(Component);

}

void UAbilityAreaSelection::TraceActors(FAbilityTargetInfo info, float Radius, FCollisionShape CollisionShape)
{
    auto Actor = GetActorOwner();
    check(Actor);
    auto World = Actor->GetWorld();
    check(World);

    FVector Start = info.Position;
    FVector End = info.End;
    FQuat   Rotation = info.Rotation;
    
    ECollisionChannel CollisionChannel = ECC_Pawn;
    FCollisionResponseParams ResponseParams(CollisionResponse);    

    FCollisionQueryParams Params;
    if(bIgnoreOwner) ActorsToIgnore.AddUnique(Actor);
    Params.AddIgnoredActors(ActorsToIgnore);
    Params.bIgnoreBlocks = bIgnoreBlocks;
    Params.bIgnoreTouches = bIgnoreTouches;
    Params.OwnerTag = OwnerTag;
    Params.TraceTag = TraceTag;

    TArray<FHitResult> HitResults;

    bool isHit = World->SweepMultiByChannel(HitResults, Start, End, Rotation, CollisionChannel, CollisionShape , Params);

    if(isHit)
        for(auto HitResult : HitResults)
            for(auto Action : info.AbilityActions)
                Action->Execute(HitResult);        
    
}

void UAbilitySphereSelection::Execute_Implementation(FAbilityTargetInfo info)
{
    TraceActors(info, Radius, FCollisionShape::MakeSphere(Radius));
}

AAbilityActor::AAbilityActor()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AAbilityActor::BeginPlay()
{
    Super::BeginPlay();
}

void AAbilityActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}


void UAbilityActorSpawner::Execute_Implementation(FAbilityTargetInfo info) 
{
    auto OwnerActor = GetActorOwner();

    FActorSpawnParameters params;

    auto Object = OwnerActor->GetWorld()->SpawnActor<AAbilityActor>(ActorClass, SpawnPoint, params);

    Object->TargetInfo = info;
    Object->UAbilityActorSpawner = this;
}