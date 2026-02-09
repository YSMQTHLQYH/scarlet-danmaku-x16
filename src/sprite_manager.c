#include "sprite_manager.h"
#include "zp_utils.h"
#include "memory_map.h"


/* ALL OF THESE ARE HARD CODED IN ASM BTW*/
/* also don't forget to switch ram bank to MEM_BANK_SPRITE_TABLE*/

// the arrays for buffering the data are just stored in a high ram directly via a pointer
// we are not actually defining an array, just a pointer to it

// we are leaving room for 256 sprites, even if we only have up to 128
// this makes reading and writting faster (just high byte is attr_n and lower byte is spr_n)
uint8_t* sprite_attr_addr_l = (uint8_t*)(HIGH_RAM_START + 0);
uint8_t* sprite_attr_addr_h = (uint8_t*)(HIGH_RAM_START + 0x100);
uint8_t* sprite_attr_x = (uint8_t*)(HIGH_RAM_START + 0x200);
uint8_t* sprite_attr_x_h = (uint8_t*)(HIGH_RAM_START + 0x300);
uint8_t* sprite_attr_y = (uint8_t*)(HIGH_RAM_START + 0x400);
uint8_t* sprite_attr_y_h = (uint8_t*)(HIGH_RAM_START + 0x500);
uint8_t* sprite_attr_z_flip = (uint8_t*)(HIGH_RAM_START + 0x600);
uint8_t* sprite_attr_size_palette = (uint8_t*)(HIGH_RAM_START + 0x700);


// array of bools, 128 bytes long
// size is MAX_SPRITES (128), interleaved in the gaps between the sprite data buffers
static uint8_t* sprites_used = (uint8_t*)(HIGH_RAM_START + 0x80);
// size is 4 * MAX_SPRITE_OBJECTS (256 currently, so 0xA800 to 0xA900)
_sSpriteObject* sprite_object = (_sSpriteObject*)(HIGH_RAM_START + 0x800);

void SpriteManagerInit() {
    uint8_t i;
    x16_ram_bank = MEM_BANK_SPRITE_TABLE;
    for (i = 0; i < MAX_SPRITES; i++) {
        sprites_used[i] = 0;
        sprite_attr_addr_l[i] = 0;
        sprite_attr_addr_h[i] = 0;
        sprite_attr_x[i] = 0;
        sprite_attr_x_h[i] = 0;
        sprite_attr_y[i] = 0;
        sprite_attr_y_h[i] = 0;
        sprite_attr_z_flip[i] = 0;
        sprite_attr_size_palette[i] = 0;
    }
    for (i = 0; i < MAX_SPRITE_OBJECTS; i++) {
        sprite_object[i].count = 0;
        sprite_object[i].width = 0;
        sprite_object[i].spr_index = 0;
        sprite_object[i].spr_size = 0;
    }
}

//  ---- (multi)sprite objects, for things that don't need multiple instances of

uint8_t CreateSpriteObject(uint8_t priority, uint8_t count, uint8_t width, uint8_t spr_size) {
    uint8_t i;
    x16_ram_bank = MEM_BANK_SPRITE_TABLE;
    // search for free object slots
    for (i = 0; i < MAX_SPRITE_OBJECTS; i++) {
        // we only need to check if count is 0, (you wouldn't have a sprite object with 0 sprites)
        if (sprite_object[i].count == 0) {
            sprite_object[i].spr_index = SpriteManagerReserve(count, priority);
            if (sprite_object[i].spr_index == 0xFF) {
                // couldn't get any free hardware sprites
                sprite_object[i].spr_index = 0;
                return 0xFF;
            }
            // we're still here so we got the sprites
            sprite_object[i].count = count;
            sprite_object[i].width = width;
            sprite_object[i].spr_size = spr_size;
            EMU_DEBUG_2(sprite_object[i].spr_index);
            return i;
        }
    }
    // didn't have any object slots free
    return 0xFF;
}
void FreeSpriteObject(uint8_t obj_index) {
    if (obj_index >= MAX_SPRITE_OBJECTS) { return; }
    x16_ram_bank = MEM_BANK_SPRITE_TABLE;
    SpriteManagerFree(sprite_object[obj_index].count, sprite_object[obj_index].spr_index);
    sprite_object[obj_index].count = 0;
    sprite_object[obj_index].width = 0;
    sprite_object[obj_index].spr_index = 0;
    sprite_object[obj_index].spr_size = 0;
}

void SpriteObjectSetAddr(uint8_t obj_index, uint8_t sheet_n, uint8_t* data) {
#define I           zpc0.h
#define TOP_INDEX   zpc0.l
#define CONV        zpc0
#define PTR         zptr0
    //uint8_t i; // using zpc0.h instead of this, we might as well
    PTR = data;
    if (obj_index >= MAX_SPRITE_OBJECTS) { return; }
    x16_ram_bank = MEM_BANK_SPRITE_TABLE;
    TOP_INDEX = sprite_object[obj_index].spr_index + sprite_object[obj_index].count; // zpc not meant to be used as two separate varaiables but idc
    for (I = sprite_object[obj_index].spr_index; I < TOP_INDEX; I++) {
        sprite_attr_addr_l[I] = *(PTR++);
        sprite_attr_addr_h[I] = sheet_n;
    }
    if (I > 1) {
        //CONV.l = sprite_object[obj_index].spr_index + sprite_object[obj_index].count; // zpc0.l is still this
        CONV.h = SPRITE_ATTR_ADDR_L;
        SpriteManagerNotifyChanged(CONV.w);
        CONV.h = SPRITE_ATTR_ADDR_H;
        SpriteManagerNotifyChanged(CONV.w);
    }
    CONV.l = sprite_object[obj_index].spr_index;
    CONV.h = SPRITE_ATTR_ADDR_L;
    SpriteManagerNotifyChanged(CONV.w);
    CONV.h = SPRITE_ATTR_ADDR_H;
    SpriteManagerNotifyChanged(CONV.w);
#undef PTR
#undef CONV
#undef TOP_INDEX
#undef I
}

void SpriteObjectSetPosition(uint8_t obj_index, uint16_t x, uint16_t y) {
#define CONV_AUX    zpc0
#define CONV_X      zpc0
#define CONV_Y      zpc1
#define SIZE_X      zpa0
#define SIZE_Y      zpa1
    uint8_t i, j, top_index;
    if (obj_index >= MAX_SPRITE_OBJECTS) { return; }
    CONV_Y.w = y & 0x03FF;
    CONV_X.w = x & 0x03FF;
    x16_ram_bank = MEM_BANK_SPRITE_TABLE;
    if (sprite_object[obj_index].count == 1) {
        // make this a separate function ?
        i = sprite_object[obj_index].spr_index;
        sprite_attr_x[i] = CONV_X.l;
        sprite_attr_x_h[i] = CONV_X.h;
        sprite_attr_y[i] = CONV_Y.l;
        sprite_attr_y_h[i] = CONV_Y.h;

        CONV_AUX.l = sprite_object[obj_index].spr_index;
        CONV_AUX.h = SPRITE_ATTR_X_L;
        SpriteManagerNotifyChanged(CONV_AUX.w);
        CONV_AUX.h = SPRITE_ATTR_X_H;
        SpriteManagerNotifyChanged(CONV_AUX.w);
        CONV_AUX.h = SPRITE_ATTR_Y_L;
        SpriteManagerNotifyChanged(CONV_AUX.w);
        CONV_AUX.h = SPRITE_ATTR_Y_H;
        SpriteManagerNotifyChanged(CONV_AUX.w);
        return;
    }
    switch (sprite_object[obj_index].spr_size & 0x03) {
    case 3:
        SIZE_X = 64;
        break;
    case 2:
        SIZE_X = 32;
        break;
    case 1:
        SIZE_X = 16;
        break;
    case 0:
    default:
        SIZE_X = 8;
        break;
    }
    switch ((sprite_object[obj_index].spr_size >> 2) & 0x03) {
    case 3:
        SIZE_Y = 64;
        break;
    case 2:
        SIZE_Y = 32;
        break;
    case 1:
        SIZE_Y = 16;
        break;
    case 0:
    default:
        SIZE_Y = 8;
        break;
    }
    top_index = sprite_object[obj_index].spr_index + sprite_object[obj_index].count;
    for (j = sprite_object[obj_index].spr_index; j < top_index; j += sprite_object[obj_index].width) {
        for (i = j; i < j + sprite_object[obj_index].width; i++) {
            sprite_attr_x[i] = CONV_X.l;
            sprite_attr_x_h[i] = CONV_X.h;
            sprite_attr_y[i] = CONV_Y.l;
            sprite_attr_y_h[i] = CONV_Y.h;
            CONV_X.w += SIZE_X;
        }
        CONV_Y.w += SIZE_Y;
        CONV_X.w = x;
    }
    //if (sprite_object[obj_index].count > 1) { // already exited early becasue of this
    CONV_AUX.l = sprite_object[obj_index].spr_index + sprite_object[obj_index].count;
    CONV_AUX.h = SPRITE_ATTR_X_L;
    SpriteManagerNotifyChanged(CONV_AUX.w);
    CONV_AUX.h = SPRITE_ATTR_X_H;
    SpriteManagerNotifyChanged(CONV_AUX.w);
    CONV_AUX.h = SPRITE_ATTR_Y_L;
    SpriteManagerNotifyChanged(CONV_AUX.w);
    CONV_AUX.h = SPRITE_ATTR_Y_H;
    SpriteManagerNotifyChanged(CONV_AUX.w);
    //}

    CONV_AUX.l = sprite_object[obj_index].spr_index;
    CONV_AUX.h = SPRITE_ATTR_X_L;
    SpriteManagerNotifyChanged(CONV_AUX.w);
    CONV_AUX.h = SPRITE_ATTR_X_H;
    SpriteManagerNotifyChanged(CONV_AUX.w);
    CONV_AUX.h = SPRITE_ATTR_Y_L;
    SpriteManagerNotifyChanged(CONV_AUX.w);
    CONV_AUX.h = SPRITE_ATTR_Y_H;
    SpriteManagerNotifyChanged(CONV_AUX.w);
#undef SIZE_Y
#undef SIZE_X
#undef CONV_Y
#undef CONV_X
#undef CONV
}

void SpriteObjectSetZFlip(uint8_t obj_index, uint8_t z) {
#define CONV    zpc0
    uint8_t i, top_index;
    if (obj_index >= MAX_SPRITE_OBJECTS) { return; }
    x16_ram_bank = MEM_BANK_SPRITE_TABLE;
    top_index = sprite_object[obj_index].spr_index + sprite_object[obj_index].count;
    for (i = sprite_object[obj_index].spr_index; i < top_index; i++) {
        sprite_attr_z_flip[i] = z;
    }
    CONV.h = SPRITE_ATTR_Z_FLIP;
    if (i > 1) {
        CONV.l = top_index;
        SpriteManagerNotifyChanged(CONV.w);
    }
    CONV.l = sprite_object[obj_index].spr_index;
    SpriteManagerNotifyChanged(CONV.w);
#undef CONV
}
void SpriteObjectSetSizePalette(uint8_t obj_index, uint8_t size, uint8_t palette) {
#define CONV    zpc0
    uint8_t i, a, top_index;
    if (obj_index >= MAX_SPRITE_OBJECTS) { return; }
    x16_ram_bank = MEM_BANK_SPRITE_TABLE;
    a = palette & 0x0F;
    sprite_object[obj_index].spr_size = size;
    a += size << 4;
    top_index = sprite_object[obj_index].spr_index + sprite_object[obj_index].count;
    for (i = sprite_object[obj_index].spr_index; i < top_index; i++) {
        sprite_attr_size_palette[i] = a;
    }
    CONV.h = SPRITE_ATTR_SIZE_PALETTE;
    if (i > 1) {
        CONV.l = top_index;
        SpriteManagerNotifyChanged(CONV.w);
    }
    CONV.l = sprite_object[obj_index].spr_index;
    SpriteManagerNotifyChanged(CONV.w);
#undef CONV
}

//  ---- reserving hardware sprites

// high priority: low indexes
static uint8_t SmCheckHigh(uint8_t n) {
    uint8_t i, j, aux = 0;
    for (i = 63; i < 0xFF; i--) { // can't do >= 0 on unsigned, checks for underflow instead
        // aux is number of free slots in a row
        if (sprites_used[i] == 0) {
            aux++;
        } else {
            aux = 0;
        }
        if (aux >= n) {
            // got them, i is at the first slot of group
            for (j = i; j < i + n; j++) {
                sprites_used[j] = 1;
            }
            return i;
        }
    }
    // couldn't find any free block of slots
    return 0xFF;
}
// low priority: high indexes
static uint8_t SmCheckLow(uint8_t n) {
    uint8_t i, j, aux = 0;
    for (i = 64; i <= 127; i++) {
        // aux is number of free slots in a row
        if (sprites_used[i] == 0) {
            aux++;
        } else {
            aux = 0;
        }
        if (aux >= n) {
            // got them, i is at the first slot of group
            for (j = i; j > i - n; j--) {
                sprites_used[j] = 1;
            }
            return i - n + 1;
        }
    }
    // couldn't find any free block of slots
    return 0xFF;
}

uint8_t SpriteManagerReserve(uint8_t n, uint8_t priority) {
    uint8_t i, used = 0;
    x16_ram_bank = MEM_BANK_SPRITE_TABLE;
    if (priority < MAX_SPRITES) {
        //asked for specific index
        for (i = priority; i < priority + n; i++) {
            used += sprites_used[i];
        }
        if (used == 0) {
            // that specific index is free
            for (i = priority; i < priority + n; i++) {
                sprites_used[i] = 1;
            }
            return priority;
        }
        // that specific index is NOT free
        return 0xFF;
    }
    // just give me a sprite idc which one
    switch (priority) {
    case SPR_PRIORITY_HIGH:
        // lower indexes render first
        i = SmCheckHigh(n);
        if (i != 0xFF) {
            // slots found
            return i;
        }
        // couldn't find anything in high, trying low
        i = SmCheckLow(n);
        if (i != 0xFF) {
            // slots found
            return i;
        }
        // still nothing :(
        return 0xFF;
    case SPR_PRIORITY_LOW:
        // higher indexes render last
        i = SmCheckLow(n);
        if (i != 0xFF) {
            // slots found
            return i;
        }
        // couldn't find anything in low, trying high
        i = SmCheckHigh(n);
        if (i != 0xFF) {
            // slots found
            return i;
        }
        // still nothing :(
        return 0xFF;

    default:
        //invalid index
        return 0xFF;
    }
}
void SpriteManagerFree(uint8_t n, uint8_t index) {
    uint8_t i;
    x16_ram_bank = MEM_BANK_SPRITE_TABLE;
    for (i = index; i < index + n; i++) {
        sprites_used[i] = 0;
    }
}
