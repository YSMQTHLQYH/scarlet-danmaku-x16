#include "graphics_utils.h"
#include "x16.h"
#include "memory_map.h"
#include "zp_utils.h"

void SetColorPalette(uint8_t palette_n, uint16_t* color_array) {
    uint8_t i;
    _uColorConv c;
    _uConv16 addr = { 0 };
    VERA_CTRL = 0;
    VERA_ADDRx_H = ADDR_INC_1 + 1;
    addr.h = MEM_VRAM_1_VERA_COLOR_PALETTE_M;
    addr.w += (palette_n << 5);
    VERA_ADDRx_M = addr.h;
    VERA_ADDRx_L = addr.l;
    for (i = 0; i < 16; i++) {
        c.u16 = color_array[i];
        VERA_DATA0 = c.l;
        VERA_DATA0 = c.h;
    }
}