; * Amstrad CPC Firmware call definitions
; * Based on The Amstrad CPC Firmware Guide - http://cantrell.org.uk/david/tech/cpc/cpc-firmware/
; * TODO: add all firmware calls, add documentation


; * Text VDU functions

; TXT INITIALISE
; Initialises the text VDU to the state it is in when the system is reset
; Includes resetting all indirections, selecting stream 0, setting text paper to 0
; and text pen to 1, moves cursor to the top-left, and setting the text writing mode
; to opaque.
; Exit:  AF, BC, DE and HL are corrupt
txt_initialise	equ &bb4e


; TXT RESET
; Resets VDU indirections and the control code table
; Exit:  AF, BC, DE and HL are corrupt
txt_reset	equ &bb51

; TXT VDU ENABLE
; Allows characters to be printed on the current stream
; Exit:  AF is corrupt
txt_vdu_enable	equ &bb54

; TXT VDU DISABLE
; Prevents characters from being printed on the current stream
; Exit:  AF is corrupt
txt_vdu_disable	equ &bb57

; TXT OUTPUT
; Output a character or control code to the screen
; If in graphics printing mode, then control codes are printed and not obeyed
; Entry:  A contains the character to output (0x00 to 0x1f for control codes)
; Exit:  All registers preserved
txt_output	equ &bb5a

; TXT WR CHAR
; Prints a character at the current cursor position, control codes are printed
; and not obeyed
; Entry:  A contains the character to be printed
; Exit:  AF, BC, DE, and HL are corrupt
txt_wr_char	equ &bb5d

; TXT RD CHAR
; Read a character at the current cursor position
; Exit:  If successful:
;          A contains the character read
;          Carry is true
;        otherwise:
;          A = 0
;          Carry is false
;        in all cases:
;          All other flags are corrupt
txt_rd_char	equ &bb60

; TXT SET GRAPHIC
; Enables or disables graphics print character mode.  Controls codes will
; be printed and not obeyed
; Entry:  To enable, A must be non-zero
;         To disable, A contains 0
; Exit:  AF is corrupt
txt_set_graphic	equ	&bb63

; TXT WIN ENABLE
; Sets the boundaries of the current text window in physical coordinates
; Window is not cleared, but the cursor is moved to its top left.
; Entry:  H contains the column number of one edge
;         D contains the column number of the other edge
;         L contains the line number of one edge
;         E contains the line number of the other edge
; Exit:  AF, BC, DE and HL are corrupt
txt_win_enable	equ &bb66

; TXT GET WINDOW
; Returns the size of the current window in physical coordinates
; Exit:  H contains the column number of the left edge
;        D contains the column number of the right edge
;        L contains the line number of the top edge
;        E contains the line number of the bottom edge
;        A is corrupt
;        Carry is false if the window covers the entire screen
txt_get_window	equ &bb69

; TXT CLEAR WINDOW
; Clears the window of the current stream, and moves the cursor to its top-left
; corner
; Exit:  AF, BC, DE and HL are corrupt
txt_clear_window	equ &bb6c

; TXT SET COLUMN
; Set the cursor's horizontal position
; Entry:  A contains the logical column number to move the cursor to.
; Exit:  AF and HL corrupt
txt_set_column	equ &bb6f

; TXT SET ROW
; Set the cursor's vertical position
; Entry:  A contains the logical line number to move the cursor to.
; Exit:  AF and HL corrupt
txt_set_row	equ &bb72

; TXT SET CURSOR
; Set the cursor's vertical and horizontal position
; Entry:  H contains the logical column number
;         L contains the logical line number
; Exit:  AF and HL corrupt
txt_set_cursor	equ &bb75

; TXT GET CURSOR
; Get the cursor's current position
; Roll count is increased when the screen is scrolled down, decreased
; when scrolled up.
; Exit:  H contains the logical column number
;        L contains the logical line number
;        A contains the roll count
;        Flags are corrupt
txt_get_cursor	equ &bb78

; TXT CUR ENABLE
; Allows the text cursor to be displayed (if allowed by TXT CUR ON)
; Intended for use by the user
; Exit:  AF is corrupt
txt_cur_enable	equ &bb7b

; TXT CUR DISABLE
; Prevents the text cursor from being displayed
; Intended for use by the user
; Exit:  AF is corrupt
txt_cur_disable	equ	&bb7e

; TXT CUR ON
; Allows the text cursor to be displayed
; Intended for use by the OS
; Exit:  AF is corrupt
txt_cur_on	equ &bb81

; TXT CUR OFF
; Prevents the text cursor from being displayed
; Intended for use by the OS
; Exit:  AF is corrupt
txt_cur_off	equ &bb84

; TXT VALIDATE
; Checks whether a cursor position is within the current window
; Entry:  H contains the logical column number
;         L contains the logical line number
; Exit:  H contains the logical column where the next character would be printed
;        L contains the logical line number
;        If printing at this location would cause the screen to scroll up:
;          Carry is false
;          B contains &FF
;        If printing at this location would cause the screen to scroll down:
;          Carry is false
;          B contains 0
;        If printing at this location would not cause the screen to scroll:
;          Carry is true
;          B is corrupt
;        Regardless:
;          A and all other flags are corrupt
txt_validate	equ &bb87

; TXT PLACE CURSOR
; Puts a "cursor blob" on the screen at the current cursor location
; It is possible to have more than one cursor in a window; do not use
; this function twice without calling TXT REMOVE CURSOR in between
; Exit:  AF is corrupt
txt_place_cursor	equ &bb8a

; TXT REMOVE CURSOR
; Removes a "cursor blob" from the current cursor location
; Should be used only to remove cursors created by TXT PLACE CURSOR
; Exit:  AF is corrupt
txt_remove_cursor	equ &bb8d

; TXT SET PEN
; Sets the foreground pen for the current stream
; Entry:  A contains the pen number to use
; Exit:  AF and HL corrupt
txt_set_pen	equ &bb90

; TXT GET PEN
; Gets the foreground pen for the current stream
; Exit:  A contains the pen number
;        Flags are corrupt
txt_get_pen	equ &bb93

; TXT SET PAPER
; Sets the background paper for the current stream
; Entry:  A contains the paper number to use
; Exit:  AF and HL corrupt
txt_set_paper	equ &bb96

; TXT GET PAPER
; Gets the background paper for the current stream
; Exit:  A contains the paper number
;        Flags are corrupt
txt_get_paper	equ &bb99

; TXT INVERSE
; Swaps the current pen and paper colours for the current stream
; Exit:  AF and HL corrupt
txt_inverse	equ &bb9c

; TXT SET BACK
; Sets the character write mode to either opaque or transparent
; Entry:  A contains 0 for opaque mode, non-zero for transparent mode
; Exit:  AF and HL corrupt
txt_set_back	equ &bb9f

; TXT GET BACK
; Gets the character write mode for the current stream
; Exit:  A is non-zero if in transparent mode
;        A contains 0 if in opaque mode
;        DE, HL and flags are corrupt
txt_get_back	equ &bba2

; TXT GET MATRIX
; Gets the address of a character matrix
; Entry:  A contains the character whose matrix is to be found
; Exit:  HL contains the address of the matrix
;        Carry is true if matrix is user-defined, false if in lower ROM
;        A and other flags corrupt
txt_get_matrix	equ &bba5

; TXT SET MATRIX
; Installs a matrix for a user-defined character
; Entry:  A contains the character to be defined
;         HL contains the address of the matrix to be used
; Exit:  Carry is true if character is user-definable, false if not (no action is taken)
;        A, BC, DE, HL and other flags corrupt
txt_set_matrix	equ &bba8

; TXT SET M TABLE
; Sets the address of a user-defined matrix table
; Entry:  DE contains the first character in the table
;         HL contains the address of the table in the central 32k of RAM
; Exit:  If no existing tables:
;          Carry is true
;          A and HL corrupt
;        otherwise:
;          Carry is false
;          A contains the first character
;          HL contains the address of the matrix
;        In all cases:
;          BC, DE and all other flags corrupt
txt_set_m_table	equ &bbab

; TXT GET M TABLE
; Gets the address of a user-defined matrix table
; Exit:  If no existing tables:
;          Carry is true
;          A and HL corrupt
;        otherwise:
;          Carry is false
;          A contains the first character
;          HL contains the address of the matrix
;        In all cases:
;          BC, DE and all other flags corrupt
txt_get_m_table	equ &bbae

; TXT GET CONTROLS
; Gets the address of the control code table
; The table has 32 entries of 3 bytes each:
;   Byte 1 = number of parameters
;   Bytes 2 and 3 = address of the routine in lower ROM, to execute the control code
; Exit:  HL contains the address of the table
txt_get_controls	equ &bbb1

; TXT STR SELECT
; Selects a new VDU stream
; Entry:  A contains the stream to change to
; Exit:  A contains the previously selected stream
;        HL and flags is corrupt
txt_str_select	equ &bbb4

; TXT SWAP STREAMS
; Swaps the attributes of two streams
; Exchanges the pen, paper, window size, cursor position, character write mode,
; and graphic character mode between the two streams.
; Entry:  B and C contain the stream numbers to exchanges
; Exit:  AF, BC, DE and HL corrupt
txt_swap_streams	equ &bbb7


; * Screen Pack functions

; SCR SET MODE
; Set the screen mode
; Resets windows, graphics origin, screen offset is zeroed, and current stream is 0.
; Entry:  A contains the mode number - same as BASIC MODE command
; Exit:  AF, BC, DE and HL corrupt
scr_set_mode	equ &bc0e

; * Cassette/AMSDOS manager

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
