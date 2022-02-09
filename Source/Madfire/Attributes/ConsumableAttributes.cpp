#include "ConsumableAttributes.h"
#include "Character/Components/AttributesComponent.h"
#include "Character/CharacterBase.h"


FTimerManager &UDynamicAttribute::GetTimerManager() const
{
    const UActorComponent *parent = Cast<UActorComponent>(GetOuter());
    check(parent != nullptr && "DynamicAttribute: ivalid parent");
    ACharacterBase *CharParent = Cast<ACharacterBase>(parent->GetOwner());
    check(CharParent != nullptr && "DynamicAttribute: ivalid character parent");
    check(GetWorld() != nullptr && "DynamicAttribute: ivalid world");
    return GetWorld()->GetTimerManager();
}
