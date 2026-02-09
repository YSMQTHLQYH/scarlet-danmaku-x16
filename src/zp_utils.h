#ifndef ZP_UTILS_H
#define ZP_UTILS_H
//  ----  Assorted stuff that didn't fit elsewhere
// too specific to be game code but not exactly hardware stuff that goes in x16.h


#include <stdint.h>
#include "memory_map.h"

typedef union {
    struct { uint8_t l; uint8_t h; };
    uint16_t w;
} _uConv16;

//  ---- actual zero page scratch variables
//  function calls might change the values stored here!
extern uint8_t zpa0;
extern uint8_t zpa1;
extern uint8_t zpa2;
extern uint8_t zpa3;
extern uint8_t zpa4;
extern uint8_t zpa5;
extern uint8_t zpa6;
#pragma zpsym ("zpa0");
#pragma zpsym ("zpa1");
#pragma zpsym ("zpa2");
#pragma zpsym ("zpa3");
#pragma zpsym ("zpa4");
#pragma zpsym ("zpa5");
#pragma zpsym ("zpa6");

extern _uConv16 zpc0;
extern _uConv16 zpc1;
extern _uConv16 zpc2;
extern _uConv16 zpc3;
#pragma zpsym ("zpc0");
#pragma zpsym ("zpc1");
#pragma zpsym ("zpc2");
#pragma zpsym ("zpc3");

extern uint8_t* zptr0;
extern uint8_t* zptr1;
extern uint8_t* zptr2;
extern uint8_t* zptr3;
#pragma zpsym ("zptr0");
#pragma zpsym ("zptr1");
#pragma zpsym ("zptr2");
#pragma zpsym ("zptr3");



//  ---- function that do lower level stuff

// custom IRQ and it's things
void CustomIrq();

// for extending the default irq handler instead of replacing it
// make sure this has been set to something before setting CustomIrq as the interrupt handler
extern void* kernal_irq_func_ptr;

// increases by one every frame during VSYNC
// at start of scanline 0x1E0 (480)
extern volatile uint8_t frame_count;








#endif