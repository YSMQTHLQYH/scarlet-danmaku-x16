#include "profiler.h"
#include "x16.h"

uint16_t ProfilerSegmentBuffer0[PROFILER_MAX_SEGMENTS] = { 0 };
uint16_t ProfilerSegmentBuffer1[PROFILER_MAX_SEGMENTS] = { 0 };

static uint8_t current_segment_buffer = 0;
static uint16_t* buffer = ProfilerSegmentBuffer0;
static uint8_t current_segment = 0;

// time in scanlines since start of segment, going over 525 if more than a frame
static uint16_t last_time = 0;
// last result from RDTIM, which updates once per frame
static uint8_t last_kernal_time = 0;

uint16_t* ProfilerGetPreviousBlock() {
    if (current_segment_buffer == 0) { return ProfilerSegmentBuffer1; } else { return ProfilerSegmentBuffer0; }
}

void ProfilerBeginBlock() {
    if (current_segment_buffer == 0) {
        current_segment_buffer = 1;
        buffer = ProfilerSegmentBuffer1;
    } else {
        current_segment_buffer = 0;
        buffer = ProfilerSegmentBuffer0;
    }
    current_segment = 0;
}

uint8_t ProfilerEndSegment();

uint8_t ProfilerEndBlock();


void GetTime() {

}