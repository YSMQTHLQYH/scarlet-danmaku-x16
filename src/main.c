#include <stdio.h>
#include <stdint.h>
#include "x16.h"
#include "zp_utils.h"
#include "input_action.h"
#include "graphics_utils.h"
#include "bitmap_layer.h"
#include "sprite_manager.h"
#include "bullet.h"

#include "zsm_player.h"
#include "text.h"

#include "profiler.h"
#include "math_tests.h"




void test_sprite() {
    static uint16_t x = 100, y = 100;
    static uint8_t b = 0, frame = 0, p_obj = 0;
    uint8_t j, c;
    uint16_t i;
    _uConv16 addr;
    _sFileLoadCtx pl = { 0 };
    x16_ram_bank = MEM_BANK_SPRITE_TABLE;
    // fill sprite graphics
    addr.w = MEM_VRAM_0_UNUSED_2_START;
    VERA_CTRL = 0;
    VERA_ADDRx_H = 0x10;
    VERA_ADDRx_M = addr.h;
    VERA_ADDRx_L = addr.l;

    if (b == 0) {
        b = 1;
        for (j = 0; j < 16; j++) {
            for (i = 0; i < 256; i++) {
                c = (uint8_t)i;
                VERA_DATA0 = c;
            }
        }
        pl.filename = "test assets/testplayer.bin";
        pl.name_lenght = 12 + 14;
        pl.dest_addr = (void*)MEM_VRAM_1_KERNAL_CHARSET_START;
        pl.header_mode = FILE_LOAD_HEADERLESS;
        pl.target_mode = FILE_LOAD_VRAM_P1;
        if (!load_file(&pl)) {
            EMU_DEBUG_1(0xFA);
            EMU_DEBUG_1(pl.error_code);
        }
        pl.filename = "test assets/programmerart.bin";
        pl.name_lenght = 12 + 17;
        pl.dest_addr = (void*)(MEM_VRAM_0_UNUSED_2_START + 4096);
        pl.header_mode = FILE_LOAD_HEADERLESS;
        pl.target_mode = FILE_LOAD_VRAM_P0;
        if (!load_file(&pl)) {
            EMU_DEBUG_1(0xFA);
            EMU_DEBUG_1(pl.error_code);
        }

        p_obj = CreateSpriteObject(SPR_PRIORITY_HIGH, 1, 1, 0b1010);
        addr.w = MEM_VRAM_1_KERNAL_CHARSET_START >> 5;
        addr.h |= 0x08; // addr bit 16
        SpriteObjectSetAddr(p_obj, addr.h | 0x00, &addr.l);
        SpriteObjectSetZFlip(p_obj, 0x0C);
        SpriteObjectSetSizePalette(p_obj, 0b1010, 15);

    }
    if (IsActionPressed(ACTION_LEFT))x--;
    if (IsActionPressed(ACTION_RIGHT))x++;
    if (IsActionPressed(ACTION_UP))y--;
    if (IsActionPressed(ACTION_DOWN))y++;
    SpriteObjectSetPosition(p_obj, x, y);


    // set up hardware sprite
    addr.w = MEM_VRAM_1_VERA_SPRITE_ATTR_START;
    VERA_ADDRx_H = 0x11;
    VERA_ADDRx_M = addr.h;
    VERA_ADDRx_L = addr.l;

    // sprite programmerart
    addr.w = (MEM_VRAM_0_UNUSED_2_START + 4096);
    addr.w += ((frame++ >> 3) << 8);
    addr.w >>= 5;
    VERA_DATA0 = addr.l; // addr 12-5
    VERA_DATA0 = addr.h | 0x80; // addr 16-13
    VERA_DATA0 = 0x20;//x
    VERA_DATA0 = 0;
    VERA_DATA0 = 0x100;//y
    VERA_DATA0 = 0;
    VERA_DATA0 = 0x0C;//z, flip
    VERA_DATA0 = 0x50;//size, palete

    return;


}

static void Init();
static void PrintPrevProfBlock();
static void PrintProfilerBitmapFrame(uint8_t buffer_n);



char test_song_file_name_1[] = "test assets/greenmotor.zsm";
#define SONG_NAME_1_LENGTH    12 + 14
char test_song_file_name_2[] = "test assets/splashwave.zsm";
#define SONG_NAME_2_LENGTH    12 + 14
char test_song_file_name_3[] = "test assets/all ur base.zsm";
#define SONG_NAME_3_LENGTH    12 + 15

char str_lag[] = "--";
char str_wait_count[] = "----";
uint8_t lag_so = 0, wc_so = 0;


uint8_t show_debug = 1;
uint8_t last_tick = 0, current_tick = 0;
void main() {
    uint16_t wait_count = 0, lag_count = 0;
    uint8_t test_number = 0;
    char* selected_song = 0;
    uint8_t song_name_length = 0;
    uint8_t i;

    // emulator debug  mode
    *(uint8_t*)0x9FB0 = 1;

    //  ---- IRQ
    kernal_irq_func_ptr = IrqGetVector();
    if (kernal_irq_func_ptr == 0) {
        //  ---- something went wrong somehow
        print_emul_debug("FAILED TO READ KERNAL VECTOR");
        while (1) { asm("nop"); }; // might as well just crash the program at this point
    }
    IrqSetVector(CustomIrq);
    // IRQ set



    printf("enter 1 - 3 to play load, anything else to skip\n");
    switch (getchar()) {
    case '1':
        selected_song = test_song_file_name_1;
        song_name_length = SONG_NAME_1_LENGTH;
        break;
    case '2':
        selected_song = test_song_file_name_2;
        song_name_length = SONG_NAME_2_LENGTH;
        break;
    case '3':
        selected_song = test_song_file_name_3;
        song_name_length = SONG_NAME_3_LENGTH;
        break;
    }

    if (selected_song != 0) {
        if (ZsmLoad(selected_song, song_name_length, 8)) {
            //success
            printf("\nloaded song from bank: %u to bank: %u\n", zsm.start_bank, zsm.end_bank);
            ZsmPlay();
        } else {
            //error
            printf("\nzsm file load error: %u\n", zsm.load_error_code);
        }
    }


    printf("Enter test number:\n");
    if (scanf("%u", &test_number)) {
        if (test_number >= MATH_TEST_COUNT) test_number = 0;
    }
    printf("Test #: %u", test_number);


    //    ---- done with placeholder print stuff

    MathTestsinit();
    Init();

    // setup for timer
    last_tick = frame_count;

    //  --- main loop
    while (1) {
        ProfilerBeginBlock();
        // segment 0: clear buffer
        BitmapSwapBuffers();
        //BitmapLayerFillRect(bitmap_back_buffer, 1, 0, 0, 224, 240);
        BitmapLayerClearGameArea(bitmap_back_buffer, 1);
        ProfilerEndSegment();

        // segment 1: music
        ZsmTick();
        ProfilerEndSegment();

        // segment 2: input
        HandleInputActions();
        //show inputs on screen
        if (IsActionJustPressed(ACTION_DEBUG)) {
            if (!show_debug) {
                show_debug = 1;
                Print4BppBitmapStr("0", 0, 134, 50);
                Print4BppBitmapStr("1", 0, 134, 62);
                Print4BppBitmapStr("0", 1, 134, 50);
                Print4BppBitmapStr("1", 1, 134, 62);
            } else {
                show_debug = 0;
                BitmapLayerFillRect(0, 0, 224, 48, 76, 22);
                BitmapLayerFillRect(1, 0, 224, 48, 76, 22);
            }
        }
        if (show_debug) {
            JoystickDrawToBitmap(0, bitmap_back_buffer, 140, 48);
            JoystickDrawToBitmap(1, bitmap_back_buffer, 140, 60);
            InputActionDrawToBitmap(bitmap_back_buffer, 118, 64);
        }
        ProfilerEndSegment();

        // segment 3: placeholder dummy
        test_sprite();
        SpriteManagerWriteChanges();
        ProfilerEndSegment();

        // segment 4: placeholder dummy
        for (i = 0; i < 16; i++) {
            BitmapSetPixel(bitmap_back_buffer, i + (i << 4), 50 + i, 100);
            BitmapSetPixel(bitmap_back_buffer, i + (i << 4), 55 + i, 101);
            BitmapSetPixel(bitmap_back_buffer, i + (i << 4), 60 + i, 102);
            BitmapSetPixel(bitmap_back_buffer, i + (i << 4), 65 + i, 103);
        }
        ProfilerEndSegment();

        // segment 5: placeholder dummy
        ProfilerEndSegment();

        // segment 6: math
        MathTest(test_number);
        ProfilerEndSegment();

        // segment 7: profiler print
        StrUint8Hex(lag_count, str_lag);
        StrUint16Hex(wait_count, str_wait_count);
        PrintSpriteStr(lag_so, str_lag);
        PrintSpriteStr(wc_so, str_wait_count);

        PrintPrevProfBlock();
        ProfilerEndBlock();


        //snprintf(debug_buffer, 64, "lag: %i spare: %i        \n", lag_count, wait_count);
        //print_emul_debug(debug_buffer);


        // ----     wait for next frame
        // Get the "next" time and keep looping until it changes
        current_tick = frame_count;
        lag_count = current_tick - last_tick;
        wait_count = 0;
        last_tick += lag_count;
        while (last_tick == current_tick) {
            current_tick = frame_count;
            wait_count += 1;
        }
        last_tick = current_tick;
    }

}

uint16_t color_palette[] = {
    0x2A3, 0xBD2, 0xEFC, 0x6EF,
    0xFD4, 0xFAD, 0x2EC, 0x0AC,
    0xF93, 0xE36, 0xC3B, 0x168,
    0xE53, 0xA23, 0x739, 0x214,
};
static void Init() {
    uint8_t s;
    //  ---- IRQ
    VERA_IEN |= 0x01; //enables VSYNC interrupt if it wasn't already for some reason

    //  ---- text
    s = HijackRomCharset(12, 4, 3);
    s += HijackRomCharset(12, 2, 3);
    KernalScreenSetCharset(3);
    if (s == 0) {
        printf("Hijacked KERNAL font successfully\n");
    } else {
        printf("Failed to hijack KERNAL font\n");
    }


    //  ---- bullets
    s = BulletManagerInit();
    if (s == 0) {
        printf("Loaded bullet lookup table\n");
    } else {
        printf("Failed to load lookup table\n");
    }

    InputActionInit(0);

    //  ---- sprites
    SpriteManagerInit();

    lag_so = CreateSpriteStr(SPR_PRIORITY_HIGH, 2, 0x0C, 3);
    wc_so = CreateSpriteStr(SPR_PRIORITY_HIGH, 4, 0x0C, 3);
    SpriteObjectSetPosition(lag_so, 288, 210);
    SpriteObjectSetPosition(wc_so, 272, 226);

    //  ---- graphics
    VERA_CTRL = 0x00;
    VERA_DC0_VIDEO = 0x61;
    VERA_DC0_HSCALE = 64;
    VERA_DC0_VSCALE = 64;

    BitmapInit(0);
    PrintProfilerBitmapFrame(0);
    PrintProfilerBitmapFrame(1);

    SetColorPalette(15, color_palette);
}

#define SEGMENT_COUNT   8
char* prf_frame_str[] = {
    "clear:",
    "music:",
    "input:",
    "seg 3:",
    "seg 4:",
    "seg 5:",
    "math :",
    "text :",
};

char prf_str[] = "----";
static void PrintPrevProfBlock() {
    static uint8_t created = 0;
    static uint8_t str_obj[SEGMENT_COUNT + 1] = { 0 };
    uint8_t i;
    _uConv16 t;
    if (!created) {
        for (i = 0; i < SEGMENT_COUNT; i++) {
            str_obj[i] = CreateSpriteStr(SPR_PRIORITY_HIGH, 4, 0x0C, 3);
            SpriteObjectSetPosition(str_obj[i], 272, 100 + (i << 3));
        }
        str_obj[i] = CreateSpriteStr(SPR_PRIORITY_HIGH, 4, 0x0C, 3);
        SpriteObjectSetPosition(str_obj[i], 272, 104 + (i << 3));
        created = 1;
    }
    for (i = 0; i < SEGMENT_COUNT; i++) {
        t.w = profiler_segment_previous[i];
        StrUint16Hex(t.w, prf_str);
        PrintSpriteStr(str_obj[i], prf_str);
        //Print2BppBitmapStr(prf_str, bitmap_front_buffer, 62, 170 + (i << 3));
    }
    // total
    StrUint16Hex(profiler_previous_total, prf_str);
    PrintSpriteStr(str_obj[i], prf_str);

}


static void PrintProfilerBitmapFrame(uint8_t buffer_n) {
    uint8_t i;
    Print4BppBitmapStr("\"profiler\"", buffer_n, 112, 72);
    Print4BppBitmapStr("time", buffer_n, 112, 80);
    Print4BppBitmapStr("(scanlines)", buffer_n, 112, 88);

    for (i = 0; i < SEGMENT_COUNT; i++) {
        Print4BppBitmapStr(prf_frame_str[i], buffer_n, 112, 100 + (i << 3));
    }
    Print4BppBitmapStr("total:", buffer_n, 112, 104 + (i << 3));
    Print4BppBitmapStr("sl/frame:", buffer_n, 112, 112 + (i << 3));
    Print4BppBitmapStr("020d", buffer_n, 136, 120 + (i << 3));

    Print4BppBitmapStr("lag frame:", buffer_n, 112, 202);
    Print4BppBitmapStr("spare cpu:", buffer_n, 112, 218);

    // joystick
    if (show_debug) {
        Print4BppBitmapStr("0", 0, 134, 50);
        Print4BppBitmapStr("1", 0, 134, 62);
        Print4BppBitmapStr("0", 1, 134, 50);
        Print4BppBitmapStr("1", 1, 134, 62);
    }
}

