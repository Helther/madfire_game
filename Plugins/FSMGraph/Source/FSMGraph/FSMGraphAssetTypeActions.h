#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"

class FAssetTypeActions_FSMGraph : public FAssetTypeActions_Base
{
public:
	FText GetName() const override;
	FColor GetTypeColor() const override;
	UClass *GetSupportedClass() const override;
	void OpenAssetEditor(const TArray<UObject*> &InObject, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;
	uint32 GetCategories() override;

private:
	EAssetTypeCategories::Type FSMGraphCategory;
};
