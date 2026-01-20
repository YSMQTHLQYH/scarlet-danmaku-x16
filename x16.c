#include "x16.h"

typedef union {
    struct { uint8_t u8_l; uint8_t u8_h; };
    uint16_t u16;
} _uConv16;

//  ---- memory
volatile uint8_t* const ram_bank = (uint8_t*)0x0000;
volatile uint8_t* const rom_bank = (uint8_t*)0x0001;



//   ---- VERA
volatile _sVeraReg* const vera = (void*)0x9F20;




//  ---- OPM
volatile uint8_t* const OPM_addr = (uint8_t*)0x9F40;
volatile uint8_t* const OPM_data = (uint8_t*)0x9F41;

void OpmWrite(uint8_t addr, uint8_t data) {
    while (OPM_STATUS & OPM_FLAG_BUSY) {}//waste time while waiting for OPM to finish it's thing
    // OPM is ready to write to
    *OPM_addr = addr;
    // waste a bit more time for OPM to process the addreess
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    // ok now we "should" be ready
    *OPM_data = data;
}


//  ---- assorted KERNAL functions
static uint8_t a0, a1, a2;
void KernalScreenSetCharset(uint8_t charset) {
    a0 = charset;
    asm("lda %v", a0);
    asm("jsr %w", KERNAL_SCREEN_SET_CHARSET);
}

//  ---- files


uint8_t load_file(_sFileLoadCtx* ctx) {
    // file name
    _uConv16 w;
    w.u16 = (uint16_t)ctx->filename;
    a0 = w.u8_l;
    a1 = w.u8_h;
    a2 = ctx->name_lenght;

    asm("ldx %v", a0);
    asm("ldy %v", a1);
    asm("lda %v", a2);
    asm("jsr %w", KERNAL_SETNAM);

    // file parameters
    a2 = ctx->header_mode;
    asm("lda #$01"); // file number 1
    asm("ldx #$08"); // device number 8 (sdcard)
    asm("ldy %v", a2);
    asm("jsr %w", KERNAL_SETLFS);

    // load file
    w.u16 = (uint16_t)ctx->dest_addr;
    a0 = w.u8_l;
    a1 = w.u8_h;
    a2 = ctx->target_mode;
    asm("ldx %v", a0);
    asm("ldy %v", a1);
    asm("lda %v", a2);
    asm("jsr %w", KERNAL_LOAD);

    // error handling
    // carry flag set if error
#ifndef __INTELLISENSE__ // disable for intellisense to avoid false error
    asm("bcs %g", jmp_load_error); // branches if carry set (error) // ok this works but vscode things this is an error
#endif //vscode you stoopid
    // no error, file loaded successfully
    asm("stx %v", a0);
    asm("sty %v", a1);
    w.u8_l = a0;
    w.u8_h = a1;
    ctx->file_end = (void*)w.u16;

    return 1;
jmp_load_error:
    // return error code
    asm("sta %v", a2);
    ctx->error_code = a2;
    return 0;
}


// ---- emulator debugger

char debug_buffer[DEBUG_BUFFER_SIZE] = { 0 };
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

