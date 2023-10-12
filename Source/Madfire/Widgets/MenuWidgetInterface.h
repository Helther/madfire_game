// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MenuWidgetInterface.generated.h"

/**
 * 
 */
UCLASS()
class MADFIRE_API UMenuWidgetInterface : public UUserWidget
{
	GENERATED_BODY()

public:

	void RemoveFromParent() override;

	
};
