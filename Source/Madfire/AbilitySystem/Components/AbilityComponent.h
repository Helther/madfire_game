#pragma once


#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "AbilityComponent.generated.h"

class UCombatComponent;
class UAbstractAbility;


/*
* Тут тоже немного изменилось. Компонент теперь хранит кастера как чарактера,
* а не Актора. В Скиле он тоже теперь чарактер. (Думаю понятно почему)
*/
UCLASS(EditInlineNew, Abstract, Blueprintable, BlueprintType)
class UAbilityComponent : public UObject
{
    GENERATED_BODY()

public:
    UAbilityComponent(const FObjectInitializer &f);

    virtual void ComponentInit(UAbstractAbility *parent);
    virtual bool ComponentSetup(ACharacter *Component);

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "AbilitySystem")
    void OnComponentInit();

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "AbilitySystem")
    void OnComponentSetup();

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "AbilitySystem")
    void ActivateComponent();
    virtual void ActivateComponent_Implementation() {  }

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "AbilitySystem")
    void UpdateComponent();
    virtual void UpdateComponent_Implementation() {  }

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "AbilitySystem")
    void DeactivateComponent();
    virtual void DeactivateComponent_Implementation() { }

    UAbstractAbility *GetOwner();
};

/*
Нужные компонентов:
	Проджектай (Хоминг Проджектайл)
	Область (Компонент Область)
	СамПерсонаж - тут просто прослойка которая будет выполнять все экшоны на персонаже,
				  который создал этот компонент.
	Луч до первого таргета (Луч на определённое растояние)
	
*/
