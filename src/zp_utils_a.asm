.segment "EXTZP": zeropage
;we have the range of $A9 to $FF to play arround with, neither KERNAL nor CC65 are going to touch this area


; $A9 to $AF, slightly awkward 7 byte area, 7 single-byte pseudo-registers should be good enough
.org $A9
zpa0:   .res 1
zpa1:   .res 1
zpa2:   .res 1
zpa3:   .res 1
zpa4:   .res 1
zpa5:   .res 1
zpa6:   .res 1

; $B0 to $BF, leaving room for 8 variables 16-bit each, probably 4 numbers and 4 pointers
zpc0:   .res 2
zpc1:   .res 2
zpc2:   .res 2
zpc3:   .res 2
zptr0:  .res 2
zptr1:  .res 2
zptr2:  .res 2
zptr3:  .res 2

; $C0 to $FF I'll just leave this reserved for now, 64 bytes is plenty for several stuff
; a 4x4 float transform matrix, lol no
; I doubt we will even need 32-bit varaiables for anything, but we have room for them
; realistically this area will be for an array used in some fancy algorithm and/or reserved storage for some critical varaibles

; aliases to export to C
.exportzp _zpa0, _zpa1, _zpa2, _zpa3, _zpa4, _zpa5, _zpa6
_zpa0 = zpa0
_zpa1 = zpa1
_zpa2 = zpa2
_zpa3 = zpa3
_zpa4 = zpa4
_zpa5 = zpa5
_zpa6 = zpa6

.exportzp _zpc0, _zpc1, _zpc2, _zpc3
_zpc0  = zpc0
_zpc1  = zpc1
_zpc2  = zpc2
_zpc3  = zpc3

.exportzp _zptr0, _zptr1, _zptr2, _zptr3
_zptr0 = zptr0
_zptr1 = zptr1
_zptr2 = zptr2
_zptr3 = zptr3