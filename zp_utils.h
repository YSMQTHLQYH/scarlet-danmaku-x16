#ifndef ZP_UTILS_H
#define ZP_UTILS_H
//  ----  Assorted stuff that didn't fit elsewhere
// too specific to be game code but not exactly hardware stuff that goes in x16.h

#include <stdint.h>

typedef union {
    struct { uint8_t u8_l; uint8_t u8_h; };
    uint16_t u16;
} _uConv16;


// custom IRQ and it's things
void CustomIrq();


// for extending the default irq handler instead of replacing it
// make sure this has been set to something before setting CustomIrq as the interrupt handler
extern void* kernal_irq_func_ptr;

// increases by one every frame during VSYNC
extern volatile uint8_t frame_count;








#endif