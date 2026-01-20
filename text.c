#include "text.h"
#include "x16.h"

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