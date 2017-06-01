; * Amstrad CPC Firmware call definitions
; * Based on The Amstrad CPC Firmware Guide - http://cantrell.org.uk/david/tech/cpc/cpc-firmware/
; * TODO: add all firmware calls, add documentation


; * Kernel functions

; KL CHOKE OFF
kl_choke_off		equ &bcc8

; KL ROM WALK
kl_rom_walk		equ &bccb

; KL INIT_BACK
kl_init_back		equ &bcce

; KL LOG EXT
kl_log_ext		equ &bcd1

; KL FIND COMMAND
kl_find_command		equ &bcd4

; KL NEW FRAME FLY
kl_new_frame_fly	equ &bcd7

; KL ADD FRAME FLY
kl_add_frame_fly	equ &bcda

; KL DEL FRAME FLY
kl_del_frame_fly	equ &bcdd

; KL NEW FAST TICKER
kl_new_fast_ticker	equ &bce0

; KL ADD FAST TICKER
kl_add_fast_ticker	equ &bce3

; KL DEL FAST TICKER
kl_del_fast_ticker	equ &bce6

; KL ADD TICKER
kl_add_ticker		equ &bce9

; KL DEL TICKER
kl_del_ticker		equ &bcec

; KL INIT EVENT
kl_init_event		equ &bcef

; KL EVENT
kl_event		equ &bcf2

; KL SYNC RESET
kl_sync_reset		equ &bcf5

; KL DEL SYNCHRONOUS
kl_del_synchronous	equ &bcf8

; KL NEXT SYNC
kl_next_sync		equ &bcfb

; KL DO SYNC
kl_do_sync		equ &bcfe

; KL DONE SYNC
kl_done_sync		equ &bd01

; KL EVENT DISABLE
kl_event_disable	equ &bd04

; KL EVENT ENABLE
kl_event_enable		equ &bd07

; KL DISARM EVENT
kl_disarm_event		equ &bd0a

; KL TIME PLEASE
kl_time_please		equ &bd0d

; KL TIME SET
kl_time_set		equ &bd10


; * High Kernel Jumpblock

; KL UPPER ROM ENABLE
kl_u_rom_enable		equ &b900

; KL UPPER ROM DISABLE
kl_u_rom_disable	equ &b903

; KL LOWER ROM ENABLE
kl_l_rom_enable		equ &b906

; KL LOWER ROM DISABLE
kl_l_rom_disable	equ &b909

; KL ROM RESTORE
kl_rom_restore		equ &b90c

; KL ROM SELECT
kl_rom_select		equ &b90f

; KL CURR SELECTION
kl_curr_selection	equ &b912

; KL PROBE ROM
kl_probe_rom		equ &b915

; KL ROM DESELECT
kl_rom_deselect		equ &b918

; KL LDIR
kl_ldir			equ &b91b

; KL LDDR
kl_lddr			equ &b91e

; KL POLL SYNCHRONOUS
kl_poll_synchronous	equ &b921

; KL SCAN NEEDED
kl_scan_needed		equ &b92a


; * Key Manager functions

; KM INITIALISE
km_initialise		equ &bb00

; KM RESET
km_reset		equ &bb03

; KM WAIT CHAR
km_wait_char		equ &bb06

; KM READ CHAR
km_read_char		equ &bb09

; KM CHAR RETURN
km_char_return		equ &bb0c

; KM SET EXPAND
km_set_expand		equ &bb0f

; KM GET EXPAND
km_get_expand		equ &bb12

; KM EXP BUFFER
km_exp_buffer		equ &bb15

; KM WAIT KEY
km_wait_key		equ &bb18

; KM READ KEY
km_read_key		equ &bb1b

; KM TEST KEY
km_test_key		equ &bb1e

; KM GET STATE
km_get_state		equ &bb21

; KM GET JOYSTICK
km_get_joystick		equ &bb24

; KM SET TRANSLATE
km_set_translate	equ &bb27

; KM GET TRANSLATE
km_get_translate	equ &bb2a

; KM SET SHIFT
km_set_shift		equ &bb2d

; KM GET SHIFT
km_get_shift		equ &bb30

; KM SET CONTROL
km_set_control		equ &bb33

; KM GET CONTROL
km_get_control		equ &bb36

; KM SET REPEAT
km_set_repeat		equ &bb39

; KM GET REPEAT
km_get_repeat		equ &bb3c

; KM SET DELAY
km_set_delay		equ &bb3f

; KM GET DELAY
km_get_delay		equ &bb42

; KM ARM BREAK
km_arm_break		equ &bb45

; KM DISARM BREAK
km_disarm_break		equ &bb48

; KM BREAK EVENT
km_break_event		equ &bb4b


; * Text VDU functions

; TXT INITIALISE
; Initialises the text VDU to the state it is in when the system is reset
; Includes resetting all indirections, selecting stream 0, setting text paper to 0
; and text pen to 1, moves cursor to the top-left, and setting the text writing mode
; to opaque.
; Exit:  AF, BC, DE and HL are corrupt
txt_initialise		equ &bb4e


; TXT RESET
; Resets VDU indirections and the control code table
; Exit:  AF, BC, DE and HL are corrupt
txt_reset		equ &bb51

; TXT VDU ENABLE
; Allows characters to be printed on the current stream
; Exit:  AF is corrupt
txt_vdu_enable		equ &bb54

; TXT VDU DISABLE
; Prevents characters from being printed on the current stream
; Exit:  AF is corrupt
txt_vdu_disable		equ &bb57

; TXT OUTPUT
; Output a character or control code to the screen
; If in graphics printing mode, then control codes are printed and not obeyed
; Entry:  A contains the character to output (0x00 to 0x1f for control codes)
; Exit:  All registers preserved
txt_output		equ &bb5a

; TXT WR CHAR
; Prints a character at the current cursor position, control codes are printed
; and not obeyed
; Entry:  A contains the character to be printed
; Exit:  AF, BC, DE, and HL are corrupt
txt_wr_char		equ &bb5d

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
txt_rd_char		equ &bb60

; TXT SET GRAPHIC
; Enables or disables graphics print character mode.  Controls codes will
; be printed and not obeyed
; Entry:  To enable, A must be non-zero
;         To disable, A contains 0
; Exit:  AF is corrupt
txt_set_graphic		equ &bb63

; TXT WIN ENABLE
; Sets the boundaries of the current text window in physical coordinates
; Window is not cleared, but the cursor is moved to its top left.
; Entry:  H contains the column number of one edge
;         D contains the column number of the other edge
;         L contains the line number of one edge
;         E contains the line number of the other edge
; Exit:  AF, BC, DE and HL are corrupt
txt_win_enable		equ &bb66

; TXT GET WINDOW
; Returns the size of the current window in physical coordinates
; Exit:  H contains the column number of the left edge
;        D contains the column number of the right edge
;        L contains the line number of the top edge
;        E contains the line number of the bottom edge
;        A is corrupt
;        Carry is false if the window covers the entire screen
txt_get_window		equ &bb69

; TXT CLEAR WINDOW
; Clears the window of the current stream, and moves the cursor to its top-left
; corner
; Exit:  AF, BC, DE and HL are corrupt
txt_clear_window	equ &bb6c

; TXT SET COLUMN
; Set the cursor's horizontal position
; Entry:  A contains the logical column number to move the cursor to.
; Exit:  AF and HL corrupt
txt_set_column		equ &bb6f

; TXT SET ROW
; Set the cursor's vertical position
; Entry:  A contains the logical line number to move the cursor to.
; Exit:  AF and HL corrupt
txt_set_row		equ &bb72

; TXT SET CURSOR
; Set the cursor's vertical and horizontal position
; Entry:  H contains the logical column number
;         L contains the logical line number
; Exit:  AF and HL corrupt
txt_set_cursor		equ &bb75

; TXT GET CURSOR
; Get the cursor's current position
; Roll count is increased when the screen is scrolled down, decreased
; when scrolled up.
; Exit:  H contains the logical column number
;        L contains the logical line number
;        A contains the roll count
;        Flags are corrupt
txt_get_cursor		equ &bb78

; TXT CUR ENABLE
; Allows the text cursor to be displayed (if allowed by TXT CUR ON)
; Intended for use by the user
; Exit:  AF is corrupt
txt_cur_enable		equ &bb7b

; TXT CUR DISABLE
; Prevents the text cursor from being displayed
; Intended for use by the user
; Exit:  AF is corrupt
txt_cur_disable		equ &bb7e

; TXT CUR ON
; Allows the text cursor to be displayed
; Intended for use by the OS
; Exit:  AF is corrupt
txt_cur_on		equ &bb81

; TXT CUR OFF
; Prevents the text cursor from being displayed
; Intended for use by the OS
; Exit:  AF is corrupt
txt_cur_off		equ &bb84

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
txt_validate		equ &bb87

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
txt_set_pen		equ &bb90

; TXT GET PEN
; Gets the foreground pen for the current stream
; Exit:  A contains the pen number
;        Flags are corrupt
txt_get_pen		equ &bb93

; TXT SET PAPER
; Sets the background paper for the current stream
; Entry:  A contains the paper number to use
; Exit:  AF and HL corrupt
txt_set_paper		equ &bb96

; TXT GET PAPER
; Gets the background paper for the current stream
; Exit:  A contains the paper number
;        Flags are corrupt
txt_get_paper		equ &bb99

; TXT INVERSE
; Swaps the current pen and paper colours for the current stream
; Exit:  AF and HL corrupt
txt_inverse		equ &bb9c

; TXT SET BACK
; Sets the character write mode to either opaque or transparent
; Entry:  A contains 0 for opaque mode, non-zero for transparent mode
; Exit:  AF and HL corrupt
txt_set_back		equ &bb9f

; TXT GET BACK
; Gets the character write mode for the current stream
; Exit:  A is non-zero if in transparent mode
;        A contains 0 if in opaque mode
;        DE, HL and flags are corrupt
txt_get_back		equ &bba2

; TXT GET MATRIX
; Gets the address of a character matrix
; Entry:  A contains the character whose matrix is to be found
; Exit:  HL contains the address of the matrix
;        Carry is true if matrix is user-defined, false if in lower ROM
;        A and other flags corrupt
txt_get_matrix		equ &bba5

; TXT SET MATRIX
; Installs a matrix for a user-defined character
; Entry:  A contains the character to be defined
;         HL contains the address of the matrix to be used
; Exit:  Carry is true if character is user-definable, false if not (no action is taken)
;        A, BC, DE, HL and other flags corrupt
txt_set_matrix		equ &bba8

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
txt_set_m_table		equ &bbab

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
txt_get_m_table		equ &bbae

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
txt_str_select		equ &bbb4

; TXT SWAP STREAMS
; Swaps the attributes of two streams
; Exchanges the pen, paper, window size, cursor position, character write mode,
; and graphic character mode between the two streams.
; Entry:  B and C contain the stream numbers to exchanges
; Exit:  AF, BC, DE and HL corrupt
txt_swap_streams	equ &bbb7


; * Graphics VDU functions

; GRA INITIALISE
gra_initialise		equ &bbba

; GRA RESET
gra_reset		equ &bbbd

; GRA MOVE ABSOLUTE
gra_move_absolute	equ &bbc0

; GRA MOVE RELATIVE
gra_move_relative	equ &bbc3

; GRA ASK CURSOR
gra_ask_cursor		equ &bbc6

; GRA SET ORIGIN
gra_set_origin		equ &bbc9

; GRA GET ORIGIN
gra_get_origin		equ &bbcc

; GRA WIN WIDTH
gra_win_width		equ &bbcf

; GRA WIN HEIGHT
gra_win_height		equ &bbcf

; GRA GET W WIDTH
gra_get_w_width		equ &bbd5

; GRA GET W HEIGHT
gra_get_w_height	equ &bbd8

; GRA CLEAR WINDOW
gra_clear_window	equ &bbdb

; GRA SET PEN
gra_set_pen		equ &bbde

; GRA GET PEN
gra_get_pen		equ &bbe1

; GRA SET PAPER
gra_set_paper		equ &bbe4

; GRA GET PAPER
gra_get_paper		equ &bbe7

; GRA PLOT ABSOLUTE
gra_plot_absolute	equ &bbea

; GRA PLOT RELATIVE
gra_plot_relative	equ &bbed

; GRA TEST ABSOLUTE
gra_test_absolute	equ &bbf0

; GRA TEST RELATIVE
gra_test_relative	equ &bbf3

; GRA LINE ABSOLUTE
gra_line_absolute	equ &bbf6

; GRA LINE RELATIVE
gra_line_relative	equ &bbf9

; GRA WR CHAR
gra_wr_char		equ &bbfc



; * Screen Pack	functions

; SCR INITIALISE
scr_initialise		equ &bbff

; SCR RESET
scr_reset		equ &bc02

; SCR SET OFFSET
scr_set_ofset		equ &bc05

; SCR SET BASE
scr_set_base		equ &bc08

; SCR GET LOCATION
scr_get_location	equ &bc0b

; SCR SET MODE
; Set the screen mode
; Resets windows, graphics origin, screen offset is zeroed, and current stream is 0.
; Entry:  A contains the mode number - same as BASIC MODE command
; Exit:  AF, BC, DE and HL corrupt
scr_set_mode		equ &bc0e

; SCR GET MODE
; Gets the current screen mode
; Exit:  If mode is 0:
;          Carry is true
;          Zero is false
;          A contains 0
;        If mode is 1:
;          Carry is false
;          Zero is true
;          A contains 1
;        If mode is 2:
;          Carry and Zero are false
;          A contains 2
;        All other flags are corrupt
scr_get_mode		equ &bc11

; SCR CLEAR
; Clears the whole screen
; Exit:  AF, BC, DE, and HL corrupt
scr_clear		equ &bc14

; SCR CHAR LIMITS
scr_char_limits		equ &bc17

; SCR CHAR POSITION
scr_char_position	equ &bc1a

; SCR DOT POSITION
scr_dot_position	equ &bc1d

; SCR NEXT BYTE
scr_next_byte		equ &bc20

; SCR PREV BYTE
scr_prev_byte		equ &bc23

; SCR NEXT LINE
scr_next_line		equ &bc26

; SCR PREV LINE
scr_prev_line		equ &bc29

; SCR INK ENCODE
scr_ink_encode		equ &bc2c

; SCR INK DECODE
scr_ink_decode		equ &bc2f

; SCR SET INK
scr_set_ink		equ &bc32

; SCR GET INK
scr_get_ink		equ &bc35

; SCR SET BORDER
scr_set_border		equ &bc38

; SCR GET BORDER
scr_get_border		equ &bc3b

; SCR SET FLASHING
scr_set_flashing	equ &bc3e

; SCR GET FLASHING
scr_get_flashing	equ &bc41

; SCR FILL BOX
scr_fill_box		equ &bc44

; SCR FLOOD BOX
scr_flood_box		equ &bc47

; SCR CHAR INVERT
scr_char_invert		equ &bc4a

; SCR HW ROLL
scr_hw_roll		equ &bc4d

; SCR SW ROLL
scr_sw_roll		equ &bc50

; SCR UNPACK
scr_unpack		equ &bc53

; SCR REPACK
scr_repack		equ &bc56

; SCR ACCESS
scr_access		equ &bc59

; SCR PIXELS
scr_pixels		equ &bc5c

; SCR HORIZONTAL
scr_horizontal		equ &bc5f

; SCR VERTICAL
scr_vertical		equ &bc62


; * Cassette/AMSDOS manager

; CAS INITIALISE
cas_initialise		equ &bc65

; CAS SET SPEED
cas_set_speed		equ &bc68

; CAS NOISY
cas_noisy		equ &bc6b

; CAS START MOTOR
cas_start_motor		equ &bc6e

; CAS STOP MOTOR
cas_stop_motor		equ &bc71

; CAS RESTORE MOTOR
cas_restore_motor	equ &bc74

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
cas_in_open		equ &bc77

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
cas_in_close		equ &bc7a

; CAS IN ABANDON
; Abandons an input file
; Exit:  AF, BC, DE and HL corrupt
cas_in_abandon		equ &bc7d

; CAS IN CHAR
; Reads a single byte from a file
; Exit:  If a byte was read:
;          Carry is true
;          Zero is false
;          A contains the byte read
;        If the end of the file was reached:
;          Carry and Zero are false
;          A contains an error number (corrupt on 464)
;        If ESC was pressed:
;          Carry is false
;          Zero is true
;          A contains an error number (corrupt on 464)
;        In all cases:
;          IX and other flags corrupt
cas_in_char		equ &bc80

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
cas_in_direct		equ &bc83

; CAS RETURN
cas_return		equ &bc86

; CAS TEST EOF
cas_test_eof		equ &bc89

; CAS OUT OPEN
cas_out_open		equ &bc8c

; CAS OUT CLOSE
cas_out_close		equ &bc8f

; CAS OUT ABANDON
cas_out_abandon		equ &bc92

; CAS OUT CHAR
cas_out_char		equ &bc95

; CAS OUT DIRECT
cas_out_direct		equ &bc98

; CAS CATALOG
cas_catalog		equ &bc9b

; CAS WRITE
cas_write		equ &bc9e

; CAS READ
cas_read		equ &bca1

; CAS CHECK
cas_check		equ &bca4


; * Sound Manager

; SOUND RESET
sound_reset		equ &bca7

; SOUND QUEUE
sound_queue		equ &bcaa

; SOUND CHECK
sound_check		equ &bcad

; SOUND ARM EVENT
sound_arm_event		equ &bcb0

; SOUND RELEASE
sound_release		equ &bcb3

; SOUND HOLD
sound_hold		equ &bcb6

; SOUND CONTINUE
sound_continue		equ &bcb9

; SOUND AMPL ENVELOPE
sound_ampl_envelope	equ &bcbc

; SOUND TONE ENVELOPE
sound_tone_envelope	equ &bcbf

; SOUND AMPL ADDRESS
sound_a_address		equ &bcc2

; SOUND TONE ADDRESS
sound_t_address		equ &bcc5


; * Machine Pack

; MC BOOT PROGRAM
mc_boot_program		equ &bd13

; MC START PROGRAM
mc_start_program	equ &bd16

; MC WAIT FLYBACK
; Waits until a frame flyback occurs
; Frame flyback occurs when the screen is not being written to, so screen
; manipulations can be made without flickering or ghosting
mc_wait_flyback		equ &bd19

; MC SET MODE
mc_set_mode		equ &bd1c

; MC SCREEN OFFSET
mc_screen_offset	equ &bd1f

; MC CLEAR INKS
mc_clear_inks		equ &bd22

; MC SET INKS
mc_set_inks		equ &bd25

; MC RESET PRINTER
mc_reset_printer	equ &bd28

; MC PRINT CHAR
mc_print_char		equ &bd2b

; MC BUSY PRINTER
mc_busy_printer		equ &bd2e

; MC SEND PRINTER
mc_send_printer		equ &bd31

; MC SOUND REGISTER
mc_sound_register	equ &bd34

; JUMP RESTORE
jump_restore		equ &bd37


; * 664/6128 only

; KM SET LOCKS (664/6128 only)
km_set_locks		equ &bd3a

; KM FLUSH (664/6128 only)
km_flush		equ &bd3d

; TXT ASK STATE (664/6128 only)
txt_ask_state		equ &bd40

; GRA DEFAULT (664/6128 only)
gra_default		equ &bd43

; GRA SET BACK (664/6128 only)
gra_set_back		equ &bd46

; GRA SET FIRST (664/6128 only)
gra_set_first		equ &bd49

; GRA SET LINE MASK (664/6128 only)
gra_set_line_mask	equ &bd4c

; GRA FROM USER (664/6128 only)
gra_from_user		equ &bd4f

; GRA FILL (664/6128 only)
gra_fill		equ &bd52

; SCR SET POSITION (664/6128 only)
scr_set_position	equ &bd55

; MC PRINT TRANSLATION (664/6128 only)
mc_print_translation	equ &bd58

; KL BANK SWITCH (6128 only)
kl_bank_switch		equ &bd5b


; * Firmware Indirections

; TXT DRAW CURSOR
txt_draw_cursor		equ &bdcd

; TXT UNDRAW CURSOR
txt_undraw_cursor	equ &bdd0

; TXT WRITE CHAR
txt_write_char		equ &bdd3

; TXT UNWRITE
txt_unwrite		equ &bdd6

; TXT OUT ACTION
txt_out_action		equ &bdd9

; GRA PLOT
gra_plot		equ &bddc

; GRA TEST
gra_test		equ &bddf

; GRA LINE
gra_line		equ &bde2

; SCR READ
scr_read		equ &bde5

; SCR WRITE
scr_write		equ &bde8

; SCR MODE CLEAR
scr_mode_clear		equ &bdeb

; KM TEST BREAK
km_test_break		equ &bdee

; MC WAIT PRINTER
mc_wait_printer		equ &bdf1

; KM SCAN KEYS
km_scan_keys		equ &bdf4


; * Maths Firmware

; MOVE REAL (664/6128)
move_real		equ &bdc1

; MOVE REAL (464)
move_real_464		equ &bd3d

; INTEGER TO REAL (664/6128)
integer_to_real		equ &bd64

; INTEGER TO REAL (464)
integer_to_real_464	equ &bd40

; BINARY TO REAL (664/6128)
binary_to_real		equ &bd67

; BINARY TO REAL (464)
binary_to_real_464	equ &bd43

; REAL TO INTEGER (664/6128)
real_to_integer		equ &bd6a

; REAL TO INTEGER (464)
real_to_integer_464	equ &bd46

; REAL TO BINARY (664/6128)
real_to_binary		equ &bd6d

; REAL TO BINARY (464)
real_to_binary_464	equ &bd49

; REAL FIX (664/6128)
real_fix		equ &bd70

; REAL FIX (464)
real_fix_464		equ &bd4c

; REAL INT (664/6128)
real_int		equ &bd73

; REAL INT (464)
real_int_464		equ &bd4f

; REAL 10^A (664/6128)
real_10_a		equ &bd79

; REAL 10^A (464)
real_10_a_464		equ &bd55

; REAL ADDITION (664/6128)
real_addition		equ &bd7c

; REAL ADDITION (464)
real_addition_464	equ &bd58

; REAL REVERSE SUBTRACTION (664/6128)
real_reverse_subtraction	equ &bd82

; REAL REVERSE SUBTRACTION (464)
real_reverse_subtraction_464	equ &bd5e

; REAL MULTIPLICATION (664/6128)
real_multiplication	equ &bd85

; REAL MULTIPLICATION (464)
real_multiplication_464	equ &bd61

; REAL DIVISION (664/6128)
real_division		equ &bd88

; REAL DIVISION (464)
real_division_464	equ &bd64

; REAL COMPARISON (664/6128)
real_comparison		equ &bd8e

; REAL COMPARISON (464)
real_comparison_464	equ &bd6a

; REAL UNARY MINUS (664/6128)
real_unary_minus	equ &bd91

; REAL UNARY MINUS (464)
real_unary_minus_464	equ &bd6d

; REAL SIGNUM/SGN (664/6128)
real_signum_sgn		equ &bd94

; REAL SIGNUM/SGN (464)
real_signum_sgn_464	equ &bd70

; SET ANGLE MODE (664/6128)
set_angle_mode		equ &bd97

; SET ANGLE MODE (464)
set_angle_mode_464	equ &bd73

; REAL PI (664/6128)
real_pi			equ &bd9a

; REAL PI (464)
real_pi_464		equ &bd76

; REAL SQR (664/6128)
real_sqr		equ &bd9d

; REAL SQR (464)
real_sqr_464		equ &bd79

; REAL POWER (664/6128)
real_power		equ &bda0

; REAL POWER (464)
real_power_464		equ &bd7c

; REAL LOG (664/6128)
real_log		equ &bda3

; REAL LOG (464)
real_log_464		equ &bd7f

; REAL LOG 10 (664/6128)
real_log_10		equ &bda6

; REAL LOG 10 (464)
real_log_10_464		equ &bd82

; REAL EXP (664/6128)
real_exp		equ &bda9

; REAL EXP (464)
real_exp_464		equ &bd85

; REAL SINE (664/6128)
real_sine		equ &bdac

; REAL SINE (464)
real_sine_464		equ &bd88

; REAL COSINE (664/6128)
real_cosine		equ &bdaf

; REAL COSINE (464)
real_cosine_464		equ &bd8b

; REAL TANGENT (664/6128)
real_tangent		equ &bdb2

; REAL TANGENT (464)
real_tangent_464	equ &bd8e

; REAL ARCTANGENT (664/6128)
real_arctangent		equ &bdb5

; REAL ARCTANGENT (464)
real_arctangent_464	equ &bd91

; REAL SUBTRACTION (464 only)
real_subtraction	equ &bd5b

; REAL EXPONENT ADDITION (464 only)
real_exponent_addition	equ &bd67

; INTEGER ADDITION (464 only)
integer_addition	equ &bdac

; INTEGER SUBTRACTION (464 only)
integer_subtraction	equ &bdaf

; INTEGER REVERSE SUBTRACTION (464 only)
integer_reverse_subtraction	equ &bdb2

; INTEGER MULTIPLICATION (464 only)
integer_multiplicaton	equ &bdb5

; INTEGER DIVISION (464 only)
integer_division	equ &bdb8

; INTEGER DIVISION 2 (464 only)
integer_division_2	equ &bdbb

; INTEGER COMPARISON (464 only)
integer_comparison	equ &bdc4

; INTEGER UNARY MINUS (464 only)
integer_unary_minus	equ &bdc7

; INTEGER SIGNUM/SGN (464 only)
integer_signum_sgn	equ &bdca

; TEXT INPUT (664/6128 only)
text_input		equ &bd5e

; REAL RND (664/6128 only)
real_rnd		equ &bd7f

; REAL RND(0) (664/6128 only)
real_rnd_0		equ &bd8b
