#include "x16.h"

volatile uint8_t* const ram_bank = (uint8_t*)0x0000;
volatile uint8_t* const rom_bank = (uint8_t*)0x0001;

volatile _sVeraReg* const vera = (void*)0x9F20;

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

