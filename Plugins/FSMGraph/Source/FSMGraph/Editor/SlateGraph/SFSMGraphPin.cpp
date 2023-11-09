#include "SFSMGraphPin.h"

void SFSMGraphPin::Construct(const FArguments &InArgs, UEdGraphPin *InPin)
{
    this->SetCursor(EMouseCursor::Default);
    bShowLabel = true;

    GraphPinObj = InPin;
    SBorder::FArguments Arguments;

    Arguments.OnMouseButtonDown(this, &SFSMGraphPin::OnPinMouseDown);
    Arguments.Cursor(this, &SFSMGraphPin::GetPinCursor);
    Arguments.Padding(FMargin(10.f));

    SBorder::Construct(Arguments);
}

TSharedRef<SWidget> SFSMGraphPin::GetDefaultValueWidget()
{
    return SNew(STextBlock);
}

