	; SDCC CRT0 for ROM (ascii16x)
	; author bengalack 2024

	.module crt0

	.globl	_main

	.area _HEADER (ABS)
	.org    0x4000

;----------------------------------------------------------
;	ROM Header
	.db		#0x41				; ROM ID
	.db		#0x42				; ROM ID
	.dw		#init				; Program start
	.dw		#0x0000				; BASIC's CALL instruction not expanded
	.dw		#0x0000				; BASIC's IO DEVICE not expanded
	.dw		#0x0000	        	; BASIC program
	.dw		#0x0000				; Reserved
	.dw		#0x0000				; Reserved
	.dw		#0x0000				; Reserved

	; .ascii	"ASCII16X"

init:: 							; will enter in DI initially!
	jp		_main				

;----------------------------------------------------------
;	Segments order
;----------------------------------------------------------
	.area _CODE
	.area _HOME
	.area _GSINIT
	.area _GSFINAL
	.area _INITIALIZER
	.area _ROMDATA
	.area _DATA
	.area _INITIALIZED
	.area _HEAP