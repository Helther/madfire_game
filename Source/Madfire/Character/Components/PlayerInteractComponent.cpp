// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerInteractComponent.h"

#include "TraceFunctional/TraceDefines.h"
#include "Kismet/GameplayStatics.h"


// Sets default values for this component's properties
UPlayerInteractComponent::UPlayerInteractComponent()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;

    // ...
}


// Called when the game starts
void UPlayerInteractComponent::BeginPlay()
{
    Super::BeginPlay();

    // ...
}

void UPlayerInteractComponent::OnComponentCreated()
{
    Super::OnComponentCreated();
    Owner = Cast<ACharacter>(GetOwner());
}


// Called every frame
void UPlayerInteractComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


    CheckForInteractable();
    // ...
}

TScriptInterface<IInteractableInterface> UPlayerInteractComponent::GetInteractableInView()
{
    FVector cameraLocation;
    FRotator cameraRotation;

    Owner->Controller->GetPlayerViewPoint(cameraLocation, cameraRotation);

    FVector actorLocation = Owner->GetActorLocation();
    actorLocation.Z += 50;
    FVector direction = cameraRotation.Vector();
    float cv = FMath::Abs(sin((cameraRotation.Pitch/180)*PI));

    float cameraOffset = (cameraLocation - actorLocation).Length();

    FVector cameraEndTrace = cameraLocation + (direction * (MaxInteractionDistance + cameraOffset + CorrectionValue*cv));
    FVector endTrace  = actorLocation + (direction * (MaxInteractionDistance + CorrectionValue*cv));

    FTraceParams traceParams;
    traceParams.isDebug = drawTrace;

    traceParams.ActorsToIgnore.Add(Owner);
    FTraceSpawnParams traceSpawnParams;
    traceSpawnParams.Start = cameraLocation;
    traceSpawnParams.End   = cameraEndTrace;

    
    FHitResult hit(ForceInit);
    check(GetWorld() != nullptr && "Interactable Component: No world");
    TScriptInterface<IInteractableInterface> result;

    if(UTraceManager::LineTraceSingleByChannel(GetWorld(), traceParams, traceSpawnParams, hit, ECollisionChannel::ECC_Visibility))
    {
        traceSpawnParams.End = actorLocation + ((direction) * hit.Distance);
    }


    hit = FHitResult(ForceInit);

    UTraceManager::SphereTraceSingleByChannel(GetWorld(),SphereRadius,traceParams, traceSpawnParams, hit, EInteractChannel);
    
    //auto interactable = Cast<IInteractableInterface*>(hit.GetActor()); //old Version

    const auto actor = hit.GetActor();


    if (actor != nullptr)
    {
        const bool isInteractable = actor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass());
        if (isInteractable)
            result = TScriptInterface<IInteractableInterface>(actor);
    }

    return result;
}

void UPlayerInteractComponent::CheckForInteractable()
{
    check(Owner != nullptr && "Player Interact component: Owner is Invalid");
    check(Owner->Controller != nullptr && "Player Interact component: Controller is Invalid");

    auto interactable = GetInteractableInView();

    if (FocusedInteractable != interactable)
    {
        if (FocusedInteractable)
            FocusedInteractable->Execute_OnEndFocus(
              Cast<UObject>(FocusedInteractable.GetObject()));

        HasNewFocus = true;
    }

    FocusedInteractable = interactable;

    if (interactable && HasNewFocus)
    {
        interactable->Execute_OnBeginFocus(Cast<UObject>(FocusedInteractable.GetObject()));

        HasNewFocus = true;
    }
}

TArray<EInteractType> UPlayerInteractComponent::InteractList(int32 interactFlags)
{
    TArray<EInteractType> result;
    //TODO: It
    for(int32 flag = 1; flag < static_cast<uint8>(EInteractType::MAX); flag++)
    {
        int32 mask = 1 << flag;
        if((interactFlags & mask) != 0)
            result.Add(static_cast<EInteractType>(flag));
    }
    return result;
}
