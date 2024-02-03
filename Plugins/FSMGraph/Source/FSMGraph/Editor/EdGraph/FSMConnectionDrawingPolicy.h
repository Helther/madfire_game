#pragma once

#include "CoreMinimal.h"
#include "ConnectionDrawingPolicy.h"


class FSMConnectionDrawingPolicy : public FConnectionDrawingPolicy
{
public:
	FSMConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float ZoomFactor, const FSlateRect &InClippingRect, FSlateWindowElementList &InDrawElements, UEdGraph *InGraphObj);


	void DetermineWiringStyle(UEdGraphPin *, UEdGraphPin* , FConnectionParams &) override;
	void Draw(TMap<TSharedRef<SWidget>, FArrangedWidget> &, FArrangedChildren &ArrangedNodes) override;
	void DetermineLinkGeometry(FArrangedChildren &, TSharedRef<SWidget> &, UEdGraphPin *, UEdGraphPin *, FArrangedWidget *&, FArrangedWidget *&) override;
	void DrawSplineWithArrow(const FGeometry &, const FGeometry &, const FConnectionParams &) override;
	void DrawSplineWithArrow(const FVector2D& , const FVector2D& , const FConnectionParams& ) override;
	void DrawPreviewConnector(const FGeometry &, const FVector2D &, const FVector2D &, UEdGraphPin *) override;
	FVector2D ComputeSplineTangent(const FVector2D &, const FVector2D &) const override;

protected:
	UEdGraph *GraphObj;
	TMap<UEdGraphNode*, int32> NodeWidgetMap;

protected:
	void Internal_DrawLineWithArrow(const FVector2D &StartAnchorPoint, const FVector2D &EndAnchorPoint, const FConnectionParams &Params);

	void DrawCircle(const FVector2D &Center, float Radius, const FLinearColor &Color, const int NumLineSegments);
	TArray<FVector2D> TempPoints;

	static constexpr float RelinkHandleHoverRadius = 20.f;
};
