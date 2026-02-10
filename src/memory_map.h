#ifndef MEMORY_MAP_H
#define MEMORY_MAP_h
/*
Not actually a file with code,
it's just to have a single place with a list of which parts of memory are owned by what.
Just so we don't give the same memory page to two different things

Low RAM is not here, that is left up to the compiler (for now?)

Still in the code so we can put macros for the addresses here
*/

/*
Don't know where to put this comment
having general sprites be in sheets of 8KB each is very convinient
A: sprites have to registers for the address of the graphics data, annoyingly it's bits 12-5 for the low byte and 16-13 for high byte
(Not what you would expect it to be), bit shifting by the specific number every time we deal with sprites will be annoying
incrementing the high byte increases the address by 0x2000, which is 8KB
this means we can just store "sheet number" and the index inside the sheet, which would map exactly to the high and low bytes respectively
B: having sprite sheet be same size as ram bank will make it easier to load the entire game into high ram at boot
not strictly needed but it simplifies the logic as we only need to store bank number of each spritesheet

size of spritesheet 128x128x bpp or 128x64 8bpp
256 sprites each    8x8             4bpp
128 sprites each    16x8            4bpp,   8x8             8bpp
64 sprites each     16x16 / 32x8    4bpp,   16x8            8bpp
32 sprites each     32x16 / 64x8    4bpp,   16x16 / 32x8    8bpp
16 sprites eaach    32x32 / 64x16   4bpp,   32x16 / 64x8    8bpp
8 sprites each      64x32           4bpp,   32x32 / 64x16   8bpp
4 sprites each      64x64           4bpp,   64x32           8bpp
2 sprite each                               64x64           8bpp

*/

/*    VRAM PAGE 0 START    */
//  ---- Frame buffer 0 of bitmap 1 (main game bullets)
#define MEM_VRAM_0_BITMAP_1_START   0X0000
#define MEM_VRAM_0_BITMAP_1_END     0X4AFF

#define MEM_BITMAP_1_ADDR_M   0x00

//  ---- Unused 1
#define MEM_VRAM_0_UNUSED_1_START   0x4B00
#define MEM_VRAM_0_UNUSED_1_END     0x4FFF
/*
A gap of 0x500 (1280) bytes until the next nicely aligned area
2 of these because one for each frame buffer, ideas for each of them:
> 320x8 4bpp bitmap (switch from interrupt)
> 640 colors to swap mid frame with VeraFX cache (too much cpu time in interrupts needed for this to be useful?)
> 40 8x8 4bpp sprites (or 10 16x16 ones, or 5 16x32 ones, or 2.5 32x32 ones)
> 20 8x8 8bpp sprites (or 5 16x16, or a 80x16 nice icon)
*/

//  ---- Unused 2
#define MEM_VRAM_0_UNUSED_2_START   0xA000
#define MEM_VRAM_0_UNUSED_2_END     0xCFFF
/*
Maybe *the* bullet table here? I would like it to be a nice round number so maybe at 0x8000?
needs (at least) px,vx,py,vy, all of them 16-bit, 8 bytes per bullet
say we want 1000 bullets, that's 8KB, 0x2000
maybe max of 1536 bullets so its from 0x5000 to 0x8000
pretty ambitious ngl, just the math_test at that size takes 405 out of the 525 scanlines, it's going to be slower with added drawing
each scanline is 254 clock cycles (~6 scanlines per clock cycle in *that* function)
*/

//  ---- Text
#define MEM_VRAM_0_2BPP_FONT_1_START    0xD000
#define MEM_VRAM_0_2BPP_FONT_1_END      0xDFFF
#define MEM_VRAM_0_4BPP_FONT_1_START    0xE000
#define MEM_VRAM_0_4BPP_FONT_1_END      0xFFFF
#define MEM_FONT_VRAM_PAGE    0
#define MEM_2BPP_FONT_1_ADDR_M 0xD0
#define MEM_4BPP_FONT_1_ADDR_M 0xE0
/*     VRAM PAGE 0 END     */


/*    VRAM PAGE 1 START    */
//  ---- Frame buffer 1 of bitmap 1 (same address as frame buffer 0)
#define MEM_VRAM_1_BITMAP_1_START   0X0000
#define MEM_VRAM_1_BITMAP_1_END     0X4AFF

//#define MEM_BITMAP_1_ADDR_M   0x00 


//  ---- Unused 1
#define MEM_VRAM_1_UNUSED_1_START   0x4B00
#define MEM_VRAM_1_UNUSED_1_END     0xEFFF

//  ---- KERNAL charset (not actually used in game but needed free to load fonts)
/*
This area can be used for anything once the game has loaded (say, once we reach the title screen)
Anyhing that's 2KB
> Map data for a map that's 1024 tiles (exactly the minimun size of 32x32)
> A cool gradient 320x6 in a 8bpp bitmap, changing layer modes on an interrupt
> or 320x12 at 4bpp, then we don't have to worry about changing color palette
> A single 64x64 4bpp sprite (or  64 8x8 ones, or 16 16x16 ones)
> 2 32x32 8bpp sprites (or 32 8x8 ones, or 8 16x16 ones)
> Player sprite 16x16 8bpp with 8 frames of animation (that locks in place a large ammount of the color palette though)
*/
#define MEM_VRAM_1_KERNAL_CHARSET_START 0xF000
#define MEM_VRAM_1_KERNAL_CHARSET_END   0xF7FF
#define MEM_VRAM_1_KERNAL_FONT_ADDR_M   0xF0


//  ---- Unused 2
/*
There's probably something useful we can do with these 448 bytes in this comfy little nook
Not quite enough for a 64*8 8bpp sprite :/
> 7 icons each 8x8x8?
> Or 14 icons each 8x8x4
> Maybe a 112x8 lifebar (segments repeated for fullscreen probs)
*/
#define MEM_VRAM_1_UNUSED_2_START   0xF800
#define MEM_VRAM_1_UNUSED_2_END     0xF9BF

//  ---- VERA Hardware registers
#define MEM_VRAM_1_VERA_PSG_START   0xF9C0
#define MEM_VRAM_1_VERA_PSG_END     0xF9FF
#define MEM_VRAM_1_VERA_COLOR_PALETTE_START 0xFA00
#define MEM_VRAM_1_VERA_COLOR_PALETTE_END   0xFBFF
#define MEM_VRAM_1_VERA_SPRITE_ATTR_START   0xFC00
#define MEM_VRAM_1_VERA_SPRITE_ATTR_END     0xFFFF

// These belong on x16.h, as they aren't specific to this game
// for the sake of consistency, I will add them here anyways and comment them out on x16.h
#define MEM_VRAM_1_VERA_PSG_M    0xF9
#define MEM_VRAM_1_VERA_PSG_L    0xC0
#define MEM_VRAM_1_VERA_COLOR_PALETTE_M     0xFA
#define MEM_VRAM_1_VERA_SPRITE_ATTR_M       0xFC
#define MEM_SIZE_SPRITE_ATTR                8


/*     VRAM PAGE 1 END     */

/*
hypothetical 4bpp bitmap
(most/all of this also applies for a hypothetical 2bpp 320*480... and I supose a monocrome full 640/480 is close enough)
frame buffer: 0x0000 to 0x95FF
however the actual game area is going to be at most 256 pixels wide (possibly 192 or 224), leaving a pocket in the side we can reuse
(so long we cover it up with sprites)
we can't put a big sprite or tilemap because the data needs to be contigious, it's effectively 240 tiny pockets of 32/48/64 bytes each
just enough for 1 or 2 sprites... like text
we can place the font on that area, it does make the awkward area extend to 0x9FFF
leaving 16 pockets of 96/112/128 bytes (easier to fit sprites or just another bitmap for interrupt effects)
the 256 pixel game area fits exactly one font, the 192 one fits either two fonts or one 8x16 one
the 224 fits one font and leaves 240/256 very awkward pockets of 16 bytes (not even a single sprite)
only possibly useful for graphics to be blitted to the bitmap (it'll be hard to use all 240 of them)
Meanwhile on the side of the other frame buffer
We can fit *the* table with the bullets, each bullet needs 8 bytes and we already plan to have them in blocks of 32...
256 bytes per block, we can lay them vertically and it fits perfectly.
the 256/224/192 game area can fit  32/48/64 blocks, total of 1024/1536/2048 bullets

either way the awkward area is from 0x0000 to 0xA000, that 5/8ths  of the entire VRAM, leaving us with ~48KB in total for the actual game graphics (1.56KB in hardware registers)
and we need *at least* 2KB for a 64x64 sprite to cover the secret pocket on the side of the game (or just crop it out and accept rectangular pixels :/)

probably for layer0 a tilemap with the minimun size of 32x32 tiles (also 2KB), we can stream the data so long we don't scroll too fast
we don't want any of that NES 2bpp for the actual background so 4bpp minimum
tiles are from 8x8 to 16x16, each tile can range from 32 bytes to 256 bytes
I ain't drawing 256 color background tiles and using 16 pixel wide tiles is pointless if we are scrolling mostly vertically
only options left are 8x8 and 8x16 (32 or 64 bytes per tile)
there's no minimum/maximum tileset size, only restriction is it needs to start 2KB aligned
makes sense to align the entire thing to 2KB, so multiples of 64 8x8 tiles or 32 8x16 (probably just one of these, maybe two)

sprites
If streaming graphics data is fast enough we could have the player be a single 32x32 8bpp sprite and use streaming for animation
If it's not that fast it's either 2-4 frames (1 KB each)
or compromise with 4 bpp (512 bytes/frame) and maybe even multiple sprites to make a "24x24 sprite" (288 bytes/frame)

character portrait
ideally 6 sprites each 64x64 8bpp, total of 128x192, 24KB... not happening unless during dialog (replace a frame buffer)
96x192 portraits for 18KB, we can cut their legs off (96x128) for 12KB, plausible
that at 4bpp takes only 6KB, good enough if we only ever have one character on screen during gameplay

TALLY COUNT (total space of  ~46.4KB)
2KB to hide "repurposed" bitmap
2KB tilemap data
2/4KB tilemap graphics
1-4KB nice animated player sprite (size depends on performance, not amount of graphics)
6/12KB character portrait(s)
We will round the first four to 8KB
Tally count of 14KB with stretch goal of 20KB

We still need
> the actual game sprites, probably a bunch of 16x16 4bpp ones (128 bytes a pop)
> some way to display text on the ~~hud~~ garbage under a rug area
(can't do 2bpp sprites, using sprites to render text will quickly use all hardware sprite slots)
best option is more of those 64x64 sprites (2KB each :/)
> bosses should also get the player "nice sprite" treatment, (let's say 4KB)
actually this can be shared with the regular enemy sprites (we don't need both at the same time)
*/

/*
even more hypothetical 240x320 4bpp mode (so we can have vertical monitors)
frame buffers are still using 0x0000 to 0x95FF, BUT we can't repurpose any of it now (75KB in total)
(though we can actually render the GUI on the bitmap layer) 2KB saved i guess
the 0x9600 to 0xA000 area is actually usable like normal, 2.5KB more
we still need to find room for at least 32 bullet blocks (256 bytes each), at least 8KB
remember that we also need room for a full font (8KB)
we could only include half a font (non standard charset?) and leave room for 48 blocks of bullets (16KB)
the absolute minimun text we need is 26 letters + 10 digits, the 0x9600-0xA000 area has room for 80. really awkward charset but it's good enough

lets say we do that on one bank and on the other we leave room for some interrupt effect
46.5KB in total
8-16KB for the bullet data
8KB per sprite sheet (we need at least 2, probably no more than 4)
the actual bullet graphics to copy paste (16 bytes? per graphic), 1 or 2 KB should be fine
12KB for two portraits
uh total
12+12+24 and we are already over budget...
if we use 128x64 portraits we save 4KB, if we instead code it such that there's only one portrait at a time we save 6KB instead
we can barely fit that way (12+6+24 = 42KB, absolute minimum tilemap is 2+2KB)
we will probably need to stream the graphics for the player animations though...
also a second byte for the y position... and somehow adjust the level design to work at both 256x240 and 240x320...

oh yeah having the level design work at different resolution
working the game logic in some "world space coordiantes" and have a scaling factor is not happening
we can store all level data (starting positions and speeds) in one of the two resolutions and scale before starting
that would mean having two versions of everyhing...
even if we survive the spaghetti from that we will still have a different sized hitbox!
alternatively, have the game run at 240x320 and cut off the top section of the screen in horizontal mode...
I suppose we could always have the game area in 240x240 and just move around the GUI but at that point why bother with vertical monitors
idk fam, this sounds like a bad idea from game balance pov
*/

/*    BANKED RAM START     */
#define MEM_BANK_SPRITE_TABLE   2

/*     BANKED RAM END      */

#endif