#pragma once

#include "CoreMinimal.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "WeakSpotPhysicalMat.generated.h"


UENUM(BlueprintType)
enum class EWeakSpotType : uint8
{
    Head = 0    UMETA(DisplayName = "Head"),
    Hand        UMETA(DisplayName = "Hand"),
    Leg         UMETA(DisplayName = "Leg")
};

/*
 * Base class for blueprint classes created from and for skeletal mesh physics asset.
 * 
 * To create and assign a weak spot open mesh physics asset, choose collision volume, 
 * select physMaterial, press "create new", configure it, done!
 * 
 * To add a new EWeakSpotType update the enum and modify the Projectile Overlap and Character TakeDamage functions
 * and you set!
 */
UCLASS(BlueprintType, Blueprintable)
class MADFIRE_API UWeakSpotPhysicalMat : public UPhysicalMaterial
{
    GENERATED_BODY()
public:
    UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
    EWeakSpotType Type;
};
