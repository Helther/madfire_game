#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "Interactable.generated.h"


UENUM(BlueprintType, meta = (Bitflags))
enum class EInteractType : uint8
{
    NONE       = 0,  //00
    Use        = 1,  //01
    Pickup     = 2,  //02
    Talk       = 4,  //04
    Inspect    = 8,  //08
    Steal      = 16, //10
    Move       = 32, //20
    Open       = 64, //40
    MAX        = 128,//80
    /// Some other actions
};
ENUM_CLASS_FLAGS(EInteractType)


UINTERFACE(Blueprintable, MinimalAPI)
class UInteractableInterface : public UInterface
{
    GENERATED_UINTERFACE_BODY()
};

class IInteractableInterface
{
    GENERATED_IINTERFACE_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Interactable")
    bool OnInteract(EInteractType InteractType, ACharacter *controller);

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Interactable")
    uint8 GetInteractFlags() const;

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Interactable")
    FText GetInteractableName() const;

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Interactable")
    bool OnBeginFocus();

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Interactable")
    bool OnEndFocus();
};
