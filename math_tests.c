#include "math_tests.h"
#include "x16.h"

#define TEST    4
#define TEST_SIZE   64
uint16_t test_arr[TEST * TEST_SIZE] = { 0 };

typedef union {
    struct { uint8_t u8_l; uint8_t u8_h; };
    uint16_t u16;
} _uConv16;

void MathTestsinit() {
    uint16_t i = 0;
    uint8_t h = 0;
    uint8_t j[TEST_SIZE + 1] = { 0 };

    //set test table in VRAM
    vera->CTRL = 0;
    vera->ADDRx_H = 0x10;
    vera->ADDRx_M = 0;
    vera->ADDRx_L = 0;

    for (h = 0; h < TEST_SIZE; h++) {
        vera->DATA0 = h + 1;
    }
    vera->DATA0 = 0;
    //set high ram
    *ram_bank = 1;
    for (i = 0; i < RAM_BANK_SIZE; i++) {
        HIGH_RAM_8(i) = (uint8_t)i;
        if (i == 0) { HIGH_RAM_8(i) = 1; }
    }
}

void MathTest(_eMathTest t) {
    uint8_t i = 0, j = 0;
    _uConv16 w;

    // sets this up as a dummy target to write to
    vera->CTRL = 0;
    vera->ADDRx_H = (INC_0 << 4);

    switch (t) {
    case MATH_TEST_FOR:
        for (j = 0; j < TEST; j++) {
            for (i = 0; i < TEST_SIZE; i++) {
                vera->DATA0 = HIGH_RAM_16(i) + (int8_t)HIGH_RAM_8(i + 0x1000);
            }
        }
        break;

    case MATH_TEST_WHILE:
        for (j = 0; j < TEST; j++) {
            HIGH_RAM_16(TEST_SIZE) = 0;
            i = 0;
            while (HIGH_RAM_16(i) != 0) {
                vera->DATA0 = HIGH_RAM_16(i) + (int8_t)HIGH_RAM_8(i + 0x1000);
                i++;
            }
        }
        break;

    case MATH_TEST_VERA_UNTIL0:

        vera->CTRL = 1;
        vera->ADDRx_H = (INC_1 << 4);

        for (j = 0; j < TEST; j++) {

            vera->ADDRx_M = 0;
            vera->ADDRx_L = 0;
            i = 0;

            w.u8_h = vera->DATA1;
            while (w.u8_h != 0) {
                w.u8_l = vera->DATA1;
                vera->DATA0 = w.u16 + (int8_t)HIGH_RAM_8(i + 0x1000);
                w.u8_h = vera->DATA1;
                i++;
                //EMU_DEBUG_1(i);
            }
        }
        break;


    default:
        break;
    }
}
