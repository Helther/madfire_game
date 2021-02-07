#pragma once

#include "AbilityAction.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Variant.h"

#include "AbilityComponent.generated.h"


/*
* Тут тоже немного изменилось. Компонент теперь хранит кастера как чарактера,
* а не Актора. В Скиле он тоже теперь чарактер. (Думаю понятно почему)
*/
UCLASS(Abstract, Blueprintable, BlueprintType)
class AAbilityComponent : public AActor
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category = "Spell System")
    static bool Execute(AActor *Target, const TArray<TSubclassOf<UAbilityAction>> &List,
                        int Empowerment, ACharacter *Caster, AActor *Causer);

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Spell System")
    bool ComponentSetup(ACharacter *Parent, int32 ActionParameter);

    UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
    ACharacter *Caster = nullptr;

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Spell System")
    void OnDestroy(AActor *hzActor);

    int32 ActionEmpowerment = 0;
};

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
};


/*
* Тут я добавил два новых параметра - Сдвиг позиции и сдвиг направления.
* Но пока я хз как заставить их работать. Можешь попробовать пока
*/
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


/*
* А вот это кое-что новое. Я тут подумал, что ТУПО компонент, который является Актором
* Лучше хранить вместе с ним набор параметров, которые будут помогать спавнить компоненты в нужном нам месте.
*/
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