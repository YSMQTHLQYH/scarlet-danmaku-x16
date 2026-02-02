#ifndef GRAPHICS_UTILS_H
#define GRAPHICS_UTILS_H

#include <stdint.h>

/* C specification does NOT define what order structs of bitfields are written in for some stupid reason*/
/* This may not work on other compilers*/
typedef union {
    struct
    {
        uint8_t b : 4;
        uint8_t g : 4;
        uint8_t r : 4;
    };
    struct {
        uint8_t l;
        uint8_t h;
    };
    uint16_t u16;
}_uColorConv;

// sets vera's color palette registers, writes a full 16 color palette
// palette_n is palette index (0 to 15)
void SetColorPalette(uint8_t palette_n, uint16_t* color_array);



#endif