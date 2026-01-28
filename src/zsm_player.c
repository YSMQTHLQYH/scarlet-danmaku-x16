#include "zsm_player.h"
#include "x16.h"
#include "memory_map.h"

#define ZSM_START   HIGH_RAM_START
#define ZSM_HEADER (*(_sZsmHeader*)ZSM_START)

static uint8_t ZsmNextByte();
static void StopAllPlayingSounds();

_sZsmPlayer zsm;

void ZsmTick() {
    uint8_t cmd, n;
    uint8_t i;
    // exit if we aren't playing anything
    if (zsm.state != ZSM_STATE_PLAYING) { return; }
    zsm.delay_ticks_left--;
    if (zsm.delay_ticks_left > 0) { return; }

    // set up registers for memory mapping
    SET_RAM_BANK(zsm.index_bank);
    vera->CTRL = 0x00; // using DATA0 for PSG loading
    vera->ADDRx_H = 0x01;// using addr_inc = 0, all registers are on page 1 (second half)
    vera->ADDRx_M = MEM_VRAM_1_VERA_PSG_M;

    while (1) {
        // read
        cmd = ZsmNextByte();
        switch (cmd & 0xC0) { // switch(cmd_type)
        case 0x00:
            // ---- PSG_WRITE
            vera->ADDRx_L = (cmd & 0x3F) + MEM_VRAM_1_VERA_PSG_L;
            vera->DATA0 = ZsmNextByte();
            break;

        case 0x40:
            n = cmd & 0x3F;
            if (n == 0) {
                // ---- EXTCMD
                // not implemented (for now?)
                print_emul_debug("EXTCMD in ZSM file (not supported)");
                break;
            }
            // ---- FM_WRITE

            for (i = 0; i < n; i++) {
                OpmWrite(ZsmNextByte(), ZsmNextByte());
            }
            break;

        default:
            // ---- DELAY
            // or end of file if n == 0
            zsm.delay_ticks_left = (cmd & 0x7F);
            goto zsm_tick_end;
        }
    }
zsm_tick_end:
    if (zsm.delay_ticks_left == 0) {
        // ---- EOF
        print_emul_debug("Song end point reached");
        //TODO: implement looping
        ZsmStop();
    }

}
static uint8_t ZsmNextByte() {
    uint8_t r = *(zsm.index++);
    if (zsm.index > (uint8_t*)HIGH_RAM_END) {
        zsm.index = (uint8_t*)HIGH_RAM_START;
        SET_RAM_BANK(++zsm.index_bank);
    }
    return r;
}

static void StopAllPlayingSounds() {
    uint8_t i;
    //  ---- PSG
    vera->CTRL = 0x00; // using DATA0 for PSG loading
    vera->ADDRx_H = 0x11;// using addr_inc = 1, all registers are on page 1 (second half)
    vera->ADDRx_M = MEM_VRAM_1_VERA_PSG_M;
    vera->ADDRx_L = MEM_VRAM_1_VERA_PSG_L;
    for (i = 0; i < 64; i++) {
        vera->DATA0 = 0;
    }
    //  ---- OPM
    for (i = 0; i < 8; i++) {
        OpmWrite(0x08, i);
    }
}


uint8_t ZsmLoad(char* file_name, uint8_t name_length, uint8_t dest_bank) {
    _sFileLoadCtx fl = { 0 };

    if (zsm.state != ZSM_STATE_NO_SONG_LOADED) { ZsmStop(); }

    fl.filename = file_name;
    fl.name_lenght = name_length;
    fl.header_mode = FILE_LOAD_HEADERLESS;
    fl.target_mode = FILE_LOAD_RAM;
    fl.dest_addr = (void*)ZSM_START;
    SET_RAM_BANK(dest_bank);
    if (load_file(&fl)) {
        // load success

        // load function handles ram_bank and leaves it at the end of file
        // saving it now because we need to switch bank to check header
        zsm.end_bank = *ram_bank;
        SET_RAM_BANK(dest_bank);

        if (ZSM_HEADER.magic_header[0] != 0x7A || ZSM_HEADER.magic_header[1] != 0x6D) {
            //whatever was loaded was NOT a .zsm file
            EMU_DEBUG_1(ZSM_HEADER.magic_header[0]);
            EMU_DEBUG_2(ZSM_HEADER.magic_header[1]);
            zsm.state = ZSM_STATE_NO_SONG_LOADED;
            return 0;
        }
        // thing loaded is a valid .zsm file
        zsm.state = ZSM_STATE_STOPPED;
        zsm.start_bank = dest_bank;

        zsm.index = (uint8_t*)(ZSM_START + sizeof(_sZsmHeader));
        zsm.index_bank = dest_bank;
        zsm.delay_ticks_left = 1;
        return 1;
    } else {
        //error
        zsm.load_error_code = fl.error_code;
        EMU_DEBUG_2(fl.error_code);
        zsm.state = ZSM_STATE_NO_SONG_LOADED;
        return 0;
    }

}

void ZsmStop() {
    if (zsm.state == ZSM_STATE_NO_SONG_LOADED) { return; }
    zsm.state = ZSM_STATE_STOPPED;
    zsm.index = (uint8_t*)(ZSM_START + sizeof(_sZsmHeader));
    zsm.index_bank = zsm.start_bank;
    zsm.delay_ticks_left = 1;
    StopAllPlayingSounds();
}

void ZsmPause() {
    if (zsm.state == ZSM_STATE_PLAYING) {
        zsm.state = ZSM_STATE_PAUSED;
        StopAllPlayingSounds();
    }
}

void ZsmPlay() {
    if (zsm.state == ZSM_STATE_NO_SONG_LOADED) { return; }
    zsm.state = ZSM_STATE_PLAYING;
}
