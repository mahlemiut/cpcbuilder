; Amstrad CPC Firmware call definitions
; Based on The Amstrad CPC Firmware Guide - http://cantrell.org.uk/david/tech/cpc/cpc-firmware/
; TODO: add all firmware calls, add documentation


; Text VDU functions

; TXT OUTPUT
; Output a character or control code to the screen
; Entry:  A contains the character to output (0x00 to 0x1f for control codes)
; Exit:  All registers preserved
txt_output	equ &bb5a

; Screen Pack functions

; SCR SET MODE
; Set the screen mode
; Resets windows, graphics origin, screen offset is zeroed, and current stream is 0.
; Entry:  A contains the mode number - same as BASIC MODE command
; Exit:  AF, BC, DE and HL corrupt
scr_set_mode	equ &bc0e

; Cassette/AMSDOS manager

; CAS IN OPEN
; Opens an input buffer and reads the first block of the file
; Entry:  B contains the length of the filename
;         HL points to the filename's address
;         DE points to the address of the input buffer
; Exit:  If file was opened successfully:
;          Carry is true, Zero is false
;          HL points to the address of a buffer containing the header data
;          DE points to the address of the destination of the file
;          BC contains the file's length
;          A contains the file's type
;        If file is already open:
;          Carry and Zero are false
;          A contains an error number (664/6128 only)
;          BC, DE, HL corrupt
;        If user pressed ESC:
;          Carry is false, Zero is true
;          A contains an error number (664/6128 only)
;          BC, DE, HL corrupt
;        In all cases:
;          IX and other flags are corrupt
cas_in_open	equ &bc77

; CAS IN CLOSE
; Closes an input file
; Exit:  If successful:
;          Carry is true
;          A is corrupt
;        If the stream was not open:
;          Carry is false
;          A contains the error number (664/6128 only)
;        In all cases:
;          BC, DE, HL and all other flags corrupt
cas_in_close	equ &bc7a

; CAS IN DIRECT
; Read an entire file into memory
; Entry:  HL points to the address to load the file to.
; Exit:  If successful:
;          Carry is true, Zero is false
;          HL contains the entry address
;          A is corrupt
;        If file is not open:
;          Carry and Zero are false
;          HL is corrupt
;          A contains error number (664/6128) or is corrupt (464)
;        If ESC was pressed:
;          Carry is false, Zero is true
;          HL is corrupt
;          A contains error number (664/6128) or is corrupt (464)
;        In all cases:
;          BC, DE, IX and other flags corrupt
cas_in_direct	equ &bc83
