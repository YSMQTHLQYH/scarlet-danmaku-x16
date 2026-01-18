#ifndef ZSM_PLAYER_H
#define ZSM_PLAYER_H

#include <stdint.h>

typedef enum {
    ZSM_STATE_NO_SONG_LOADED,
    ZSM_STATE_STOPPED,     // reset to beginning of song
    ZSM_STATE_PAUSED,
    ZSM_STATE_PLAYING
}_eZsmPlayerState;



// state machine only playing one song at a time
// to change songs just reset it instead of making a new one
// for the sake of implementation, the song has to be stored exclusively on high ram
typedef struct {
    //_sZsmHeader* song_header; probably replaced with some macro
    _eZsmPlayerState state;
    uint8_t start_bank;
    uint8_t end_bank;

    // current byte being read
    // the actual memory adress, so index_h ranges only from 0xA0 to 0xBF
    union {
        struct { uint8_t index_l; uint8_t index_h; };
        uint16_t index;
    };
    uint8_t index_bank;

    //uint8_t tick_rate //(for now?) only support for 60Hz

    uint8_t delay_ticks_left; // first decrement, then read if == 0
}_sZsmPlayer;
// singleton just to make the functions nicer to type, not like we can have two songs at once...
extern _sZsmPlayer zsm;


// values are little endian (byte[0] is low bye, byte[1] is second lowest, ect)
// all offsets are relative to start of header
typedef struct {
    char magic_header[2]; // "zm", 0x7A 0x6D
    uint8_t version;
    //no "uint24_t" sigh
    uint8_t loop_point[3]; // offset from start to loop point, 0 = no loop
    uint8_t PCM_offset[3]; // offset from start to beginning of PCM table, 0 = no PCM data
    // bits correspond to channels used by the music file
    uint8_t FM_channel_mask;
    uint16_t PSG_channel_mask;

    uint16_t tick_rate; // song delay rate in Hz (usually 60)
    uint16_t reserved;  // reserved for future version of ZSM, should be 0 for now
}_sZsmHeader;


// call from main loop on every frame
void ZSM_tick();

// returns 0 on load error, 1 on success
// does not begin playback
uint8_t ZSM_load(char* file_name, uint8_t name_length, uint8_t dest_bank);
// stops and resets to beginning of song
void ZSM_stop();
// pauses song (to be resumed)
void ZSM_pause();
// begin / resume song
void ZSM_play();









#endif