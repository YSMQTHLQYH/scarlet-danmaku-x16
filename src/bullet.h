#ifndef BULLET_H
#define BULLET_H

#include <stdint.h>

#define BULLETS_PER_BLOCK   32
#define BULLET_BLOCK_COUNT  32

/* These HAVE to be multiples of two or everyhing explodes*/
typedef enum {
    BULLET_GRAPHIC_DUMMY = 0,
    BULLET_GRAPHIC_PIXEL = 2,
}_eBulletGraphicType;

typedef struct {
    uint8_t remaining_bullets;
    uint8_t graphic_type; //_eBulletGraphicType
    uint8_t color;

}_sBulletBlock;
extern _sBulletBlock bullet_block[BULLET_BLOCK_COUNT];

typedef struct {
    uint8_t count;
    uint8_t graphic_type; //_eBulletGraphicType
    uint8_t color;
    uint8_t count_per_subblock;

    uint8_t x_start;
    int8_t x_offset;
    int8_t x_offset_subblock;
    uint8_t y_start;
    int8_t y_offset;
    int8_t y_offset_subblock;
    int8_t angle_start;
    int8_t angle_offset;
    int8_t angle_offset_subblock;
    uint8_t speed_start;
    int8_t speed_offset;
    int8_t speed_offset_subblock;
}_sBulletSpawnCfg;


uint8_t BulletManagerInit();

// uses zptr3, zpa5 and zpa6!
void SpawnBulletBlock(_sBulletSpawnCfg* cfg);

void BulletTick();

// THE function
void BulletBlockTick(uint8_t block_index);

//void TestBulletBlit(uint8_t buffer_n, uint8_t x, uint8_t y);
void TestBulletBlit(uint8_t buffer_n);





#endif