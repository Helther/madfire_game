#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LevelList.h"
#include "MovementComponentBase.h"
#include "CombatComponent.h"
#include "EquipmentComponent.h"
#include "AttributesComponent.h"
#include "InventoryComponent.h"
#include "CharacterBase.generated.h"


UENUM(BlueprintType, meta = (Bitflags))
// Character Control Bit mask with values as priorities (the more the better)
enum ECCState
{
	NoCC = 0					UMETA(DisplayName = "Sleeping"),
	Sleep = 1					UMETA(DisplayName = "LostAllBalance"),
	LossOfBalance = 2	UMETA(DisplayName = "Stunned"),
	Stun = 4					UMETA(DisplayName = "KnockedDown"),
	KnockDown = 8			UMETA(DisplayName = "UnderStasis"),
	Stasis	= 16				
};
ENUM_CLASS_FLAGS(ECCState)

/**
 * Obvious Class Decription
 */
UCLASS()
class MADFIRE_API ACharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACharacterBase(const FObjectInitializer& ObjectInitializer);

	float TakeDamage(float DamageAmount, 
                    const struct FDamageEvent &DamageEvent,
					class AController *EventInstigator,
					AActor *DamageCauser) override;

	virtual void Update();
	void OnJumped_Implementation() override;
	UFUNCTION(BlueprintCallable, Category = "Character Base")
		void OnUpdate(); //!< Blueprint interface  for Blueprints

	UFUNCTION(BlueprintCallable, Category = "Character Base")
		void StartAnimation(UAnimMontage* AnimMontage, AnimTypes Type, float InPlayRate, FName StartSectionName = NAME_None);

	UFUNCTION(BlueprintCallable, Category = "Character Base")
		void InterruptAnimation(ECCState Source, int OldState);

	// interface for global interrupt of any visual activity(fighting, parkour)
	UFUNCTION(BlueprintCallable, Category = "Character Base")
		void InterruptAction(ECCState Source);

	UBaseMovementComponent* GetMovementComponent() const override;
  UCombatComponent* GetCombatComponent() const;
	UAttributesComponent* GetAttributesComponent() const;
	ULevelComponent* GetLevelComponent() const;
	UEquipmentComponent* GetEquipmentComponent() const;
	UInventoryComponent* GetInventoryComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Character Base")
		virtual void DoLevelUp(const FLevelInfo &levelInfo);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Bitmask, BitmaskEnum = "ECCState"),
    Category = "Character Stats")
		int CurrentCCState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Stats", 
		meta = (ClampMin = "0.0", UIMin = "0.0"))
		float MaxWeightMod;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Stats")	
		float MaxWeight;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite, Category = "Character Components", 
		VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
		UCombatComponent *CombatComponent;
	UPROPERTY(BlueprintReadWrite, Category = "Character Components",
		VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
		UAttributesComponent *AttributesComponent;
	UPROPERTY(BlueprintReadWrite, Category = "Character Components", 
		VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
		ULevelComponent *LevelComponent;
	UPROPERTY(BlueprintReadWrite, Category = "Character Components", 
		VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
		UEquipmentComponent *EquipmentComponent;
	UPROPERTY(BlueprintReadWrite, Category = "Character Components", 
		VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
		UInventoryComponent *InventoryComponent;

		void CalculateMaxWeight();
};
