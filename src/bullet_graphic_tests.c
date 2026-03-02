#include "bullet_graphic_tests.h"
#include "x16.h"
#include "zp_utils.h"
#include "bitmap_layer.h"

uint8_t calculate_cache_mask(uint8_t m);

#define TEST_SIZE   128
uint8_t* test_arr_x = (uint8_t*)0xB000;
uint8_t* test_arr_y = (uint8_t*)0xB800;
#define TEST_ADDR_M (uint8_t)0x96

uint8_t stamp[4] = { 0x06, 0xEE, 0xE6, 0x00 };
uint8_t cache_mask[6][4] = { 0 };
uint8_t cache_mask_readable[6][4] = {
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
uint8_t poly_cache_mask[4][8] = { 0 };
uint8_t poly_cache_mask_readable[4][8] = {
    // first column
    {0x8F, 0xC7, 0xE3, 0xF1, 0xF8, 0xFC, 0xFE, 0xFF},
    {0x07, 0x83, 0xC1, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE},

    // second column
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x3F, 0x1F},
    {0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x3F, 0x1F, 0x0F},
};


void GfxTestsinit() {
    uint8_t i = 0, j = 0;
    x16_ram_bank = 3;

    for (j = 0; j < 16; j++) {
        for (i = 0; i < (TEST_SIZE >> 4); i++) {
            test_arr_x[(j * (TEST_SIZE >> 4)) + i] = (i << 4) + i + i + i + 20 + (j & 15);
            test_arr_y[(j * (TEST_SIZE >> 4)) + i] = (j << 3) + i + 20;
        }
    }


    //bullet sprite for copy pasting
    VERA_CTRL = 0;
    VERA_ADDRx_H = 1 + ADDR_INC_1;
    VERA_ADDRx_M = 0xF8;
    VERA_ADDRx_L = 0;
    for (j = 0; j < 5; j++) {
        for (i = 0; i < 4; i++) {
            VERA_DATA0 = stamp[i];
        }
    }

    //mask for cache writes
    for (j = 0; j < 6; j++) {
        for (i = 0; i < 4; i++) {
            cache_mask[j][i] = calculate_cache_mask(cache_mask_readable[j][i]);
        }
    }
    for (j = 0; j < 4; j++) {
        for (i = 0; i < 8; i++) {
            poly_cache_mask[j][i] = calculate_cache_mask(poly_cache_mask_readable[j][i]);
        }
    }

}
// turns mask_human_readable into mask for vera fx cache writes
// bit 7 for leftmost pixel, bit 6 for secondleftmost pixel, etc
// 0 is write, 1 is mask away
uint8_t calculate_cache_mask(uint8_t m) {
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

static void test_2x1();
static void test_5x5();
static void test_copy_paste_vram();
static void test_copy_paste_zp();
static void test_5x5_hop();
static void test_cache_1_color();
static void test_cache_1_color_dec();
static void test_polygon_hack();
static void test_polygon_hack_nibble();
static void test_polygon_hack_cache();

void GfxhTest(_eBulletGfxTest t) {
    uint8_t i = 0;
    x16_ram_bank = 3;

    switch (t) {
    case GFX_TEST_NOTHING:
        // does nothing, just to measure baseline lag of everything else
        break;

    case GFX_TEST_2x1:
        test_2x1();
        break;

    case GFX_TEST_5x5:
        test_5x5();
        break;

    case GFX_TEST_COPY_PASTE_VRAM:
        test_copy_paste_vram();
        break;

    case GFX_TEST_COPY_PASTE_ZP:
        test_copy_paste_zp();
        break;

    case GFX_TEST_5x5_HOP:
        test_5x5_hop();
        break;

    case GFX_TEST_CACHE_1_COLOR:
        test_cache_1_color();
        break;

    case GFX_TEST_CACHE_1_COLOR_DEC:
        test_cache_1_color_dec();
        break;

    case GFX_TEST_POLYGON_HACK:
        test_polygon_hack();
        break;

    case GFX_TEST_POLYGON_HACK_NIBBLE:
        test_polygon_hack_nibble();
        break;

    case GFX_TEST_POLYGON_HACK_CACHE:
        test_polygon_hack_cache();
        break;


    default:
        break;
    }
}

static void test_2x1() {
    VERA_ADDRx_H = bitmap_back_buffer + ADDR_INC_160;
    zpa0 = 0;
    do {
        asm("ldy %v", zpa0);
        asm("lda $B800, y");
        asm("sta %v", zpa1);
        zpc0.w = LookupY(zpa1);
        asm("ldy %v", zpa0);
        asm("lda $B000, y");
        asm("lsr"); // bit 0 moves into C
        asm("sta %v", zpa1);
        zpc0.w += zpa1;
        VERA_ADDRx_L = zpc0.l;
        VERA_ADDRx_M = zpc0.h;

        asm("ldx #$EE");
        asm("stx %v", VERA_DATA0);

        zpa0++;
    } while (zpa0 < TEST_SIZE);
}

static void test_5x5() {
    VERA_CTRL = DCSEL_2;
    VERA_DC2_FX_CTRL = 0x80; //transparent writes
    VERA_ADDRx_H = bitmap_back_buffer + ADDR_INC_160;

    zpa0 = 0;
    do {

        asm("ldy %v", zpa0);
        asm("lda $B800, y");
        asm("sta %v", zpa1);
        zpc0.w = LookupY(zpa1);
        asm("ldy %v", zpa0);
        asm("lda $B000, y");
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


        zpa0++;
    } while (zpa0 < TEST_SIZE);
    VERA_CTRL = 0;
}

static void test_copy_paste_vram() {
    VERA_CTRL = 0;
    VERA_ADDRx_H = bitmap_back_buffer + ADDR_INC_1;

    VERA_CTRL = 1;
    VERA_ADDRx_H = ADDR_INC_1 + 1;

    zpa0 = 0;
    do {
        //graphic addr (data 1)
        VERA_CTRL = 1;
        VERA_ADDRx_L = 0;
        VERA_ADDRx_M = 0xF8;

        // pixel addr (data 0)
        VERA_CTRL = 0;
        asm("ldy %v", zpa0);
        asm("lda $B800, y");
        asm("sta %v", zpa1);
        zpc0.w = LookupY(zpa1);
        asm("ldy %v", zpa0);
        asm("lda $B000, y");
        asm("lsr"); // bit 0 moves into C
        asm("sta %v", zpa1);
        zpc0.w += zpa1;
        VERA_ADDRx_L = zpc0.l;
        VERA_ADDRx_M = zpc0.h;


        for (zpa1 = 0; zpa1 < 5; zpa1++) {
            VERA_DATA0 = VERA_DATA1;
            VERA_DATA0 = VERA_DATA1;
            VERA_DATA0 = VERA_DATA1;
            VERA_DATA0 = VERA_DATA1;
            zpc0.w += 160;
            VERA_ADDRx_L = zpc0.l;
            VERA_ADDRx_M = zpc0.h;
        }

        zpa0++;
    } while (zpa0 < TEST_SIZE);

    VERA_CTRL = 0;
}

static void test_copy_paste_zp() {
    VERA_CTRL = 0;
    VERA_ADDRx_H = bitmap_back_buffer + ADDR_INC_1;

    for (zpa1 = 0; zpa1 < 20; zpa1++) {
        asm("lda %v", zpa1);
        asm("tax");
        asm("and #$03");
        asm("tay");
        asm("lda %v, y", stamp);
        asm("sta 2, x");
    }

    zpa0 = 0;
    do {
        // pixel addr (data 0)
        asm("ldy %v", zpa0);
        asm("lda $B800, y");
        asm("sta %v", zpa1);
        zpc0.w = LookupY(zpa1);
        asm("ldy %v", zpa0);
        asm("lda $B000, y");
        asm("lsr"); // bit 0 moves into C
        asm("sta %v", zpa1);
        zpc0.w += zpa1;
        VERA_ADDRx_L = zpc0.l;
        VERA_ADDRx_M = zpc0.h;



        asm("lda 2");
        asm("sta %v", VERA_DATA0);
        asm("lda 3");
        asm("sta %v", VERA_DATA0);
        asm("lda 4");
        asm("sta %v", VERA_DATA0);
        asm("lda 5");
        asm("sta %v", VERA_DATA0);
        zpc0.w += 160;
        VERA_ADDRx_L = zpc0.l;
        VERA_ADDRx_M = zpc0.h;

        asm("lda 6");
        asm("sta %v", VERA_DATA0);
        asm("lda 7");
        asm("sta %v", VERA_DATA0);
        asm("lda 8");
        asm("sta %v", VERA_DATA0);
        asm("lda 9");
        asm("sta %v", VERA_DATA0);
        zpc0.w += 160;
        VERA_ADDRx_L = zpc0.l;
        VERA_ADDRx_M = zpc0.h;

        asm("lda 10");
        asm("sta %v", VERA_DATA0);
        asm("lda 11");
        asm("sta %v", VERA_DATA0);
        asm("lda 12");
        asm("sta %v", VERA_DATA0);
        asm("lda 13");
        asm("sta %v", VERA_DATA0);
        zpc0.w += 160;
        VERA_ADDRx_L = zpc0.l;
        VERA_ADDRx_M = zpc0.h;

        asm("lda 14");
        asm("sta %v", VERA_DATA0);
        asm("lda 15");
        asm("sta %v", VERA_DATA0);
        asm("lda 16");
        asm("sta %v", VERA_DATA0);
        asm("lda 17");
        asm("sta %v", VERA_DATA0);
        zpc0.w += 160;
        VERA_ADDRx_L = zpc0.l;
        VERA_ADDRx_M = zpc0.h;

        asm("lda 18");
        asm("sta %v", VERA_DATA0);
        asm("lda 19");
        asm("sta %v", VERA_DATA0);
        asm("lda 20");
        asm("sta %v", VERA_DATA0);
        asm("lda 21");
        asm("sta %v", VERA_DATA0);



        zpa0++;
    } while (zpa0 < TEST_SIZE);

    VERA_CTRL = 0;
}

static void test_5x5_hop() {
    VERA_CTRL = DCSEL_2 + 1;
    VERA_DC2_FX_CTRL = 0x08;
    VERA_ADDRx_H = bitmap_back_buffer + ADDR_INC_320;

    zpa0 = 0;
    do {
        asm("ldy %v", zpa0);
        asm("lda $B800, y");
        asm("sta %v", zpa1);
        zpc0.w = LookupY(zpa1);
        asm("ldy %v", zpa0);
        asm("lda $B000, y");
        asm("lsr"); // bit 0 moves into C
        asm("sta %v", zpa1);
        zpc0.w += zpa1;
        VERA_ADDRx_L = zpc0.l;
        VERA_ADDRx_M = zpc0.h;

        VERA_ADDRx_H = bitmap_back_buffer + ADDR_INC_320;
        asm("ldx #$2E");
        asm("ldy #$E2");
        asm("stx %v", VERA_DATA1);
        asm("sty %v", VERA_DATA1);
        asm("stx %v", VERA_DATA1);
        asm("sty %v", VERA_DATA1);
        asm("stx %v", VERA_DATA1);
        asm("sty %v", VERA_DATA1);

        zpc0.w += 160;
        VERA_ADDRx_L = zpc0.l;
        VERA_ADDRx_M = zpc0.h;
        asm("ldx #$6D");
        asm("ldy #$D6");
        asm("stx %v", VERA_DATA1);
        asm("sty %v", VERA_DATA1);
        asm("stx %v", VERA_DATA1);
        asm("sty %v", VERA_DATA1);

        zpc0.w -= 158;
        VERA_ADDRx_L = zpc0.l;
        VERA_ADDRx_M = zpc0.h;
        VERA_ADDRx_H = bitmap_back_buffer + ADDR_INC_160;
        asm("ldx #$90");
        asm("ldy #$81");
        asm("stx %v", VERA_DATA1);
        asm("sty %v", VERA_DATA1);
        asm("sty %v", VERA_DATA1);
        asm("sty %v", VERA_DATA1);
        asm("stx %v", VERA_DATA1);


        zpa0++;
    } while (zpa0 < TEST_SIZE);

    VERA_CTRL = 0;
}

static void test_cache_1_color() {
    VERA_CTRL = DCSEL_6;
    VERA_DC6_FX_CACHE_L = 0xE6;
    VERA_DC6_FX_CACHE_M = 0xA2;
    VERA_DC6_FX_CACHE_H = 0x47;
    VERA_DC6_FX_CACHE_U = 0xD3;

    VERA_CTRL = DCSEL_2;
    VERA_DC2_FX_CTRL = 0x50;
    VERA_DC2_FX_MULT = 0x00;
    VERA_ADDRx_H = bitmap_back_buffer + ADDR_INC_160;


    zpa0 = 0;
    do {
        asm("ldy %v", zpa0);
        asm("lda $B800, y");
        asm("sta %v", zpa1);
        zpc0.w = LookupY(zpa1);
        asm("ldy %v", zpa0);
        asm("lda $B000, y");
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
        zpa0++;
    } while (zpa0 < TEST_SIZE);

    VERA_CTRL = 0;
}

static void test_cache_1_color_dec() {
    VERA_CTRL = DCSEL_6;
    VERA_DC6_FX_CACHE_L = 0xE6;
    VERA_DC6_FX_CACHE_M = 0xA2;
    VERA_DC6_FX_CACHE_H = 0x47;
    VERA_DC6_FX_CACHE_U = 0xD3;

    VERA_CTRL = DCSEL_2;
    VERA_DC2_FX_CTRL = 0x50;
    VERA_DC2_FX_MULT = 0x00;
    //VERA_ADDRx_H = bitmap_back_buffer + ADDR_INC_160;

    zpa2 = bitmap_back_buffer + ADDR_INC_160;       // down
    zpa3 = bitmap_back_buffer + ADDR_INC_4;         // right
    zpa4 = bitmap_back_buffer + ADDR_INC_160 + 8;   // up
    zpa5 = bitmap_back_buffer + ADDR_INC_4 + 8;     // right

    zpa0 = 0;
    do {
        asm("ldy %v", zpa0);
        asm("lda $B800, y");
        asm("sta %v", zpa1);
        zpc0.w = LookupY(zpa1);
        asm("ldy %v", zpa0);
        asm("lda $B000, y");
        asm("pha");
        asm("lsr"); // bit 0 moves into C
        asm("sta %v", zpa1);
        zpc0.w += zpa1;
        VERA_ADDRx_L = zpc0.l;
        VERA_ADDRx_M = zpc0.h;


        asm("pla");
        asm("and #$07");
        asm("sta %v", zpa3);
        // left (l = top/bottom, h = middle)
        zpc1.l = poly_cache_mask[0][zpa3];
        zpc1.h = poly_cache_mask[1][zpa3];
        // right
        zpc2.l = poly_cache_mask[2][zpa3];
        zpc2.h = poly_cache_mask[3][zpa3];


        asm("lda %v", zpa2);
        asm("sta %v", VERA_ADDRx_H);
        asm("ldx %v", zpc1);
        asm("ldy %v + 1", zpc1);
        asm("stx %v", VERA_DATA0);
        asm("sty %v", VERA_DATA0);
        asm("sty %v", VERA_DATA0);
        asm("sty %v", VERA_DATA0);
        asm("stx %v", VERA_DATA0);

        asm("ldx %v", zpc2);
        asm("ldy %v + 1", zpc2);

        asm("lda %v", zpa3);
        asm("sta %v", VERA_ADDRx_H);
        asm("stx %v", VERA_DATA0);
        asm("lda %v", zpa4);
        asm("sta %v", VERA_ADDRx_H);
        asm("sty %v", VERA_DATA0);
        asm("sty %v", VERA_DATA0);
        asm("sty %v", VERA_DATA0);
        asm("stx %v", VERA_DATA0);




        zpa0++;
    } while (zpa0 < TEST_SIZE);

    VERA_CTRL = 0;
}

static void test_polygon_hack() {
    VERA_CTRL = DCSEL_2;
    VERA_ADDRx_H = bitmap_back_buffer + ADDR_INC_160;
    VERA_DC2_FX_CTRL = 0x82; //polygon
    VERA_CTRL = DCSEL_3 + 1;
    VERA_ADDRx_H = ADDR_INC_1;
    VERA_DC3_FX_X_INCR_L = 0;
    VERA_DC3_FX_X_INCR_H = 0;
    VERA_CTRL = DCSEL_4;
    VERA_DC4_FX_X_POS_H = 0;
    VERA_DC4_FX_X_POS_L = 0;

    zpa0 = 0;
    do {

        asm("ldy %v", zpa0);
        asm("lda $B800, y");
        asm("sta %v", zpa1);
        zpc0.w = LookupY(zpa1);
        VERA_ADDRx_L = zpc0.l;
        VERA_ADDRx_M = zpc0.h;

        asm("ldy %v", zpa0);
        asm("lda $B000, y");
        asm("lsr"); // bit 0 moves into C
        asm("sta %v", VERA_DC4_FX_X_POS_L);
        //asm("sta %v", zpa1);
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


        asm("lda %v", VERA_DATA1);
        asm("ldx %v", zpc1);
        asm("stx %v", VERA_DATA1);
        asm("ldx %v", zpc2);
        asm("stx %v", VERA_DATA1);
        asm("ldx %v", zpc3);
        asm("stx %v", VERA_DATA1);
        asm("sta %v", VERA_DATA0);

        asm("lda %v", VERA_DATA1);
        asm("ldx %v + 1", zpc1);
        asm("stx %v", VERA_DATA1);
        asm("ldx %v + 1", zpc2);
        asm("stx %v", VERA_DATA1);
        asm("ldx %v + 1", zpc3);
        asm("stx %v", VERA_DATA1);
        asm("sta %v", VERA_DATA0);

        asm("lda %v", VERA_DATA1);
        asm("ldx %v + 1", zpc1);
        asm("stx %v", VERA_DATA1);
        asm("ldx %v + 1", zpc2);
        asm("stx %v", VERA_DATA1);
        asm("ldx %v + 1", zpc3);
        asm("stx %v", VERA_DATA1);
        asm("sta %v", VERA_DATA0);

        asm("lda %v", VERA_DATA1);
        asm("ldx %v + 1", zpc1);
        asm("stx %v", VERA_DATA1);
        asm("ldx %v + 1", zpc2);
        asm("stx %v", VERA_DATA1);
        asm("ldx %v + 1", zpc3);
        asm("stx %v", VERA_DATA1);
        asm("sta %v", VERA_DATA0);

        asm("lda %v", VERA_DATA1);
        asm("ldx %v", zpc1);
        asm("stx %v", VERA_DATA1);
        asm("ldx %v", zpc2);
        asm("stx %v", VERA_DATA1);
        asm("ldx %v", zpc3);
        asm("stx %v", VERA_DATA1);
        asm("sta %v", VERA_DATA0);

        zpa0++;
    } while (zpa0 < TEST_SIZE);

    VERA_CTRL = 0;
}

static void test_polygon_hack_nibble() {
    VERA_CTRL = DCSEL_2;
    VERA_ADDRx_H = bitmap_back_buffer + ADDR_INC_160;
    VERA_DC2_FX_CTRL = 0x86; //transparency polygon, 4bit
    VERA_CTRL = DCSEL_3 + 1;
    VERA_ADDRx_H = ADDR_INC_0 + 4; //inc = 0.5
    VERA_DC3_FX_X_INCR_L = 0;
    VERA_DC3_FX_X_INCR_H = 0;
    VERA_CTRL = DCSEL_4;
    VERA_DC4_FX_X_POS_H = 0;
    VERA_DC4_FX_X_POS_L = 0;

    zpa0 = 0;
    do {

        asm("ldy %v", zpa0);
        asm("lda $B800, y");
        asm("sta %v", zpa1);
        zpc0.w = LookupY(zpa1);
        VERA_ADDRx_L = zpc0.l;
        VERA_ADDRx_M = zpc0.h;

        asm("ldy %v", zpa0);
        asm("lda $B000, y");
        //asm("lsr"); // bit 0 moves into C
        asm("sta %v", VERA_DC4_FX_X_POS_L);
        //asm("sta %v", zpa1);

        zpa2 = 0x66;
        zpa3 = 0xEE;

        asm("ldx %v", zpa2);
        asm("lda %v", zpa3);

        asm("ldy %v", VERA_DATA1);
        asm("stz %v", VERA_DATA1);
        asm("stx %v", VERA_DATA1);
        asm("stx %v", VERA_DATA1);
        asm("stx %v", VERA_DATA1);
        asm("stz %v", VERA_DATA1);
        asm("sty %v", VERA_DATA0);

        asm("ldy %v", VERA_DATA1);
        asm("stx %v", VERA_DATA1);
        asm("eor #$99");
        asm("sta %v", VERA_DATA1);
        asm("eor #$99");
        asm("sta %v", VERA_DATA1);
        asm("sta %v", VERA_DATA1);
        asm("stx %v", VERA_DATA1);
        asm("sty %v", VERA_DATA0);

        //*
        asm("ldy %v", VERA_DATA1);
        asm("stx %v", VERA_DATA1);
        asm("sta %v", VERA_DATA1);
        asm("sta %v", VERA_DATA1);
        asm("sta %v", VERA_DATA1);
        asm("stx %v", VERA_DATA1);
        asm("sty %v", VERA_DATA0);
        //*/

        asm("ldy %v", VERA_DATA1);
        asm("stx %v", VERA_DATA1);
        asm("sta %v", VERA_DATA1);
        asm("sta %v", VERA_DATA1);
        asm("eor #$99");
        asm("sta %v", VERA_DATA1);
        asm("eor #$99");
        asm("stx %v", VERA_DATA1);
        asm("sty %v", VERA_DATA0);

        asm("ldy %v", VERA_DATA1);
        asm("stz %v", VERA_DATA1);
        asm("stx %v", VERA_DATA1);
        asm("stx %v", VERA_DATA1);
        asm("stx %v", VERA_DATA1);
        asm("stz %v", VERA_DATA1);
        asm("sty %v", VERA_DATA0);


        zpa0++;
    } while (zpa0 < TEST_SIZE);

    VERA_CTRL = 0;
}

static void test_polygon_hack_cache() {
    VERA_CTRL = DCSEL_6;
    VERA_DC6_FX_CACHE_L = 0xE6;
    VERA_DC6_FX_CACHE_M = 0xA2;
    VERA_DC6_FX_CACHE_H = 0x47;
    VERA_DC6_FX_CACHE_U = 0xD3;

    VERA_CTRL = DCSEL_2;
    VERA_ADDRx_H = bitmap_back_buffer + ADDR_INC_160;
    VERA_DC2_FX_CTRL = 0x56; // cache write, one byte cycling, polygon, 4bit
    VERA_DC2_FX_MULT = 0x00; // two byte cache
    VERA_CTRL = DCSEL_3 + 1;
    VERA_ADDRx_H = bitmap_back_buffer + ADDR_INC_4;
    VERA_DC3_FX_X_INCR_L = 0;
    VERA_DC3_FX_X_INCR_H = 0;
    VERA_CTRL = DCSEL_4;
    VERA_DC4_FX_X_POS_H = 0;
    VERA_DC4_FX_X_POS_L = 0;

    zpa0 = 0;
    do {
        asm("ldy %v", zpa0);
        asm("lda $B800, y");
        asm("sta %v", zpa1);
        zpc0.w = LookupY(zpa1);
        VERA_ADDRx_L = zpc0.l;
        VERA_ADDRx_M = zpc0.h;

        asm("ldy %v", zpa0);
        asm("lda $B000, y");

        asm("sta %v", VERA_DC4_FX_X_POS_L);



        asm("and #$07");
        asm("sta %v", zpa3);
        // top/bottom
        zpc1.l = poly_cache_mask[0][zpa3];
        zpc1.h = poly_cache_mask[2][zpa3];
        // middle
        zpc2.l = poly_cache_mask[1][zpa3];
        zpc2.h = poly_cache_mask[3][zpa3];


        zpa2 = VERA_DATA1;
        VERA_DATA1 = zpc1.l;
        VERA_DATA1 = zpc1.h;
        zpa2 = VERA_DATA0;

        zpa2 = VERA_DATA1;
        VERA_DATA1 = zpc2.l;
        VERA_DATA1 = zpc2.h;
        zpa2 = VERA_DATA0;
        zpa2 = VERA_DATA1;
        VERA_DATA1 = zpc2.l;
        VERA_DATA1 = zpc2.h;
        zpa2 = VERA_DATA0;
        zpa2 = VERA_DATA1;
        VERA_DATA1 = zpc2.l;
        VERA_DATA1 = zpc2.h;
        zpa2 = VERA_DATA0;

        zpa2 = VERA_DATA1;
        VERA_DATA1 = zpc1.l;
        VERA_DATA1 = zpc1.h;
        zpa2 = VERA_DATA0;







        zpa0++;
        //} while (zpa0 < 1);
    } while (zpa0 < TEST_SIZE);

    VERA_CTRL = DCSEL_2;
    VERA_DC2_FX_CTRL = 0x00;
    VERA_DC2_FX_MULT = 0x00;
    VERA_CTRL = 0;
}
//

