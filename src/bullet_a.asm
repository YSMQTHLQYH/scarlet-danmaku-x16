.importzp _zpa0, _zpa1;, _zpc0
.import _VERA_ADDRx_L, _VERA_ADDRx_M, _VERA_ADDRx_H
.import _VERA_DATA0, _VERA_DATA1, _VERA_CTRL
.importzp VERA_ADDR_INC_160, VERA_ADDR_INC_320

.import _bitmap_back_buffer
.import lookup_y_high, lookup_y_low

.importzp GAME_AREA_WIDTH_BYTES, GAME_AREA_WIDTH, GAME_AREA_HEIGHT

.export _bullet_block

BULLET_BLOCK_COUNT = 32
TABLE_VRAM_PAGE = 0

.struct _sBulletBlock
    remaining_bullets .byte
    graphic_type .byte
    color .byte
.endstruct

.segment "DATA"
_bullet_block: .res (.sizeof(_sBulletBlock) * BULLET_BLOCK_COUNT)

graphic_type_jump_table:
.word GraphicDummy
.word GraphicPixel

.segment "CODE"

.export _BulletBlockTick
_BulletBlockTick = BulletBlockTick

;block index is passed as argument, starts in register A
.proc BulletBlockTick
    index = _zpa0
    bullets_left = _zpa1
    sta index
    stz bullets_left

    ;configure vera to read table
    ;DATA0 for reading table, DATA1 for writting updated position
    stz _VERA_CTRL ;addr_sel = 0
    lda #(VERA_ADDR_INC_160 + TABLE_VRAM_PAGE)
    sta _VERA_ADDRx_H
    stz _VERA_ADDRx_M
    lda #GAME_AREA_WIDTH_BYTES
    adc index
    tax
    stx _VERA_ADDRx_L

    lda #1
    sta _VERA_CTRL ;addr_sel = 1
    lda #(VERA_ADDR_INC_320 + TABLE_VRAM_PAGE)
    sta _VERA_ADDRx_H
    stz _VERA_ADDRx_M
    stx _VERA_ADDRx_L

    ;bullet position update
    clc
    lda _VERA_DATA0 ;py_l
    adc _VERA_DATA0 ;vy_l
    sta _VERA_DATA1 ;updated py_l
    lda _VERA_DATA0 ;py_h
    adc _VERA_DATA0 ;vy_h
    ;YES we are only checking after doing the first two additions
    cmp #GAME_AREA_HEIGHT
    bcc delete_y_check_passed ;branches if A < value
    ;bullet deleted, write dummy positions
    ;actually we don't even bother clearing the subpixel position, should be fine so long speed is less than 7 per frame or so
    ;do a dummy half pass
    lda _VERA_DATA0 ;px_l
    lda _VERA_DATA0 ;vx_l
    lda _VERA_DATA0 ;px_h
    lda _VERA_DATA0 ;vx_h
    lda #$F7
    sta _VERA_DATA1
    bne deleted_dummy_draw
delete_y_check_passed:
    sta _VERA_DATA1 ;updated py_h
    tay

    clc
    lda _VERA_DATA0 ;px_l
    adc _VERA_DATA0 ;vx_l
    sta _VERA_DATA1 ;updated px_l
    lda _VERA_DATA0 ;px_h
    adc _VERA_DATA0 ;vx_h

    cmp #GAME_AREA_WIDTH
    bcc delete_x_check_passed ;branches if A < value
    ; bullet outside in x axis, delete bullet
    lda #$F7
    deleted_dummy_draw:
    sta _VERA_DATA1 ;px_h
    pha
    pha

    bne bullet_update_done
delete_x_check_passed:
    sta _VERA_DATA1 ;updated px_H
    pha ;px
    phy ;py
    inc bullets_left
bullet_update_done:


    ;render bullets
    ;find addr in _bullet_block
    lda index
    clc
    adc index
    adc index
    tay ;start of _bullet_block[index]

    lda bullets_left
    sta _bullet_block, y
    
    iny
    ldx _bullet_block, y

    jmp (graphic_type_jump_table, x) ;X16 docs have the syntax for this wrong...
.endproc


.proc GraphicDummy
    pla ;py
    pla ;px
    rts
.endproc

.proc GraphicPixel
    aux = _zpa0
    stz _VERA_CTRL
    lda _bitmap_back_buffer
    sta _VERA_ADDRx_H

    ply ;py
    pla ;px
    cpy #GAME_AREA_HEIGHT
    bcc bullet_exists ;branches if A < value
    ;if we are here the bullet is deleted/offscreen
    jmp skip
bullet_exists:
    lsr ;div by two to get pixel coord
    sta aux
    ;calculate VRAM address of top left corner
    ldx lookup_y_high, y
    lda lookup_y_low, y
    clc
    adc aux
    bcc :+
    inx
    :
    stx _VERA_ADDRx_M
    sta _VERA_ADDRx_L

    lda #$11
    sta _VERA_DATA0
skip:




    rts
.endproc