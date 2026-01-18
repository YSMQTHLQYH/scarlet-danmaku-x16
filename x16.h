#ifndef X16_H
#define X16_H

#include <stdint.h>


//to call:
//asm("jsr %w", macro);
#define KERNAL_RDTIM    0xFFDE

#define KERNAL_SETNAM   0xFFBD
#define KERNAL_SETLFS   0xFFBA
#define KERNAL_LOAD     0xFFD5


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
#define HIGH_RAM_8(i)   *(uint8_t*)(HIGH_RAM_START + i)
#define HIGH_RAM_16(i)  *(uint16_t*)(HIGH_RAM_START + i + i)
extern volatile uint8_t* const ram_bank;
#define SET_RAM_BANK(bank)  (*ram_bank) = bank;

//I don't think it's actually called "high rom" but whatever it makes sense
#define ROM_BANK_SIZE   0x4000
#define HIGH_ROM_START  0xC000
#define HIGH_ROM_8(i)   *(uint8_t*)(HIGH_ROM_START + i)
#define HIGH_ROM_16(i)  *(uint16_t*)(HIGH_ROM_START + i + i)
extern volatile uint8_t* const rom_bank;
#define SET_ROM_BANK(bank)  (*rom_bank) = bank;



//  ---- vera

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

extern volatile _sVeraReg* const vera;


//   ---- debug emulator api thing

#define EMU_DEBUG_1(a)  *(uint8_t*)0x9FB9 = a;
#define EMU_DEBUG_2(a)  *(uint8_t*)0x9FBA = a;
#define DEBUG_BUFFER_SIZE   256
void print_emul_debug(char* str);
extern char debug_buffer[DEBUG_BUFFER_SIZE];

#endif //X16_H