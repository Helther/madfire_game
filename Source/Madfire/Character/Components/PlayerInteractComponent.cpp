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

    FVector startTrace = Owner->GetActorLocation();
    startTrace.Z += 50;
    FVector direction = cameraRotation.Vector();
    float cv = FMath::Abs(sin((cameraRotation.Pitch/180)*PI));

    FVector endTrace  = startTrace + (direction * (MaxInteractionDistance + CorrectionValue*cv));

    FTraceParams traceParams;
    traceParams.isDebug = drawTrace;

    traceParams.ActorsToIgnore.Add(Owner);
    FTraceSpawnParams traceSpawnParams;
    traceSpawnParams.Start = startTrace;
    traceSpawnParams.End   = endTrace;


    FHitResult hit(ForceInit);
    check(GetWorld() != nullptr && "Interactable Component: No world");

    if(UTraceManager::LineTraceSingleByChannel(GetWorld(), traceParams, traceSpawnParams, hit, ECollisionChannel::ECC_Visibility))
    {
        traceSpawnParams.End = startTrace + ((direction) * hit.Distance);
    }


    hit = FHitResult(ForceInit);

    UTraceManager::SphereTraceSingleByChannel(GetWorld(),SphereRadius,traceParams, traceSpawnParams, hit,EInteractChannel);

    TScriptInterface<IInteractableInterface> result;

    //auto interactable = Cast<IInteractableInterface*>(hit.GetActor()); //old Version

    const auto actor = hit.GetActor();


    if (actor != nullptr)
    {
        const bool isInteractable = hit.GetActor()->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass());
        if (isInteractable)
        {
            result.SetInterface(actor);
            result.SetObject(hit.GetActor());
        }
    }

    return result;
}

void UPlayerInteractComponent::CheckForInteractable()
{
    if (!Owner || !Owner->Controller)
        return;

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
