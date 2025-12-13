; this file is glass-assembler format
    include once "src/include_gen/defines.glass.inc"

    org 0H

Seg0:	        ds	004000H


    SECTION Seg0                        ; "AB"-startup. Code
        INCBIN "startup.rom"
    ENDS