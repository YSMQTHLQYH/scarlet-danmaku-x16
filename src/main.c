#include <stdio.h>
#include <stdint.h>
#include "x16.h"
#include "zp_utils.h"
#include "input_action.h"
#include "zsm_player.h"
#include "text.h"
#include "bitmap_layer.h"

#include "profiler.h"
#include "math_tests.h"



//void Update();

void test_sprite() {
    static union { uint16_t w; uint8_t b[2]; } pos = { 0 };
    uint16_t c;
    vera->CTRL = 0x00;
    vera->DC0.VIDEO = 0x61;
    vera->DC0.HSCALE = 64;
    vera->DC0.VSCALE = 64;
    vera->ADDRx_H = 0x11;
    vera->ADDRx_M = MEM_VRAM_1_VERA_SPRITE_ATTR_M;
    vera->ADDRx_L = 0;
    c = (MEM_4BPP_FONT_1_ADDR_M) << 3;
    c += 0xE9;
    vera->DATA0 = (uint8_t)c; // addr 12-5
    vera->DATA0 = (uint8_t)(c >> 8); // addr 16-13
    pos.w += 0x0280;
    vera->DATA0 = pos.b[1];//x
    vera->DATA0 = 0;
    vera->DATA0 = 0x7F;//y
    vera->DATA0 = 0;
    vera->DATA0 = 0x0C;//z, flip
    vera->DATA0 = 0x00;//size, palete
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

uint8_t show_debug = 1;
uint8_t last_tick = 0, current_tick = 0;
void main() {
    uint16_t wait_count = 0, lag_count = 0;
    uint8_t test_number = 0;
    char* selected_song = 0;
    uint8_t song_name_length = 0;

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

        // segment 0: music
        ZsmTick();
        ProfilerEndSegment();

        // segment 1: input
        HandleInputActions();
        //show inputs on screen
        if (IsActionJustPressed(ACTION_DEBUG)) {
            if (!show_debug) {
                show_debug = 1;
                Print2BppBitmapStr("0", bitmap_front_buffer, 67, 52);
                Print2BppBitmapStr("1", bitmap_front_buffer, 67, 64);
            } else { show_debug = 0; BitmapFillRect(bitmap_front_buffer, 0, 52, 50, 26, 22); }
        }
        if (show_debug) {
            JoystickDrawToBitmap(0, bitmap_front_buffer, 70, 50);
            JoystickDrawToBitmap(1, bitmap_front_buffer, 70, 62);
            InputActionDrawToBitmap(bitmap_front_buffer, 52, 66);
        }
        ProfilerEndSegment();

        // segment 2: placeholder dummy

        ProfilerEndSegment();

        // segment 3: placeholder dummy
        ProfilerEndSegment();
        test_sprite();

        // segment 4: math
        MathTest(test_number);
        ProfilerEndSegment();

        // segment 5: profiler print
        StrUint8Hex(lag_count, str_lag);
        StrUint16Hex(wait_count, str_wait_count);
        PrintSpriteStr(str_lag, 1, 288, 202, 0);
        PrintSpriteStr(str_wait_count, 2, 272, 210, 0);

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

static void Init() {
    uint8_t s;
    //  ---- IRQ
    vera->IEN |= 0x01; //enables VSYNC interrupt if it wasn't already for some reason

    //  ---- text
    s = HijackRomCharset(12, 4, 2);
    s += HijackRomCharset(12, 2, 3);
    KernalScreenSetCharset(3);
    if (s == 0) {
        printf("Hijacked KERNAL font successfully\n");
    } else {
        printf("Failed to hijack KERNAL font\n");
    }

    InputActionInit(0);

    //  ---- graphics
    vera->CTRL = 0x00;
    vera->DC0.VIDEO = 0x61;
    vera->DC0.HSCALE = 64;
    vera->DC0.VSCALE = 64;

    BitmapInit(0);
    PrintProfilerBitmapFrame(0);
}

char prf_str[] = "----";
static void PrintPrevProfBlock() {
    uint8_t i, x, a;
    _uConv16 t;
    for (i = 0; i < profiler_previous_segment_count; i++) {
        t.u16 = profiler_segment_previous[i];
        if (t.u8_h == 0) {
            // skips the top two digits if they are 0
            a = 2;
            x = 16;
        } else {
            a = 0;
            x = 0;
        }
        StrUint16Hex(t.u16, &prf_str[0]);
        PrintSpriteStr(&prf_str[a], i + 3, 272 + x, 100 + (i << 3), 0);
        //Print2BppBitmapStr(prf_str, bitmap_front_buffer, 62, 170 + (i << 3));
    }
    // total
    StrUint16Hex(profiler_previous_total, prf_str);
    PrintSpriteStr(prf_str, i + 3, 272, 104 + (i << 3), 0);

}

#define SEGMENT_COUNT   6
char* prf_frame_str[] = {
    "music:",
    "input:",
    "seg 2:",
    "seg 3:",
    "math :",
    "text :",
};

static void PrintProfilerBitmapFrame(uint8_t buffer_n) {
    uint8_t i;
    Print2BppBitmapStr("profiler time", buffer_n, 52, 80);
    Print2BppBitmapStr("(scanlines)", buffer_n, 54, 88);

    for (i = 0; i < SEGMENT_COUNT; i++) {
        Print2BppBitmapStr(prf_frame_str[i], buffer_n, 56, 100 + (i << 3));
    }
    Print2BppBitmapStr("total:", buffer_n, 56, 104 + (i << 3));
    Print2BppBitmapStr("1 frame=020d", buffer_n, 52, 112 + (i << 3));

    Print2BppBitmapStr("lag frame:", buffer_n, 48, 202);
    Print2BppBitmapStr("spare cpu:", buffer_n, 48, 210);

    // joystick
    if (show_debug) {
        Print2BppBitmapStr("0", bitmap_front_buffer, 67, 52);
        Print2BppBitmapStr("1", bitmap_front_buffer, 67, 64);
    }
}

