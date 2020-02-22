// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Classes/GameFramework/ProjectileMovementComponent.h"
#include "Classes/Components/CapsuleComponent.h"
#include "Classes/Components/SphereComponent.h"
#include "AbilitySystem/AbilityComponent.h"
#include "Projectile.generated.h"

/**
 * 
 */

//IT SHOULD BE UPPERCASE BECAUSE OF TOXA (F)
UENUM(BlueprintType)
enum class EProjectileDestroyPolicy : uint8
{
	OnHit,
	OnTargetHit,
	Lifespawn
}; 
ENUM_CLASS_FLAGS(EProjectileDestroyPolicy);

UCLASS()
class AAbstractProjectile : public AAbilityComponent
{
	GENERATED_BODY()

public:
	AAbstractProjectile();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, meta = (Bitmask, BitmaskEnum = "EProjectileDestroyPolicy"), Category = "Projectile")
		uint8 DestroyPolicy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<TSubclassOf<UAbilityAction>> ActionList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<TSubclassOf<UAbilityAction>> ActionListOnDestroy;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Collision")
		USphereComponent *CollisionComp;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Movement")
		UProjectileMovementComponent * MovementComponent;

	UFUNCTION()
		void Hit(UPrimitiveComponent* HitComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			FVector NormalImpulse,
			const FHitResult &Hit);

	UFUNCTION()
		void Overlap(UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex,
			bool bFromSweep,
			const FHitResult &SweepResult);

	UFUNCTION()
		void EndOverlap(UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex);
};