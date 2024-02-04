#pragma once

#include "CoreMinimal.h"
#include "EdGraphUtilities.h"
#include "FSMGraphFactory.generated.h"



struct FSMGraphPinConnectionFactory : public FGraphPanelPinConnectionFactory
{
public:
    class FConnectionDrawingPolicy *CreateConnectionPolicy(const class UEdGraphSchema * Schema, int32 InBackLayerID, int32 InFrontLayerID, float ZoomFactor, const class FSlateRect &InClippingRect, class FSlateWindowElementList &InDrawElements, class UEdGraph *InGraphObj) const override;
};

UCLASS()
class UFSMGraphAssetFactory : public UFactory
{
    GENERATED_BODY()

public:
    UFSMGraphAssetFactory(const FObjectInitializer &ObjectInitializer);

	UObject *FactoryCreateNew(UClass *Class, UObject *InParent, FName Name, EObjectFlags Flags, UObject *Context, FFeedbackContext *Warn) override;
    bool CanCreateNew() const override;
    bool ShouldShowInNewMenu() const override;
};

