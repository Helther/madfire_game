#pragma once

#include "CoreMinimal.h"
#include "GraphAssetEditorSettings.generated.h"

UENUM(BlueprintType)
enum class EAutoLayoutStrategy : uint8
{
	Tree,
	ForceDirected
};

UCLASS()
class UGraphAssetEditorSettings : public UObject
{
    GENERATED_BODY()

public:
    UGraphAssetEditorSettings();
    ~UGraphAssetEditorSettings() override;

    UPROPERTY(EditDefaultsOnly)
    float OptimalDistance;

    UPROPERTY(EditDefaultsOnly, AdvancedDisplay)
    EAutoLayoutStrategy AutoLayoutStrategy;

    UPROPERTY(EditDefaultsOnly, AdvancedDisplay)
    int32 MaxIteraion;

    UPROPERTY(EditDefaultsOnly, AdvancedDisplay)
    bool bFirstPassOnly;

    UPROPERTY(EditDefaultsOnly, AdvancedDisplay)
    bool bRandomInit;

    UPROPERTY(EditDefaultsOnly, AdvancedDisplay)
    float InitTemperature;

    UPROPERTY(EditDefaultsOnly, AdvancedDisplay)
    float CoolDownRate;
};