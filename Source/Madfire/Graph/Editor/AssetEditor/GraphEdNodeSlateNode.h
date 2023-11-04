#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"

class UGraphEdGraphNode;

class SGraphEdGraphNode : public SGraphNode
{
public:
    SLATE_BEGIN_ARGS(SGraphEdGraphNode) {}
    SLATE_END_ARGS()

    void Construct(const FArguments &InArgs, UGraphEdGraphNode *InNode);

    void UpdateGraphNode() override;
    void CreatePinWidgets() override;
    void AddPin(const TSharedRef<SGraphPin> &PinToAdd) override;
    bool IsNameReadOnly() const override;

    void OnNameTextCommited(const FText &InText, ETextCommit::Type CommitInfo);

    virtual FSlateColor GetBorderBackgroundColor() const;
    virtual FSlateColor GetBackgroundColor() const;
    virtual EVisibility GetDragOverMarkerVisibility() const;
    virtual const FSlateBrush *GetNameIcon() const;

protected:
    TSharedPtr<SBorder> NodeBody;
    TSharedPtr<SHorizontalBox> OutputPinBox;
};