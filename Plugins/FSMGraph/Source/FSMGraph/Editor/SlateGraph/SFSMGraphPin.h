#pragma once

#include "CoreMinimal.h"
#include "SGraphPin.h"


class SFSMOutputPin : public SGraphPin
{
public:
	SLATE_BEGIN_ARGS(SFSMOutputPin){}
	SLATE_END_ARGS()

	void Construct(const FArguments &InArgs, UEdGraphPin *InPin)
	{
		this->SetCursor(EMouseCursor::Default);

		bShowLabel =true;

		GraphPinObj = InPin;
		check(GraphPinObj != nullptr);

		const UEdGraphSchema *Schema = GraphPinObj->GetSchema();
		check(Schema);

		SBorder::Construct( SBorder::FArguments()
			.BorderImage(this, &SFSMOutputPin::GetPinBorder)
			.BorderBackgroundColor(this, &SFSMOutputPin::GetPinColor)
			.OnMouseButtonDown(this, &SFSMOutputPin::OnPinMouseDown)
			.Cursor(this, &SFSMOutputPin::GetPinCursor) );
	}

protected:
	TSharedRef<SWidget> GetDefaultValueWidget() override
	{
		return SNew(STextBlock);
	}
	const FSlateBrush *GetPinBorder() const
	{
		return (IsHovered()) ?
			FAppStyle::GetBrush(TEXT("Graph.StateNode.Pin.BackgroundHovered")) :
			FAppStyle::GetBrush(TEXT("Graph.StateNode.Pin.Background"));
	}
};