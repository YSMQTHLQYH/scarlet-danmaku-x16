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
#define VERA_TEST_REPEAT(i)    w.h = vera->DATA1;\
        w.l = vera->DATA1;\
        w.w += (int8_t)vera->DATA1;\
        vera->DATA0 = w.h + (int8_t)vera->DATA1;\
        vera->DATA0 = w.l;\
// comment "w.u16 += (int8_t)vera->DATA1;\" line out for 8 bit second number



// ---- vera definitions
// this were in x16.h, turns out the compiler is very inneficient at handling these when defined in C
// moving everything to just variables declared in assembly, copying this here so we can still do the comparasions
typedef struct {
    uint8_t CONFIG;
    uint8_t MAPBASE;
    union { uint8_t TILEBASE; uint8_t BITMAPBASE; };
    uint8_t HSCROLL_L;
    union { uint8_t HSCROLL_H; uint8_t BITMAP_COLOR_OFFSET; };
    uint8_t VSCROLL_L;
    uint8_t VSCROLL_H;
}_sVeraLayerReg;

typedef struct
{
    uint8_t ADDRx_L;
    uint8_t ADDRx_M;
    uint8_t ADDRx_H; // dammit c for not letting me use bitfields for this cuz order of bits is up to commpilerion {    struct {      uint8_t addr_increment : 4;      uint8_t DECR : 1;      uint8_t nibble_increment : 1;      uint8_t nibble_address : 1;      uint8_t a
    uint8_t DATA0;
    uint8_t DATA1;
    uint8_t CTRL;
    uint8_t IEN;
    uint8_t ISR;
    union { uint8_t IRQLINE_L; uint8_t SCANLINE_L; };

    union {
        struct { uint8_t VIDEO; uint8_t HSCALE; uint8_t VSCALE; uint8_t BORDER; } DC0;
        struct { uint8_t HSTART; uint8_t HSTOP; uint8_t VSTART; uint8_t VSTOP; } DC1;
        struct { uint8_t FX_CTRL; uint8_t FX_TILEBASE; uint8_t FX_MAPBASE; uint8_t FX_MULT; } DC2;
    };

    _sVeraLayerReg LAYER0;
    _sVeraLayerReg LAYER1;
    uint8_t AUDIO_CTRL;
    uint8_t AUDIO_RATE;
    uint8_t AUDIO_DATA;
    uint8_t SPI_DATA;
    uint8_t SPI_CTRL;


} _sVeraReg;

volatile _sVeraReg* const vera = (void*)0x9F20;


//  ---- test actually for real

#define TEST_RAM_BANK   4


#define TEST    8
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
    x16_ram_bank = TEST_RAM_BANK;
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
        x16_ram_bank = TEST_RAM_BANK;
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
        x16_ram_bank = TEST_RAM_BANK;
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

            w.h = vera->DATA1;
            while (w.h != 0) {
                w.l = vera->DATA1;
                w.w += (int8_t)vera->DATA1; // comment this line out for 8 bit second number
                //vera->DATA0 = w.u16 + (int8_t)vera->DATA1; //also 16 bit version
                vera->DATA0 = w.h + (int8_t)vera->DATA1;
                vera->DATA0 = w.l;
                w.h = vera->DATA1;
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

        // we have doubled perfomance by hardcoding the VERA addresses manually,
        // instead of a constant pointer to the VERA addresses
        // you would expect any decent compiler to see a CONSTANT pointer and hard code the addresses instead of shuffling multiple bytes to set up a pointer...
    case MATH_TEST_VERA_BETTER_WRITE:

        VERA_CTRL = 1;
        VERA_ADDRx_H = ADDR_INC_1;

        for (j = 0; j < TEST; j++) {

            VERA_ADDRx_M = TEST_ADDR_M;
            VERA_ADDRx_L = 0;
            //i = 0;

            w.h = VERA_DATA1;
            while (w.h != 0) {
                w.l = VERA_DATA1;
                w.w += (int8_t)VERA_DATA1; // comment this line out for 8 bit second number
                //vera->DATA0 = w.w + (int8_t)vera->DATA1; //also 16 bit version
                VERA_DATA0 = w.h + (int8_t)VERA_DATA1;
                VERA_DATA0 = w.l;
                w.h = VERA_DATA1;
                //i++;
            }
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
