#ifndef TEST_H   
#define TEST_H

#include <stdint.h>

#define TEXT_SPRITE_MAX_LENGHT  16
#define TEXT_SPRITE_INDEX_START 0x60
#define TEXT_MAX_SPRITES        0x20

// all on page 0
#define FONT_2BPP_START 0xD0// starts at 0:D000
#define FONT_4BPP_START 0xE0// starts at 0:E000


// loads KERNAL's monocrome font and makes a 2/4bpp copy of it for bitmap graphics / sprites
// returns 0 on success, 1 on failure
uint8_t HijackRomCharset(uint8_t charset, uint8_t font_bpp, uint8_t color);


// str_slot is "slot" string is written to, set it to 0 to write to any free slot, set to anything else to replace that str
// returns str_slot of text written, 0 if there wasn't any free slots
uint8_t PrintSpriteStr(char* str, uint8_t str_slot, uint16_t x, uint16_t y, uint8_t palette);
// clears str_slot and sprites used by it
void FreeSpriteStr(uint8_t str_slot);



#endif