#include <stdio.h>
#include <stdint.h>
#include "x16.h"
#include "zp_utils.h"
#include "zsm_player.h"
#include "text.h"

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
    vera->ADDRx_M = VERA_REG_SPRITE_ATTR_M;
    vera->ADDRx_L = 0;
    c = (FONT_4BPP_START) << 3;
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


char test_song_file_name_1[] = "test assets/greenmotor.zsm";
#define SONG_NAME_1_LENGTH    12 + 14
char test_song_file_name_2[] = "test assets/splashwave.zsm";
#define SONG_NAME_2_LENGTH    12 + 14
char test_song_file_name_3[] = "test assets/all ur base.zsm";
#define SONG_NAME_3_LENGTH    12 + 15

char str_lag_count[] = "lag--,----";

uint8_t last_tick = 0, current_tick = 0;
void main() {
    uint16_t wait_count = 0, lag_count = 0;
    uint8_t test_number = 0;
    char* selected_song = 0;
    uint8_t song_name_length = 0;

    //debug
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

    printf("Hello, World!\n");

    Init();

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


    //snprintf(debug_buffer, 255, "hello");
    //print_emul_debug(debug_buffer);
    MathTestsinit();

    // setup for timer
    last_tick = frame_count;

    //  --- main loop
    while (1) {
        ProfilerBeginBlock();
        //Update();
        ZsmTick();
        ProfilerEndSegment();

        test_sprite();
        ProfilerEndSegment();

        MathTest(test_number);
        ProfilerEndSegment();

        StrUint8Hex(lag_count, &str_lag_count[3]);
        StrUint16Hex(wait_count, &str_lag_count[6]);
        PrintSpriteStr(str_lag_count, 1, 216, 216, 0);

        PrintPrevProfBlock(ProfilerEndBlock());



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
    KernalScreenSetCharset(3);
    if (s == 0) {
        printf("Hijacked KERNAL font successfully\n");
    } else {
        printf("Failed to hijack KERNAL font\n");
    }
}

char prf_str_0[] = "----";
char prf_str_1[] = "t:----";
static void PrintPrevProfBlock(uint8_t count) {
    uint8_t i, j;
    /*
    uint16_t* block = profiler_previous_times;
    for (i = 0; i < profiler_previous_block_segment_count; i++) {
        prf_str_0[0] = i + '0';
        StrUint16Hex(block[i], &prf_str_0[0]);
        PrintSpriteStr(prf_str_0, 0, 248, 170 + (i << 3), 0);
    }
    StrUint16Hex(profiler_previous_block_total_time, &prf_str_1[2]);
    PrintSpriteStr(prf_str_1, 0, 232, 170 + (i << 3), 0);
    */

    for (i = 0; i < count; i++) {
        prf_str_0[0] = i + '0';
        StrUint16Hex(profiler_segment[i], &prf_str_0[0]);
        PrintSpriteStr(prf_str_0, i + 2, 248, 170 + (i << 3), 0);

    }
}
/*
void Update() {
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
*/



