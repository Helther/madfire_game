#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "Interactable.generated.h"


UENUM(BlueprintType, meta = (Bitflags))
enum class EInteractType : uint8
{
    NONE       = 0,
    Use        = 1,
    Pickup     = 2,
    Talk       = 4,
    Inspect    = 8,
    Steal      = 16,
    Move       = 32,
    DisarmTrap = 64,
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
    bool OnInteract(EInteractType EInteractType, ACharacter *controller);

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Interactable")
    uint8 GetInteractFlags() const;

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Interactable")
    bool OnBeginFocus();

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Interactable")
    bool OnEndFocus();
};