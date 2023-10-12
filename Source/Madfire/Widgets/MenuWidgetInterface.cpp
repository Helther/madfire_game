// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/MenuWidgetInterface.h"
#include "Character/PlayerCharController.h"

void UMenuWidgetInterface::RemoveFromParent() 
{
    auto controller = Cast<APlayerCharController>(GetOwningPlayer());
    if (controller)
        controller->ReturnToGameplay();    
    UUserWidget::RemoveFromParent();
}
