// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAnimInstance.h"
#include "CharacterBase.h"

void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Character = Cast<ACharacterBase>(GetOwningActor());
    //cast fails while haven't started game yet in editor, but otherwise it's fine
    //check(Character != nullptr && "UCharAnimInstance error: No owner")
    Super::NativeUpdateAnimation(DeltaSeconds);
}


