; Cartridge test demo

org 0

	; initialise the CRTC
	ld hl,crtc
	ld bc,&bc00
crtc_loop:
	out (c),c
	ld a,(hl)
	inc b
	out (c),a
	dec b
	inc c
	inc hl
	ld a,c
	cp 16
	jr nz,crtc_loop

	; set to mode 0
	ld bc,&7f80
	out (c),c

	; copy in screen data
	ld bc,&df01
	out (c),c
	
	ld bc,&4000
	ld de,&4000
	ld hl,&c000
	ldir

	; set palette
	ld hl,palette
	ld bc,&7f00
pal_loop:
	out (c),c
	ld a,(hl)
	set 6,a
	out (c),a
	inc c
	inc hl
	ld a,16
	cp c
	jr nz,pal_loop
	
loop:  jp loop


palette:	defb 20,4,21,28,24,6,30,0,31,14,7,10,3,11,20,20

crtc:		defb &3f,&28,&2e,&8e,&26,0,&19,&1e,0,7,0,0,&10,0,0,0