#include "text.h"
#include "x16.h"



typedef union {
    struct { uint8_t u8_l; uint8_t u8_h; };
    uint16_t u16;
} _uConv16;

// from 0x1F000 to 0x1F7FF
#define KERNAL_FONT_VRAM_ADDR   0xF0

uint8_t HijackRomCharset(uint8_t charset, uint8_t font_bpp, uint8_t color) {
    uint16_t i;
    uint8_t row, px, c, d_in, d_out;
    uint8_t buf[8];
    if (charset == 0) { return 1; }
    if (font_bpp != 2 && font_bpp != 4) { return 1; }
    KernalScreenSetCharset(charset);

    // DATA1 for reading KERNAL font
    vera->CTRL = 1;
    vera->ADDRx_H = 0x11; // inc = 1, page = 1
    vera->ADDRx_M = KERNAL_FONT_VRAM_ADDR;
    vera->ADDRx_L = 0;

    // DATA0 for writting our font
    vera->CTRL = 0;
    vera->ADDRx_H = 0x10; // inc = 1, page = 0
    vera->ADDRx_M = (font_bpp == 4) ? FONT_4BPP_START : FONT_2BPP_START;
    vera->ADDRx_L = 0;

    c = (font_bpp == 4) ? (color & 0x0F) : (color & 0x03);

    for (i = 0; i < 0x100; i++) {
        // ---- copy one character
        for (row = 0; row < 8; row++) {
            // ---- read one row of pixels

            // -- read it from KERNAL charset
            d_in = vera->DATA1;
            for (px = 0; px < 8; px++) {
                if (d_in & (0x80 >> px)) {
                    // pixel has color
                    buf[px] = c;
                } else {
                    // pixel is transparent
                    buf[px] = 0;
                }
            }
            // -- turn it into 2/4 bpp format and write it out to VRAM
            if (font_bpp == 4) {
                for (px = 0; px < 8; px += 2) {
                    d_out = (buf[px] << 4);
                    d_out += buf[px + 1];
                    vera->DATA0 = d_out;
                }
            } else { // font_bpp == 2
                for (px = 0; px < 8; px += 4) {
                    d_out = (buf[px] << 6);
                    d_out += (buf[px + 1] << 4);
                    d_out += (buf[px + 2] << 2);
                    d_out += buf[px + 3];
                    vera->DATA0 = d_out;
                }
            }
            // copied one row of pixels
        }
        // copied one character
    }
    return 0;
}



//  ---- sprite text

#define TEXT_SPRITE_STR_SLOTS   8
uint8_t sprite_str_slot[TEXT_SPRITE_STR_SLOTS] = { 0 };
// index is the number of hardware sprite, value is which str slot owns it (whoever called dibs on it first)
// 0 means no slot has claimed it (and it's up for grabs)
uint8_t sprite_dibs[TEXT_MAX_SPRITES] = { 0 };


uint8_t PrintSpriteStr(char* str, uint8_t str_slot, uint16_t x, uint16_t y, uint8_t palette) {
    _uConv16 w;
    uint8_t i;
    uint8_t next_char;
    uint8_t j = 0;
    uint8_t slot = str_slot;

    if (slot == 0) {
        //  ---- caller asked for "whatever" free slot idc
        for (i = 0; i < TEXT_SPRITE_STR_SLOTS; i++) {
            if (sprite_str_slot[i] == 0) {
                // found one
                slot = i + 1;
                break;
            }
        }
        if (slot == 0) {
            // no free slots found
            return 0;
        }
    }

    vera->CTRL = 0x00; // using DATA0
    vera->ADDRx_H = 0x11; // sprite attr table is on page 1, using addr_inc = 1

    next_char = str[0];
    // loops through list of SPRITES, not of characters in string
    for (i = 0; i < TEXT_MAX_SPRITES; i++) {
        if (next_char) {// if next_char is 0 we reached end of str
            //EMU_DEBUG_2(sprite_dibs[i]);
            // there was a bug, added a few print statments for debugging, the one from the line above fixed the bug somehow
            // commented it back out, and the bug is STILL FIXED
            // fck schrödinger and his bug function collapse

            if (sprite_dibs[i] == slot || sprite_dibs[i] == 0) {
                // sprite is free or already owned by us
                sprite_dibs[i] = slot;

                // draw the thing
                w.u16 = ((uint16_t)(VERA_REG_SPRITE_ATTR_M) << 8);
                w.u16 += TEXT_SPRITE_INDEX_START;
                w.u16 += (i << 3); // sprite attr is 8 bytes per sprite
                vera->ADDRx_M = w.u8_h;
                vera->ADDRx_L = w.u8_l;

                // attr byte 0: addr_l (bits 12-5)
                // attr byte 1: addr_h (bits 16-3) (and mode but we want 0 (4bpp) anyways)
                w.u16 = (FONT_4BPP_START << 3); // FONT_4BPP_START is bits 15-7, // bit 16 is always 0 anyways
                w.u16 += next_char; // increasing addr_l by 1 already increases index by 32 bytes (size of one tile)
                vera->DATA0 = w.u8_l;
                vera->DATA0 = w.u8_h;
                // attr byte 2: x (bits 7-0)
                // attr byte 3: x (bits 9-8)
                w.u16 = x + (j << 3); // j is char index, moving x by 8 per char written
                vera->DATA0 = w.u8_l;
                vera->DATA0 = w.u8_h;
                // attr byte 4: y (bits 7-0)
                // attr byte 5: y (bits 9-8)
                w.u16 = y;
                vera->DATA0 = w.u8_l;
                vera->DATA0 = w.u8_h;
                // attr byte 6: collision mask, z-depth, flip
                vera->DATA0 = 0x0C; // we don't want collision nor flip, z = 3 to always render on top
                // attr byte 7: height, width, palette
                vera->DATA0 = (palette & 0x0F); // font is 8x8, we want height and width to be 0


                // next char
                next_char = str[++j];
            }
        } else {
            // keep iterating to clear any now unused sprite
            if (sprite_dibs[i] == slot) {
                // delete sprite
                w.u16 = ((uint16_t)(VERA_REG_SPRITE_ATTR_M) << 8);
                w.u16 += (i << 3); // sprite attr is 8 bytes per sprite
                w.u16 += 6; // we turn sprite off by just setting z to 0, which is in byte 6
                vera->ADDRx_M = w.u8_h;
                vera->ADDRx_L = w.u8_l;
                vera->DATA0 = 0;
                // mark as unused
                sprite_dibs[i] = 0;
            }
        }

    }
    sprite_str_slot[slot - 1] = j; // mark slot as used
    return slot;
}
void FreeSpriteStr(uint8_t str_slot) {
    _uConv16 w;
    uint8_t i;
    sprite_str_slot[str_slot] = 0; // mark slot as not used

    vera->CTRL = 0x00; // using DATA0
    vera->ADDRx_H = 0x01; // sprite attr table is on page 1, we don't need addr_inc
    for (i = 0; i < TEXT_MAX_SPRITES; i++) {
        if (sprite_dibs[i] == str_slot) {
            // sprite is claimed by this slot
            // delete sprite
            w.u16 = ((uint16_t)(VERA_REG_SPRITE_ATTR_M) << 8);
            w.u16 += (i << 3); // sprite attr is 8 bytes per sprite
            w.u16 += 6; // we turn sprite off by just setting z to 0, which is in byte 6
            vera->ADDRx_M = w.u8_h;
            vera->ADDRx_L = w.u8_l;
            vera->DATA0 = 0;
            // mark sprite as not used by str
            sprite_dibs[i] = 0;
        }
    }
}


//  ---- Conversions to str

void StrUint8Dec(uint8_t u, char* c) {
    uint8_t b = 0, a = u;
    if (a > 199) {
        a -= 200;
        c[0] = '2';
    } else if (a > 99) {
        a -= 100;
        c[0] = '1';
    } else { c[0] = ' '; }

    while (a > 9) {
        a -= 10;
        b++;
    }
    c[1] = '0' + b;
    c[2] = '0' + a;
}
void StrUint8Hex(uint8_t u, char* c) {
    uint8_t a = u >> 4;
    uint8_t o = (a > 9) ? 0x37 : '0';
    c[0] = o + a;
    a = u & 0x0F;
    o = (a > 9) ? 0x37 : '0';
    c[1] = o + a;
}
void StrUint16Hex(uint16_t u, char* c) {
    uint16_t m = 0xF000;
    uint16_t a;
    uint8_t o, i, s = 12;
    for (i = 0; i < 4; i++) {
        a = u & m;
        a >>= s;
        o = (a > 9) ? 0x37 : '0';
        c[i] = o + a;
        s -= 4;
        m >>= 4;
    }
}