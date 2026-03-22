#include "sfx_player.h"

#include "x16.h"
#include "zp_utils.h"

#define MAX_SFX_PLAYING 4
#define SFX_PSG_VOICE_START (16 - MAX_SFX_PLAYING)
#define SFX_PSG_START_L     (MEM_VRAM_1_VERA_PSG_L + (SFX_PSG_VOICE_START * 4))
//psg voice registers are from 0xF9C0 to 0xF9FF, no need to touch high byte
#define SFX_PSG_START_M     MEM_VRAM_1_VERA_PSG_M

typedef struct {
    uint8_t sfx_id;
    uint8_t ticks_left;
    uint8_t waveform;
}_sSfxPlaying;
_sSfxPlaying sfx_playing[MAX_SFX_PLAYING] = { 0 };

//TODO: make this for real
void SfxTick() {
    uint8_t i;
    VERA_CTRL = 0;
    VERA_ADDRx_H = ADDR_INC_1 + 1;
    VERA_ADDRx_M = SFX_PSG_START_M;
    VERA_ADDRx_L = SFX_PSG_START_L;

    for (i = 0; i < MAX_SFX_PLAYING; i++) {
        if (sfx_playing[i].sfx_id == 0) {
            VERA_DATA0 = 0;
            VERA_DATA0 = 0;
            VERA_DATA0 = 0;
            VERA_DATA0 = 0;
            continue;
        }
        if (--sfx_playing[i].ticks_left == 0) {
            //sfx finished
            sfx_playing[i].sfx_id = 0;
            VERA_DATA0 = 0;
            VERA_DATA0 = 0;
            VERA_DATA0 = 0;
            VERA_DATA0 = 0;
            continue;
        }
        VERA_DATA0 = 0x9D; //frequency_l
        VERA_DATA0 = 0x04; //frequency_h
        VERA_DATA0 = 0xF0; //RL, volume
        VERA_DATA0 = sfx_playing[i].waveform; //waveform, PW/XOR
    }
}

void SfxPlay(uint8_t sfx_id) {
    uint8_t i;
    for (i = 0; i < MAX_SFX_PLAYING; i++) {
        if (sfx_playing[i].sfx_id == 0) goto found;
    }
    return;
found:
    sfx_playing[i].sfx_id = sfx_id;
    sfx_playing[i].waveform = sfx_id;
    sfx_playing[i].ticks_left = 30;
}