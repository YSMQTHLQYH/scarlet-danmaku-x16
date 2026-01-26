#include "profiler.h"
#include "x16.h"
#include "zp_utils.h"

#include <stdio.h>

// incremented in vsync interrupt, reset to 0 whenever read
extern volatile uint8_t profiler_vsync_queue;

uint16_t scanlines_from_frame_count = 0;

uint16_t profiler_segment[PROFILER_MAX_SEGMENTS] = { 0 };
static uint16_t last_time = 0;
static uint8_t current_segment = 0;

uint16_t ProfilerGetTimestamp() {
    uint16_t current_scanline = VeraGetScanline();
    // offset scanline so vsync alings with current_scanline resetting back to 0
    if (current_scanline >= SCANLINE_VSYNC) { current_scanline -= SCANLINES_PER_FRAME; }
    current_scanline += (SCANLINES_PER_FRAME - SCANLINE_VSYNC);
    // add the time from frames
    while (profiler_vsync_queue) {
        scanlines_from_frame_count += SCANLINES_PER_FRAME;
        profiler_vsync_queue--;
    }
    return scanlines_from_frame_count + current_scanline;
}

void ProfilerBeginBlock() {
    uint8_t i;
    for (i = 0; i < PROFILER_MAX_SEGMENTS; i++) {
        profiler_segment[i] = 0;
    }
    last_time = ProfilerGetTimestamp();
    current_segment = 0;
}
void ProfilerEndSegment() {
    uint16_t t = ProfilerGetTimestamp();

    if (t - last_time > 0x4FFF) {
        while (1) {
            EMU_DEBUG_1(t);
            EMU_DEBUG_2(last_time);
        }
    }

    profiler_segment[current_segment++] = t - last_time;
    last_time = t;
}
uint8_t ProfilerEndBlock() {
    uint16_t t = ProfilerGetTimestamp();
    profiler_segment[current_segment++] = t - last_time;
    //last_time = t;
    return current_segment;
}


