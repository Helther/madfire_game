#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Containers/UnrealString.h"
#include "HAL/Platform.h"
#include "Internationalization/Text.h"
#include "Math/Color.h"
#include "Math/Vector2D.h"
#include "SGraphNode.h"
#include "SNodePanel.h"
#include "Styling/SlateColor.h"
#include "Templates/SharedPointer.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class UFSMEdGraphEdge;

class SFSMGraphEdge : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SFSMGraphEdge) {}
	SLATE_END_ARGS()

	void Construct(const FArguments &InArgs, UFSMEdGraphEdge *InNode);

	//SNode Interface
	void GetNodeInfoPopups(FNodeInfoContext *Context, TArray<FGraphInformationPopupInfo> &Popups) const override;
	void MoveTo(const FVector2D &NewPosition, FNodeSet &NodeFilter, bool bMarkDirty = true) override;
	bool RequiresSecondPassLayout() const override;
	void PerformSecondPassLayout(const TMap<UObject *, TSharedRef<SNode>> &NodeToWidgetLookup) const override;
	//End of SNode interface

	// SGraphNode interface
	void UpdateGraphNode() override;
	
	//End of SGraphNode

	// SWidget interface
	void OnMouseEnter(const FGeometry &MyGeometry, const FPointerEvent &MouseEvent) override;
	void OnMouseLeave(const FPointerEvent &MouseEvent) override;
	// End of SWidget interface

	void PositionBetweenTwoNodesWithOffset(const FGeometry &StartGeom, const FGeometry &EndGeom, int32 NodeIndex, int32 MaxNodes) const;
	static FLinearColor StaticGetTransitionColor(UFSMEdGraphEdge *TransNode, bool bIsHovered);
	
private:
	TSharedPtr<STextEntryPopup> TextEntryWidget;

	mutable TWeakPtr<SNode> PrevStateNodeWidgetPtr;

private:
	FText GetPreviewCornerText(bool reverse) const;
	FSlateColor GetTransitionColor() const;
	const FSlateBrush *GetTransitionIconImage() const;
	TSharedRef<SWidget> GenerateRichTooltip();
};