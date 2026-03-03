#include "hud.h"
#include "x16.h"
#include "zp_utils.h"
#include "graphics_utils.h"
#include "sprite_manager.h"
#include "text.h"

#include "profiler.h"

static uint8_t back_sprite_object;
static const uint8_t back_sprite_addr_data[8] = {
    0x20, 0x80,
    0x20, 0x80,
    0x00, 0x80,
    0x40, 0x80,
};
uint16_t hud_palette[] = {
    0xF97, 0xF44, 0xD12, 0x801,
    0xCA8, 0xDC9, 0xEEC, 0xEEE,
    0x222, 0x546, 0xA6B, 0xD7C,
    0x06C, 0x0AC, 0x0FE, 0x6FB,
};
#define HUD_PALETTE 14

uint8_t HudInit() {
    //  ---- load sprite sheet(s)
    _sFileLoadCtx fl = { 0 };
    fl.filename = "test assets/debughuda.bin";
    fl.name_lenght = 12 + 13;
    fl.dest_addr = (void*)MEM_VRAM_1_HUD_SPRITESHEET_START;
    fl.header_mode = FILE_LOAD_HEADERLESS;
    fl.target_mode = FILE_LOAD_VRAM_P1;
    if (LoadFile(&fl)) {
        EMU_DEBUG_1(0xFA);
        EMU_DEBUG_1(fl.error_code);
        return 1;
    }
    fl.filename = "test assets/debughudb.bin";
    fl.name_lenght = 12 + 13;
    fl.dest_addr = (void*)(MEM_VRAM_1_HUD_SPRITESHEET_START + 0x1000);
    fl.header_mode = FILE_LOAD_HEADERLESS;
    fl.target_mode = FILE_LOAD_VRAM_P1;
    if (LoadFile(&fl)) {
        EMU_DEBUG_1(0xFA);
        EMU_DEBUG_1(fl.error_code);
        return 1;
    }


    //  ---- create sprites for bitmap cover
    back_sprite_object = CreateSpriteObject(120, 8, 2, 0b1110);
    if (back_sprite_object == 0xFF) { return 1; }
    SpriteObjectSetAddr(back_sprite_object, HUD_SPRITESHEET_NUMBER, (uint8_t*)back_sprite_addr_data);
    SpriteObjectSetPosition(back_sprite_object, 224, -16);
    SpriteObjectSetZFlip(back_sprite_object, 0b1000);
    SpriteObjectSetSizePalette(back_sprite_object, 0b1110, HUD_PALETTE);
    // manually set size of sprites on the right to 64x64
    sprite_attr_size_palette[121] |= 0xF0;
    sprite_attr_size_palette[123] |= 0xF0;
    sprite_attr_size_palette[125] |= 0xF0;
    sprite_attr_size_palette[127] |= 0xF0;

    SetColorPalette(HUD_PALETTE, hud_palette);

    return 0;
}




static char prf_str[] = "----";
void PrintPrevProfBlock() {
    static uint8_t created = 0;
    static uint8_t update_index = 0;
    static uint8_t str_obj[PROFILER_SEGMENT_COUNT + 1] = { 0 };
    uint8_t i;
    _uConv16 t;
    if (!created) {
        for (i = 0; i < PROFILER_SEGMENT_COUNT; i++) {
            str_obj[i] = CreateSpriteStr(SPR_PRIORITY_HIGH, 4, 0x0C, 15);
            SpriteObjectSetPosition(str_obj[i], 260, 113 + (i << 3));
        }
        str_obj[i] = CreateSpriteStr(SPR_PRIORITY_HIGH, 4, 0x0C, 15);
        SpriteObjectSetPosition(str_obj[i], 260, 117 + (i << 3));
        created = 1;
    }
    /*
    for (i = 0; i < PROFILER_SEGMENT_COUNT; i++) {
        t.w = profiler_segment_previous[i];
        StrUint16Hex(t.w, prf_str);
        PrintSpriteStr(str_obj[i], prf_str);
        //Print2BppBitmapStr(prf_str, bitmap_front_buffer, 62, 170 + (i << 3));
    }
    */
    t.w = profiler_segment_previous[update_index];
    StrUint16Hex(t.w, prf_str);
    PrintSpriteStr(str_obj[update_index++], prf_str);
    if (update_index >= PROFILER_SEGMENT_COUNT) { update_index = 0; }
    // total
    StrUint16Hex(profiler_previous_total, prf_str);
    PrintSpriteStr(str_obj[PROFILER_SEGMENT_COUNT], prf_str);

}