#include "profiler.h"
#include "x16.h"
#include "zp_utils.h"

#include <stdio.h>

// incremented in vsync interrupt, reset to 0 whenever read
extern volatile uint8_t profiler_vsync_queue;

uint16_t scanlines_from_frame_count = 0;

static uint16_t profiler_segment[PROFILER_MAX_SEGMENTS] = { 0 };
static uint16_t last_time = 0;
static uint8_t current_segment = 0;

uint16_t profiler_segment_previous[PROFILER_MAX_SEGMENTS] = { 0 };
uint16_t profiler_previous_total = 0;
uint8_t profiler_previous_segment_count = 0;

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
    /*
    if (t - last_time > 0x7FFF) {
        while (1) {
            EMU_DEBUG_1(t);
            EMU_DEBUG_2(last_time);
        }
    }
    */
    profiler_segment[current_segment++] = t - last_time;
    last_time = t;
}
void ProfilerEndBlock() {
    uint8_t i;
    uint16_t t = ProfilerGetTimestamp();
    profiler_segment[current_segment++] = t - last_time;
    //last_time = t;
    profiler_previous_total = 0;
    for (i = 0; i < PROFILER_MAX_SEGMENTS; i++) {
        profiler_segment_previous[i] = profiler_segment[i];
        profiler_previous_total += profiler_segment[i];
    }
    profiler_previous_segment_count = current_segment;
}


