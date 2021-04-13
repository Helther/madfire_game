#pragma once

#include "../../Attributes/Attributes.h"
#include "../../Attributes/Skills.h"
#include "Components/ActorComponent.h"
#include "Object.h"
#include "ExperienceComponent.generated.h"

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
class UExperienceComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UExperienceComponent(const class FObjectInitializer &ObjectInitilizer);
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
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Attributes")
		TArray<FLevelInfo> LevelList;

#if WITH_EDITOR
    void PostEditChangeProperty(struct FPropertyChangedEvent &e)
    {
        Super::PostEditChangeProperty(e);

        FName PropertyName = (e.Property != nullptr) ? e.Property->GetFName() : NAME_None;
        if(PropertyName == GET_MEMBER_NAME_CHECKED(UExperienceComponent, LevelList))
        {
            Update();
        }

    }
#endif
     /*
     void MYClass::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
     {
         setMyValue(_sensorType, true);
         // Call the base class version
         Super::PostEditChangeProperty(PropertyChangedEvent);
     }
     #endif*/

    // clang-format on
private:
    bool IsDirty = false;
};
