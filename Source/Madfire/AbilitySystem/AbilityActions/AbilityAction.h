#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Templates/SubclassOf.h"
#include "GameplayTask.h"
#include "UObject/NoExportTypes.h"

#include "AbilityAction.generated.h"


class UCombatComponent;
class UAbstractAbility;

/*
 * Тут пока всё нориально. В интерфейс экшона добавим параметр
 * Empowerment - который должен определять степерь усиления, если нужно.
 * Ну и Экшон Колекшон переехал сюда
 */
UCLASS(Abstract, Blueprintable, HideDropdown, EditInlineNew)
class UAbilityAction : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY()
    UAbstractAbility *Ability;

    UPROPERTY()
    UCombatComponent *CombatComponent;


    /*
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Skill System")
    bool Execute(AActor *Target,
                 int32 Empowerment = 0);    //Empowerment намного лучше, я считаю

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
    ACharacter *Caster = nullptr;    //тот, кто скастовал заклинание

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
    AActor *Causer =    nullptr;    //То, что является причиной этого действия - то есть проджектайл, или область, или ещё что в таком же духе
    */
};

/*
UCLASS(Blueprintable, HideDropdown)
class UAbilityActionCollection : public UAbilityAction
{
    GENERATED_BODY()

public:
    /// dead code - TODO: remove if just works
    //UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Actions")
    //bool Execute(AActor *Target, int32 Empowerment = 0);
    bool Execute_Implementation(AActor *Target, int32 Empowerment = 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TSubclassOf<UAbilityAction>> ActionList;
};*/
