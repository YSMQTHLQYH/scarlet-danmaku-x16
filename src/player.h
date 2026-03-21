#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>
#include "zp_utils.h"

typedef struct
{
    _uConv16 x, y;
    uint8_t spr_obj;
    uint8_t hitbox_spr_obj;
    uint8_t hitbox_visible : 1;
}_sPlayer;

// returns 0 on success, 1 on failure
uint8_t PlayerInit(_sPlayer* p);

void PlayerFree(_sPlayer* p);

// uses zpc0/1/2/3!!!!! 
void PlayerTick(_sPlayer* p);





#endif
