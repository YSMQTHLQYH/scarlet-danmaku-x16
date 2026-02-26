.import popa

.import _VERA_ADDRx_H, _VERA_ADDRx_M, _VERA_ADDRx_L
.import _VERA_CTRL, _VERA_DATA0
.import _VERA_DC2_FX_CTRL
.import _VERA_DC6_FX_CACHE_L, _VERA_DC6_FX_CACHE_M, _VERA_DC6_FX_CACHE_H, _VERA_DC6_FX_CACHE_U

.importzp _zpa0, _zpa1, _zpc0

SCREEN_WIDTH_BYTES = 160
;constants that ca65 cant realize is just a number
.importzp VERA_ADDR_INC_4
.importzp MEM_BITMAP_1_ADDR_M


.export _BitmapLayerFillRect
.export _BitmapLayerClearGameArea
.export _BitmapSetPixel
.export _LookupYInit, _LookupY
; underscore for exporting to C
_BitmapLayerFillRect = BitmapLayerFillRect
_BitmapLayerClearGameArea = BitmapLayerClearGameArea
_BitmapSetPixel = BitmapSetPixel
_LookupYInit = LookupYInit
_LookupY = LookupY

.segment "DATA"
.org $9D00 - $0800 ;$0800 is cc65's stack size
lookup_y_low:  .res 256
lookup_y_high: .res 256
.reloc
.export lookup_y_low, lookup_y_high

.segment "CODE"


.proc LookupYInit
    lda #0
    ldx #0
    ldy #0
@loop:
    ;write value to table (doing this first because table starts at 00)
    sta lookup_y_low, y
    pha ;cant do abs, y with X register
    txa
    sta lookup_y_high, y
    pla
    ;calculate next value
    clc
    adc #SCREEN_WIDTH_BYTES ;low byte is in A
    bcc :+
    inx ;high byte is in X
:
    ;next iteration
    iny ;also sets Z when Y == 0
    bne @loop ;branches when Z clear
    rts
.endproc

;args (y, starts in A, totally not confusing)
.proc LookupY
    tay ;y is now in Y
    lda lookup_y_high, y
    tax
    lda lookup_y_low, y ;you could argue all regesters have y in them...
    rts ;return values go in AX
.endproc

;args (buffer_n, color, _x, _y), _y starts in A
.proc BitmapSetPixel
    ;vera ctrl
    ldy #0
    sty _VERA_CTRL

    ;vera addr
    jsr LookupY ;_y already in A
    sta _zpa0   ;low
    phx         ;high
    jsr popa ;_x
    plx
    clc
    adc _zpa0
    sta _VERA_ADDRx_L
    bcc :+
    inx
:
    stx _VERA_ADDRx_M

    ;save color for later
    jsr popa ;color
    pha

    ;vera addr_h
    jsr popa ;buffer_n
    and #$01
    sta _VERA_ADDRx_H

    ;color write
    pla
    sta _VERA_DATA0

    rts
.endproc


;args (buffer_n, color, _x, _y, w, h), h starts in A
.proc BitmapLayerFillRect
    ;c_sp 0 = w
    ;c_sp 1 = y
    ;c_sp 2 = x
    ;c_sp 3 = color
    ;c_sp 4 = buffer_n
;  -- calculate size
    sta _zpa1 ;h
    jsr popa ;w
    lsr
    lsr
    lsr ;convet w from pixels to increments of 4x2 bytes
    pha ;w
    jsr popa ;_y
    sta _zpa0 ;_y ;jsr might mess with registers
    jsr popa ;_x


    lsr ;convert _x from pixels to number of bytes offset 
    pha ;_x
;h is goign to be used at the end of a row
;:x and w are only needed at the start of a row
;_y is only needed on the first iteration, it just goes on the cpu stack (last)
;h uses are interleaved with _x/w so we can't have all in the stack
;keeping h in zp and _x/w in the stack
    ldy _zpa0 ;y(1) ;jsr might mess with registers
    phy ;y(1)


;  -- write color to vera cache
    lda #(6 << 1)
    sta _VERA_CTRL
    jsr popa ;color
    and #$0F ;a0 = color & 0x0F
    sta _zpa0
    asl
    asl
    asl
    asl; A <<= 4
    ora _zpa0; A |= a0
    sta _VERA_DC6_FX_CACHE_L
    sta _VERA_DC6_FX_CACHE_M
    sta _VERA_DC6_FX_CACHE_H
    sta _VERA_DC6_FX_CACHE_U

; -- set vera addr_h
    jsr popa ;buffer_n
    and #$01
    ora #VERA_ADDR_INC_4
    sta _VERA_ADDRx_H
    ;enable cache write
    lda #(2 << 1)
    sta _VERA_CTRL
    lda #$40
    sta _VERA_DC2_FX_CTRL

;  -- setup loop
    pla ;_y
    jsr LookupY ; A = low, X = high
    sta _zpc0       ;low
    stx _zpc0 + 1   ;high
    pla ;_x, we only need to apply the offset at the start
    clc
    adc _zpc0
    sta _zpc0
    bcc :+
    inc _zpc0 + 1
:
    ;h is in _zpa1, w is the only thing left in the stack
    ;we can acutally use the index registers as indeces
    ldy _zpa1
    plx
    stx _zpa1 ;switcharoo, w is in _zpa1
@loop_col:
    ;load address to vera
    lda _zpc0 + 1
    sta _VERA_ADDRx_M
    lda _zpc0
    sta _VERA_ADDRx_L
    ;calculate next address
    clc
    adc #SCREEN_WIDTH_BYTES
    sta _zpc0
    bcc :+
    inc _zpc0 + 1
:
    ;setup inner loop
    ldx _zpa1
    lda #0 ;0 to write full cache, $FF to mask full cache, each bit masks each nibble of cache
@loop_row:
    sta _VERA_DATA0
    dex ;sets z if X == 0
    bne @loop_row ;branches if Z clear

    dey ;sets z if Y == 0
    bne @loop_col ;branches if Z clear

;  -- done
;turn off VERA_FX, just in case
    lda #(2 << 1)
    sta _VERA_CTRL
    lda #0
    sta _VERA_DC2_FX_CTRL
    sta _VERA_CTRL
    rts
.endproc


;args (buffer_n, color), color starts in A
;_x, _y, w, h are hardcoded (because it's faster)
;ca65, what do you mean a number literal is not a constant expresion!!!!
.define GAME_AREA_W 28 ;number of writes (4 bytes each, ei 8 pixels each), it's width/8
.define GAME_AREA_H 240
.proc BitmapLayerClearGameArea
;  -- write color to vera cache
    ldx #(6 << 1)
    stx _VERA_CTRL
    ;jsr popa ;color is already on A
    and #$0F ;a0 = color & 0x0F
    sta _zpc0
    asl
    asl
    asl
    asl; A <<= 4
    ora _zpc0; A |= a0
    sta _VERA_DC6_FX_CACHE_L
    sta _VERA_DC6_FX_CACHE_M
    sta _VERA_DC6_FX_CACHE_H
    sta _VERA_DC6_FX_CACHE_U

    ; -- set vera addr_h
    jsr popa ;buffer_n
    and #$01
    ora #VERA_ADDR_INC_4
    sta _VERA_ADDRx_H
    ;enable cache write
    lda #(2 << 1)
    sta _VERA_CTRL
    lda #$40
    sta _VERA_DC2_FX_CTRL

    ;  -- setup loop
    lda #0 ;_y starts at 0
    sta _zpc0       ;low
    sta _zpc0 + 1   ;high
    ;_x = 0 so we don't need to offset here

    ;we can acutally use the index registers as indeces
    ldy #GAME_AREA_H
    ;we don't need an X index, loops is being unrolled 
@loop_col:
    ;load address to vera
    lda _zpc0 + 1
    sta _VERA_ADDRx_M
    lda _zpc0
    sta _VERA_ADDRx_L
    ;calculate next address
    clc
    adc #SCREEN_WIDTH_BYTES
    sta _zpc0
    bcc :+
    inc _zpc0 + 1
:
    lda #0 ;0 to write full cache, $FF to mask full cache, each bit masks each nibble of cache
    ;loop_row:
    .repeat GAME_AREA_W
    sta _VERA_DATA0
    .endrep


    dey ;sets z if Y == 0
    bne @loop_col ;branches if Z clear

;  -- done
;turn off VERA_FX, just in case
    lda #(2 << 1)
    sta _VERA_CTRL
    lda #0
    sta _VERA_DC2_FX_CTRL
    sta _VERA_CTRL
    rts
.endproc