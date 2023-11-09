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

protected:
	TSharedPtr<SVerticalBox> ContentWidget;
	TSharedPtr<SBorder>		 ErrorBorder;
	TSharedPtr<STextBlock>	 NodeHeader;

	void CreateContent();
	void CreateHeader();

};