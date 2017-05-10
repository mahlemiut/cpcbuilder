; Screen loading test app.

	include "firmware.inc"
	org &8000

.start:
	; enter mode 2 (640x200)
	ld a,2
	call scr_set_mode

	; open file
	ld b,10  ; filename length
	ld hl,.filename  ; filename
	ld de,.buffer  ; buffer
	call cas_in_open
	call z,.openfail  ; display fail message if open fails
	
	; load file contents into RAM
	ld hl,&c000
	call cas_in_direct
	call z,.openfail
	
	; close file
	call &bc7a

	; and exit
	jr .exit

.openfail:
	ld hl,.msg_openfail
.txtloop:
	ld a,(hl)
	cp 0
	jr z,.txtdone
	call txt_output
	inc hl
	jr .txtloop
.txtdone:
	ret
	
.exit:
	ret

.msg_openfail:
	defb "Unable to open screen data."
	defb 13,10,0
.filename:
	defb "shock2.bin"
	defb 0
.buffer:
	defs 2048
	; will be used for file buffer by the BIOS
