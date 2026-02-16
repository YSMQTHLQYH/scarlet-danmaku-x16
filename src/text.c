#include "text.h"
#include "x16.h"
#include "memory_map.h"
#include "bitmap_layer.h"
#include "sprite_manager.h"



typedef union {
    struct { uint8_t u8_l; uint8_t u8_h; };
    uint16_t u16;
} _uConv16;

// from 0x1F000 to 0x1F7FF
//#define MEM_VRAM_1_KERNAL_FONT_ADDR_M   0xF0

uint8_t HijackRomCharset(uint8_t charset, uint8_t font_bpp, uint8_t color) {
    uint16_t i;
    uint8_t row, px, c, d_in, d_out;
    uint8_t buf[8];
    if (charset == 0) { return 1; }
    if (font_bpp != 2 && font_bpp != 4) { return 1; }
    KernalScreenSetCharset(charset);

    // DATA1 for reading KERNAL font
    VERA_CTRL = 1;
    VERA_ADDRx_H = 0x11; // inc = 1, page = 1
    VERA_ADDRx_M = MEM_VRAM_1_KERNAL_FONT_ADDR_M;
    VERA_ADDRx_L = 0;

    // DATA0 for writting our font
    VERA_CTRL = 0;
    VERA_ADDRx_H = 0x10; // inc = 1, page = 0
    VERA_ADDRx_M = (font_bpp == 4) ? MEM_4BPP_FONT_1_ADDR_M : MEM_2BPP_FONT_1_ADDR_M;
    VERA_ADDRx_L = 0;

    c = (font_bpp == 4) ? (color & 0x0F) : (color & 0x03);

    for (i = 0; i < 0x100; i++) {
        // ---- copy one character
        for (row = 0; row < 8; row++) {
            // ---- read one row of pixels

            // -- read it from KERNAL charset
            d_in = VERA_DATA1;
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
                    VERA_DATA0 = d_out;
                }
            } else { // font_bpp == 2
                for (px = 0; px < 8; px += 4) {
                    d_out = (buf[px] << 6);
                    d_out += (buf[px + 1] << 4);
                    d_out += (buf[px + 2] << 2);
                    d_out += buf[px + 3];
                    VERA_DATA0 = d_out;
                }
            }
            // copied one row of pixels
        }
        // copied one character
    }
    return 0;
}



//  ---- sprite text

uint8_t CreateSpriteStr(uint8_t spr_priority, uint8_t lenght, uint8_t z_flip, uint8_t palette) {
    uint8_t index = CreateSpriteObject(spr_priority, lenght, lenght, 0);
    if (index == 0xFF) { return 0xFF; }
    SpriteObjectSetZFlip(index, z_flip);
    SpriteObjectSetSizePalette(index, 0, palette);
    return index;
}
void FreeSpriteStr(uint8_t spr_obj) {
    FreeSpriteObject(spr_obj);
}
void PrintSpriteStr(uint8_t spr_obj, char* str) {
    uint8_t c;
    if (spr_obj >= MAX_SPRITE_OBJECTS) { return; }
    c = sprite_object[spr_obj].count;
    if (c == 0) { return; }
    SpriteObjectSetAddr(spr_obj, 0x00 | (MEM_4BPP_FONT_1_ADDR_M >> 5), (uint8_t*)str);
}


//  ----  Bitmap text
#define TEXT_BITMAP_MAX_LENGHT  32

/*
    uses VeraFX to copy from VRAM multiple bytes at once

    each 2bpp 8x8 char in the font is 16 bytes total, pixels are stored contiguously left to right, top to bottom
    such that B0-B15 is byte number and every '-' is a pixel
    |B0 ----|B1 ----|
    |B2 ----|B3 ----|
    |B4 ----|B5 ----|
    |B6 ----|B7 ----|
    |B8 ----|B9 ----|
    |B10----|B11----|
    |B12----|B13----|
    |B14----|B15----|

    each line of bitmap is 320 pixels, at 2bpp that means 80 bytes per line
    while there isn't an easy way to write them in order, setting ADDRx_INC to 80 means we can easly write
    0->2->4->6->8->10->12->14 in a row without touching ADDRx ourselves, (same applies for the odd numbered bytes)

    we just have to read the bytes from the font while skipping every other byte,
    which we can easily do with ADDRx_INC = 2

    we just have to read from one ADDR and immediately write it into the other ADDR
    repeat that 8 times, move the addresses a bit and then do *that* again 8 times
*/
void Print2BppBitmapStr(char* str, uint8_t buffer_n, uint8_t x, uint8_t y) {
    _uConv16 font_addr, pixel_addr;
    uint8_t i;
    char next_char;


    // ---- ADDRx_H and ADDRx_INC doesn't change during entire function so we can set them at the start once
    // -- VERA_DATA0 for reading the characters
    // selects ADDR0
    VERA_CTRL = 0;
    VERA_ADDRx_H = ADDR_INC_2 | MEM_FONT_VRAM_PAGE;
    // -- VERA_DATA1 for writting the characters into bitmap
    VERA_CTRL = 1;
    VERA_ADDRx_H = ADDR_INC_80 | buffer_n;

    for (i = 0; i < TEXT_BITMAP_MAX_LENGHT; i++) {
        next_char = str[i];
        if (next_char == 0)break; // if this happens we have reached the end of the string
        //  ----  Blit one character

        // get address of char in font
        font_addr.u16 = (next_char << 4);
        font_addr.u8_h += MEM_2BPP_FONT_1_ADDR_M;
        // get address of target starting pixel
        pixel_addr.u16 = lookup_bitmap_y[y];
        pixel_addr.u16 += (x >> 1) + i + i;

        // setup for even-byte pass
        VERA_CTRL = 0;
        VERA_ADDRx_M = font_addr.u8_h;
        VERA_ADDRx_L = font_addr.u8_l;
        VERA_CTRL = 1;
        VERA_ADDRx_M = pixel_addr.u8_h;
        VERA_ADDRx_L = pixel_addr.u8_l;

        // setup carry flag as boolean variable
        // set = even pass, clear = odd pass
        asm("sec");
    copy_paste_x8:
        asm("lda $9F23"); // A = VERA_DATA0
        asm("nop");
        asm("sta $9F24"); // VERA_DATA1 = A
        asm("lda $9F23");
        asm("nop");
        asm("sta $9F24");
        //2
        asm("lda $9F23");
        asm("nop");
        asm("sta $9F24");
        asm("lda $9F23");
        asm("nop");
        asm("sta $9F24");
        //4
        asm("lda $9F23");
        asm("nop");
        asm("sta $9F24");
        asm("lda $9F23");
        asm("nop");
        asm("sta $9F24");
        //6
        asm("lda $9F23");
        asm("nop");
        asm("sta $9F24");
        asm("lda $9F23");
        asm("nop");
        asm("sta $9F24");

        //8
        asm("bcs %g", odd_pass); // if(odd_pass)continue;
        continue;
    odd_pass:
        // setup for odd-byte pass
        font_addr.u16++;
        pixel_addr.u16++;
        VERA_CTRL = 0;
        VERA_ADDRx_M = font_addr.u8_h;
        VERA_ADDRx_L = font_addr.u8_l;
        VERA_CTRL = 1;
        VERA_ADDRx_M = pixel_addr.u8_h;
        VERA_ADDRx_L = pixel_addr.u8_l;
        asm("clc"); //odd_pass = true
        goto copy_paste_x8;

    }


}

/*
    uses VeraFX to copy from VRAM multiple bytes at once
    same as 2bpp, copy pasted explaination just in case

    each 4bpp 8x8 char in the font is 32 bytes total, pixels are stored contiguously left to right, top to bottom
    such that B0-B31 is byte number and every '-' is a pixel
    |B0 --|B1 --|B2 --|B3 --|
    |B4 --|B5 --|B6 --|B7 --|
    |B8 --|B9 --|B10--|B11--|
    |B12--|B13--|B14--|B15--|
    |B16--|B17--|B18--|B19--|
    |B20--|B21--|B22--|B23--|
    |B24--|B25--|B26--|B27--|
    |B28--|B29--|B30--|B31--|

    each line of bitmap is 320 pixels, at 4bpp that means 160 bytes per line
    while there isn't an easy way to write them in order, setting ADDRx_INC to 160 means we can easly write
    0->4->8->12->16->20->24->28 in a row without touching ADDRx ourselves, (same applies for the other 3 columns of bytes)

    we just have to read the bytes from the font while skipping every three bytes between each read,
    which we can easily do with ADDRx_INC = 4

    we just have to read from one ADDR and immediately write it into the other ADDR
    repeat that 8 times, move the addresses a bit and then do *that* again 8 times, and again and again
*/
void Print4BppBitmapStr(char* str, uint8_t buffer_n, uint8_t x, uint8_t y) {
    _uConv16 font_addr, pixel_addr;
    uint8_t i;
    char next_char;

    // ---- ADDRx_H and ADDRx_INC doesn't change during entire function so we can set them at the start once
    // -- VERA_DATA0 for reading the characters
    // selects ADDR0
    VERA_CTRL = 0;
    VERA_ADDRx_H = ADDR_INC_4 | MEM_FONT_VRAM_PAGE;
    // -- VERA_DATA1 for writting the characters into bitmap
    VERA_CTRL = 1;
    VERA_ADDRx_H = ADDR_INC_160 | buffer_n;

    for (i = 0; i < TEXT_BITMAP_MAX_LENGHT; i++) {
        next_char = str[i];
        if (next_char == 0)break; // if this happens we have reached the end of the string
        //  ----  Blit one character

        // get address of char in font
        font_addr.u16 = (next_char << 5);
        font_addr.u8_h += MEM_4BPP_FONT_1_ADDR_M;
        // get address of target starting pixel
        pixel_addr.u16 = lookup_bitmap_y[y];
        pixel_addr.u16 += x + (i << 2);

        // setup for even-byte pass
        VERA_CTRL = 0;
        VERA_ADDRx_M = font_addr.u8_h;
        VERA_ADDRx_L = font_addr.u8_l;
        VERA_CTRL = 1;
        VERA_ADDRx_M = pixel_addr.u8_h;
        VERA_ADDRx_L = pixel_addr.u8_l;

        // setup carry flag as boolean variable
        // set = pass 1-3 , clear = pass4
        asm("sec");
        asm("ldy #3");
    copy_paste_x8:
        asm("phy");
        asm("lda $9F23"); // A = VERA_DATA0
        asm("nop");
        asm("sta $9F24"); // VERA_DATA1 = A
        asm("lda $9F23");
        asm("nop");
        asm("sta $9F24");
        //2
        asm("lda $9F23");
        asm("nop");
        asm("sta $9F24");
        asm("lda $9F23");
        asm("nop");
        asm("sta $9F24");
        //4
        asm("lda $9F23");
        asm("nop");
        asm("sta $9F24");
        asm("lda $9F23");
        asm("nop");
        asm("sta $9F24");
        //6
        asm("lda $9F23");
        asm("nop");
        asm("sta $9F24");
        asm("lda $9F23");
        asm("nop");
        asm("sta $9F24");

        //8
        asm("bcs %g", next_pass); // if(last_pass)continue;
        asm("ply");
        continue;
    next_pass:
        // setup for odd-byte pass
        font_addr.u16++;
        pixel_addr.u16++;
        VERA_CTRL = 0;
        VERA_ADDRx_M = font_addr.u8_h;
        VERA_ADDRx_L = font_addr.u8_l;
        VERA_CTRL = 1;
        VERA_ADDRx_M = pixel_addr.u8_h;
        VERA_ADDRx_L = pixel_addr.u8_l;
        asm("ply");
        asm("dey"); //pass--
        asm("sec");
        asm("bne %g", copy_paste_x8); // branch if pass != 0
        asm("clc"); //last_pass = true
        goto copy_paste_x8;

    }
}


//  ----  Conversions to str

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