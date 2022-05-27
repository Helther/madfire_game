#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "Interactable.generated.h"


UENUM(BlueprintType, meta = (Bitflags))
enum class EInteractType : uint8
{
    NONE       = 0,
    Use        = 1, //1
    Pickup     = 2, //2
    Talk       = 3, //4
    Inspect    = 4, //8
    Steal      = 5, //32
    Move       = 6, //64
    Open       = 7, //128
    MAX           ,
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
    UPARAM(meta=(Bitmask, BitmaskEnum = EInteractType)) int32 GetInteractFlags() const;

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Interactable")
    FText GetInteractableName() const;

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Interactable")
    bool OnBeginFocus();

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Interactable")
    bool OnEndFocus();
};
