#include <stdio.h>
#include <stdint.h>
#include "x16.h"
#include "zsm_player.h"

#include "math_tests.h"



void Update();

void test_sprite() {
    static union { uint16_t w; uint8_t b[2]; } pos = { 0 };
    vera->CTRL = 0x00;
    vera->DC0.VIDEO = 0x61;
    vera->DC0.HSCALE = 64;
    vera->DC0.VSCALE = 64;
    vera->ADDRx_H = 0x11;
    vera->ADDRx_M = VERA_REG_SPRITE_ATTR_M;
    vera->ADDRx_L = 0;
    vera->DATA0 = 0b10000001;
    vera->DATA0 = 0x0F;
    pos.w += 0x0080;
    vera->DATA0 = pos.b[1];//x
    vera->DATA0 = 0;
    vera->DATA0 = 0x7F;//y
    vera->DATA0 = 0;
    vera->DATA0 = 0x0C;
    vera->DATA0 = 0x02;
}


uint8_t last_tick = 0, current_tick = 0;
void main() {
    uint16_t wait_count = 0, lag_count = 0;
    uint8_t test_counter = 0, test_number = 0;
    char* selected_song = 0;
    uint8_t song_name_length = 0;
    char test_song_file_name_1[] = "test assets/greenmotor.zsm";
#define SONG_NAME_1_LENGTH    12 + 14
    char test_song_file_name_2[] = "test assets/splashwave.zsm";
#define SONG_NAME_2_LENGTH    12 + 14
    char test_song_file_name_3[] = "test assets/all ur base.zsm";
#define SONG_NAME_3_LENGTH    12 + 15

    printf("Hello, World!\n");


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
            printf("\nenter \'y\' to play song, anything else to skip\n");
            if (getchar() == 'y') {
                printf("playing\n");
                ZsmPlay();
            }
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
    // jsr = Jump to Subroutine
    // This is the memory location of the RDTIM (Read Timer) Kernal Function
    asm("jsr %w", KERNAL_RDTIM);
    // sta = Store Accumulator
    // This copies the value from the A register to a memory location
    // In this case its the memory location of our "start" variable
    asm("sta %v", last_tick);


    //  --- main loop
    while (1) {
        //Update();
        ZsmTick();
        test_sprite();

        switch (test_counter) {
        case 0:

            break;
        case 1:
            /* code */
            break;
        case 2:
            MathTest(test_number);
            break;

        case 3:
            //printf("lag: %i spare: %i\n", lag_count, wait_count);
            snprintf(debug_buffer, 64, "lag: %i spare: %i        \n", lag_count, wait_count);
            print_emul_debug(debug_buffer);
            break;

        default:
            test_counter = 0;
        }
        test_counter++;


        // wait for next frame


        // Now that we have the start time
        // Get the "next" time and keep looping until it changes
        asm("jsr %w", KERNAL_RDTIM);
        asm("sta %v", current_tick);
        lag_count = current_tick - last_tick;
        wait_count = 0;
        last_tick += lag_count;
        while (last_tick == current_tick) {
            asm("jsr %w", KERNAL_RDTIM);
            asm("sta %v", current_tick);
            wait_count += 1;
        }
        last_tick = current_tick;

    }

}

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




