#include "player.h"

#include "x16.h"
#include "input_action.h"
#include "bitmap_layer.h"
#include "graphics_utils.h"
#include "sprite_manager.h"
#include "sfx_player.h"

#include "player_shot.h"

//TODO: set up a proper global selection for this
const uint8_t selected_joystick = 0;


#define POS_MIN_X       0x01
#define POS_MAX_X       0xDE
#define POS_MIN_Y       0x01
#define POS_MAX_Y       0xEE

#define SPRITE_OFFSET_X         15
#define SPRITE_OFFSET_Y         16
#define HITBOX_SPRITE_OFFSET_X  1
#define HITBOX_SPRITE_OFFSET_Y  1

#define SPEED_FAST_ORTHOGONAL    0x0280
#define SPEED_FAST_DIAGONAL      0x01C4
#define SPEED_SLOW_ORTHOGONAL    0x0140
#define SPEED_SLOW_DIAGONAL      0x00E2



uint16_t color_palette[] = {
    0x2A3, 0xBD2, 0xEFC, 0x6EF,
    0xFD4, 0xFAD, 0x2EC, 0x0AC,
    0xF93, 0xE36, 0xC3B, 0x168,
    0xE53, 0xA23, 0x739, 0x214,
};

uint8_t PlayerInit(_sPlayer* p) {
    _sFileLoadCtx load = { 0 };
    _uConv16 addr;
    uint8_t i;
    load.filename = "test assets/testplayer.bin";
    load.name_lenght = 12 + 14;
    load.dest_addr = (void*)MEM_VRAM_1_KERNAL_CHARSET_START;
    load.header_mode = FILE_LOAD_HEADERLESS;
    load.target_mode = FILE_LOAD_VRAM_P1;
    if (LoadFile(&load)) {
        EMU_DEBUG_1(0xFA);
        EMU_DEBUG_1(load.error_code);
        return 1;
    }
    // sprite object
    p->spr_obj = CreateSpriteObject(SPR_PRIORITY_HIGH, 1, 1, 0b1010);
    if (p->spr_obj == 0xFF) { return 1; }
    addr.w = MEM_VRAM_1_KERNAL_CHARSET_START >> 5;
    addr.h |= 0x08; // addr bit 16
    SpriteObjectSetAddr(p->spr_obj, addr.h, &addr.l);
    SpriteObjectSetZFlip(p->spr_obj, 0x08);
    SpriteObjectSetSizePalette(p->spr_obj, 0b1010, 15);

    SetColorPalette(15, color_palette);

    //hitbox sprite
    p->hitbox_spr_obj = CreateSpriteObject(SPR_PRIORITY_HIGH, 1, 1, 0b0000);
    if (p->hitbox_spr_obj == 0xFF) { return 1; }
    addr.w += 16;
    SpriteObjectSetAddr(p->hitbox_spr_obj, addr.h, &addr.l);
    SpriteObjectSetZFlip(p->hitbox_spr_obj, 0x00);
    SpriteObjectSetSizePalette(p->hitbox_spr_obj, 0, 0);

    //generate sprite for hitbox (just a 3x3 for now)
    VERA_CTRL = 0;
    VERA_ADDRx_H = ADDR_INC_1 + 1;
    VERA_ADDRx_M = 0xF2;
    VERA_ADDRx_L = 0x00;
    VERA_DATA0 = 0x11;
    VERA_DATA0 = 0x10;
    VERA_DATA0 = 0x00;
    VERA_DATA0 = 0x00;
    VERA_DATA0 = 0x12;
    VERA_DATA0 = 0x10;
    VERA_DATA0 = 0x00;
    VERA_DATA0 = 0x00;
    VERA_DATA0 = 0x11;
    VERA_DATA0 = 0x10;
    for (i = 0; i < 22; i++) {
        VERA_DATA0 = 0;
    }



    p->x.w = 0x7000;
    p->y.w = 0xB400;
    p->hitbox_visible = 0;
    return 0;
}

void PlayerFree(_sPlayer* p) {
    if (p == 0) { return; }
    FreeSpriteObject(p->spr_obj);
}


enum {
    DIR_X_P_ORTH = 0x01,
    DIR_X_P_DIAG = 0x02,
    DIR_X_N_ORTH = 0x03,
    DIR_X_N_DIAG = 0x04,
    DIR_Y_P_ORTH = 0x10,
    DIR_Y_P_DIAG = 0x20,
    DIR_Y_N_ORTH = 0x30,
    DIR_Y_N_DIAG = 0x40,
};
// joystick format is 0b____ in order UDLR
const uint8_t dir_lookup[16] = {
    0,                              //____
    DIR_X_P_ORTH,                   //___R
    DIR_X_N_ORTH,                   //__L_
    0,                              //__LR
    DIR_Y_P_ORTH,                   //_D__
    DIR_Y_P_DIAG | DIR_X_P_DIAG,    //_D_R
    DIR_Y_P_DIAG | DIR_X_N_DIAG,    //_DL_
    DIR_Y_P_ORTH,                   //_DLR
    DIR_Y_N_ORTH,                   //U___
    DIR_Y_N_DIAG | DIR_X_P_DIAG,    //U__R
    DIR_Y_N_DIAG | DIR_X_N_DIAG,    //U_L_
    DIR_Y_N_ORTH,                   //U_LR
    0,                              //UD__
    DIR_X_P_ORTH,                   //UD_R
    DIR_X_N_ORTH,                   //UDL_
    0,                              //UDLR
};
void PlayerTick(_sPlayer* p) {
#define SPEED_ORTHOGONAL    zpc0
#define SPEED_DIAGONAL      zpc1
#define PX                  zpc2
#define PY                  zpc3
    uint8_t dir;
    uint8_t pixel_read, mask;
    if (p == 0) { return; }
    //    ----  movement
    if (IsActionPressed(ACTION_FOCUS)) {
        SPEED_ORTHOGONAL.w = SPEED_SLOW_ORTHOGONAL;
        SPEED_DIAGONAL.w = SPEED_SLOW_DIAGONAL;
        if (!p->hitbox_visible) {
            p->hitbox_visible = 1;
            SpriteObjectSetZFlip(p->hitbox_spr_obj, 0x0C);
        }
    } else {
        SPEED_ORTHOGONAL.w = SPEED_FAST_ORTHOGONAL;
        SPEED_DIAGONAL.w = SPEED_FAST_DIAGONAL;
        if (p->hitbox_visible) {
            p->hitbox_visible = 0;
            SpriteObjectSetZFlip(p->hitbox_spr_obj, 0x00);
        }
    }
    // reading raw joystick for movement, we only care about buttons being pressed or not
    // and we don't need to remap the d-pad (we would need for keyboard controls but KERNAL already translates that to joystick)

    // doing it a oneliner wouldn't be an optimization normally, but I already know that cc65 isn't that smart...
    // this at least saves having another variable on the c_stack
    // dpad = joystick_raw[selected_joystick].mask_0 & 0x0F;
    // dir = dir_lookup[dpad];

    dir = dir_lookup[joystick_raw[selected_joystick].mask_0 & 0x0F];
    PX.w = p->x.w;
    PY.w = p->y.w;
    // X axis
    switch (dir & 0x0F) {
    case DIR_X_P_ORTH:
        PX.w += SPEED_ORTHOGONAL.w;
        if (PX.h >= POS_MAX_X) PX.w = ((uint16_t)POS_MAX_X << 8);
        break;
    case DIR_X_P_DIAG:
        PX.w += SPEED_DIAGONAL.w;
        if (PX.h >= POS_MAX_X) PX.w = ((uint16_t)POS_MAX_X << 8);
        break;
    case DIR_X_N_ORTH:
        PX.w -= SPEED_ORTHOGONAL.w;
        if (PX.h < POS_MIN_X || PX.h >= POS_MAX_X) PX.w = ((uint16_t)POS_MIN_X << 8);
        break;
    case DIR_X_N_DIAG:
        PX.w -= SPEED_DIAGONAL.w;
        if (PX.h < POS_MIN_X || PX.h >= POS_MAX_X) PX.w = ((uint16_t)POS_MIN_X << 8);
        break;
    default:
        break;
    }
    // Y axis
    switch (dir & 0xF0) {
    case DIR_Y_P_ORTH:
        PY.w += SPEED_ORTHOGONAL.w;
        if (PY.h >= POS_MAX_Y) PY.w = ((uint16_t)POS_MAX_Y << 8);
        break;
    case DIR_Y_P_DIAG:
        PY.w += SPEED_DIAGONAL.w;
        if (PY.h >= POS_MAX_Y) PY.w = ((uint16_t)POS_MAX_Y << 8);
        break;
    case DIR_Y_N_ORTH:
        PY.w -= SPEED_ORTHOGONAL.w;
        if (PY.h < POS_MIN_Y || PY.h >= POS_MAX_Y) PY.w = ((uint16_t)POS_MIN_Y << 8);
        break;
    case DIR_Y_N_DIAG:
        PY.w -= SPEED_DIAGONAL.w;
        if (PY.h < POS_MIN_Y || PY.h >= POS_MAX_Y) PY.w = ((uint16_t)POS_MIN_Y << 8);
        break;
    default:
        break;
    }

#undef SPEED_ORHOGONAL
#undef SPEED_DIAGONAL
    // these use zpc0 and zpc1
    SpriteObjectSetPosition(p->spr_obj, PX.h - SPRITE_OFFSET_X, PY.h - SPRITE_OFFSET_Y);
    SpriteObjectSetPosition(p->hitbox_spr_obj, PX.h - HITBOX_SPRITE_OFFSET_X, PY.h - HITBOX_SPRITE_OFFSET_Y);
    p->x.w = PX.w;
    p->y.w = PY.w;

    //    ---- hit detection
    // O(1) hit detection babyyyyy!!!
#define ADDR    zpc0
    VERA_CTRL = 0;
    VERA_ADDRx_H = bitmap_back_buffer;
    ADDR.w = LookupY(PY.h);
    ADDR.w += PX.h >> 1;
    VERA_ADDRx_M = ADDR.h;
    VERA_ADDRx_L = ADDR.l;
    mask = (PX.h & 1) ? 0x0F : 0xF0;
    pixel_read = VERA_DATA0;
    if ((pixel_read &= mask) != 0) {
        // HIT
        SfxPlay(0x20);
        EMU_DEBUG_1(pixel_read);
    }
#undef ADDR

#undef PX
#undef PY


    //    ----- shooting
    if (IsActionJustPressed(ACTION_SHOOT)) {
        shoot(p);
    }
}