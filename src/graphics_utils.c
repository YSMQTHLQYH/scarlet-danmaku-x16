#include "graphics_utils.h"
#include "x16.h"
#include "memory_map.h"
#include "zp_utils.h"

void SetColorPalette(uint8_t palette_n, uint16_t* color_array) {
    uint8_t i;
    _uColorConv c;
    _uConv16 addr = { 0 };
    vera->CTRL = 0;
    vera->ADDRx_H = ADDR_INC_1 + 1;
    addr.u8_h = MEM_VRAM_1_VERA_COLOR_PALETTE_M;
    addr.u16 += (palette_n << 5);
    vera->ADDRx_M = addr.u8_h;
    vera->ADDRx_L = addr.u8_l;
    for (i = 0; i < 16; i++) {
        c.u16 = color_array[i];
        vera->DATA0 = c.l;
        vera->DATA0 = c.h;
    }
}