#include "SFSMGraphEdge.h"
#include "../EdGraph/FSMEdGraphEdge.h"
#include "../EdGraph/FSMEdGraphNode.h"
#include "ConnectionDrawingPolicy.h"
#include "SGraphPanel.h"

void SFSMGraphEdge::Construct(const FArguments &InArgs, UFSMEdGraphEdge *InNode) 
{
    GraphNode = InNode;
    UpdateGraphNode();
}

void SFSMGraphEdge::GetNodeInfoPopups(FNodeInfoContext *Context, TArray<FGraphInformationPopupInfo> &Popups) const
{
}

void SFSMGraphEdge::MoveTo(const FVector2D &NewPosition, FNodeSet &NodeFilter, bool bMarkDirty)
{}

bool SFSMGraphEdge::RequiresSecondPassLayout() const
{
    return true; 
}

void SFSMGraphEdge::PerformSecondPassLayout(const TMap<UObject *, TSharedRef<SNode>> &NodeToWidgetLookup) const
{
    UFSMEdGraphEdge *TransNode = CastChecked<UFSMEdGraphEdge>(GraphNode);

    FGeometry StartGeom;
    FGeometry EndGeom;

    int32 TransIndex = 0;
    int32 NumOfTrans = 1;

    UFSMEdGraphNode *PrevState = TransNode->GetPreviousState();
    UFSMEdGraphNode *NextState = TransNode->GetNextState();

    if((PrevState != nullptr) && (NextState != nullptr))
    {
        const TSharedRef<SNode> *pPrevNodeWidget = NodeToWidgetLookup.Find(PrevState);
        const TSharedRef<SNode> *pNextNodeWidget = NodeToWidgetLookup.Find(NextState);
        if(pPrevNodeWidget && pNextNodeWidget)
        {
            const TSharedRef<SNode> &PrevNodeWidget = *pPrevNodeWidget;
            const TSharedRef<SNode> &NextNodeWidget = *pNextNodeWidget;

            StartGeom = FGeometry(FVector2D(PrevState->NodePosX, PrevState->NodePosY), FVector2D::ZeroVector, PrevNodeWidget->GetDesiredSize(), 1.f);
            EndGeom = FGeometry(FVector2D(NextState->NodePosX, NextState->NodePosY), FVector2D::ZeroVector, NextNodeWidget->GetDesiredSize(), 1.f);

            TArray<UFSMEdGraphEdge*> Transitions;
            PrevState->GetTransitionList(Transitions, false);

            Transitions = Transitions.FilterByPredicate([NextState](const UFSMEdGraphEdge *InTransition) -> bool { return InTransition->GetNextState() == NextState; });
        
            TransIndex = Transitions.IndexOfByKey(TransNode);
            NumOfTrans = Transitions.Num();

            PrevStateNodeWidgetPtr = PrevNodeWidget;
        }
    }
    PositionBetweenTwoNodesWithOffset(StartGeom, EndGeom, TransIndex, NumOfTrans);
}

void SFSMGraphEdge::UpdateGraphNode() 
{
    InputPins.Empty();
    OutputPins.Empty();

    RightNodeBox.Reset();
    LeftNodeBox.Reset();

    ContentScale.Bind(this, &SGraphNode::GetContentScale);
    GetOrAddSlot(ENodeZone::Center)
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
        [
            SNew(SOverlay)
            +SOverlay::Slot()
            [
                SNew(SImage)
                .Image(FAppStyle::GetBrush("Graph.TransitionNode.ColorSpill"))
                .ColorAndOpacity(this, &SFSMGraphEdge::GetTransitionColor)
            ]
            +SOverlay::Slot()
            [
                SNew(SImage)
                .Image(this, &SFSMGraphEdge::GetTransitionIconImage)
            ]
        ];
}

void SFSMGraphEdge::OnMouseEnter(const FGeometry &MyGeometry, const FPointerEvent &MouseEvent)
{
    UFSMEdGraphEdge *Edge = CastChecked<UFSMEdGraphEdge>(GraphNode);
    if(UEdGraphPin *Pin = Edge->GetInputPin())
        GetOwnerPanel()->AddPinToHoverSet(Pin);

    SGraphNode::OnMouseEnter(MyGeometry, MouseEvent);
}

void SFSMGraphEdge::OnMouseLeave(const FPointerEvent &MouseEvent) 
{
    UFSMEdGraphEdge *Edge = CastChecked<UFSMEdGraphEdge>(GraphNode);
    if(UEdGraphPin *Pin = Edge->GetInputPin())
        GetOwnerPanel()->RemovePinFromHoverSet(Pin);

    SGraphNode::OnMouseLeave(MouseEvent);
}

void SFSMGraphEdge::PositionBetweenTwoNodesWithOffset(const FGeometry &StartGeom, const FGeometry &EndGeom, int32 NodeIndex, int32 MaxNodes) const
{
    const FVector2D StartCenter = FGeometryHelper::CenterOf(StartGeom);
    const FVector2D EndCenter = FGeometryHelper::CenterOf(EndGeom);
    const FVector2D SeedPoint = (StartCenter + EndCenter) * 0.5f;

    const FVector2D StartAnchorPoint = FGeometryHelper::FindClosestPointOnGeom(StartGeom, SeedPoint);
    const FVector2D EndAnchorPoint = FGeometryHelper::FindClosestPointOnGeom(EndGeom, SeedPoint);

    const float Height = 30.f;
    
    const FVector2D DesiredNodeSize = GetDesiredSize();
    FVector2D DeltaPos(EndAnchorPoint - StartAnchorPoint);

    if(DeltaPos.IsNearlyZero())
        DeltaPos = FVector2D(10.f, 0.f);

    const FVector2D Normal = FVector2D(DeltaPos.Y, -DeltaPos.X).GetSafeNormal();
    const FVector2D NewCenter = StartAnchorPoint + (0.5f * DeltaPos) + (Height * Normal);

    FVector2D DeltaNormal = DeltaPos.GetSafeNormal();

    const float MultiNodeSpace = 0.2f;
    const float MultiNodeStep = (1.f + MultiNodeSpace);

    const float MultiNodeStart = -((MaxNodes -1) * MultiNodeStep) / 2.f;
    const float MultiNodeOffset = MultiNodeStart + (NodeIndex * MultiNodeStep);

    const FVector2D NewCorner = NewCenter - (0.5f * DesiredNodeSize) + (DeltaNormal * MultiNodeOffset * DesiredNodeSize.Size());

    GraphNode->NodePosX = static_cast<int32>(NewCorner.X);
    GraphNode->NodePosY = static_cast<int32>(NewCorner.Y);
}

FLinearColor SFSMGraphEdge::StaticGetTransitionColor(UFSMEdGraphEdge *TransNode, bool bIsHovered)
{
    return FLinearColor();
}

FText SFSMGraphEdge::GetPreviewCornerText(bool reverse) const 
{
    return FText(); 
}

FSlateColor SFSMGraphEdge::GetTransitionColor() const 
{
    return FLinearColor(0.4f, 0.4f, 1.0f); 
}

const FSlateBrush *SFSMGraphEdge::GetTransitionIconImage() const 
{    
    return FAppStyle::GetBrush("Graph.TransitionNode.Icon"); 
}

TSharedRef<SWidget> SFSMGraphEdge::GenerateRichTooltip() 
{
    UFSMEdGraphEdge *TransNode = CastChecked<UFSMEdGraphEdge>(GraphNode);

    return SNew(STextBlock).Text(FText::FromString("Error: No Graph"));
}
