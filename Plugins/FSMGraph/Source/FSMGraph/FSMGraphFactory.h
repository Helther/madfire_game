#pragma once

#include "CoreMinimal.h"
#include "FSMGraphFactory.generated.h"


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

