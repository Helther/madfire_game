#include "Character/CharacterAnimInstanceBase.h"

#include "Character/CharacterBase.h"

const FName FAnimSlotNames::FullBody = "FullBody";
const FName FAnimSlotNames::UpperBody = "UpperBody";
const FName FAnimSlotNames::Head = "Head";
const FName FAnimSlotNames::RightHand = "RightHand";
const FName FAnimSlotNames::LeftHand = "LeftHand";

const FName FAnimGroupNames::MainGroup = "Main";

void UCharacterAnimInstanceBase::NativeUpdateAnimation(float DeltaSeconds)
{
    Character = Cast<ACharacterBase>(GetOwningActor());
    //cast fails while haven't started game yet in editor, but otherwise it's fine
    //check(Character != nullptr && "UCharAnimInstance error: No owner");
    Super::NativeUpdateAnimation(DeltaSeconds);
}
