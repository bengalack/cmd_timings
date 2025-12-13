;----------------------------------------------------------
;	Segments order (instead of the normal CRT)
;----------------------------------------------------------
	.area _HEADER (ABS)
	.area _CODE
	.area _HOME
	.area _GSINIT
	.area _GSFINAL
	.area _INITIALIZER
	.area _ROMDATA
	.area _DATA
	.area _INITIALIZED
	.area _HEAP