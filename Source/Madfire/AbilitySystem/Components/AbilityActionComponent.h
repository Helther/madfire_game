#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Templates/SubclassOf.h"
#include "GameplayTask.h"
#include "UObject/NoExportTypes.h"
#include "AbilityComponent.h"

#include "AbilityActionComponent.generated.h"


class UCombatComponent;
class UAbstractAbility;

/*
 * Тут пока всё нориально. В интерфейс экшона добавим параметр
 * Empowerment - который должен определять степерь усиления, если нужно.
 * Ну и Экшон Колекшон переехал сюда
 */
UCLASS(Abstract, Blueprintable, HideDropdown, EditInlineNew)
class UAbilityAction : public UAbilityComponent
{
    GENERATED_BODY()

public:



};

