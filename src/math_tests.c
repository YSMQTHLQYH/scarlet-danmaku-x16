#include "math_tests.h"
#include "x16.h"
#include "zp_utils.h"

//    ----macro schenanigans
#define REPEAT1(a, i)   a(i)
#define REPEAT2(a, i)   REPEAT1(a, i) REPEAT1(a, i+1)
#define REPEAT4(a, i)   REPEAT2(a, i) REPEAT2(a, i+2)
#define REPEAT8(a, i)   REPEAT4(a, i) REPEAT4(a, i+4)
#define REPEAT16(a, i)  REPEAT8(a, i) REPEAT8(a, i+8)
#define REPEAT32(a, i)  REPEAT16(a, i) REPEAT16(a, i+16)
#define REPEAT64(a, i)  REPEAT32(a, i) REPEAT32(a, i+32)

#define REPEAT_N(n, a)  REPEAT##n(a, 0)

// _uConv16 w has to exist in scope for this btw
#define VERA_TEST_REPEAT(i)    w.u8_h = vera->DATA1;\
        w.u8_l = vera->DATA1;\
        w.u16 += (int8_t)vera->DATA1;\
        vera->DATA0 = w.u8_h + (int8_t)vera->DATA1;\
        vera->DATA0 = w.u8_l;\
// comment "w.u16 += (int8_t)vera->DATA1;\" line out for 8 bit second number

//  ----

#define TEST    32
#define TEST_SIZE   64
uint16_t test_arr[TEST * TEST_SIZE] = { 0 };
#define TEST_ADDR_M 0x4B

void MathTestsinit() {
    uint16_t i = 0;
    uint8_t h = 0;
    uint8_t j[TEST_SIZE + 1] = { 0 };



    //set test table in VRAM
    vera->CTRL = 0;
    vera->ADDRx_H = 0x10;
    vera->ADDRx_M = TEST_ADDR_M;
    vera->ADDRx_L = 0;

    for (h = 0; h < TEST_SIZE; h++) {
        vera->DATA0 = h + 1;
        vera->DATA0 = h + 1;
        vera->DATA0 = h + 1;
        // comment this one out for 8 bit second number
        vera->DATA0 = h + 1;
    }
    vera->DATA0 = 0;
    //set high ram
    SET_RAM_BANK(1);
    for (i = 0; i < RAM_BANK_SIZE; i++) {
        HIGH_RAM_8(i) = (uint8_t)i;
        if ((uint8_t)i == 0) { HIGH_RAM_8(i) = 1; }
    }
}

static uint8_t aux = 0;

void MathTest(_eMathTest t) {
    uint8_t i = 0, j = 0;
    _uConv16 w;

    // sets this up as a dummy target to write to
    vera->CTRL = 0;
    vera->ADDRx_H = ADDR_INC_0;
    vera->ADDRx_M = 0;
    vera->ADDRx_L = 0;

    switch (t) {
    case MATH_TEST_NOTHING:
        // does nothing, just to measure baseline lag of everything else
        break;
    case MATH_TEST_FOR:
        SET_RAM_BANK(1);
        for (j = 0; j < TEST; j++) {
            for (i = 0; i < TEST_SIZE; i++) {
                //vera->DATA0 = HIGH_RAM_16(i) + (int8_t)HIGH_RAM_8(i + 0x1000);
                HIGH_RAM_16(i) += HIGH_RAM_16(i + 0x0800);
                vera->DATA0 = HIGH_RAM_8(i);
                vera->DATA0 = HIGH_RAM_8(i + 1);
            }
        }
        break;

    case MATH_TEST_WHILE:
        SET_RAM_BANK(1);
        for (j = 0; j < TEST; j++) {
            HIGH_RAM_16(TEST_SIZE) = 0;
            i = 0;
            while (HIGH_RAM_16(i) != 0) {
                //vera->DATA0 = HIGH_RAM_16(i) + (int8_t)HIGH_RAM_8(i + 0x1000);
                HIGH_RAM_16(i) += HIGH_RAM_16((i & 0x03FF) + 0x0800);
                vera->DATA0 = HIGH_RAM_8(i);
                vera->DATA0 = HIGH_RAM_8(i + 1);
                i++;
            }
        }
        break;

    case MATH_TEST_VERA_UNTIL0:

        vera->CTRL = 1;
        vera->ADDRx_H = ADDR_INC_1;

        for (j = 0; j < TEST; j++) {

            vera->ADDRx_M = TEST_ADDR_M;
            vera->ADDRx_L = 0;
            //i = 0;

            w.u8_h = vera->DATA1;
            while (w.u8_h != 0) {
                w.u8_l = vera->DATA1;
                w.u16 += (int8_t)vera->DATA1; // comment this line out for 8 bit second number
                //vera->DATA0 = w.u16 + (int8_t)vera->DATA1; //also 16 bit version
                vera->DATA0 = w.u8_h + (int8_t)vera->DATA1;
                vera->DATA0 = w.u8_l;
                w.u8_h = vera->DATA1;
                //i++;
            }
        }
        break;

        // so much effort with this stupid macro only for it to be slightly slower
        // nvm the other function was wrong, this is slightly faster
    case MATH_TEST_VERA_REPEAT_MACRO:
        vera->CTRL = 1;
        vera->ADDRx_H = ADDR_INC_1;

        for (j = 0; j < TEST; j++) {
            vera->ADDRx_M = TEST_ADDR_M;
            vera->ADDRx_L = 0;

            REPEAT_N(64, VERA_TEST_REPEAT)

        }

        break;

        // same as MATH_TEST_VERA_UNTIL0 but translated to assembly
        // lMFAO this is ~7 times faster, cc65 you kinda dumb
    case MATH_TEST_VERA_UNTIL0_ASM:
        //vera->CTRL = 1;
        asm("lda #$1");
        asm("sta $9F25");
        //vera->ADDRx_H = (INC_1 << 4);
        asm("lda #$10");
        asm("sta $9F22");

        //for (j = 0; j < TEST; j++) {
        asm("ldy #%b", TEST);
        asm("sty %v", aux);
    vera_u0_asm_big_loop: //this label name is kinda shit but whatever, has to be specific for which case in the switch (in theory)


        asm("lda #%b", TEST_ADDR_M); // A = TEST_ADDR_M
        asm("sta $9F21"); //vera->ADDRx_M = TEST_ADDR_M;
        asm("lda #0"); // A = 0;
        asm("sta $9F20"); //vera->ADDRx_L = 0;

        asm("ldx $9F24"); // (w.u8_h): X = vera->DATA1;
    vera_u0_asm_little_loop: // checked at the end first statement is repeated line above
        asm("lda $9F24"); // (w.u8_l): A = vera->DATA1;
        //w.u16 += (vera->DATA1 << 8) + vera->DATA1 ; 
        // 16 bit add
        asm("clc"); // clear carry
        asm("adc $9F24"); // A += vera->DATA1;
        asm("tay"); // Y = (w.u8_l): A 
        asm("txa"); // A = (w.u8_h): X 
        asm("adc $9F24"); // A += vera->DATA1;
        // write w.u16 to vera->DATA0
        asm("sta $9F23"); // vera->DATA0 = A (w.u8_h)
        asm("sty $9F23"); // vera->DATA0 = Y (w.u8_l)

        // check for equivalen of while (w.u8_h != 0) 
        asm("ldx $9F24"); // (w.u8_h): X = vera->DATA1;
        asm("bne %g", vera_u0_asm_little_loop); // if Z == 0 (x != 0)


        asm("dec %v", aux);
        asm("bne %g", vera_u0_asm_big_loop);
        //}
        break;


    case MATH_TEST_WASTE_TIME:
        for (i = 0; i < 0x80; i++) {
            for (j = 0; j < 0xFF; j++) {
                asm("nop");
                asm("nop");
                asm("nop");
            }
        }
        break;

    default:
        break;
    }
}
