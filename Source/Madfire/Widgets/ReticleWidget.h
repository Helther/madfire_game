#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ReticleWidget.generated.h"


UCLASS()
class MADFIRE_API UReticleWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    UReticleWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    /** Returns the current weapon's diametrical spread angle, in degrees */
    UFUNCTION(BlueprintCallable, BlueprintPure)
    float ComputeSpreadAngle() const;

    /** Returns the current weapon's maximum spread radius in screenspace units (pixels) */
    UFUNCTION(BlueprintCallable, BlueprintPure)
    float ComputeMaxScreenspaceSpreadRadius() const;
};
