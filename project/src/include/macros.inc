;-------------------------
; IN:  			A - the value
; 				reg - reg-number
; Modifies: A
; Set a value in reg number n
; Cost: 32 cycles (6 bytes)
;-------------------------
.macro vdp reg
	out 	( VDPPORT1 ), a
	ld  	a, #reg | 0x80   ; sets write bit

	out 	( VDPPORT1 ), a
.endm

;-------------------------
; IN: 		A - the page num [0,3]
; Modifies: A
;-------------------------
.macro setActiveScr5PageFromA
	rrca
	rrca
	rrca
	or 	#0b00011111
	vdp 2
.endm

;-------------------------
; Modifies: A
;-------------------------
.macro vdpOut, registercommand, data
	ld 	a, data
	out ( VDPPORT1 ), a
	ld 	a, registercommand
	out ( VDPPORT1 ), a
.endm

;-------------------------
; Modifies: A
;-------------------------
.macro vdpOutRef, registercommand, addr
	ld 	a, ( addr )
	out ( VDPPORT1 ), a
	ld 	a, registercommand
	out ( VDPPORT1 ), a
.endm

;-------------------------
; Modifies: A
;-------------------------
.macro vdpDisableScreen4and5
    ;ld a, ( #VDP_0 + #1 )    ; disable screen (reset bit 6)
    ;and #0b10111111

    ld a, #0b00100010  ; disable screen (reset bit 6)
	vdp 1
.endm

;-------------------------
; Modifies: A
;       Bit 7	Bit 6	Bit 5	Bit 4	Bit 3	Bit 2	Bit 1	Bit 0	
; R#1:	4/16K	BL		IE0		M1		M2		0		SI		MAG		(MSX1/2/2+/MSX turbo R)
;-------------------------
.macro vdpEnableScreen4and5
    ;ld a, ( #VDP_0 + #1 )    ; enable screen (set bit 6)

    ld a, #0b01100010  ; 
	vdp 1
.endm

;-------------------------
; Modifies: A
; This macro must never
; include parameters
; S#2 must be set in vdp before call
;-------------------------
.macro vdpWaitHBlank ?noparameter_label1, ?noparameter_label2
noparameter_label1:
	in 	a, ( VDPPORT1 )      ; wait until start of HBLANK
	and #0b00100000
	jp 	nz, noparameter_label1
noparameter_label2:
	in 	a,( VDPPORT1 )      ; wait until end of HBLANK
	and #0b00100000
	jp 	z, noparameter_label2
.endm

;-------------------------
; Modifies: A, B
; This macro must never include parameters
; S#2 must be set in vdp before call
; https://www.msx.org/wiki/VDP_Status_Registers#Status_Register_2
;-------------------------
.macro vdpWaitHBlankUsesAB ?noparameter_label1, ?noparameter_label2
	ld	b, #0b00100000
noparameter_label1:
	in 	a, ( VDPPORT1 )      ; wait until start of HBLANK
	and	b
	jp 	nz, noparameter_label1
noparameter_label2:
	in 	a,( VDPPORT1 )      ; wait until end of HBLANK
	and	b
	jp 	z, noparameter_label2
.endm

;-------------------------
; IN:  			A - statusreg
; OUT: 			-
; Modifies: 	A
;-------------------------
.macro vdpSetStatusReg
	out		( VDPPORT1 ), a			;	status register number
	ld		a, #0x8F				;	VDP register R#15 (set 7th bit to signal reg)
	out		( VDPPORT1 ), a			;	out VDP register number
.endm

;-------------------------
; tries to hit (wait until) the first border edge is crossed
; Modifies: A, B
; This macro must never include parameters
; https://www.msx.org/wiki/VDP_Status_Registers#Status_Register_2
; HR is set during scanning of HBLANK area of the screen, i.e. when right and left borders of the screen are drawn.
;-------------------------
.macro vdpWaitForHBlankStartUsesAB ?loop_over_border, ?loop_over_mid

	ld 		a, #2 
	vdpSetStatusReg
	ld		b, #0b00100000		; HR bit. is set during scanning of HBLANK area of the screen

	in 		a, ( VDPPORT1 )
	and		b					
	jp		z, loop_over_mid	; if the first peek: HR==1 (=after mid screen), we bail on this first HR
								; if not, we wait out this border, and as well as next mid screen
loop_over_border:
	in 		a,( VDPPORT1 ) 		; wait until end of HBLANK
	and		b
	jp 		nz, loop_over_border

loop_over_mid:
	in 		a, ( VDPPORT1 )      		; wait until start of HBLANK
	and		b
	jp 		z, loop_over_mid
.endm

;-------------------------
; IN:  			A - statusreg
; OUT: 			A - status-value
; Modifies: 	A
; Cost: 		44+5 cycles :()
;-------------------------
.macro vdpGetStatusNoRestore
	vdpSetStatusReg
	nop
	in		a, ( VDPPORT1 )			;	read VDP S#n
.endm

;-------------------------
; IN:
; OUT:
; Modifies: A
; Also: Must be done after
; vdpGetStatusNoRestore
;-------------------------
.macro vdpRestoreStatusReg0
	xor		a	
	vdpSetStatusReg
.endm

;-------------------------
; IN:  			HL - 16-bit value
; 				A  - bit 16 (=17th bit)
; Modifies: HL, A
; Cost: 124 cycles
; Set VDP port #98 to start writing at address AHL (17-bit) 
;-------------------------
.macro setVdpAddressNI
	rlc     h
	rla
	rlc     h
	rla
	srl     h
	srl     h
	vdp     #14           ; set bits 14-16
	ld      a, l          ; set bits 0-7
	out     ( #VDPPORT1 ), a
	ld      a, h           ;set bits 8-13
.endm

.macro setVdpWriteNI
	setVdpAddressNI
	or      #64           ; + write access
	out     ( #VDPPORT1 ), a       
.endm

;-------------------------
; IN:  		HL - 16-bit value (only 14 are used)
; Modifies: A
; Cost: 50
;-------------------------
.macro setLowerBitsVDPWriteAddressFromHL
	ld		a, l
	out		( VDPPORT1 ), a
	ld		a, h
	and		#0b00111111
	or		#0b01000000 ; sets write
	out		( VDPPORT1 ), a
.endm

;-------------------------
; IN:  			DE - 16-bit value
;				A  - bit 16
; Modifies: DE, A
; Cost: 124 cycles
; Set VDP port #98 to start writing at address AHL (17-bit) 
;-------------------------
.macro setVdpAddressNI_DE
	rlc     d
	rla
	rlc     d
	rla
	srl     d
	srl     d
	vdp     #14           ; set bits 15-17
	ld      a, e          ; set bits 0-7
	out     ( #VDPPORT1 ), a
	ld      a, d           ;set bits 8-14
.endm

.macro setVdpWriteNI_DE
	setVdpAddressNI_DE
	or      #64           ; + write access
	out     ( #VDPPORT1 ), a
.endm

.macro setVdpReadNI_DE
	setVdpAddressNI_DE
	out     ( #VDPPORT1 ), a
.endm

;-------------------------
; Modifies: AF, HL
; Unsigned
; This macro must never include parameters
; This is the fastest possible way to add an 8 bit to 16 bit: 23 cycles
;-------------------------
.macro addAtoHL ?noparametero
	ADD 	A, L
	LD 		L, A
	JR 		NC, noparametero
	INC 	H
noparametero:
.endm

;-------------------------
; Modifies: AF, DE
; Unsigned
; This macro must never include parameters
; This is the fastest possible way to add an 8 bit to 16 bit: 23 cycles
;-------------------------
.macro addAtoDE ?noparametero
	ADD 	A, E
	LD 		E, A
	JR 		NC, noparametero
	INC 	D
noparametero:
.endm

; --------------------------------------
;
; --------------------------------------
.macro setP1ROMSegmentFromA
	di
	ld		( _g_tCurSegmentPage1 ), a		; store (as per standard)
	ei
	ld		( #SEG_P1_SW ), a				; EI effective after this
.endm

; --------------------------------------
;
; --------------------------------------
.macro setP2ROMSegmentFromA
	di
	ld		( _g_tCurSegmentPage2 ), a		; store (as per standard)
	ei
	ld		( #SEG_P2_SW ), a				; EI effective after this
.endm

;-------------------------
; 
; IN: 		
; OUT: 		
; Modifies: A
;-------------------------
.macro vdpWriteRegA ?vdpreg
	out 	( VDPPORT1 ), a
	ld		a, #( vdpreg|0x80 )	
	out 	( VDPPORT1 ), a
.endm

;-------------------------
; Assumes C is set to correct port
; Will do busy waiting to wait, almost, a raster line (227,5 cycles)
; IN: 		vdpreg: number
; 			reg: cpu reg (8bit) working with out(c)
; OUT: 		
; Modifies: A
;-------------------------
.macro vdpWrtPrtCWait1LnVDPRegRegUsesAB ?vdpreg, ?reg, ?gloop

	out 	( c ), reg
	ld		a, #( vdpreg|0x80 )
	ld 		b, #13					; the above: 30 cycles
gloop:
	djnz	gloop					; 14/9
	out 	( VDPPORT1 ), a
.endm

;-------------------------
; Assumes C is set to correct port
; IN: 		
; OUT: 		
; Modifies: A
;-------------------------
.macro vdpWritePortCRegNameRegX ?vdpreg, ?reg 
	out 	( c ), reg
	ld		a, #( vdpreg|0x80 )	
	out 	( VDPPORT1 ), a
.endm

; ------------------------
; IN: 		A - char color
; Modifies: A
;-------------------------
.macro vdpSetBorderColorNoStoreNI
	out	( VDPPORT1 ), a        ; a : [ 0..15 ]
	ld	a, #128 + #7
	out	( VDPPORT1 ), a
.endm

; ------------------------
; Assumes:
; IN: 		A - char color
; Modifies: A
;-------------------------
.macro vdpSetBorderColorNI
	out	( VDPPORT1 ), a        ; a : [ 0..15 ]
	; ld	( _uBgColor ), a
	ld	a, #128 + #7
	out	( VDPPORT1 ), a
.endm

; ; ------------------------
; ; Assumes: 	_uBgColor is available
; ; IN:
; ; Modifies: A
; ;-------------------------
; .macro vdpRestoreBorderColorNI
; 	ld	a, ( _uBgColor )
; 	out	( VDPPORT1 ), a        ; a : [ 0..15 ]
; 	ld	a, #128 + #7
; 	out	( VDPPORT1 ), a
; .endm

;-------------------------
; Runs HALT in case of cease fire
; To allow next interrupt to happen at the precise time
; Assumes:	Being in EI (and leaves in EI)
; Modifies: AF
; No parameter should be given
;-------------------------
.macro haltIfCeaseFireDIEI ?noparameters

	; must be in EI entering to let any waiting interrupts come through to get latest accurate value of _g_bLSCeaseFire
	di
	ld		a, ( _g_bLSCeaseFire )
	or		a
	ei							; happens after next instruction
	jp		z, noparameters

	halt						; in case of cease fire, we must just allow next interrupt to happen at precise time

noparameters:
	; di
.endm

;-------------------------
; A wait, but is releasing the DI constantly, to allow Interrupts to pop by
; le
; Modifies: AF
; Wait for VDP-commands. No parameter should be given
;-------------------------
.macro vdpReadyForgivingDIEI ?forgiving_noparameter

forgiving_noparameter:
	di
	ld	a,#2
	out	( VDPPORT1 ),a          ;select status register 2
	ld	a,#128+#15
	out	( VDPPORT1 ),a

	ei					; always happens AFTER next command, also adds wait
	in	a,( VDPPORT1 )

	and #1
	jp	nz, forgiving_noparameter
.endm




