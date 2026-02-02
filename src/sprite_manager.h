#ifndef SPRITE_MANAGER_H
#define SPRITE_MANAGER_H
#include <stdint.h>
#include "x16.h"
#include "intellisense_macro.h"

#define MAX_SPRITES 128

// sprite writes to VRAM are buffered here and written all at once to reduce overhead of setting Vera's data addr
// there's probably a compsci term for doing it like this
extern _sSpriteAttr _sprite_attr_lowest_changed;
extern _sSpriteAttr _sprite_attr_highest_changed;
typedef enum {
    SPRITE_ATTR_ADDR_L,
    SPRITE_ATTR_ADDR_H,
    SPRITE_ATTR_X_L,
    SPRITE_ATTR_X_H,
    SPRITE_ATTR_Y_L,
    SPRITE_ATTR_Y_H,
    SPRITE_ATTR_Z_FLIP,
    SPRITE_ATTR_SIZE_PALETTE,
}_eSpriteAttrIndex;

// -- for writting to this from c

// bits 12-5 of addr
extern uint8_t* sprite_attr_addr_l;
// bits 16-13 of addr, bit 7 of this is mode (0 = 4bpp, 1 = 8bpp)
extern uint8_t* sprite_attr_addr_h;
// main pos x
extern uint8_t* sprite_attr_x;
// pos x bits 9-8, rest is unnused
extern uint8_t* sprite_attr_x_h;
// main pos y
extern uint8_t* sprite_attr_y;
// pos xybits 9-8, rest is unnused
//extern uint8_t sprite_attr_y_h[MAX_SPRITES]; // commenting this out because we probably wont ever use it

// bit 0 = H-flip, bit 1 = V-flip, bits 3-2 = Z-depth, bits 7-4 = collision mask
extern uint8_t* sprite_attr_z_flip;
// bits 3-0 = palette, bits 5-4 = width, bits 7-6 = height
extern uint8_t* spirte_attr_size_palette;


// this will probably be called a lot so I'll make it in asm from the start
// (low byte is sprite_n, high byte is attr_n)
extern void fastcall SpriteManagerNotifyChanged(uint16_t attr_spr);
extern void fastcall SpriteManagerWriteChanges();

#endif