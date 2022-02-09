#pragma once

#include "TraceFunctional.h"

/*
 * Сдесь будут задефайнены используемые трейсы
 *
 *
 */

constexpr ECollisionChannel EWallChannel     = ECollisionChannel::ECC_GameTraceChannel1;
constexpr ECollisionChannel EInteractChannel = ECollisionChannel::ECC_GameTraceChannel2;
constexpr ECollisionChannel EWeaponChannel   = ECollisionChannel::ECC_GameTraceChannel3;