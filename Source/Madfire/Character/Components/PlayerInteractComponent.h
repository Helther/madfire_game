// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../../Interactable/Interactable.h"
#include "../../TraceFunctional/TraceFunctional.h"
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "PlayerInteractComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UPlayerInteractComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    UPlayerInteractComponent();

    void OnComponentCreated() override;

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction *ThisTickFunction) override;

protected:
    UPROPERTY(BlueprintReadWrite, Category = "Interaction")
    TScriptInterface<IInteractableInterface> FocusedInteractable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float MaxInteractionDistance = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
    bool drawTrace = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float SphereRadius = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float CorrectionValue = 400;

    TScriptInterface<IInteractableInterface> GetInteractableInView();

    void CheckForInteractable();

    bool HasNewFocus;
    ACharacter *Owner = nullptr;
};
