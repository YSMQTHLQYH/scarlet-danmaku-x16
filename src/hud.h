#ifndef HUD_H
#define HUD_H

#include <stdint.h>
#define PROFILER_SEGMENT_COUNT   8

// returns 0 on success
uint8_t HudInit();

void PrintPrevProfBlock();



#endif