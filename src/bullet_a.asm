.importzp _zpa0
.import _VERA_ADDRx_L, _VERA_ADDRx_M, _VERA_ADDRx_H
.import _VERA_DATA0, _VERA_DATA1, _VERA_CTRL
.importzp VERA_ADDR_INC_160, VERA_ADDR_INC_320

.importzp GAME_AREA_WIDTH_BYTES

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
    sta index

    ;configure vera to read table
    ;DATA0 for reading table, DATA1 for writting updated position
    stz _VERA_CTRL ;addr_sel = 0
    lda #(VERA_ADDR_INC_160 + TABLE_VRAM_PAGE)
    sta _VERA_ADDRx_H
    stz _VERA_ADDRx_M
    ldx #GAME_AREA_WIDTH_BYTES
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
    tax

    lda _VERA_DATA0 ;py_h
    tay
    adc _VERA_DATA0 ;vy_h
    ;YES we are only checking after doing the first two addition
    cpy #$FF
    bne bullet_deleted_check_passed
    ;bullet deleted, do a dummy pass
    ;well, the remainer of the pass...
    lda _VERA_DATA0 ;px_l
    lda _VERA_DATA0 ;vx_l
    lda _VERA_DATA0 ;px_h
    lda _VERA_DATA0 ;vx_h

    lda #$FF
    sta _VERA_DATA1 ;py_l
    sta _VERA_DATA1 ;py_h
    sta _VERA_DATA1 ;px_l
    sta _VERA_DATA1 ;px_h
    pha
    pha

    bne bullet_update_done
bullet_deleted_check_passed:
    stx _VERA_DATA1 ;updated py_l
    sta _VERA_DATA1 ;updated py_h
    pha

    clc
    lda _VERA_DATA0 ;px_l
    adc _VERA_DATA0 ;vx_l
    sta _VERA_DATA1 ;updated px_l

    lda _VERA_DATA0 ;px_h
    adc _VERA_DATA0 ;vx_h
    sta _VERA_DATA1 ;updated px_H
    pha
bullet_update_done:


    ;render bullets
    ;find addr in _bullet_block
    lda index
    clc
    adc index
    adc index
    tay ;start of _bullet_block[index]

    iny
    ldx _bullet_block, y

    jmp (graphic_type_jump_table, x) ;X16 docs have the syntax for this wrong...
.endproc


.proc GraphicDummy
    pla
    sta $9FB9
    pla
    sta $9FB9
    rts
.endproc

.proc GraphicPixel
    pla
    sta $9FB9
    pla
    sta $9FBA
    rts
.endproc