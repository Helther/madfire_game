#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameFramework/Character.h"

#include "AbstractRequirement.generated.h"

/**
* Итак, дальше будут объяснения, что тут происходит
*/

// Это абстрактный класс наших требований
// У Требований будут два параметра
// 1) Сами требования (Подходит ли перрсонаж под них или нет)
// 2) Описание Требований
// TODO: наследовать от UObject
UCLASS(EditInlineNew, abstract, Blueprintable)
class UAbstractRequirement : public UDataAsset
{
    GENERATED_BODY()    //Непонятный макрос Анриала, который генерирует over 9000 ненужных строчек кода
public:
    // Это функция проверки Требований
    // Входной параметр - персонаж, которого будет проверять
    // Выходной параметр - подходит персонаж (труъ) или нет (не труъ)
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Requirements Interface")
    bool CheckRequirement(ACharacter *character);

    ///TODO: Возможно имеет смысл сделать проверку просто BlueprintCallable
    /// И виртуальной, и сделать все основные классы проверок на плюсах
    /// Подумай об этом.

    // Это функция получения описания требования
    // В ней сначала осуществляется подстановка аргументов
    // А потом они подставляются в параметр RequirementDescription
    // Она возвращает Текст.
    UFUNCTION(BlueprintPure, Category = "Requirments Interface")
    FText GetStringOfRequirement();

protected:    //Кометарий о том, что нужно объявление для protected:
    //Это наш форматированный текст
    UPROPERTY(EditAnywhere, Category = "Requirements")
    FText RequirementDescription;

    //Это аргументы для форматированного текста
    //см. cpp файл для понимания этой хрени
    FFormatNamedArguments Arguments;

    //Это функция, в которой составляется набор аргументов.
    virtual void SetDescriptionParameters(){};
};


//Коментарий о том, что из-за коментариев код выглядит сложнее чем он есть


//Это связка двух требований, которые должны выполняться оба вместе
//Думаю это и так понятно по названию
UCLASS()
class UAndRequirement : public UAbstractRequirement
{
    GENERATED_BODY()
public:
    /// dead code - TODO: remove if just works
    //UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Requirement Interface")
    //bool CheckRequirement(ACharacter *character);
    bool CheckRequirement_Implementation(ACharacter *character);

private:
    //Требование один
    UPROPERTY(EditAnywhere, Instanced)
    UAbstractRequirement *RequirementLHS = nullptr;

    //Требование два
    UPROPERTY(EditAnywhere, Instanced)
    UAbstractRequirement *RequirementRHS = nullptr;

protected:
    virtual void SetDescriptionParameters() override;
};


//Это связка двух требований, из которых хотя бы одно должно выполняться
UCLASS()
class UOrRequirement : public UAbstractRequirement
{
    GENERATED_BODY()
public:
    /// dead code - TODO: remove if just works
    //UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Requirement Interface")
    //bool CheckRequirement(ACharacter *character);
    bool CheckRequirement_Implementation(ACharacter *character);

private:
    //Требование два
    UPROPERTY(EditAnywhere, Instanced)
    UAbstractRequirement *RequirementLHS = nullptr;

    //Требование один
    UPROPERTY(EditAnywhere, Instanced)
    UAbstractRequirement *RequirementRHS = nullptr;

protected:
    virtual void SetDescriptionParameters() override;
};


UCLASS(EditInlineNew, abstract, Blueprintable)
class UAbilityRequirement : public UAbstractRequirement
{
    GENERATED_BODY()    //Непонятный макрос Анриала, который генерирует over 9000 ненужных строчек кода
public:
    // В этой функции будет происходить поглощение (вычитание) необходимых статов
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Requirements Interface")
    bool Consume(ACharacter *character);
};


//Тестовый класс требований.
UCLASS()
class UTestRequirement : public UAbstractRequirement
{
    GENERATED_BODY()
public:
    /// dead code - TODO: remove if just works
    //UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Requirement Interface")
    //bool CheckRequirement(ACharacter *character);
    bool CheckRequirement_Implementation(ACharacter *character);

protected:
    //Параметр, который мы проверяем
    UPROPERTY(EditAnywhere)
    float CheckableSpeed;
    virtual void SetDescriptionParameters() override;
};


UCLASS()
class UTestRequirementTwo : public UTestRequirement
{
    GENERATED_BODY()
public:
    /// dead code - TODO: remove if just works
    //UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Requirement Interface")
    //bool CheckRequirement(ACharacter *character);
    bool CheckRequirement_Implementation(ACharacter *character);
};

UCLASS()
class ATestRequirementActor : public AActor
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced)
    TArray<UAbstractRequirement *> Requirements;
};
