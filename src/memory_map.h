#ifndef MEMORY_MAP_H
#define MEMORY_MAP_h
/*
Not actually a file with code,
it's just to have a single place with a list of which parts of memory are owned by what.
Just so we don't give the same memory page to two different things

Low RAM is not here, that is left up to the compiler (for now?)

Still in the code so we can put macros for the addresses here
*/

/*    VRAM PAGE 0 START    */
//  ---- Frame buffer 0 of bitmap 1 (main game bullets)
#define MEM_VRAM_0_BITMAP_1_START   0X0000
#define MEM_VRAM_0_BITMAP_1_END     0X4AFF

#define MEM_BITMAP_1_ADDR_M   0x00

//  ---- Unused 1
#define MEM_VRAM_0_UNUSED_1_START   0x4B00
#define MEM_VRAM_0_UNUSED_1_END     0xCFFF

//  ---- Text
#define MEM_VRAM_0_2BPP_FONT_1_START    0xD000
#define MEM_VRAM_0_2BPP_FONT_1_END      0xDFFF
#define MEM_VRAM_0_4BPP_FONT_1_START    0xE000
#define MEM_VRAM_0_4BPP_FONT_1_END      0xFFFF
#define MEM_FONT_VRAM_PAGE    0
#define MEM_2BPP_FONT_1_ADDR_M 0xD0
#define MEM_4BPP_FONT_1_ADDR_M 0xE0
/*     VRAM PAGE 0 END     */


/*    VRAM PAGE 1 START    */
//  ---- Frame buffer 1 of bitmap 1 (same address as frame buffer 0)
#define MEM_VRAM_1_BITMAP_1_START   0X0000
#define MEM_VRAM_1_BITMAP_1_END     0X4AFF

//#define MEM_BITMAP_1_ADDR_M   0x00 


//  ---- Unused 1
#define MEM_VRAM_1_UNUSED_1_START   0x4B00
#define MEM_VRAM_1_UNUSED_1_END     0xEFFF

//  ---- KERNAL charset (not actually used in game but needed free to load fonts)
#define MEM_VRAM_1_KERNAL_CHARSET_START 0xF000
#define MEM_VRAM_1_KERNAL_CHARSET_END   0xF7FF
#define MEM_VRAM_1_KERNAL_FONT_ADDR_M   0xF0

//  ---- Unused 2
#define MEM_VRAM_1_UNUSED_2_START   0xF800
#define MEM_VRAM_1_UNUSED_2_END     0xF9BF

//  ---- VERA Hardware registers
#define MEM_VRAM_1_VERA_PSG_START   0xF9C0
#define MEM_VRAM_1_VERA_PSG_END     0xF9FF
#define MEM_VRAM_1_VERA_COLOR_PALETTE_START 0xFA00
#define MEM_VRAM_1_VERA_COLOR_PALETTE_END   0xFBFF
#define MEM_VRAM_1_VERA_SPRITE_ATTR_START   0xFC00
#define MEM_VRAM_1_VERA_SPRITE_ATTR_END     0xFFFF

// These belong on x16.h, as they aren't specific to this game
// for the sake of consistency, I will add them here anyways and comment them out on x16.h
#define MEM_VRAM_1_VERA_PSG_M    0xF9
#define MEM_VRAM_1_VERA_PSG_L    0xC0
#define MEM_VRAM_1_VERA_COLOR_PALETTE_M     0xFA
#define MEM_VRAM_1_VERA_SPRITE_ATTR_M       0xFC
#define MEM_SIZE_SPRITE_ATTR                8


/*     VRAM PAGE 1 END     */

/*    BANKED RAM START     */

/*     BANKED RAM END      */

#endif