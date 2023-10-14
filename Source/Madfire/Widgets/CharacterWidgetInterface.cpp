// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/CharacterWidgetInterface.h"
#include "Character/PlayerCharController.h"

void UCharacterWidgetInterface::RemoveFromParent()
{
    auto controller = Cast<APlayerCharController>(GetOwningPlayer());
    if (controller)
        controller->ReturnToGameplay();    
    UUserWidget::RemoveFromParent();
}
