#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"
#include "NotifyHook.h"
#include "IDetailsView.h"

class SFSMGraphNode : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SFSMGraphNode){}
	SLATE_END_ARGS()

	//Interface SGraphNode
	void Construct(const FArguments &InArgs, UEdGraphNode *InNode);
	void UpdateGraphNode() override;
	void CreatePinWidgets() override;
	void AddPin(const TSharedRef<SGraphPin> &PinToAdd) override;
	bool IsNameReadOnly() const override;
	void OnNameTextCommited(const FText &InText, ETextCommit::Type CommitInfo);

	// SWidget interface
	void OnMouseEnter(const FGeometry &MyGeometry, const FPointerEvent &MouseEvent) override;
	void OnMouseLeave(const FPointerEvent &MouseEvent) override;
	// End of SWidget interface

protected:
	TSharedPtr<SVerticalBox>     PinsNodeBox;

	FLinearColor GetBackgroundColor() {return FLinearColor::Gray;}
	const FSlateBrush *GetPinBorder() const;
	const FSlateBrush *GetNameIcon() const;

};