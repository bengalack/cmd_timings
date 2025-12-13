; this file is glass-assembler format

ALIGN: MACRO ?boundary
           ds ?boundary - 1 - ($ + ?boundary - 1) % ?boundary
       ENDM

STD_SUB_HEADER: MACRO
                    DW ROMLOC_LEVEL_TBT - START
                    DW ROMLOC_LEVEL_VERT_ADJUST - START
                    DW ROMLOC_LEVEL_OBJS - START
                    DW ROMLOC_LEVEL_SCI - START
                    DW ROMLOC_LEVEL_IPP -START

                    DB ( ROMLOC_LEVEL_PAT - START ) / 0x4000 ; SEG_OFFSET_PAT
                    DB ( ROMLOC_LEVEL_PTC - START ) / 0x4000 ; SEG_OFFSET_PTC
                    DB ( ROMLOC_LEVEL_SAT - START) / 0x4000 ; SEG_OFFSET_SAT
                ENDM

; ymin, ymax, xmin, xmax
LEFT_LOW:       MACRO
                db 4,12,0,6
                ENDM

RIGHT_LOW:      MACRO
                db 4,12,10,15
                ENDM

TOP_LOW:        MACRO
                db 0,6,4,12
                ENDM

BOTTOM_LOW:     MACRO
                db 10,15,4,12
                ENDM

LEFT_TALL:      MACRO
                db 4,12,4,15
                ENDM

RIGHT_TALL:     MACRO
                db 4,12,0,12
                ENDM

TOP_TALL:       MACRO
                db 0,12,4,12
                ENDM

BOTTOM_TALL:    MACRO
                db 4,15,4,12
                ENDM

COLL_FULL:      MACRO
                db 0,15,0,15
                ENDM

; used in mono - a tiny line to ride on
TOP_TINY        MACRO
                db 0,1,1,15
                ENDM

TOP_LOW_WIDE:   MACRO
                db 0,7,0,15
                ENDM

BOTTOM_LOW_WIDE:MACRO
                db 10,15,0,15
                ENDM

CYLINDER_VERT:  MACRO
                db 0,15,4,12
                ENDM

