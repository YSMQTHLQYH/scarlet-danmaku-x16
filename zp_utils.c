#include "zp_utils.h"
#include "x16.h"

void* kernal_irq_func_ptr;
uint8_t volatile frame_count = 0;


void CustomIrq() {
    // ---- read VSYNC
    //IrqVsync();
    asm("lda $9f27"); //vera->ISR
    asm("and #$01"); //A &= 0x01
#ifndef __INTELLISENSE__ // dammit vscode
    asm("beq %g", irq_not_vsync); // branch if Z flag set (result was 0)
#endif
    asm("inc %v", frame_count); //frame_count++
irq_not_vsync:

    // continue with KERNAL's normal irq handler, I don't want to also have to reverse engineer the inner bits of that
    asm("jmp (%v)", kernal_irq_func_ptr);
}
