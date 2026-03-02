#ifndef BULLET_H
#define BULLET_H

#include <stdint.h>
#include "intellisense_macro.h"

uint8_t BulletManagerInit();

void BulletTick();

//void TestBulletBlit(uint8_t buffer_n, uint8_t x, uint8_t y);
void TestBulletBlit(uint8_t buffer_n);





#endif