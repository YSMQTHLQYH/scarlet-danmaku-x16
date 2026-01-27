#ifndef X16_H
#define X16_H

#include <stdint.h>


//to call:
//asm("jsr %w", macro);
#define KERNAL_RDTIM    0xFFDE

#define KERNAL_SETNAM   0xFFBD
#define KERNAL_SETLFS   0xFFBA
#define KERNAL_LOAD     0xFFD5

#define KERNAL_SCREEN_SET_CHARSET   0xFF62

// ---- KERNAL functions
uint8_t KernalReadTimer();
void KernalScreenSetCharset(uint8_t charset);

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
extern volatile uint8_t* const ram_bank;
#define SET_RAM_BANK(bank)  (*ram_bank) = bank

//I don't think it's actually called "high rom" but whatever it makes sense
#define ROM_BANK_SIZE   0x4000
#define HIGH_ROM_START  0xC000
// i'll implement high rom end if I ever use it
#define HIGH_ROM_8(i)   *(uint8_t*)(HIGH_ROM_START + i)
#define HIGH_ROM_16(i)  *(uint16_t*)(HIGH_ROM_START + i + i)
extern volatile uint8_t* const rom_bank;
#define SET_ROM_BANK(bank)  (*rom_bank) = bank


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



//  ---- vera
#define SCANLINES_PER_FRAME 525
#define SCANLINE_VSYNC      480

typedef struct {
    uint8_t CONFIG;
    uint8_t MAPBASE;
    union { uint8_t TILEBASE; uint8_t BITMAPBASE; };
    uint8_t HSCROLL_L;
    union { uint8_t HSCROLL_H; uint8_t BITMAP_COLOR_OFFSET; };
    uint8_t VSCROLL_L;
    uint8_t VSCROLL_H;
}_sVeraLayerReg;

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

extern volatile _sVeraReg* const vera;

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