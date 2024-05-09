#pragma once

#include "AbilityComponent.h"
#include "../../Character/AbstractRequirement.h"
#include "AbilityRequirementsComponent.generated.h"


UCLASS(EditInlineNew, abstract, Blueprintable, BlueprintType)
class UAbilityRequirement : public UAbilityComponent
{
    GENERATED_BODY()    //Непонятный макрос Анриала, который генерирует over 9000 ненужных строчек кода
public:
    // В этой функции будет происходить поглощение (вычитание) необходимых статов
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Requirements Interface")
    bool Consume(ACharacter *character);

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Requirements Interface")
    bool CheckRequirement(ACharacter *character);

};