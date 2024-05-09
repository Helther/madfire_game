#pragma once


#include "Engine/Classes/Components/AudioComponent.h"
#include "AbilitySystem/Components/AbilityComponent.h"
#include "Sound/SoundCue.h"
#include "AbilitySoundComponent.generated.h"


UCLASS(EditInlineNew, Blueprintable, BlueprintType)
class UAbilitySoundComponent : public UAbilityComponent
{
    GENERATED_BODY()

public:
    bool ComponentSetup(ACharacter *Component) override;

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "AbilitySystem")
    UAudioComponent* AudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    FName SocketName = "S_Head";

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    EAttachmentRule LocationRule;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    EAttachmentRule RotationRule;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    EAttachmentRule ScaleRule;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
    bool bWeldSimulatedBodies;

    UFUNCTION(BlueprintCallable, Category = "AbilitySystem")
    void PlaySound(USoundCue* AbilitySound, bool Loop = false);

    UFUNCTION(BlueprintCallable, Category = "AbilitySystem")
    void StopSound();


    //TODO: Do we need this?
    static constexpr auto VolumeMultiplier = 1.f;
    static constexpr auto PitchAmount = 1.f;
    // UAbilityComponent interface
public:
    void ComponentInit(UAbstractAbility *parent) override; //Called only on

    [[deprecated("Не уверен что этот интерфейс подходит, т.к. что лучше удалить за ненадобностью")]]
    void ActivateComponent_Implementation() override;
    void UpdateComponent_Implementation() override;
    void DeactivateComponent_Implementation() override;

private:
    bool IsValid();
};
