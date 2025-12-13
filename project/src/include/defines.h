// --------------------------------------------------------------------------
// This file is parsed to be included in the asm-files too.
// DO NOT USE BLOCK-COMMENTS - The parser will freak out!
// Put "/* ## IGNORE IN .inc-FILES ## */" as last part to ignore it in the inc-script
// Normal comment: Use CTRL+' in VS-Code :-)
//

// NOTE ABOUT THIS IMPLEMENTATION:
// Code is dependent on VDP ports being successive numbers (I know - hacky)
//
//
#pragma once

// --- ROM MAPPER SWITCHES  -------------------------------------------------
// #define SEG_P0_SW               	        0x5000	// Segment switch on page 0000h-3FFFh (NEO16MAPPER)
#define SEG_P1_SW               	        0x6000	// Segment switch on page 4000h-7FFFh (ASCII 16k Mapper) https://www.msx.org/wiki/MegaROM_Mappers#ASC16_.28ASCII.29
#define SEG_P2_SW               	        0x7000	// Segment switch on page 8000h-BFFFh (ASCII 16k Mapper) https://www.msx.org/wiki/MegaROM_Mappers#ASC16_.28ASCII.29

// --- STATIC LOCATIONS/SIZES -----------------------------------------------
//
// #define LOC_CUR_SEG_P1            	        0xFFEB  // 1 or 2 bytes
// #define LOC_CUR_SEG_P2            	        0xFFE9  // 1 or 2 bytes

// --- BIOS -----------------------------------------------------------------
//
#define VDP_0                               0xF3DF          // #F3DF - #F3E6: registers 0-7
#define VDP_8 	                            0xFFE7          // #FFE7 - #FFF6: registers 8-23
#define LINL40                              0xF3AE

#define NUM_STORED_VDP_VALUES_0             8
#define NUM_STORED_VDP_VALUES_8             2
#define NUM_STORED_VDP_VALUES_TOTAL         (NUM_STORED_VDP_VALUES_0+NUM_STORED_VDP_VALUES_8)

// #define VDP_25 	                            0xFFFA-25     // #FFFA - #FFFC: registers 25-27 (there is no reg 24)

// --- VDP / VRAM ADDRESSES ----------------------------------------------------
//
#define VDPIO                               0x98            // VRAM Data (Read/Write)
#define VDPPORT1                            0x99
#define VDPPALETTE                          0x9A
#define VDPSTREAM                           0x9B

// --- MEGAROM SEGMENTS AND RELATED -----------------------------------------
//
#define SEG_I_P1_STARTUP_SEGMENT            0   // startup code ("AB")

// --- COLORS ---------------------------------------------------------------
//
#define COLOR_BG                            0
#define COLOR_BLACK                         1
#define COLOR_RED                           8
#define COLOR_WHITE                         15

// --- GFX CMD CODING AID ---------------------------------------------------
//
// 
#define VDPCMD_LMMM		                        0b10010000 // LOGICAL COPY BLOCK
#define VDPCMD_LMMV		                        0b10000000 // LOGICAL FILL
#define VDPCMD_ABORT	                        0

#define VDPCMD_HMMC		                        0b11110000 // FAST COPY BLOCK FROM MEM (2 and 2 pix horz)
#define VDPCMD_HMMM		                        0b11010000 // FAST COPY BLOCK (2 and 2 pix horz)
#define VDPCMD_YMMM                             0b11100000 // FASTEST COPY BLOCK (only Y differs)
#define VDPCMD_HMMV		                        0b11000000 // FAST FILL (2 and 2 pix horz)

#define VDPCMD_LINE		                        0b01110000 // LINE

#define LOGICAL_OP_IMP                          0b0000 // DC=SC
#define LOGICAL_OP_AND                          0b0001 // DC=SCxDC
#define LOGICAL_OP_OR                           0b0010 // DC=SC+DC
#define LOGICAL_OP_EOR                          0b0011 // DC=SCxDC+SCxDC
#define LOGICAL_OP_NOT                          0b0100 // DC=SC

#define LOGICAL_OP_TIMP                         0b1000 // if SC=0 then DC=DC else DC=SC
#define LOGICAL_OP_TAND                         0b1001 // if SC=0 then DC=DC else DC=SCxDC
#define LOGICAL_OP_TOR                          0b1010 // if SC=0 then DC=DC else DC=SC+DC
#define LOGICAL_OP_TEOR                         0b1011 // if SC=0 then DC=DC else DC=SCxDC+SCxDC
#define LOGICAL_OP_TNOT                         0b1100 // if SC=0 then DC=DC else DC=SC

// --- Functionality --------------------------------------------------------
//
// 
#define enableInterrupt()	{ __asm ei   __endasm; } /* ## IGNORE IN .inc-FILES ## */
#define disableInterrupt()	{ __asm di   __endasm; } /* ## IGNORE IN .inc-FILES ## */
#define halt()				{ __asm halt __endasm; } /* ## IGNORE IN .inc-FILES ## */
#define break()				{ __asm in a,(0x2e) __endasm; } /* ## IGNORE IN .inc-FILES ## */
#define ARRAY_LENGTH(arr)   (sizeof(arr)/sizeof((arr)[0])) /* ## IGNORE IN .inc-FILES ## */

// --------------------------------------------------------------------------