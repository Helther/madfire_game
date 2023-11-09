#pragma once

#include "CoreMinimal.h"
#include "SGraphPin.h"


class SFSMGraphPin : public SGraphPin
{

public:
	void Construct(const FArguments &InArgs, UEdGraphPin *InPin);
	TSharedRef<SWidget> GetDefaultValueWidget() override;
};