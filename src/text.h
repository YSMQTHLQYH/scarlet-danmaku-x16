#ifndef TEST_H   
#define TEST_H

#include <stdint.h>



// loads KERNAL's monocrome font and makes a 2/4bpp copy of it for bitmap graphics / sprites
// returns 0 on success, 1 on failure
uint8_t HijackRomCharset(uint8_t charset, uint8_t font_bpp, uint8_t color);


// uses zpc0!
// returns index of sprite object
uint8_t CreateSpriteStr(uint8_t spr_priority, uint8_t lenght, uint8_t z_flip, uint8_t palette);
void FreeSpriteStr(uint8_t spr_obj);
// uses zpc0!
// for setting position and stuff just call the sprite object functions
// writes text to an already existing (string) sprite obeject
void PrintSpriteStr(uint8_t spr_obj, char* str);


// blits text into the bitmap layer 1, does not keep track of the text in any way
// position in the x-axis is x*4
void Print2BppBitmapStr(char* str, uint8_t buffer_n, uint8_t x, uint8_t y);

//  ---- Conversions to str



// does NOT add the null terminator, allways sets c[0] to c[2]
void StrUint8Dec(uint8_t u, char* c);
// does NOT add the null terminator, allways sets c[0] to c[1]
void StrUint8Hex(uint8_t u, char* c);
// does NOT add the null terminator, allways sets c[0] to c[3]
void StrUint16Hex(uint16_t u, char* c);



#endif