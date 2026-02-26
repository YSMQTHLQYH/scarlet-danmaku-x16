#include "bullet.h"

#include "x16.h"
#include "zp_utils.h"

/*
lookup table format
256 possible angles, index in array is angle, different array for each speed
array order is x_low, x_high, y_low, y_high
1KB in total per speed, 8 speeds per ram bank
*/

// returns 0 on success, 1 on failure
uint8_t BulletManagerInit() {
    _sFileLoadCtx fl;
    uint8_t ok;
    x16_ram_bank = MEM_BANK_BULLET_LOOKUP_0;
    fl.filename = "test assets/bullet lookup.bin";
    fl.name_lenght = 12 + 17;
    fl.header_mode = FILE_LOAD_HEADERLESS;
    fl.target_mode = FILE_LOAD_RAM;
    fl.dest_addr = (void*)HIGH_RAM_START;

    ok = load_file(&fl);
    if (!ok) {
        return 1;
    }
    ok = 0;
    return 0;
}