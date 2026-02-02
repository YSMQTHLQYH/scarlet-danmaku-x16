#include "sprite_manager.h"
#include "zp_utils.h"
#include "memory_map.h"


/* ALL OF THESE ARE HARD CODED IN ASM BTW*/
/* also don't forget to switch ram bank to MEM_BANK_SPRITE_TABLE*/

// the arrays for buffering the data are just stored in a high ram directly via a pointer
// we are not actually defining an array, just a pointer to it
uint8_t* sprite_attr_addr_l = (uint8_t*)(HIGH_RAM_START + 0);
uint8_t* sprite_attr_addr_h = (uint8_t*)(HIGH_RAM_START + MAX_SPRITES);
uint8_t* sprite_attr_x = (uint8_t*)(HIGH_RAM_START + (MAX_SPRITES * 2));
uint8_t* sprite_attr_x_h = (uint8_t*)(HIGH_RAM_START + (MAX_SPRITES * 3));
uint8_t* sprite_attr_y = (uint8_t*)(HIGH_RAM_START + (MAX_SPRITES * 4));
uint8_t* sprite_attr_y_h = (uint8_t*)(HIGH_RAM_START + (MAX_SPRITES * 5));
uint8_t* sprite_attr_z_flip = (uint8_t*)(HIGH_RAM_START + (MAX_SPRITES * 6));
uint8_t* spirte_attr_size_palette = (uint8_t*)(HIGH_RAM_START + (MAX_SPRITES * 7));

