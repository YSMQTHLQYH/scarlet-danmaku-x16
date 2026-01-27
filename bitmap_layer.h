#ifndef BITMAP_LAYER_H
#define BITMAP_LAYER_H

#include <stdint.h>
/*
this is the main bitmap layer using during the game on hardware layer 1
we are using dual frame buffers, we can afford this as it's a low resolution and bit depth anyways
conviniently we have frame buffer 0 on VRAM page 0 and frame buffer 1 on VRAM page 1, so we can use all the same addresses
*/

#define BITMAP_WIDTH    320
#define BITMAP_HEIGHT   240
//#define BITMAP_BPP      2 // I don't think there's much use for this

//#define BITMAP_ADDR_START_M   0x0000
#define BITMAP_ADDR_START_M   0x00
//#define BITMAP_ADDR_END     0x4AFF
#define BITMAP_ADDR_END_M     0x4A

// buffer currently being displayed (should NOT be written to)
extern uint8_t bitmap_front_buffer;
// buffer currently NOT being displayed (should be written to)
extern uint8_t bitmap_back_buffer;

void BitmapInit(uint8_t start_frame_buffer);
void BitmapClearBuffer(uint8_t buffer_n, uint8_t color);
void BitmapSwapBuffers();


#endif