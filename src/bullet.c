#include "bullet.h"

#include "x16.h"
#include "zp_utils.h"
#include "bitmap_layer.h"


/*
lookup table format
256 possible angles, index in array is angle, different array for each speed
array order is x_low, x_high, y_low, y_high
1KB in total per speed, 8 speeds per ram bank
*/

// returns 0 on success, 1 on failure
uint8_t BulletManagerInit() {
    _sFileLoadCtx fl;
    uint8_t ok;
    x16_ram_bank = MEM_BANK_BULLET_LOOKUP_0;
    fl.filename = "test assets/bullet lookup.bin";
    fl.name_lenght = 12 + 17;
    fl.header_mode = FILE_LOAD_HEADERLESS;
    fl.target_mode = FILE_LOAD_RAM;
    fl.dest_addr = (void*)HIGH_RAM_START;

    ok = load_file(&fl);
    if (!ok) {
        return 1;
    }

    x16_ram_bank = 3;
    asm("ldy #0");
init_loop:
    asm("lda #96");
    asm("sta $A100, y");
    asm("sta $A300, y");
    asm("lda #0");
    asm("sta $A000, y");
    asm("sta $A200, y");
    asm("lda #1");
    asm("sta $A400, y");

    asm("iny");
    asm("bne %g", init_loop);

    return 0;
}


void BulletTick() {
    /*
        uint8_t i;
        for (i = 0; i < 16; i++) {
        BitmapSetPixel(bitmap_back_buffer, i + (i << 4), 50 + i, 100);
        BitmapSetPixel(bitmap_back_buffer, i + (i << 4), 55 + i, 101);
        BitmapSetPixel(bitmap_back_buffer, i + (i << 4), 60 + i, 102);
        BitmapSetPixel(bitmap_back_buffer, i + (i << 4), 65 + i, 103);
    }
    */


    asm("ldy #0");

    asm("ldx #3");
    asm("stx 0");
loop:
    //check if bullet exists
    asm("lda $A400, y");
    asm("beq %g", skip); //branches if Z == 1 (value loaded into A = 0)

    asm("ldx #5");
    asm("stx 0");

    // read speed lookup
    asm("lda $B300, y");
    asm("sta %v + 1", zpc1);
    asm("lda $B200, y");
    asm("sta %v", zpc1);
    asm("lda $B100, y");
    asm("sta %v + 1", zpc0);
    asm("lda $B000, y");
    asm("sta %v", zpc0);

    asm("ldx #3");
    asm("stx 0");

    // add position x
    asm("lda $A000, y");
    asm("clc");
    asm("adc %v", zpc0);
    asm("sta $A000, y");
    asm("lda $A100, y");
    asm("adc %v + 1", zpc0);
    asm("sta $A100, y");
    // check for bullet offscreen
    asm("cmp #$E0"); //C = 1 if A >= 0xE0
    asm("bcc %g", end_x);
    asm("lda #0");
    asm("sta $A400, y");
end_x:

    // add position y
    asm("lda $A200, y");
    asm("clc");
    asm("adc %v", zpc1);
    asm("sta $A200, y");
    asm("lda $A300, y");
    asm("adc %v + 1", zpc1);
    asm("sta $A300, y");
    // check for bullet offscreen
    asm("cmp #$F0"); //C = 1 if A >= 0xF0
    asm("bcc %g", end_y);
    asm("lda #0");
    asm("sta $A400, y");
end_y:

skip:
    asm("iny");
    asm("iny");
    asm("iny");
    asm("iny");
    asm("bne %g", loop);
}
