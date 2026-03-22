#include "bullet.h"

#include "x16.h"
#include "zp_utils.h"
#include "bitmap_layer.h"


/*
position/velocity format in VRAM
py_l, vy_l, py_h, vy_h, px_l, vx_l, px_h, vx_h
py_h of 0xFF to mark bullet as not existing/deleted
arranged 160 bytes appart, as they are in the "bitmap" area of vram, vertically behind hud
*/
/*
lookup table format
256 possible angles, index in array is angle, different array for each speed
array order is x_low, x_high, y_low, y_high
1KB in total per speed, 8 speeds per ram bank
*/

uint8_t SpawnBulletBlock(_sBulletSpawnCfg* cfg) {
#define ANGLE   zpa6
#define SPEED   zpa5
#define X       zpc3.l
#define Y       zpc3.h
#define TABLE_PTR   zptr3
    register uint8_t i, subblock_i = 0;
    uint8_t index;
    uint8_t sub_angle, sub_speed, sub_x, sub_y;

    // find empty block
    for (index = 0; index < BULLET_BLOCK_COUNT; index++) {
        if (bullet_block[index].remaining_bullets == 0) goto found_block;
    }
    // no empty bullet block
    return 0xFF;

found_block:
    bullet_block[index].remaining_bullets = cfg->count;
    bullet_block[index].graphic_type = cfg->graphic_type;
    bullet_block[index].color = cfg->color;

    VERA_CTRL = 0;
    VERA_ADDRx_H = ADDR_INC_160;
    VERA_ADDRx_M = 0;
    VERA_ADDRx_L = BITMAP_GAME_AREA_WIDTH_BYTES + index;

    x16_ram_bank = MEM_BANK_BULLET_LOOKUP_0 + 1;

    ANGLE = cfg->angle_start;
    SPEED = cfg->speed_start;
    X = cfg->x_start;
    Y = cfg->y_start;
    if (cfg->count_per_subblock != 0) {
        sub_angle = ANGLE;
        sub_speed = SPEED;
        sub_x = X;
        sub_y = Y;
        subblock_i = cfg->count_per_subblock;
    }

    for (i = 0; i < BULLETS_PER_BLOCK; i++) {
        if (i >= cfg->count) {
            // not actually a bullet, fill it with zeros
            asm("lda #$FF");
            //y
            asm("stz %v", VERA_DATA0);
            asm("stz %v", VERA_DATA0);
            asm("sta %v", VERA_DATA0); //py_h == 0xFF
            asm("stz %v", VERA_DATA0);
            //x
            asm("stz %v", VERA_DATA0);
            asm("stz %v", VERA_DATA0);
            asm("stz %v", VERA_DATA0);
            asm("stz %v", VERA_DATA0);
        } else {
            //TODO: make this properly

            // angle (low byte of table addr)
            asm("lda %v", ANGLE);
            asm("sta %v", TABLE_PTR);

            // speed (high byte of addr, starts at A0 and increments by 4)
            asm("lda %v", SPEED);
            // check for table bank here probably
            asm("and #$07"); // speed is in range 0-7 per table
            asm("asl");
            asm("asl"); // mult by 4 (increment)
            asm("clc");
            asm("adc #$A2"); //starts with y_l (byte 2)
            asm("sta %v + 1", TABLE_PTR);

            VERA_DATA0 = 0; //py_l
            VERA_DATA0 = *TABLE_PTR; //vy_l
            VERA_DATA0 = Y; //py_h
            // oh come on VScode, what even is the "error" now?
            TABLE_PTR |= 0x0300; // modifying pointer directly, setting bits of high byte to 0b11
            VERA_DATA0 = *TABLE_PTR; //vy_h

            VERA_DATA0 = 0; //px_l
            TABLE_PTR &= 0xFCFF; // clearing bottom two bits of high byte
            VERA_DATA0 = *TABLE_PTR; //vx_l
            VERA_DATA0 = X; //px_h
            TABLE_PTR |= 0x0100;
            VERA_DATA0 = *TABLE_PTR; //vx_h


            // set up next
            if (--subblock_i == 0) { //note that subblock_i is initialized to 0, if we aren't doing subblocks this will evaluate to 0xFF 
                sub_angle += cfg->angle_offset_subblock;
                ANGLE = sub_angle;
                sub_speed += cfg->speed_offset_subblock;
                SPEED = sub_speed;
                sub_x += cfg->x_offset_subblock;
                X = sub_x;
                sub_y += cfg->y_offset_subblock;
                Y = sub_y;
                subblock_i = cfg->count_per_subblock;
            } else {
                // no subblock change
                ANGLE += cfg->angle_offset;
                SPEED += cfg->speed_offset;
                X += cfg->x_offset;
                Y += cfg->y_offset;
            }

        }
    }
#undef TABLE_PTR
#undef Y
#undef X
#undef SPEED
#undef ANGLE

    return index;
}



// test bullet draw
static uint8_t calculate_cache_mask(uint8_t m);
static uint8_t cache_mask[6][4] = { 0 };
static uint8_t cache_mask_readable[6][4] = {
    // first 4 only need one write
    {0x8F, 0xC7, 0xE3, 0xF1},
    {0x07, 0x83, 0xC1, 0xE0},
    // first pass of second 4 (cut in half)
    {0xF8, 0xFC, 0xFE, 0xFF},
    {0xF0, 0xF8, 0xFC, 0xFE},
    // second pass of second 4 (the other half)
    {0xFF, 0x7F, 0x3F, 0x1F},
    {0x7F, 0x3F, 0x1F, 0x0F},
};

// returns 0 on success, 1 on failure
uint8_t BulletManagerInit() {
    _sFileLoadCtx fl;
    uint8_t err;
    uint8_t i, j;
    x16_ram_bank = MEM_BANK_BULLET_LOOKUP_0;
    fl.filename = "test assets/bullet lookup.bin";
    fl.name_lenght = 12 + 17;
    fl.header_mode = FILE_LOAD_HEADERLESS;
    fl.target_mode = FILE_LOAD_RAM;
    fl.dest_addr = (void*)HIGH_RAM_START;

    err = LoadFile(&fl);
    if (err) {
        return 1;
    }

    //bullet blocks
    for (i = 0; i < BULLET_BLOCK_COUNT; i++) {
        bullet_block[i].color = 0;
        bullet_block[i].graphic_type = BULLET_GRAPHIC_DUMMY;
        bullet_block[i].remaining_bullets = 0;
    }

    //mask for cache writes
    for (j = 0; j < 6; j++) {
        for (i = 0; i < 4; i++) {
            cache_mask[j][i] = calculate_cache_mask(cache_mask_readable[j][i]);
        }
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
// turns mask_human_readable into mask for vera fx cache writes
// bit 7 for leftmost pixel, bit 6 for secondleftmost pixel, etc
// 0 is write, 1 is mask away
static uint8_t calculate_cache_mask(uint8_t m) {
    uint8_t o = 0;
    if (m & 0x80) o |= 0x02;
    if (m & 0x40) o |= 0x01;
    if (m & 0x20) o |= 0x08;
    if (m & 0x10) o |= 0x04;
    if (m & 0x08) o |= 0x20;
    if (m & 0x04) o |= 0x10;
    if (m & 0x02) o |= 0x80;
    if (m & 0x01) o |= 0x40;

    return o;
}



/*
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
*/

void TestBulletBlit(uint8_t buffer_n) {
    x16_ram_bank = 3;
    VERA_CTRL = DCSEL_6;
    VERA_DC6_FX_CACHE_L = 0xE6;
    VERA_DC6_FX_CACHE_M = 0xA2;
    VERA_DC6_FX_CACHE_H = 0x49;
    VERA_DC6_FX_CACHE_U = 0xB3;

    VERA_CTRL = DCSEL_2;
    VERA_DC2_FX_CTRL = 0x50;
    VERA_DC2_FX_MULT = 0x00;
    VERA_ADDRx_H = buffer_n + ADDR_INC_160;


    zpa0 = 0;
    do {
        // check if bullet exists
        asm("ldy %v", zpa0);
        asm("lda $A400, y");
        asm("beq %g", end);
        //it does, draw

        //color
        asm("lda %v", zpa0);
        asm("asl");
        asm("and #$0C");
        asm("sta %v", VERA_DC2_FX_MULT);

        asm("ldy %v", zpa0);
        asm("lda $A300, y");
        asm("sta %v", zpa1);
        zpc0.w = LookupY(zpa1);
        asm("ldy %v", zpa0);
        asm("lda $A100, y");
        asm("pha");
        asm("lsr"); // bit 0 moves into C
        asm("sta %v", zpa1);
        zpc0.w += zpa1;
        VERA_ADDRx_L = zpc0.l;
        VERA_ADDRx_M = zpc0.h;

        asm("pla");
        asm("and #$07");
        asm("cmp #$04");
        asm("bcs %g", two_pass); //branches if A >= 4
        asm("and #$03");

        asm("sta %v", zpa3);
        zpa1 = cache_mask[0][zpa3];
        zpa2 = cache_mask[1][zpa3];
        asm("ldx %v", zpa1);
        asm("ldy %v", zpa2);

        asm("stx %v", VERA_DATA0);
        asm("sty %v", VERA_DATA0);
        asm("sty %v", VERA_DATA0);
        asm("sty %v", VERA_DATA0);
        asm("stx %v", VERA_DATA0);
        asm("bra %g", end);

    two_pass:
        asm("and #$03");
        asm("sta %v", zpa3);
        zpa1 = cache_mask[2][zpa3];
        zpa2 = cache_mask[3][zpa3];
        asm("ldx %v", zpa1);
        asm("ldy %v", zpa2);
        asm("stx %v", VERA_DATA0);
        asm("sty %v", VERA_DATA0);
        asm("sty %v", VERA_DATA0);
        asm("sty %v", VERA_DATA0);
        asm("stx %v", VERA_DATA0);
        zpc0.w += 4;
        VERA_ADDRx_L = zpc0.l;
        VERA_ADDRx_M = zpc0.h;
        zpa1 = cache_mask[4][zpa3];
        zpa2 = cache_mask[5][zpa3];
        asm("ldx %v", zpa1);
        asm("ldy %v", zpa2);
        asm("stx %v", VERA_DATA0);
        asm("sty %v", VERA_DATA0);
        asm("sty %v", VERA_DATA0);
        asm("sty %v", VERA_DATA0);
        asm("stx %v", VERA_DATA0);

    end:
        zpa0 += 2;
    } while (zpa0 != 0);

    VERA_CTRL = 0;
}

static void TestBulletTick() {
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



void BulletTick() {
    uint8_t i;
    //TestBulletTick();
    for (i = 0; i < BULLET_BLOCK_COUNT; i++) {
        if (bullet_block[i].remaining_bullets > 0) {
            BulletBlockTick(i);
        }
    }
}