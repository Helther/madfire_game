#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "GraphFactory.generated.h"

UCLASS()
class UGraphFactory : public UFactory
{
	GENERATED_BODY()

public:
    UGraphFactory();
    ~UGraphFactory() override;

	UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};
