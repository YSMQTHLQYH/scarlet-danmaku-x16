.import popa

MAX_SPRITES = $80
VERA_SPRITE_ATTR_M = $FC
.importzp _x16_ram_bank
.import HIGH_RAM_START
.import _VERA_ADDRx_H, _VERA_ADDRx_M, _VERA_ADDRx_L
.import _VERA_CTRL, _VERA_DATA0
.importzp _zptr0, _zpa0
;not actually zero page, just a constant but assembler really wants me to define this to only be one byte
.importzp VERA_ADDR_INC_8
.importzp MEM_BANK_SPRITE_TABLE

.export _SpriteManagerNotifyChanged, _SpriteManagerWriteChanges
.export _sprite_attr_lowest_changed, _sprite_attr_highest_changed

; allias with _ prefix for exposing to C
_SpriteManagerNotifyChanged = SpriteManagerNotifyChanged
_SpriteManagerWriteChanges = SpriteManagerWriteChanges
_sprite_attr_lowest_changed = sprite_attr_lowest_changed
_sprite_attr_highest_changed = sprite_attr_highest_changed





.segment "DATA"
sprite_attr_lowest_changed: .res 8, $80
sprite_attr_highest_changed: .res 8, $00

.segment "CODE"

; args: attr-spr
; (low byte is sprite_n, high byte is attr_n)
.proc SpriteManagerNotifyChanged
    ;lda spr ;already passed into A
    ;ldx attr ;also already passed into X
    ;compares attr with sprite_attr_lowest_changed
    ;Carry = 1 if A >= sprite_attr_lowest_changed
    ;Carry = 0 if A < sprite_attr_lowest_changed
    cmp sprite_attr_lowest_changed, x
    bcs @passed_low_check
    ;new value is lower than previous lowest
    sta sprite_attr_lowest_changed, x
@passed_low_check:
    ;compares attr with sprite_attr_highest_changed
    ;Carry = 1 if A >= sprite_attr_highest_changed
    ;Carry = 0 if A < sprite_attr_highest_changed
    cmp sprite_attr_highest_changed, x
    bcc @passed_high_check
    ;new value is higher than previous highest
    sta sprite_attr_highest_changed, x
@passed_high_check:
    ;emulator print
    ;lda sprite_attr_lowest_changed, x
    ;sta $9FB9
    ;lda sprite_attr_highest_changed, x
    ;sta $9FBA
    rts
.endproc


.proc SpriteManagerWriteChanges
    ; set ram bank
    lda #MEM_BANK_SPRITE_TABLE ;ca65 you are kinda dumb too
    sta _x16_ram_bank
    ; setup pointer to high ram
    lda #<HIGH_RAM_START
    sta _zptr0
    lda #>HIGH_RAM_START
    sta _zptr0 + 1
    ; setup vera
    lda #0
    sta _VERA_CTRL
    lda #(VERA_ADDR_INC_8 | 1)
    sta _VERA_ADDRx_H
    ; outter loop (attr_n)
    ldy #0
@attr_loop:
    phy; outter loop index
    ; checks if there's anything to write for attr[y]
    ; we do have something to write if 
    ; _sprite_attr_lowest_changed <= _sprite_attr_highest_changed
    lda _sprite_attr_highest_changed, y
    cmp _sprite_attr_lowest_changed, y; C = 1 if A >= v, C = 0 if A < v
    bcc @skip ;branc if _sprite_attr_highest_changed < _sprite_attr_lowest_changed
    ; we DO have something to write
    pha ;highest to check during loop, should be stored into _zpa0 but we need that for something else
    lda _sprite_attr_lowest_changed, y
    pha ;index starts at lowest (same as highest, pull from stack fist)
    ; VRAM addr, low byte is lowest_n (already in A) * 8 (<<3)
    ldx #VERA_SPRITE_ATTR_M
    asl a
    bcc :+
    inx
    :asl a
    bcc :+
    inx
    :asl a
    bcc :+
    inx
    :stx _VERA_ADDRx_M
    ; offset for attr_n (0-7, value still in Y, but no opcode for A+Y...)
    sty _zpa0
    clc
    adc _zpa0
    sta _VERA_ADDRx_L
    ply ;get back lowest (index during loop)
    pla ;ok now we can store highest in _zpa0
    sta _zpa0 
    ; we are ready, index lowest is in Y, highest is in _zpa0, vera is ready
    dey ;nvm we first need to increment before writting (lowest and highest are both inclusive)
@spr_loop:
    ; actually does the writes (finally)
    iny
    lda (_zptr0), y ; loads byte from high ram table
    sta _VERA_DATA0 ; writes it to vera, autoinc already set for next sprite
    cpy _zpa0 ;C = 0 if Y < highest
    bcc @spr_loop

@skip:
    ; add to pointer for next iteration
    ;skipping from $A080 to $A100, makes writting faster at the cost of using more memory
    ; basically pretending we have up to 256 sprites
    inc _zptr0 + 1
@no_clear:
    ; iteration count
    ply
    iny
    cpy #8
    bne @attr_loop; branches if Z = 0 (_zpa0 != 8) 
    ; reset attr changed arrays
    ldy #0
    lda #0
@rh:
    sta sprite_attr_highest_changed, y
    iny
    cpy #8
    bne @rh
    ; no stx with in abs,y :/
    ldy #0
    lda #$80
@rl:
    sta sprite_attr_lowest_changed, y
    iny
    cpy #8
    bne @rl
.endproc
