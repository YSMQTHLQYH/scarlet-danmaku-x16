#ifndef PROFILER_H
#define PROFILER_H
//  ----    separates time into blocks (1 block should be one game tick/frame)
//  ----    time is measured in scanlines since previous segment, 1 frame of video (1/60th second) is 525 scanlines

#include <stdint.h>

#define PROFILER_MAX_SEGMENTS   8
// double buffers, so we can read the data from previous block
extern uint16_t ProfilerSegmentBuffer0[PROFILER_MAX_SEGMENTS];
extern uint16_t ProfilerSegmentBuffer1[PROFILER_MAX_SEGMENTS];

uint16_t* ProfilerGetPreviousBlock();

// begins segment 0
void ProfilerBeginBlock();
// ends segment i, begins segment i+1
// returns i
uint8_t ProfilerEndSegment();
// ends segment i
// returns i
uint8_t ProfilerEndBlock();




#endif