#include <stdio.h>
#include <stdint.h>

#define KERNAL_RDTIM    "jsr $FFDE"

#define EMU_DEBUG_1(a)  *(uint8_t*)0x9FB9 = a;

#define RAM_BANK_SIZE   0x2000
#define ROM_BANK_SIZE   0x4000
#define HIGH_RAM_8(i)   *(uint8_t*)(0xA000 + i)
#define HIGH_RAM_16(i)  *(uint16_t*)(0xA000 + i + i)
//I don't think it's actually called "high rom" but whatever it makes sense
#define HIGH_ROM_8(i)   *(uint8_t*)(0xC000 + i)
#define HIGH_ROM_16(i)  *(uint16_t*)(0xC000 + i + i)

volatile uint8_t* const ram_bank = (uint8_t*)0x0000;
volatile uint8_t* const rom_bank = (uint8_t*)0x0001;

typedef struct {
    uint8_t CONFIG;
    uint8_t MAPBASE;
    union { uint8_t TILEBASE; uint8_t BITMAPBASE; };
    uint8_t HSCROLL_L;
    uint8_t HSCROLL_J;
    uint8_t VSCROLL_L;
    uint8_t VSCROLL_H;
}_sVeraLayerReg;

typedef enum {
    INC_0,
    INC_1,
    INC_2,
    INC_4,
    INC_8,
    INC_16,
    INC_32,
    INC_64,
    INC_128,
    INC_256,
    INC_512,
    INC_40,
    INC_80,
    INC_160,
    INC_320,
    INC_640,
}_eVeraAddrInc;

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


typedef union {
    struct { uint8_t u8_l; uint8_t u8_h; };
    uint16_t u16;
} _uConv16;

void update();

void print_emul_debug(char* str);
char debug_buffer[256];

#define TEST    4
#define TEST_SIZE   64
uint16_t test_arr[TEST * TEST_SIZE] = { 0 };
void math_test() {
    uint8_t i = 0, j = 0;
    _uConv16 w;
    vera->CTRL = 0;
    vera->ADDRx_H = (INC_0 << 4);
    vera->CTRL = 1;
    vera->ADDRx_H = (INC_1 << 4);
    for (j = 0; j < TEST; j++) {
        //*
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
        //*/

        /*
        HIGH_RAM_16(TEST_SIZE) = 0;
        i = 0;
        //while (HIGH_RAM_16(i) != 0) {
        for (; i < TEST_SIZE;) {
            vera->DATA0 = HIGH_RAM_16(i) + (int8_t)HIGH_RAM_8(i + 0x1000);
            i++;
        }
        /*/
    }
}

uint8_t last_tick = 0, current_tick = 0;
void main() {
    uint16_t wait_count = 0, lag_count = 0;
    uint8_t test = 0;
    uint16_t i = 0;
    uint8_t h = 0;
    uint8_t j[TEST_SIZE + 1] = { 0 };
    printf("Hello, World!aaa\n");
    //snprintf(debug_buffer, 255, "hello");
    //print_emul_debug(debug_buffer);

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

    // setup for timer
    // jsr = Jump to Subroutine
    // This is the memory location of the RDTIM (Read Timer) Kernal Function
    asm(KERNAL_RDTIM);
    // sta = Store Accumulator
    // This copies the value from the A register to a memory location
    // In this case its the memory location of our "start" variable
    asm("sta %v", last_tick);

    while (1) {
        //update();

        switch (test) {
        case 0:

            break;
        case 1:
            /* code */
            break;
        case 2:
            math_test();
            break;

        case 3:
            //printf("lag: %i spare: %i\n", lag_count, wait_count);
            snprintf(debug_buffer, 64, "lag: %i spare: %i\n", lag_count, wait_count);
            print_emul_debug(debug_buffer);
            break;

        default:
            test = 0;
        }
        test++;


        // wait for next frame


        // Now that we have the start time
        // Get the "next" time and keep looping until it changes
        asm(KERNAL_RDTIM);
        asm("sta %v", current_tick);
        lag_count = current_tick - last_tick;
        wait_count = 0;
        last_tick += lag_count;
        while (last_tick == current_tick) {
            asm(KERNAL_RDTIM);
            asm("sta %v", current_tick);
            wait_count += 1;
        }
        last_tick = current_tick;

    }

}

void update() {
    uint8_t c = 0;
    uint16_t i = 0, j = 0;
    uint8_t v = 0;



    v = vera->DC0.VIDEO;
    v |= 0x10;
    //v &= ~0x20;
    vera->CTRL = 0x00;
    vera->DC0.VIDEO = v;

    vera->LAYER0.CONFIG = 0x06;
    vera->LAYER0.BITMAPBASE = 0x80;



    vera->CTRL = 0x00;
    vera->ADDRx_H = 0x11;
    vera->ADDRx_M = 0x00;
    vera->ADDRx_L = 0x00;
    vera->DC0.HSCALE = 64;
    vera->DC0.VSCALE = 64;
    for (i = 0; i < 160; i++) {
        c = i & 0x0F;
        for (j = 0; j < 80; j++) {
            vera->DATA0 = c + (c << 4);//(uint8_t)(j % 16 + (c << 4));

        }
    }


}




void print_emul_debug(char* str) {
    uint8_t i = 0;
    if (*(uint8_t*)0x9FBE == '1' && *(uint8_t*)0x9FBF == '6') {  //emulator secret handshake
        *(uint8_t*)0x9FB0 = 1; //enables debugger
        while (str[i] != 0) {
            *(uint8_t*)0x9FBB = str[i]; //emulator "stdout" thing
            i++;
        }
    }
}