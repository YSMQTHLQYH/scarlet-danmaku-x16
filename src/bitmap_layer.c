#include "bitmap_layer.h"
#include "x16.h"
#include "zp_utils.h"

uint8_t bitmap_front_buffer = 0;
uint8_t bitmap_back_buffer = 0;

uint16_t lookup_bitmap_y[BITMAP_HEIGHT] = { 0 };
void CalculateYLookup() {
    uint8_t i;
    register uint16_t y = 0;
    for (i = 0; i < BITMAP_HEIGHT; i++) {
        lookup_bitmap_y[i] = y;
        y += BITMAP_WIDTH_BYTES;
    }
}
void BitmapInit(uint8_t start_frame_buffer) {
    CalculateYLookup();
    bitmap_front_buffer = 0;
    bitmap_back_buffer = 1;
    if (start_frame_buffer) {
        bitmap_front_buffer = 1;
        bitmap_back_buffer = 0;
    }

    VERA_CTRL = 0;
    VERA_DC0_VIDEO |= (1 << 5);
#if (BITMAP_BPP == 2)
    VERA_L1_CFG = (1 << 2) + COLOR_DEPTH_2BPP;
#elif (BITMAP_BPP == 4)
    VERA_L1_CFG = (1 << 2) + COLOR_DEPTH_4BPP;
#endif
    VERA_L1_BITMAPBASE = ((bitmap_front_buffer << 7) | (MEM_BITMAP_1_ADDR_M >> 1) & 0xFC);

    BitmapClearBuffer(0, 0);
    BitmapClearBuffer(1, 0);
}

void BitmapClearBuffer(uint8_t buffer_n, uint8_t color) {
    uint8_t i, j, c;
#if (BITMAP_BPP == 2)
    i = color & 0x03;
    c = i;
    while (i) {
        i <<= 2;
        c |= i;
    }
#elif (BITMAP_BPP == 4)
    i = color & 0x0F;
    c = i;
    c |= (i << 4);
#endif
    VERA_CTRL = 0;
    VERA_ADDRx_H = (buffer_n & 1) | ADDR_INC_1;
    VERA_ADDRx_M = MEM_BITMAP_1_ADDR_M;
    VERA_ADDRx_L = 0;

    for (j = 0; j < BITMAP_HEIGHT; j++) {
        for (i = 0; i < BITMAP_WIDTH_BYTES; i++) {
            VERA_DATA0 = c;
        }
    }
}

void BitmapSwapBuffers() {
    if (bitmap_front_buffer) {
        bitmap_front_buffer = 0;
        bitmap_back_buffer = 1;
        VERA_L1_BITMAPBASE = ((MEM_BITMAP_1_ADDR_M >> 1) & 0xFC);
        return;
    }
    bitmap_front_buffer = 1;
    bitmap_back_buffer = 0;
    VERA_L1_BITMAPBASE = (0x80 + (MEM_BITMAP_1_ADDR_M >> 1) & 0xFC);
}


void BitmapFillRect(uint8_t buffer_n, uint8_t color, uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
#define ADDR    zpc0
    uint8_t i, j, c;
#if (BITMAP_BPP == 2)
    i = color & 0x03;
    c = i;
    while (i) {
        i <<= 2;
        c |= i;
    }
#elif (BITMAP_BPP == 4)
    i = color & 0x0F;
    c = i;
    c |= (i << 4);
#endif

    VERA_CTRL = 0;
    VERA_ADDRx_H = (buffer_n & 1) | ADDR_INC_1;
    ADDR.l = x;
    ADDR.h = MEM_BITMAP_1_ADDR_M;
    ADDR.w += lookup_bitmap_y[y];
    VERA_ADDRx_M = ADDR.h;
    VERA_ADDRx_L = ADDR.l;

    for (j = 0; j < h; j++) {
        for (i = 0; i < w; i++) {
            VERA_DATA0 = c;
        }
        ADDR.w += BITMAP_WIDTH_BYTES;
        VERA_ADDRx_M = ADDR.h;
        VERA_ADDRx_L = ADDR.l;
    }
#undef ADDR
}