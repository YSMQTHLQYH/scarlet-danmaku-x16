.segment "CODE"

.import popa

.export _SpriteManagerNotifyChanged, _SpriteManagerWriteChanges
.export _sprite_attr_lowest_changed, _sprite_attr_highest_changed

; allias with _ prefix for exposing to C
_SpriteManagerNotifyChanged = SpriteManagerNotifyChanged
_SpriteManagerWriteChanges = SpriteManagerWriteChanges
_sprite_attr_lowest_changed = sprite_attr_lowest_changed
_sprite_attr_highest_changed = sprite_attr_highest_changed



MAX_SPRITES=128
.import MEM_BANK_SPRITE_TABLE
sprite_attr_lowest_changed: .res 8, $80
sprite_attr_highest_changed: .res 8, $00

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

.endproc
