#pragma once

#include "../Attributes/Attributes.h"
#include "../Attributes/Skills.h"
#include "Components/ActorComponent.h"
#include "Object.h"

#include "LevelList.generated.h"

constexpr auto DefaultHPBonus = 9;
constexpr auto MaxLevel       = 30;
#define EXPERIENCE_FUCKING_EQUATION(x) ((x) *= 2)

USTRUCT(BlueprintType)
struct FLevelInfo
{
    GENERATED_USTRUCT_BODY()
    FLevelInfo();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Attributes")
    float hpBonus;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Attributes")
    FSkillList SkillList;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Attributes")
    FAttributes Attributes;
    //todo add feats
};


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ULevelComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    ULevelComponent(const class FObjectInitializer &ObjectInitilizer);
    // clang-format off
	UFUNCTION(BlueprintCallable, Category = "Character Attributes")
		void ResetLevel();

	UFUNCTION(BlueprintCallable, Category = "Character Attributes")
		void LevelUp(const FLevelInfo &levelInfo); // используется через персонажа для вызова обновы
	UFUNCTION(BlueprintPure, Category = "Character Attributes")
		FORCEINLINE int32 GetLevelNum() const { return LevelList.Num(); }

	UFUNCTION(BlueprintCallable, Category = "Character Attributes")
		void Update();
	UFUNCTION(BlueprintPure, Category = "Character Attributes")
		FORCEINLINE bool CanLevelUp() const { return (CurrentExperience >= RequiredExperience) && (LevelList.Num() < MaxLevel); }

	UFUNCTION(BlueprintPure, Category = "Character Attributes")
		static FORCEINLINE uint8 GetMaxPossibleLevel() { return MaxLevel; }

	UFUNCTION(BlueprintCallable, Category = "Component Update")// todo 
		void SetUpdate() { IsDirty = true; }
	UFUNCTION(BlueprintCallable, Category = "Component Update")
		void UnSetUpdate() { IsDirty = false; }
	UFUNCTION(BlueprintCallable, Category = "Component Update")
		bool IsUpdateNeeded() { return IsDirty; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Attributes")
		float CurrentExperience;
	UPROPERTY(BlueprintReadOnly, Category = "Character Attributes")
		float LevelHealthBonus;
	UPROPERTY(BlueprintReadOnly, Category = "Character Attributes")
		FAttributes LevelAttributes;
	UPROPERTY(BlueprintReadOnly, Category = "Character Attributes")
		FSkillList LevelSkills;
	
	UPROPERTY(BlueprintReadOnly , Category = "Character Attributes")
		float RequiredExperience = 100;
	UPROPERTY(BlueprintReadOnly, Category = "Character Attributes")
		TArray<FLevelInfo> LevelList;
    // clang-format on
private:
    bool IsDirty = false;
};
