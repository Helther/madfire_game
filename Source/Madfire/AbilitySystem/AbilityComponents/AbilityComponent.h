#pragma once


#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "AbilityComponent.generated.h"

class UAbilitySystemComponent;

/*
* Тут тоже немного изменилось. Компонент теперь хранит кастера как чарактера,
* а не Актора. В Скиле он тоже теперь чарактер. (Думаю понятно почему)
*/
UCLASS(Abstract, Blueprintable, BlueprintType)
class UAbilityComponent : public UObject
{
    GENERATED_BODY()

public:
    UAbilityComponent(const FObjectInitializer &f);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "AbilitySystem")
    bool ComponentSetup(UAbilitySystemComponent *Component);
    bool ComponentSetup_Implementation(UAbilitySystemComponent *Component);

    //I dunno what this is for
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "AbilitySystem")
    void ActivateComponent();

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "AbilitySystem")
    void UpdateComponent();

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "AbilitySystem")
    void DeactivateComponent();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
    UAbilitySystemComponent *AbilitySystemComponent; //Set through ComponentSetup

    /*
    UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
    ACharacter *Caster = nullptr;

    UFUNCTION(BlueprintPure, Category = "Spell System")
    static bool Execute(AActor *Target, const TArray<TSubclassOf<UAbilityAction>> &List,
                        int Empowerment, ACharacter *Caster, AActor *Causer);

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Ability System")
    bool ComponentSetup(ACharacter *Parent, int32 ActionParameter);

    UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
    ACharacter *Caster = nullptr;*/
};

/*
UCLASS(Abstract, Blueprintable, BlueprintType)
class ASelfComponent : public AAbilityComponent
{
    GENERATED_BODY()

public:
    /// dead code - TODO: remove if just works
    //UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Spell System")
    //bool ComponentSetup(ACharacter *Parent, int32 ActionParameter);
    bool ComponentSetup_Implementation(ACharacter *Parent, int32 ActionParameter)
    {
        Caster            = Parent;
        ActionEmpowerment = ActionParameter;
        return AAbilityComponent::Execute(Caster, ActionList, ActionEmpowerment, Caster,
                                          this);
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TSubclassOf<UAbilityAction>> ActionList;
};*/


/*
* Тут я добавил два новых параметра - Сдвиг позиции и сдвиг направления.
* Но пока я хз как заставить их работать. Можешь попробовать пока
*/
/*
UCLASS(Blueprintable, HideDropdown)
class USpawnAction : public UAbilityAction
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<AAbilityComponent> SkillComponent;

    /// dead code - TODO: remove if just works
    //UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Spell System")
    //bool Execute(AActor *Target, int32 Empowerment = 0);
    bool Execute_Implementation(AActor *Target, int32 Empowerment = 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector PositionOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator RotationOffset;
};
*/

/*
* А вот это кое-что новое. Я тут подумал, что ТУПО компонент, который является Актором
* Лучше хранить вместе с ним набор параметров, которые будут помогать спавнить компоненты в нужном нам месте.
*/
/*
USTRUCT(Blueprintable)
struct FAbilityComponentSpawnParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<AAbilityComponent> SkillComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LocationOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator RotationOffset;

    AAbilityComponent *Spawn(AActor *Caster, int param = 0);
};
*/


/*
Итак, главные вещи
Я пока не делал сами компоненты потому что там ЛЮТАЯ СРАНЬ по моему мнению.
Я позже объясню. почему.

Нужные компонентов:
	Проджектай (Хоминг Проджектайл)
	Область (Компонент Область)
	СамПерсонаж - тут просто прослойка которая будет выполнять все экшоны на персонаже,
				  который создал этот компонент.
	Луч до первого таргета (Луч на определённое растояние)
	
*/
