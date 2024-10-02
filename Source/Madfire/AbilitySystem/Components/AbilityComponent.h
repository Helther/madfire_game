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
UCLASS(EditInlineNew, Abstract, BlueprintType, meta=(BlueprintSpawnableComponent))
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

    UFUNCTION(BlueprintPure, Category="Ability Components", meta=(Keywords = "Actor Owning Parent"))
    UAbstractAbility *GetOwner() const;

    UFUNCTION(BlueprintPure, Category="Ability Components", meta=(Keywords = "Actor Owning Parent"))
    AActor *GetActorOwner() const;

    UFUNCTION(BlueprintImplementableEvent, Category="Ability Components")
    void Tick(float Delta);    

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ability Components")
    bool bIsTickable = false;

    virtual void Tick_Implementation(float Delta) {Tick(Delta);}
};

