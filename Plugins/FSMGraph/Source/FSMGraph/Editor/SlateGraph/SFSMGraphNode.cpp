#include "SFSMGraphNode.h"
#include "Slate.h"
#include "SFSMGraphPin.h"
#include "../EdGraph/FSMEdGraphNode.h"

void SFSMGraphNode::Construct(const FArguments &InArgs, UEdGraphNode *InNode)
{
	GraphNode = InNode;
	UpdateGraphNode();
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SFSMGraphNode::UpdateGraphNode() 
{
	const FMargin NodePadding = FMargin(2.f);

	InputPins.Empty();
	OutputPins.Empty();

	RightNodeBox.Reset();
	LeftNodeBox.Reset();

	TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode);

	this->ContentScale.Bind(this, &SGraphNode::GetContentScale);

	this->GetOrAddSlot(ENodeZone::Center)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("Graph.StateNode.Body"))
			.Padding(FMargin(10.f))
			.BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.1f))
			[
				SAssignNew(ErrorBorder, SBorder)
				.BorderImage(FAppStyle::GetBrush("Graph.StateNode.Body"))
				.BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.1f))
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("Graph.StateNode.Body"))
					.Padding(FMargin(10.f))
					.BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.1f))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SAssignNew(LeftNodeBox, SVerticalBox)
						]
						/*+ SVerticalBox::Slot()
						.HAlign(HAlign_Center)
						.AutoHeight()
						[
							SAssignNew(NodeHeader, STextBlock)
						]*/
						+ SVerticalBox::Slot()
						.HAlign(HAlign_Center)
						.AutoHeight()
						[
							SAssignNew(InlineEditableText, SInlineEditableTextBlock)
							.Style(FAppStyle::Get(), "Graph.StateNode.NodeTitleInlineEditableText")
							.Text(NodeTitle.Get(), &SNodeTitle::GetHeadTitle)
							.IsReadOnly(this, &SFSMGraphNode::IsNameReadOnly)
							.OnTextCommitted(this, &SFSMGraphNode::OnNameTextCommited)
							.OnVerifyTextChanged(this, &SFSMGraphNode::OnVerifyNameTextChanged)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							NodeTitle.ToSharedRef()
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SAssignNew(ContentWidget, SVerticalBox)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SAssignNew(RightNodeBox, SVerticalBox)
						]
					]
				]
			]		
		];

	CreatePinWidgets();
	CreateContent();
	CreateHeader();
}

void SFSMGraphNode::CreatePinWidgets()
{	
	UFSMEdGraphNode *EdNode = Cast<UFSMEdGraphNode>(GraphNode);
	for(int32 i = 0; i < EdNode->Pins.Num(); ++i)
    {
		UEdGraphPin *Pin = EdNode->Pins[i];
		if(!Pin->bHidden)
        {
			TSharedPtr<SGraphPin> NewPin = SNew(SFSMGraphPin, Pin);
			AddPin(NewPin.ToSharedRef());
		}
	}
}

void SFSMGraphNode::AddPin(const TSharedRef<SGraphPin> &PinToAdd)
{
	PinToAdd->SetOwner(SharedThis(this));
	if(PinToAdd->GetDirection() == EEdGraphPinDirection::EGPD_Input)
    {
		LeftNodeBox->AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.FillHeight(1.f)
			.Padding(20.f,0.f)
			[
				PinToAdd
			];
		InputPins.Add(PinToAdd);
	}
    else if(PinToAdd->GetDirection() == EEdGraphPinDirection::EGPD_Output)
    {
		RightNodeBox->AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.FillHeight(1.f)
			.Padding(20.f,0.f)
			[
				PinToAdd
			];
		OutputPins.Add(PinToAdd);
	}
    else
    {
		//TODO: Error
	}
}

bool SFSMGraphNode::IsNameReadOnly() const
{
	return false;
}

void SFSMGraphNode::OnNameTextCommited(const FText &InText, ETextCommit::Type CommitInfo)
{	
	auto EdNode = Cast<UFSMEdGraphNode>(GraphNode);

	if(EdNode && EdNode->RenameUniqueNode(InText))
    {
		UpdateGraphNode();
		NodeHeader.Get()->SetVisibility(EVisibility::Visible);
		SGraphNode::OnNameTextCommited(InText, CommitInfo);
	}
}

void SFSMGraphNode::CreateContent()
{	
	auto EdNode = Cast<UFSMEdGraphNode>(GraphNode);

	ContentWidget->AddSlot()
		[
			EdNode->GetContentWidget().ToSharedRef()
		];
}

void SFSMGraphNode::CreateHeader()
{	
	/*NodeHeader.Get()->SetText(GraphNode->GetNodeTitle(ENodeTitleType::MenuTitle));
	
	auto EdNode = Cast<UFSMEdGraphNode>(GraphNode);

	if(EdNode)
		NodeHeader.Get()->SetVisibility((EdNode->GetEdNodeName().IsEmpty()) ? EVisibility::Collapsed : EVisibility::Visible);
    else
    {
		//TODO: Error
	}*/
	FAnimNotifyEvent event;
}


END_SLATE_FUNCTION_BUILD_OPTIMIZATION