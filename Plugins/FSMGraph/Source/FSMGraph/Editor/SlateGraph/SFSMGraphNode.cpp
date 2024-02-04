#include "SFSMGraphNode.h"
#include "Slate.h"
#include "SFSMGraphPin.h"
#include "../EdGraph/FSMEdGraphNode.h"
#include "SGraphPanel.h"

void SFSMGraphNode::Construct(const FArguments &InArgs, UEdGraphNode *InNode)
{
	GraphNode = InNode;
	SetCursor(EMouseCursor::CardinalCross);	
	UpdateGraphNode();
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SFSMGraphNode::UpdateGraphNode() 
{
	const FMargin NodePadding = FMargin(2.f);

	InputPins.Empty();
	OutputPins.Empty();

	const FSlateBrush *NodeTypeIcon = GetNameIcon();
	FLinearColor TitleShadowColor(0.6f,0.6f,0.6f);
	TSharedPtr<SErrorText> ErrorText;
	TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode);

	RightNodeBox.Reset();
	LeftNodeBox.Reset();
	

	this->ContentScale.Bind(this, &SGraphNode::GetContentScale);
	this->GetOrAddSlot(ENodeZone::Center)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("Graph.StateNode.Body"))
			.Padding(0)
			.BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.1f))
			[
				SNew(SOverlay)
				//Pin AREA
				+SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SAssignNew(RightNodeBox, SVerticalBox)
				]

				// STATE NAME AREA
				+SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.Padding(10.f)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("Graph.StateNode.ColorSpill"))
					.BorderBackgroundColor(FLinearColor(0.3f, 0.3f, 0.3f))
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.Visibility(EVisibility::SelfHitTestInvisible)
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						.AutoWidth()
						[
							SAssignNew(ErrorText, SErrorText)
							.BackgroundColor(FLinearColor(0.5f, 0.0f, 0.5f))
							.ToolTipText(FText::FromString("Test"))
						]
						+SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							SNew(SImage)
							.Image(NodeTypeIcon)
						]
						+SHorizontalBox::Slot()
						.Padding(FMargin(4.f,0.f,4.f,0.f))
						[
							SNew(SVerticalBox)
							+SVerticalBox::Slot()
							.AutoHeight()
							[
								SAssignNew(InlineEditableText, SInlineEditableTextBlock)
								.Style(FAppStyle::Get(), "Graph.StateNode.NodeTitleInlineEditableText")
								.Text(NodeTitle.Get(), &SNodeTitle::GetHeadTitle)
								.OnVerifyTextChanged(this, &SFSMGraphNode::OnVerifyNameTextChanged)
								.OnTextCommitted(this, &SFSMGraphNode::OnNameTextCommited)
								.IsReadOnly(this, &SFSMGraphNode::IsNameReadOnly)
								.IsSelected(this, &SFSMGraphNode::IsSelectedExclusively)
							]
							+SVerticalBox::Slot()
							.AutoHeight()
							[
								NodeTitle.ToSharedRef()
							]
						]
					]
				]
			]
		];

	ErrorReporting = ErrorText;
	ErrorReporting->SetError(ErrorMsg);
	CreatePinWidgets();
}

void SFSMGraphNode::CreatePinWidgets()
{	
	UFSMEdGraphNode *EdNode = Cast<UFSMEdGraphNode>(GraphNode);
	
	UEdGraphPin *CurPin = EdNode->GetOutputPin();
	if(!CurPin->bHidden)
    {
		TSharedPtr<SGraphPin> NewPin = SNew(SFSMOutputPin, CurPin);
		AddPin(NewPin.ToSharedRef());
	}
}

void SFSMGraphNode::AddPin(const TSharedRef<SGraphPin> &PinToAdd)
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

bool SFSMGraphNode::IsNameReadOnly() const
{
	return false;
}

void SFSMGraphNode::OnNameTextCommited(const FText &InText, ETextCommit::Type CommitInfo)
{	
	auto EdNode = Cast<UFSMEdGraphNode>(GraphNode);
	auto newNamestr = InText.ToString();
	FName NewName = FName(*newNamestr);
	if(EdNode && EdNode->RenameUniqueNode(NewName))
    {
		UpdateGraphNode();
		//NodeHeader.Get()->SetVisibility(EVisibility::Visible);
		SGraphNode::OnNameTextCommited(InText, CommitInfo);
	}
}

void SFSMGraphNode::OnMouseEnter(const FGeometry &MyGeometry, const FPointerEvent &MouseEvent)
{
	if(const UFSMEdGraphNode *StateNode = Cast<UFSMEdGraphNode>(GraphNode))
    {
		if(const UEdGraphPin *OutputPin = StateNode->GetOutputPin())
        {
			TSharedPtr<SGraphPanel> OwnerPanel = GetOwnerPanel();
			check(OwnerPanel.IsValid());

			for(int32 LinkIndex = 0; LinkIndex < OutputPin->LinkedTo.Num(); ++LinkIndex)
				OwnerPanel->AddPinToHoverSet(OutputPin->LinkedTo[LinkIndex]);
		}
    }
	SGraphNode::OnMouseEnter(MyGeometry, MouseEvent);
}

void SFSMGraphNode::OnMouseLeave(const FPointerEvent &MouseEvent) 
{
	if(const UFSMEdGraphNode *StateNode = Cast<UFSMEdGraphNode>(GraphNode))
    {
		if(const UEdGraphPin *OutputPin = StateNode->GetOutputPin())
        {
			TSharedPtr<SGraphPanel> OwnerPanel = GetOwnerPanel();
			check(OwnerPanel.IsValid());

			for(int32 LinkIndex = 0; LinkIndex < OutputPin->LinkedTo.Num(); ++LinkIndex)
				OwnerPanel->RemovePinFromHoverSet(OutputPin->LinkedTo[LinkIndex]);
		}
	}

	SGraphNode::OnMouseLeave(MouseEvent);
}

const FSlateBrush *SFSMGraphNode::GetPinBorder() const 
{
	return (IsHovered()) ?
		FAppStyle::GetBrush(TEXT("Graph.StateNode.Pin.BackgroundHovered")) :
		FAppStyle::GetBrush(TEXT("Graph.StateNode.Pin.Background"));
}

const FSlateBrush *SFSMGraphNode::GetNameIcon() const
{
	return FAppStyle::GetBrush(TEXT("Graph.StateNode.Icon")); 
}


END_SLATE_FUNCTION_BUILD_OPTIMIZATION