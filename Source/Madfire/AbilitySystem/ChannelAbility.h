#pragma once

#include "CoreMinimal.h"
#include "AbstractAbility.h"
#include "ChannelAbility.generated.h"

/*
 */
UCLASS(Blueprintable, BlueprintType, abstract)
class UChannelAbility : public UAbstractAbility
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Parameters")
		float TickTime = 1.0f;

	/// dead code - TODO: remove if just works
	//UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		//void OnEventKeyPressed();
	void OnEventKeyPressed_Implementation();

	/// dead code - TODO: remove if just works
	//UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		//void OnEventKeyReleased();
	void OnEventKeyReleased_Implementation();

	/// dead code - TODO: remove if just works
	//UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		//void InterruptSkill();
	void InterruptSkill_Implementation();

	/// dead code - TODO: remove if just works
	//UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		//void StopSkill();
	void StopSkill_Implementation();
	
protected:
	FTimerHandle ChannelTimer;

	void StartCast(void);

	void StopCast(void);
	
	void OnSkillNotify(void);

	void OnTick(void);
};
