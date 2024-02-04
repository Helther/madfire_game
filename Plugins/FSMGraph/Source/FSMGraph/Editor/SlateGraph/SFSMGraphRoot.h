#pragma once

#include "Containers/Array.h"
#include "SGraphNode.h"
#include "Widgets/DeclarativeSyntaxSupport.h"


class SGraphPin;
class UFSMEdGraphRoot;
struct FGraphInformationPopupInfo;

class SFSMGraphRoot : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SFSMGraphRoot) {}
	SLATE_END_ARGS()

	void Construct(const FArguments &InArgs, UFSMEdGraphRoot *InNode);

	void GetNodeInfoPopups(FNodeInfoContext *, TArray<FGraphInformationPopupInfo> &) const override;
	void UpdateGraphNode() override;
	void AddPin(const TSharedRef<SGraphPin> &PinToAdd) override;
	
protected:
	FSlateColor GetBorderBackgroundColor() const;
	FText GetPreviewCornerText();
};

