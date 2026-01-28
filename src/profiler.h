#ifndef PROFILER_H
#define PROFILER_H
//  ----    separates time into blocks (1 block should be one game tick/frame)
//  ----    time is measured in scanlines since previous segment, 1 frame of video (1/60th second) is 525 scanlines

#include <stdint.h>

#define PROFILER_MAX_SEGMENTS   8
extern uint16_t profiler_segment_previous[PROFILER_MAX_SEGMENTS];
extern uint16_t profiler_previous_total;
extern uint8_t profiler_previous_segment_count;

// gets timestamp in scanlines since +-program startup
uint16_t ProfilerGetTimestamp();


void ProfilerBeginBlock();
void ProfilerEndSegment();
void ProfilerEndBlock();

#endif