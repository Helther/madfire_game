#include "Barrier.h"


float UBarrier::TakeDamage(float DamageAmount, UDamageType *DamageType, AActor *DamageCauser)
{
    check(DamageType != nullptr && "Damage Type is Invalid");
    OnHit(DamageAmount,DamageCauser);



    return 0;
}
