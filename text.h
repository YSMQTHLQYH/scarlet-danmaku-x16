#ifndef TEST_H   
#define TEST_H

#include <stdint.h>

// loads KERNAL's monocrome font and makes a 2/4bpp copy of it for bitmap graphics / sprites
// returns 0 on success, 1 on failure
uint8_t HijackRomCharset(uint8_t charset, uint8_t font_bpp, uint8_t color);

// all on page 0
#define FONT_2BPP_START 0xD0// starts at 0:D000
#define FONT_4BPP_START 0xE0// starts at 0:E000




#endif