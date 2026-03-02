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

void TestBulletBlit(uint8_t buffer_n) {
    x16_ram_bank = 3;
    VERA_CTRL = DCSEL_2;
    VERA_DC2_FX_CTRL = 0x80; //transparent writes, 4-bit mode
    VERA_ADDRx_H = buffer_n + ADDR_INC_160;

    zpa0 = 0;
    do {
        // check if bullet exists
        asm("ldy %v", zpa0);
        asm("lda $A400, y");
        asm("beq %g", skip);
        //it does, draw



        asm("lda $A300, y");
        asm("sta %v", zpa1);
        zpc0.w = LookupY(zpa1);
        asm("ldy %v", zpa0);
        asm("lda $A100, y");
        asm("lsr"); // bit 0 moves into C
        asm("sta %v", zpa1);
        asm("bcc %g", even);
        //odd:
        zpc1.w = 0x0600;
        zpc2.w = 0xEE66;
        zpc3.w = 0xE660;
        asm("bra %g", end_odd_even);
    even:
        zpc1.w = 0x6E06;
        zpc2.w = 0xEE66;
        zpc3.w = 0x6000;
    end_odd_even:
        zpc0.w += zpa1;
        VERA_ADDRx_L = zpc0.l;

        asm("lda %v +1", zpc0);
        asm("sta $9F21");

        asm("ldx %v", zpc1);
        asm("ldy %v + 1", zpc1);
        asm("stx $9F23");
        asm("sty $9F23");
        asm("sty $9F23");
        asm("sty $9F23");
        asm("stx $9F23");

        //collumn 2
        asm("inc %v", zpc0);
        asm("bne %g", addr_l2);
        asm("inc a");
    addr_l2:
        asm("sta $9F21");
        asm("ldx %v", zpc0);
        asm("stx $9F20");

        asm("ldx %v", zpc2);
        asm("ldy %v + 1", zpc2);
        asm("stx $9F23");
        asm("sty $9F23");
        asm("sty $9F23");
        asm("sty $9F23");
        asm("stx $9F23");



        //collumn 3
        asm("inc %v", zpc0);
        asm("bne %g", addr_l3);
        asm("inc a");
    addr_l3:
        asm("sta $9F21");
        asm("lda %v", zpc0);
        asm("sta $9F20");

        asm("ldx %v", zpc3);
        asm("ldy %v + 1", zpc3);
        asm("stx $9F23");
        asm("sty $9F23");
        asm("sty $9F23");
        asm("sty $9F23");
        asm("stx $9F23");


    skip:
        zpa0 += 2;
    } while (zpa0 != 0);

    VERA_CTRL = 0;
}

void BulletTick() {
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
    asm("cmp #$DC"); //C = 1 if A >= (0xE0 - 0x04)
    asm("bcs %g", off_x);
    asm("cmp #$04"); //C = 1 if A >= 0x04
    asm("bcs %g", end_x);
off_x:
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
    asm("cmp #$EC"); //C = 1 if A >= (0xF0 - 0x04)
    asm("bcs %g", off_y);
    asm("cmp #$04"); //C = 1 if A >= 0xF0
    asm("bcs %g", end_y);
off_y:
    asm("lda #0");
    asm("sta $A400, y");
end_y:

skip:
    asm("iny");
    asm("iny");
    asm("bne %g", loop);
}
