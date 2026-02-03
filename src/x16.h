#ifndef X16_H
#define X16_H

#include <stdint.h>


//to call:
//asm("jsr %w", macro);
#define KERNAL_RDTIM    0xFFDE

#define KERNAL_SETNAM   0xFFBD
#define KERNAL_SETLFS   0xFFBA
#define KERNAL_LOAD     0xFFD5

#define KERNAL_JOYSTICK_GET 0xFF56

#define KERNAL_SCREEN_SET_CHARSET   0xFF62

// ---- KERNAL functions
uint8_t KernalReadTimer();
void KernalScreenSetCharset(uint8_t charset);
// returns 0 if joystick is present, 1 if not
// mask_0/1 are the buttons pressed, bAND them with _eJoystickMask
uint8_t KernalJoystickGet(uint8_t joystick_n, uint8_t* mask_0, uint8_t* mask_1);

typedef enum {
    FILE_LOAD_IGNORE_HEADER = 0,
    FILE_LOAD_USE_HEADER = 1,
    FILE_LOAD_HEADERLESS = 2,
}_eFileLoadHeader;
typedef enum {
    FILE_LOAD_RAM = 0,
    FILE_LOAD_VERIFY = 1,
    FILE_LOAD_VRAM_P0 = 2,
    FILE_LOAD_VRAM_P1 = 3,
}_eFileLoadTarget;
typedef struct {
    //args
    char* filename;
    uint8_t name_lenght;
    _eFileLoadHeader header_mode;
    _eFileLoadTarget target_mode;
    void* dest_addr;
    //return
    union {
        void* file_end; //final byte loaded + 1
        uint8_t error_code;
    };
}_sFileLoadCtx;


// returns 0 if error, 1 if success
uint8_t load_file(_sFileLoadCtx* ctx);

//  ---- memory

#define RAM_BANK_SIZE   0x2000
#define HIGH_RAM_START  0xA000
#define HIGH_RAM_END    0xBFFF
#define HIGH_RAM_8(i)   *(uint8_t*)(HIGH_RAM_START + i)
#define HIGH_RAM_16(i)  *(uint16_t*)(HIGH_RAM_START + i + i)

//I don't think it's actually called "high rom" but whatever it makes sense
#define ROM_BANK_SIZE   0x4000
#define HIGH_ROM_START  0xC000
// i'll implement high rom end if I ever use it
#define HIGH_ROM_8(i)   *(uint8_t*)(HIGH_ROM_START + i)
#define HIGH_ROM_16(i)  *(uint16_t*)(HIGH_ROM_START + i + i)


// prefixing these with x16 because these feel too powerful to just be a simple variable
extern volatile uint8_t x16_ram_bank;
#pragma zpsym ("x16_ram_bank");
extern volatile uint8_t x16_rom_bank;
#pragma zpsym ("x16_rom_bank");

//  ---- IRQ
#define KERNAL_CINV (void**)0x0314
// replaces the current function called on IRQ interrupt by func_pointer
void IrqSetVector(void* func_pointer);
// returns the IRQ function pointer (so we can extend the interrupt function instead of replacing it)
void* IrqGetVector();



//  ---- joystick
typedef enum {
    JOYSTICK_M0_RIGHT = (1 << 0),
    JOYSTICK_M0_LEFT = (1 << 1),
    JOYSTICK_M0_DOWN = (1 << 2),
    JOYSTICK_M0_UP = (1 << 3),
    JOYSTICK_M0_START = (1 << 4),
    JOYSTICK_M0_SELECT = (1 << 5),
    JOYSTICK_M0_Y = (1 << 6),
    JOYSTICK_M0_B = (1 << 7),
    JOYSTICK_M1_R = (1 << 4),
    JOYSTICK_M1_L = (1 << 5),
    JOYSTICK_M1_X = (1 << 6),
    JOYSTICK_M1_A = (1 << 7),
}_eJoystickMask;



//  ---- VERA
#define SCANLINES_PER_FRAME 525
#define SCANLINE_VSYNC      480

typedef enum {
    DCSEL_0 = (0 << 1), // video modes
    DCSEL_1 = (1 << 1), // HV scale
    DCSEL_2 = (2 << 1), // FX config
    DCSEL_3 = (3 << 1), // FX line
    DCSEL_4 = (4 << 1), // FX poly
    DCSEL_5 = (5 << 1), // FX poly fill
    DCSEL_6 = (6 << 1), // FX cache
    DCSEL_VERSION = (63 << 1), // version number
}_eVeraDCSel;

typedef enum {
    ADDR_INC_0 = (0 << 4),
    ADDR_INC_1 = (1 << 4),
    ADDR_INC_2 = (2 << 4),
    ADDR_INC_4 = (3 << 4),
    ADDR_INC_8 = (4 << 4),
    ADDR_INC_16 = (5 << 4),
    ADDR_INC_32 = (6 << 4),
    ADDR_INC_64 = (7 << 4),
    ADDR_INC_128 = (8 << 4),
    ADDR_INC_256 = (9 << 4),
    ADDR_INC_512 = (10 << 4),
    ADDR_INC_40 = (11 << 4),
    ADDR_INC_80 = (12 << 4),
    ADDR_INC_160 = (13 << 4),
    ADDR_INC_320 = (14 << 4),
    ADDR_INC_640 = (15 << 4),
}_eVeraAddrInc;

typedef enum {
    COLOR_DEPTH_1BPP = 0,
    COLOR_DEPTH_2BPP = 1,
    COLOR_DEPTH_4BPP = 2,
    COLOR_DEPTH_8BPP = 3,
    COLOR_DEPTH_SPRITE_4BPP = (0 << 7),
    COLOR_DEPTH_SPRITE_8BPP = (1 << 7),
}_eVeraBitDepth;

// defining these in assembly and importing them like this is way faster than just doing it the C way
// entirely because the compiler is dumb
// yay for having to copy paste a bunch of variable names (and delete the _ prefix on each of them)
extern volatile uint8_t VERA_ADDRx_L, VERA_ADDRx_M, VERA_ADDRx_H;
extern volatile uint8_t VERA_DATA0, VERA_DATA1;
extern volatile uint8_t VERA_CTRL;
extern volatile uint8_t VERA_IEN, VERA_ISR;
extern volatile uint8_t VERA_IRQLINE_L, VERA_SCANLINE_L;

extern volatile uint8_t VERA_DC0_HSCALE, VERA_DC0_VIDEO, VERA_DC0_VSCALE, VERA_DC0_BORDER;
extern volatile uint8_t VERA_DC1_HSTART, VERA_DC1_HSTOP, VERA_DC1_VSTART, VERA_DC1_VSTOP;
extern volatile uint8_t VERA_DC2_FX_CTRL, VERA_DC2_FX_TILEBASE, VERA_DC2_FX_MAPBASE, VERA_DC2_FX_MULT;
extern volatile uint8_t VERA_DC3_FX_X_INCR_L, VERA_DC3_FX_X_INCR_H, VERA_DC3_FX_Y_INCR_L, VERA_DC3_FX_Y_INCR_H;
extern volatile uint8_t VERA_DC4_FX_X_POS_L, VERA_DC4_FX_X_POS_H, VERA_DC4_FX_Y_POS_L, VERA_DC4_FX_Y_POS_H;
extern volatile uint8_t VERA_DC5_FX_X_POS_S, VERA_DC5_FX_Y_POS_S, VERA_DC5_FX_POLY_FILL_L, VERA_DC5_FX_POLY_FILL_H;
extern volatile uint8_t VERA_DC6_FX_CACHE_L, VERA_DC6_FX_CACHE_M, VERA_DC6_FX_CACHE_H, VERA_DC6_FX_CACHE_U, VERA_DC6_FX_ACCUM_RESET, VERA_DC6_FX_ACCUM;
extern volatile uint8_t VERA_DC63_VER0, VERA_DC63_VER1, VERA_DC63_VER2, VERA_DC63_VER3;

extern volatile uint8_t VERA_L0_CFG, VERA_L0_MAPBASE, VERA_L0_TILEBASE, VERA_L0_BITMAPBASE;
extern volatile uint8_t VERA_L0_HSCROLL_L, VERA_L0_HSCROLL_H, VERA_L0_VSCROLL_L, VERA_L0_VSCROLL_H;
extern volatile uint8_t VERA_L1_CFG, VERA_L1_MAPBASE, VERA_L1_TILEBASE, VERA_L1_BITMAPBASE;
extern volatile uint8_t VERA_L1_HSCROLL_L, VERA_L1_HSCROLL_H, VERA_L1_VSCROLL_L, VERA_L1_VSCROLL_H;

extern volatile uint8_t VERA_AUDIO_CTRL, VERA_AUDIO_RATE, VERA_AUDIO_DATA;
extern volatile uint8_t VERA_SPI_DATA, VERA_SPI_CTRL;




/*
these all are on memory_map.h now for consistency
they do belong here though, so I'll comment them out instead
// all on page 1

// PSG reg 0 = 0xF9C0
#define VERA_REG_PSG_M    0xF9
#define VERA_REG_PSG_L    0xC0
// palette reg 0 = 0xFA00
#define VERA_REG_PALETTE_M    0xFA //low starts at 00
// spritte attributes reg 0 = 0xFC00
// ranges from 0x1FC00 to 0x1FFFF
#define VERA_REG_SPRITE_ATTR_M  0xFC
#define SPRITE_ATTR_SIZE        8
*/

typedef struct {
    uint8_t addr_l; // bits 12-5 of actual addr (of image data)
    union {
        uint8_t addr_h; // bits 16-13 (bits 3-0 of addr_h)
        uint8_t mode; //bit 7 of addr_h register; 0 =4 bpp, 1 = 8bpp
    };
    uint8_t x_l; // bits 7-0 of position
    uint8_t x_h; // bits 9-8
    uint8_t y_l; // bits 7-0 of position
    uint8_t y_h; // bits 9-8
    union {
        uint8_t flip; // bit 0 = h-flip, bit 1 = v-flip
        uint8_t z_depth; // bits 3-2; 0 = sprite dissabled, 1 = behind both layers, 2 = between the layers, 3 = on top of both layers
        uint8_t collision_mask; // bits 7-4
    };
    union {
        uint8_t palette_offset; // bits 3-0
        uint8_t size; // bits 5-4 = width, bits 7-6 = height; 0/1/2/3 = 8/16/32/64 px
    };
}_sSpriteAttr;


uint16_t VeraGetScanline();

//   ---- OPM (FM chip)
extern volatile uint8_t* const OPM_addr;
extern volatile uint8_t* const OPM_data;
#define OPM_STATUS      (*OPM_data)
#define OPM_FLAG_BUSY   0x80

void OpmWrite(uint8_t addr, uint8_t data);


//   ---- debug emulator api thing

#define EMU_DEBUG_1(a)  *(uint8_t*)0x9FB9 = a;
#define EMU_DEBUG_2(a)  *(uint8_t*)0x9FBA = a;
#define DEBUG_BUFFER_SIZE   256
void print_emul_debug(char* str);
extern char debug_buffer[DEBUG_BUFFER_SIZE];

#endif //X16_H