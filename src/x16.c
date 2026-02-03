#include "x16.h"
#include "zp_utils.h"


static uint8_t a0, a1, a2;

//  ---- memory
// defined in asm now (i hope)
//volatile uint8_t* const ram_bank = (uint8_t*)0x0000;
//volatile uint8_t* const rom_bank = (uint8_t*)0x0001;


//  ---- IRQ
void IrqSetVector(void* func_pointer) {

    asm("sei"); // dissables interrupts
    *KERNAL_CINV = func_pointer;
    asm("cli"); // re enables interrupts
}
void* IrqGetVector() {
    void* v;
    asm("sei"); // dissables interrupts, might be overkill for just reading the address but better be safe
    v = *KERNAL_CINV;
    asm("cli"); // re enables interrupts
    return v;
}

//   ---- VERA
// doing it like this was bad duz compiler dumb
//volatile _sVeraReg* const vera = (void*)0x9F20;

uint16_t VeraGetScanline() {
    _uConv16 sl;
    asm("lda $9F28"); // A = vera->scanline_l
    asm("sta %v", a0); // low byte of sl (I hope) = A
    // the BIT instruction sets flags for bits 7 and 6 specifically
    // ignoring any mask so we don't have to load anything into A
    // weird but ok
    asm("bit $9F26"); // Z = ((vera->IEN & A) == 0), N = bit 7 of vera->IEN, V = bit 6 of vera->IEN 
    asm("lda #00"); // i forgor this
#ifndef __INTELLISENSE__
    asm("bvc %g", get_scanline_end);  // branches if V == 0
#endif // vscode this is getting old
    asm("lda #01");
get_scanline_end:
    asm("sta %v", a1);
    sl.u8_l = a0;
    sl.u8_h = a1;
    return sl.u16;
}


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

// moving this to a separate function actually made it run much faster, interesting
uint8_t KernalReadTimer() {
    asm("jsr %w", KERNAL_RDTIM);
    asm("sta %v", a0);
    return a0;
}

void KernalScreenSetCharset(uint8_t charset) {
    a0 = charset;
    asm("lda %v", a0);
    asm("jsr %w", KERNAL_SCREEN_SET_CHARSET);
}

uint8_t KernalJoystickGet(uint8_t joystick_n, uint8_t* mask_0, uint8_t* mask_1) {
    a0 = joystick_n;
    asm("lda %v", a0);
    asm("jsr %w", KERNAL_JOYSTICK_GET);
    asm("sta %v", a0); // byte_0 
    asm("stx %v", a1); // byte_1
    asm("sty %v", a2); // joystick_present
    if (a2) { *mask_0 = 0xFF; *mask_1 = 0xFF; return 1; } // joystick NOT present
    // joystick present
    *mask_0 = a0;
    *mask_1 = a1;
    return 0;
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
#ifndef __INTELLISENSE__ // disable for intellisense to avoid false error // comment on the comment, i should reword that
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

