#ifndef BULLET_GRAPHICS_TESTS_H
#define BULLET_GRAPHICS_TESTS_H

#include <stdint.h>


typedef enum {
    GFX_TEST_NOTHING,
    GFX_TEST_2x1,
    GFX_TEST_5x5,
    GFX_TEST_COPY_PASTE_VRAM, //3
    GFX_TEST_COPY_PASTE_ZP,
    GFX_TEST_5x5_HOP,
    GFX_TEST_CACHE_1_COLOR,
    GFX_TEST_CACHE_1_COLOR_DEC,
    GFX_TEST_POLYGON_HACK, //8
    GFX_TEST_POLYGON_HACK_NIBBLE,
    GFX_TEST_POLYGON_HACK_CACHE,



    GFX_TEST_COUNT,
}_eBulletGfxTest;


void GfxTestsinit();
void GfxhTest(_eBulletGfxTest t);



#endif