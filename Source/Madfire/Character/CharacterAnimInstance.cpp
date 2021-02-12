// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAnimInstance.h"

#include "CharacterBase.h"

const FName FAnimSlotNames::FullBody = "FullBody";
const FName FAnimSlotNames::UpperBody = "UpperBody";
const FName FAnimSlotNames::Head = "Head";
const FName FAnimSlotNames::RightHand = "RightHand";
const FName FAnimSlotNames::LeftHand = "LeftHand";

const FName FAnimGroupNames::MainGroup = "Main";

void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Character = Cast<ACharacterBase>(GetOwningActor());
    //cast fails while haven't started game yet in editor, but otherwise it's fine
    //check(Character != nullptr && "UCharAnimInstance error: No owner");
    Super::NativeUpdateAnimation(DeltaSeconds);
}
