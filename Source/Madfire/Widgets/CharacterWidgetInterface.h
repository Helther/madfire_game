// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterWidgetInterface.generated.h"

/**
 * 
 */
UCLASS()
class UCharacterWidgetInterface : public UUserWidget
{
	GENERATED_BODY()

public:

	void RemoveFromParent() override;

	
};


