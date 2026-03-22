#include "player_shot.h"

#include "x16.h"
#include "zp_utils.h"
#include "sprite_manager.h"

uint8_t spr_data = 0xC8;

void shoot(_sPlayer* p) {
    uint8_t obj;
    obj = CreateSpriteObject(SPR_PRIORITY_LOW, 1, 1, 0b0101);
    //EMU_DEBUG_1(sprite_object[obj].spr_index);
    if (obj == 0xFF) {
        return;
    }
    SpriteObjectSetAddr(obj, TEST_SPRITESHEET_NUMBER | 0x80, &spr_data);
    SpriteObjectSetZFlip(obj, 0x0C);
    SpriteObjectSetSizePalette(obj, 0b0101, 15);
    SpriteObjectSetPosition(obj, p->x.h, p->y.h);
}
