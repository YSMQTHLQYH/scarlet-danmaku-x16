#ifndef BITMAP_LAYER_H
#define BITMAP_LAYER_H

#include <stdint.h>
#include "intellisense_macro.h"
/*
this is the main bitmap layer using during the game on hardware layer 1
we are using dual frame buffers, we can afford this as it's a low resolution and bit depth anyways
conviniently we have frame buffer 0 on VRAM page 0 and frame buffer 1 on VRAM page 1, so we can use all the same addresses
*/

#define BITMAP_WIDTH            320
#define BITMAP_HEIGHT           240
#define BITMAP_BPP              4
#define BITMAP_PIXELS_PER_BYTE  2 // inverse of BITMAP_BPP I guess
#define BITMAP_WIDTH_BYTES      160 // BITMAP_WIDTH / BITMAP_PIXELS_PER_BYTE, don't trust the compiler enough


// uses zpc0, zpa0 and zpa1!
extern void fastcall BitmapLayerFillRect(uint8_t buffer_n, uint8_t color, uint8_t x, uint8_t y, uint8_t w, uint8_t h);
// uses zpc0!
extern void fastcall BitmapLayerClearGameArea(uint8_t buffer_n, uint8_t color);
extern uint16_t fastcall LookupY(uint8_t y);

extern uint16_t lookup_bitmap_y[BITMAP_HEIGHT];

// buffer currently being displayed (should NOT be written to)
extern uint8_t bitmap_front_buffer;
// buffer currently NOT being displayed (should be written to)
extern uint8_t bitmap_back_buffer;

void BitmapInit(uint8_t start_frame_buffer);
void BitmapClearBuffer(uint8_t buffer_n, uint8_t color);
void BitmapSwapBuffers();


// uses zpc0!
// probably going to be taken out of final game, it's very inneficient to write pixels one at a time
// useful for testing though
extern void fastcall BitmapSetPixel(uint8_t buffer_n, uint8_t color, uint8_t x, uint8_t y);




#endif