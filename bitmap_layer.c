#include "bitmap_layer.h"
#include "x16.h"

uint8_t bitmap_front_buffer = 0;
uint8_t bitmap_back_buffer = 0;

void BitmapInit(uint8_t start_frame_buffer) {
    bitmap_front_buffer = 0;
    bitmap_back_buffer = 1;
    if (start_frame_buffer) {
        bitmap_front_buffer = 1;
        bitmap_back_buffer = 0;
    }

    vera->CTRL = 0;
    vera->DC0.VIDEO |= (1 << 5);
    vera->LAYER1.CONFIG = (1 << 2) + COLOR_DEPTH_2BPP;
    vera->LAYER1.BITMAPBASE = ((bitmap_front_buffer << 7) | (BITMAP_ADDR_START_M >> 1) & 0xFC);

    BitmapClearBuffer(0, 0);
    BitmapClearBuffer(1, 0);
}

void BitmapClearBuffer(uint8_t buffer_n, uint8_t color) {
    uint8_t i, j, c;
    i = color & 0x03;
    c = i;
    while (i) {
        i <<= 2;
        c |= i;
    }
    vera->CTRL = 0;
    vera->ADDRx_H = (buffer_n & 1) | ADDR_INC_1;
    vera->ADDRx_M = BITMAP_ADDR_START_M;
    vera->ADDRx_L = 0;

    for (j = 0; j < BITMAP_HEIGHT; j++) {
        for (i = 0; i < (BITMAP_WIDTH >> 2); i++) {
            vera->DATA0 = c;
        }
    }
}

void BitmapSwapBuffers() {
    if (bitmap_front_buffer) {
        bitmap_front_buffer = 0;
        bitmap_back_buffer = 1;
        vera->LAYER1.BITMAPBASE = ((BITMAP_ADDR_START_M >> 1) & 0xFC);
        return;
    }
    bitmap_front_buffer = 1;
    bitmap_back_buffer = 0;
    vera->LAYER1.BITMAPBASE = (0x80 + (BITMAP_ADDR_START_M >> 1) & 0xFC);
}