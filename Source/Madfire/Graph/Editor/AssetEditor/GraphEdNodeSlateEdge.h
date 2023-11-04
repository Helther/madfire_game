#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateColor.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SWidget.h"
#include "SNodePanel.h"
#include "SGraphNode.h"


class SToolTip;
class UGraphEdGraphEdge;

class SGraphEdGraphEdge : public SGraphNode
{
public:
    SLATE_BEGIN_ARGS(SGraphEdGraphEdge) {}
    SLATE_END_ARGS()

    void Construct(const FArguments &InArgs, UGraphEdGraphEdge *InNode);

    bool RequiresSecondPassLayout() const override;
    void PerformSecondPassLayout(const TMap<UObject*, TSharedRef<SNode>> &NodeToWidgetLookUp) const override;

    void UpdateGraphNode() override;

    void PositionBetweenTwoNodesWithOffset(const FGeometry &StartGeom, const FGeometry &EndGeom, int32 NodeIntex, int32 MaxNodes) const;

protected:
    FSlateColor GetEdgeColor() const;

private:
    TSharedPtr<STextEntryPopup> TextEntryWidget;
};