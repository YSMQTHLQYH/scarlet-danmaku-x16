#include "zsm_player.h"
#include "x16.h"

#define ZSM_START   HIGH_RAM_START
#define ZSM_HEADER (*(_sZsmHeader*)ZSM_START)

_sZsmPlayer zsm;


void ZSM_tick() {
    if (zsm.state = !ZSM_STATE_PLAYING) { return; }
    zsm.delay_ticks_left--;
    if (zsm.delay_ticks_left > 0) { return; }
}


uint8_t ZSM_load(char* file_name, uint8_t name_length, uint8_t dest_bank) {
    _sFileLoadCtx fl = { 0 };

    if (zsm.state != ZSM_STATE_NO_SONG_LOADED) { ZSM_stop(); }

    fl.filename = file_name;
    fl.name_lenght = name_length;
    fl.header_mode = FILE_LOAD_HEADERLESS;
    fl.target_mode = FILE_LOAD_RAM;
    fl.dest_addr = (void*)ZSM_START;
    SET_RAM_BANK(dest_bank);
    if (load_file(&fl)) {
        // load success
        //printf("loaded file from: %u to: %u\n", fl.dest_addr, fl.file_end);
        if (ZSM_HEADER.magic_header[0] != 0x7A || ZSM_HEADER.magic_header[1] != 0x6D) {
            //whatever was loaded was NOT a .zsm file
            zsm.state = ZSM_STATE_NO_SONG_LOADED;
            return 0;
        }
        // thing loaded is a valid .zsm file
        zsm.state = ZSM_STATE_STOPPED;
        zsm.start_bank = dest_bank;
        zsm.end_bank = *ram_bank; // load function handles ram_bank and leaves it at the end of file
        zsm.index = ZSM_START + sizeof(_sZsmHeader);
        zsm.index_bank = dest_bank;
        zsm.delay_ticks_left = 1;
        return 1;
    } else {
        //error
        //printf("file load error: %u\n", fl.error_code);
        zsm.state = ZSM_STATE_NO_SONG_LOADED;
        return 0;
    }

}

void ZSM_stop() {
    if (zsm.state == ZSM_STATE_NO_SONG_LOADED) { return; }
    zsm.state = ZSM_STATE_STOPPED;
    zsm.index = ZSM_START + sizeof(_sZsmHeader);
    zsm.index_bank = zsm.start_bank;
    zsm.delay_ticks_left = 1;
}

void ZSM_pause() {
    if (zsm.state != ZSM_STATE_PLAYING) {
        zsm.state = ZSM_STATE_PAUSED;
    }
}

void ZSM_play() {
    if (zsm.state == ZSM_STATE_NO_SONG_LOADED) { return; }
    zsm.state = ZSM_STATE_PLAYING;
}
