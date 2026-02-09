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
extern uint8_t* sprite_attr_size_palette;


void SpriteManagerInit();

// this will probably be called a lot so I'll make it in asm from the start
// (low byte is sprite_n, high byte is attr_n)
extern void fastcall SpriteManagerNotifyChanged(uint16_t attr_spr);
extern void fastcall SpriteManagerWriteChanges();


/* ---- sprite managing (keeping track of used vs unused hardware sprites) ---- */
//  ---- (multi)sprite objects, for things that don't need multiple instances of
#define MAX_SPRITE_OBJECTS  64
typedef struct {
    uint8_t count; // TOTAL number of sprites used
    uint8_t width; // number of sprites per row (height is inplied)
    uint8_t spr_index; // index of FIRST hardware sprite (they are guaranteed to be contiguous)
    uint8_t spr_size; // height/width, 2 bits each (same format that goes into vram) (used for position of multi sprite objects)
}_sSpriteObject;

extern _sSpriteObject* sprite_object;
// creates a (multi)sprite object
// priority is if it should render in front or behind already existing sprites (or an specific index)
// rest ofarguments are the same that get written object struct
// returns object index (or 0xFF if it failed)
uint8_t CreateSpriteObject(uint8_t priority, uint8_t count, uint8_t width, uint8_t spr_size);
void FreeSpriteObject(uint8_t index);

// usues zpc0 and zptr0!
// sets the address of graphics data for each sprite
// data is a pointer the the array of data (left to right, then top to bottom)
// sheet_n is bits 16 to 13 is address (shared for the entire object), also bit 7 of sheet_n is mode (0 = 4bpp, 1 = 8bpp)
void SpriteObjectSetAddr(uint8_t obj_index, uint8_t sheet_n, uint8_t* data);

// uses zpc0, zpc1, zpa0 and zpa1!
void SpriteObjectSetPosition(uint8_t obj_index, uint16_t x, uint16_t y);

// uses zpc0!
// bits 3-2 = z, bits 1-0 = flip
void SpriteObjectSetZFlip(uint8_t obj_index, uint8_t z);
// uses zpc0!
// size: bits 0-1 = width, bits 3-2 = height
void SpriteObjectSetSizePalette(uint8_t obj_index, uint8_t size, uint8_t palette);

//  ---- reserving hardware sprites
enum {
    SPR_PRIORITY_HIGH = MAX_SPRITES + 1,
    SPR_PRIORITY_LOW = MAX_SPRITES + 2,
};
/* marks hardware sprite slots as used
n: number of hardware slots to reserve, guaranteed to be contiguous
priority (/index): 0 to 127: ask for a specific hardware slot
SM_PRIORITY_HIGH: on top of already reserved sprites, SM_PRIORITY_LOW: behind already reserved sprites
(it might ignore priority if that's the only place there's room)
returns index of first sprite reseved, 0xFF if it couldn't*/
uint8_t SpriteManagerReserve(uint8_t n, uint8_t priority);
// marks hardware sprites as unused, args same as reserve()
void SpriteManagerFree(uint8_t n, uint8_t index);

#endif