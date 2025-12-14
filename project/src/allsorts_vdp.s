	.allow_undocumented
	.module startup

	.include "include/macros.inc"
	.include "include_gen/defines.inc"

	.area _CODE

; ======================================== VDP STUFF ======================================

;-----------------------------------------------
; extern void | ( u8 uHighBit, void* VRAMAddr ) __preserves_regs(b,c,h,l,iyl,iyh);
_setVdpWriteAddrNI::
_setVdpWriteNI_DE::
	setVdpWriteNI_DE
	ret
;-----------------------------------------------
;extern void     vdpWriteDataNI( u8 u ); __preserves_regs(b,c,d,e,h,l,iyl,iyh);
_vdpWriteDataNI::
 	out 	( VDPIO ), a
	ret
;-----------------------------------------------
;extern void vdpSet16x16SpritesNI();
_vdpSet16x16SpritesNI::
	ld		a, ( _g_myVDPRegs+1 )
	or		#0b00000010
	ld		( _g_myVDPRegs+1 ), a
	ld 		l, #0x01 | 0x80
	jp 		_vdp_preRegWriteNI_asm ; JUMP to this directly, vdpreg in l, data in a (MODIFIES: A)

;-----------------------------------------------
;extern void vdpSetColor0OpaqueNI();
_vdpSetColor0OpaqueNI::
	ld		a, ( _g_myVDPRegs+8 )
	or		#0b00100000
	ld		( _g_myVDPRegs+8 ), a
	ld 		l, #0x08 | 0x80
	jp 		_vdp_preRegWriteNI_asm; JUMP to this directly, vdpreg in l, data in a (MODIFIES: A)

;-----------------------------------------------
;extern void vdpSetSpritesDisabledNI();
_vdpSetSpritesDisabledNI::
	ld		a, ( _g_myVDPRegs+8 )
	or		#0b00000010
	ld		( _g_myVDPRegs+8 ), a
	ld 		l, #0x08
	jp 		_vdp_regWriteNI_asm ; JUMP to this directly, vdpreg in l, data in a (MODIFIES: A)

;-----------------------------------------------
;extern void vdpSetSpritesEnabledNI();
_vdpSetSpritesEnabledNI::
	ld		a, ( _g_myVDPRegs+8 )
	and		#~0b00000010
	ld		( _g_myVDPRegs+8 ), a
	ld 		l, #0x08
	jp 		_vdp_regWriteNI_asm ; JUMP to this directly, vdpreg in l, data in a (MODIFIES: A)

;-----------------------------------------------
;extern void vdpSetLineIntEnabledNI();
_vdpSetLineIntEnabledNI::
	ld		a, ( _g_myVDPRegs+0 )
	or 		#0b00010000
	ld		( _g_myVDPRegs+0 ), a
	ld 		l, #0x00
	jp 		_vdp_regWriteNI_asm ; JUMP to this directly, vdpreg in l, data in a (MODIFIES: A)

;-----------------------------------------------
;extern void vdpSetLineIntDisabledNI();
_vdpSetLineIntDisabledNI::
	ld		a, ( _g_myVDPRegs+0 )
	and		#~0b00010000
	ld		( _g_myVDPRegs+0 ), a
	ld 		l, #0x00
	jp 		_vdp_regWriteNI_asm ; JUMP to this directly, vdpreg in l, data in a (MODIFIES: A)

;------------------------------------------------------------------------------
; void vdp_setBorderColor( unsigned char color )
;
_vdp_setBorderColor::
	di

	vdpSetBorderColorNI

	ei
	ret

;------------------------------------------------------------------------------
; void vdp_setBorderColorNI( unsigned char color )
;
_vdp_setBorderColorNI::
	vdpSetBorderColorNI

	ret

;------------------------------------------------------------------------------
; void vdp_regWriteNI( unsigned char data, unsigned char vdpreg )
; MODIFIES: A
_vdp_regWriteNI::

_vdp_regWriteNI_asm:		; JUMP to this directly, vdpreg in l, data in a
	out		( 0x99 ), a

	ld		a, l
	or		#0x80
	out		( 0x99 ), a

	ret

_vdp_preRegWriteNI_asm:		; JUMP to this directly, vdpreg in l (preOR'ed), data in a
	out		( 0x99 ), a
	ld		a, l
	out		( 0x99 ), a

	ret

;------------------------------------------------------------------------------
_vdp_disableScreenNI::
	ld		a, ( _g_myVDPRegs+1 )
	and		#~0b01000000
	ld		( _g_myVDPRegs+1 ), a


	ld 		l, #0x01 | 0x80
	jp 		_vdp_preRegWriteNI_asm ; JUMP to this directly, vdpreg in l, data in a (MODIFIES: A)

;------------------------------------------------------------------------------
_vdp_enableScreenNI::

	ld		a, ( _g_myVDPRegs+1 )
	or		#0b01000000
	ld		( _g_myVDPRegs+1 ), a

	ld 		l, #0x01 | 0x80
	jp 		_vdp_preRegWriteNI_asm ; JUMP to this directly, vdpreg in l, data in a (MODIFIES: A)


; ---------------------------------------------------------
; IN:		HL - Address of handler
; OUT:      -
; Modifies: af, hl
; Also: 	Disables and disables interrupt
; ---------------------------------------------------------
setInterruptHandler::
	ld 		a, #0xc3 ; JUMP
	ld		( 0x0038 ), a
	ld 		( 0x0038+1 ), hl
	ret

; ---------------------------------------------------------
; Called from outside
; void setSimpleIntHandlerNI() __preserves_regs(b,c,d,e,iyl,iyh);
_setSimpleIntHandlerNI::
	ld		hl, #interruptHandlerSimple
    jp 		setInterruptHandler

; ---------------------------------------------------------
interruptHandlerSimple::
	push	af

	ld		a, #1
	vdpGetStatusNoRestore 					; Clear line int flag. 44+5 cycles gets status for sreg 1
	rra										; is the scanline-flag (bit 0) set?
  	jp 		c, line_interrupt

	xor 	a
	vdpGetStatusNoRestore 	; gets status for sreg 0

	pop		af
	ei
	ret

line_interrupt:
	ld 		a,#1
	ld		(_g_bLineIntTriggered),a

leave_interrupt:
	pop		af
	ei
	ret

;------------------------------------------------------------------------------
;  
; IN:		A: command byte
;  			DE: address of VDPCmd object
; OUT:      
; Modifies: AF, DE, BC
; Also:
;
;void executeVDPCmdNI(u8 uCmd, VDPCmd* p) __preserves_regs(h,l,iyl,iyh);
_executeVDPCmdNI::
	ex 		af, af'
	ld    	a,#32				; Set "Stream mode"
	out   	( VDPPORT1 ), a
	ld    	a,#128 + 17
	out   	( VDPPORT1 ), a    	; R#17 := 32

	ld    	c, #VDPSTREAM
	xor 	a

	ex 		de, hl

	outi						; 32: X (SXL)
	; outi						; 33: 0 (SXH)
	nop
	out 	(c),a
	nop


	outi						; 34: X (SYL)
	outi						; 35: Source page (SYH)

	outi						; 36: X (DXL)
	; outi						; 37: 0 (DXH)
	nop
	out 	(c),a
	ex 		af, af'				; prep the command early (AND abide speed)

	outi						; 38: Y (DYL)
	outi						; 39: Dest Page (DYH)

	outi						; 40: W
	outi						; 41: 0 (NXH)

	outi						; 42: H
	outi						; 43: 0 (NYH)

	outi						; 44: COLOR
	outi						; 45: 0 (ARG)
	; outi						; 46: do it
	ex 		de, hl				; preserve HL as promised (AND abide speed)
	out 	(c),a

	ret

;------------------------------------------------------------------------------
_vdpWaitForCmdReadyNI::
	ld		a,#2
	out		(VDPPORT1),a 		; select status register 2
	ld		a,#128+#15
	out		(VDPPORT1),a
	nop

VDPready:
	in		a,(VDPPORT1)

	and 	#1
	jp		nz, VDPready
	ret

;------------------------------------------------------------------------------
; extern bool vdpCmdBusyNI() __preserves_regs(b,c,d,e,h,l,iyl,iyh);
_vdpCmdBusyNI::
	ld		a,#2
	out		(VDPPORT1),a 		; select status register 2
	ld		a,#128+#15
	out		(VDPPORT1),a
	nop

	in		a,(VDPPORT1)
	and 	#1
	ret

;------------------------------------------------------------------------------
_pumpThoseOUTIsNI::

	ld 		c,#VDPIO
	ld 		hl,#0x4000 ; should be random "data" here

.rept 2048
; .rept 1024
	outi		; 2 bytes
.endm

	ret