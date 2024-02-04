#include "SFSMGraphRoot.h"
#include "SGraphPin.h"
#include "SNodePanel.h"
#include "../EdGraph/FSMEdGraphRoot.h"

void SFSMGraphRoot::Construct(const FArguments &InArgs, UFSMEdGraphRoot *InNode) 
{
	GraphNode = InNode;
	SetCursor(EMouseCursor::CardinalCross);
	UpdateGraphNode();
}

void SFSMGraphRoot::GetNodeInfoPopups(FNodeInfoContext *, TArray<FGraphInformationPopupInfo> &) const
{}

void SFSMGraphRoot::UpdateGraphNode()
{
	InputPins.Empty();
	OutputPins.Empty();

	RightNodeBox.Reset();
	LeftNodeBox.Reset();

	FLinearColor TitleShadowColor(0.6f, 0.6f, 0.6f);

	ContentScale.Bind(this, &SGraphNode::GetContentScale);
	GetOrAddSlot(ENodeZone::Center)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("Graph.StateNode.Body"))
			.Padding(0)
			.BorderBackgroundColor(this, &SFSMGraphRoot::GetBorderBackgroundColor)
			[
				SNew(SOverlay)
											
				+SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.Padding(10.f)
				[
					SAssignNew(RightNodeBox, SVerticalBox)
				]
			]
		];

	CreatePinWidgets();
}

void SFSMGraphRoot::AddPin(const TSharedRef<SGraphPin> &PinToAdd) 
{
	PinToAdd->SetOwner(SharedThis(this));
	RightNodeBox->AddSlot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.FillHeight(1.f)
		[
			PinToAdd
		];
	OutputPins.Add(PinToAdd);
}

FSlateColor SFSMGraphRoot::GetBorderBackgroundColor() const 
{
	FLinearColor InactiveStateColor(0.08f, 0.08f, 0.08f);
	FLinearColor ActiveStateColorDim(0.4f, 0.3f, 0.15f);
	FLinearColor ActiveStateColorBright(1.f, 0.6f, 0.35f);

	return InactiveStateColor;
}

FText SFSMGraphRoot::GetPreviewCornerText() 
{
	return FText::FromString("Entry point for state machine"); 
}
