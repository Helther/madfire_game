#include "FSMConnectionDrawingPolicy.h"
#include "FSMEdGraphEdge.h"
#include "FSMEdGraphNode.h"
#include "FSMEdGraphRoot.h"

FSMConnectionDrawingPolicy::FSMConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float ZoomFactor, const FSlateRect &InClippingRect, FSlateWindowElementList &InDrawElements, UEdGraph *InGraphObj) :
    FConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, ZoomFactor, InClippingRect, InDrawElements),
    GraphObj(InGraphObj)
{}

void FSMConnectionDrawingPolicy::DetermineWiringStyle(UEdGraphPin *OutputPin, UEdGraphPin *InputPin, FConnectionParams &Params)
{
    Params.bUserFlag2 = true;
    Params.AssociatedPin1 = OutputPin;
    Params.AssociatedPin2 = InputPin;
    Params.WireThickness = 1.5f;

    if(HoveredPins.Num() > 0)
    {
        ApplyHoverDeemphasis(OutputPin,InputPin, Params.WireThickness, Params.WireColor);
    }    
}

void FSMConnectionDrawingPolicy::Draw(TMap<TSharedRef<SWidget>, FArrangedWidget> &InPinGeometries, FArrangedChildren &ArrangedNodes)
{
    NodeWidgetMap.Empty();
    for(int32 NodeIndex = 0; NodeIndex < ArrangedNodes.Num(); ++NodeIndex)
    {
        FArrangedWidget &CurWidget = ArrangedNodes[NodeIndex];
        TSharedRef<SGraphNode> ChildNode = StaticCastSharedRef<SGraphNode>(CurWidget.Widget);
        NodeWidgetMap.Add(ChildNode->GetNodeObj(), NodeIndex);
    }

    FConnectionDrawingPolicy::Draw(InPinGeometries, ArrangedNodes);
}

void FSMConnectionDrawingPolicy::DetermineLinkGeometry(	FArrangedChildren& ArrangedNodes, 
	                                                    TSharedRef<SWidget>& OutputPinWidget,
	                                                    UEdGraphPin* OutputPin,
	                                                    UEdGraphPin* InputPin,
	                                                    /*out*/ FArrangedWidget*& StartWidgetGeometry,
	                                                    /*out*/ FArrangedWidget*& EndWidgetGeometry)
{
    //Default
    /*StartWidgetGeometry = PinGeometries->Find(OutputPinWidget);
	
	if (TSharedPtr<SGraphPin>* pTargetWidget = PinToPinWidgetMap.Find(InputPin))
	{
		TSharedRef<SGraphPin> InputWidget = (*pTargetWidget).ToSharedRef();
		EndWidgetGeometry = PinGeometries->Find(InputWidget);
	}*/
    if(UFSMEdGraphRoot *Root = Cast<UFSMEdGraphRoot>(OutputPin->GetOwningNode()))
    {
        StartWidgetGeometry = PinGeometries->Find(OutputPinWidget);
        UFSMEdGraphNode *State = CastChecked<UFSMEdGraphNode>(InputPin->GetOwningNode());
        int32 StateIndex = NodeWidgetMap.FindChecked(State);
        EndWidgetGeometry = &(ArrangedNodes[StateIndex]);
    }
    else if(UFSMEdGraphEdge *Edge = Cast<UFSMEdGraphEdge>(InputPin->GetOwningNode()))
    {
        UFSMEdGraphNode *PrevState = Edge->GetPreviousState();
        UFSMEdGraphNode *NextState = Edge->GetNextState();
        if((PrevState != nullptr) && (NextState != nullptr))
        {
            int32 *PrevNodeIndex = NodeWidgetMap.Find(PrevState);
            int32 *NextNodeIndex = NodeWidgetMap.Find(NextState);

            if((PrevNodeIndex != nullptr) && (NextNodeIndex != nullptr))
            {
                StartWidgetGeometry = &(ArrangedNodes[*PrevNodeIndex]);
                EndWidgetGeometry = &(ArrangedNodes[*NextNodeIndex]);
            }
        }
    }
    else
    {
        StartWidgetGeometry = PinGeometries->Find(OutputPinWidget);
        if(TSharedPtr<SGraphPin> *pTargetWidget = PinToPinWidgetMap.Find(InputPin))
        {
            TSharedRef<SGraphPin> InputWidget = (*pTargetWidget).ToSharedRef();
            EndWidgetGeometry = PinGeometries->Find(InputWidget);
        }
    }
}

void FSMConnectionDrawingPolicy::DrawSplineWithArrow(const FGeometry &StartG, const FGeometry &EndG, const FConnectionParams &Params)
{
    auto StartCenter = FGeometryHelper::CenterOf(StartG);
    auto EndCenter = FGeometryHelper::CenterOf(EndG);
    auto SeedPoint = (StartCenter + EndCenter) * 0.5f;

    auto Start = FGeometryHelper::FindClosestPointOnGeom(StartG, SeedPoint);
    auto End = FGeometryHelper::FindClosestPointOnGeom(EndG, SeedPoint);

    DrawSplineWithArrow(Start,End,Params);
}

void FSMConnectionDrawingPolicy::DrawSplineWithArrow(const FVector2D &Start, const FVector2D &End, const FConnectionParams &Params)
{
    // bUserFlag1 indicates that we need to reverse the direction of connection (used by debugger)
    auto &P0 = Params.bUserFlag1 ? End : Start;
    auto &P1 = Params.bUserFlag1 ? Start : End;

    Internal_DrawLineWithArrow(P0,P1,Params);
}

void FSMConnectionDrawingPolicy::DrawPreviewConnector(const FGeometry &PinG, const FVector2D &Start, const FVector2D &End, UEdGraphPin *Pin)
{
    FConnectionParams Params;
    DetermineWiringStyle(Pin, nullptr, Params);

    if(Pin->Direction == EEdGraphPinDirection::EGPD_Output)
    {
        DrawSplineWithArrow(FGeometryHelper::FindClosestPointOnGeom(PinG, End), End, Params);
    }
    else
    {
        DrawSplineWithArrow(FGeometryHelper::FindClosestPointOnGeom(PinG, Start), Start, Params);
    }
}

FVector2D FSMConnectionDrawingPolicy::ComputeSplineTangent(const FVector2D &Start, const FVector2D &End) const
{
    auto Delta = End - Start;
    return Delta.GetSafeNormal();
}

void FSMConnectionDrawingPolicy::Internal_DrawLineWithArrow(const FVector2D &StartAnchorPoint, const FVector2D &EndAnchorPoint, const FConnectionParams &Params)
{
    const float LineSeparationAmount = 4.5f;

    const FVector2D DeltaPos = EndAnchorPoint - StartAnchorPoint;
    const FVector2D UnitDelta = DeltaPos.GetSafeNormal();
    const FVector2D Normal = FVector2D(DeltaPos.Y, -DeltaPos.X).GetSafeNormal();

    const FVector2D DirectionBias = Normal * LineSeparationAmount;
    const FVector2D LengthBias =  ArrowRadius.X * UnitDelta;
    const FVector2D StartPoint = StartAnchorPoint + DirectionBias + LengthBias;
    const FVector2D EndPoint = EndAnchorPoint + DirectionBias - LengthBias;

    DrawConnection(WireLayerID, StartPoint, EndPoint, Params);

    const FVector2D ArrowDrawPos = EndPoint - ArrowRadius;
    const float AngleInRadians = FMath::Atan2(DeltaPos.Y, DeltaPos.X);

    FSlateDrawElement::MakeRotatedBox(
        DrawElementsList,
        ArrowLayerID,
        FPaintGeometry(ArrowDrawPos, ArrowImage->ImageSize * ZoomFactor, ZoomFactor),
        ArrowImage,
        ESlateDrawEffect::None,
        AngleInRadians,
        TOptional<FVector2D>(),
        FSlateDrawElement::RelativeToElement,
        Params.WireColor
    );
}

void FSMConnectionDrawingPolicy::DrawCircle(const FVector2D &Center, float Radius, const FLinearColor &Color, const int NumLineSegments)
{}

void FSMConnectionDrawingPolicy::DrawPinGeometries(TMap<TSharedRef<SWidget>, FArrangedWidget> &InPinGeometries, FArrangedChildren &ArrangedNodes)
{
    for (TMap<TSharedRef<SWidget>, FArrangedWidget>::TIterator ConnectorIt(InPinGeometries); ConnectorIt; ++ConnectorIt)
	{
		TSharedRef<SWidget> SomePinWidget = ConnectorIt.Key();
		SGraphPin& PinWidget = static_cast<SGraphPin&>(SomePinWidget.Get());
		UEdGraphPin* ThePin = PinWidget.GetPinObj();

		if (ThePin->Direction == EGPD_Output)
		{
			for (int32 LinkIndex=0; LinkIndex < ThePin->LinkedTo.Num(); ++LinkIndex)
			{
				FArrangedWidget* LinkStartWidgetGeometry = nullptr;
				FArrangedWidget* LinkEndWidgetGeometry = nullptr;

				UEdGraphPin* TargetPin = ThePin->LinkedTo[LinkIndex];

				DetermineLinkGeometry(ArrangedNodes, SomePinWidget, ThePin, TargetPin, /*out*/ LinkStartWidgetGeometry, /*out*/ LinkEndWidgetGeometry);

				if (( LinkEndWidgetGeometry && LinkStartWidgetGeometry ) && !IsConnectionCulled( *LinkStartWidgetGeometry, *LinkEndWidgetGeometry ))
				{
					FConnectionParams Params;
					DetermineWiringStyle(ThePin, TargetPin, /*inout*/ Params);
					const TSharedPtr<SGraphPin>* ConnectedPinWidget = PinToPinWidgetMap.Find(TargetPin);
					if (ConnectedPinWidget && ConnectedPinWidget->IsValid())
					{
						if ( PinWidget.AreConnectionsFaded() && (*ConnectedPinWidget)->AreConnectionsFaded() )
						{
							Params.WireColor.A = 0.2f;
						}
					}
					DrawSplineWithArrow(LinkStartWidgetGeometry->Geometry, LinkEndWidgetGeometry->Geometry, Params);
				}
			}
		}
	}
}
