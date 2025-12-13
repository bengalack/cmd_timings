// --------------------------------------------------------------------------
// CMD_CPU - testing paralellism of VDP commands and CPU push to VDP VRAM
// MSX and above
//
// Author: pal.hansen@gmail.com
// 
// Some abbreviations:
//   NI: No Interrupts allowed (requires DI up front)
//   SEI: Sets EI
//   DI: Sets DI
//
// These sources uses systems hungarian notation. SORRY! about that, but the
// reasons are that I mix a lot of C code and assembly code, and the notation
// really helps. I can't fully trust SDCC either - it is just not as mature
// as compilers in the PC world.
//
// Hungarian notation (ish) variants:
// 
//  u: unsigned char
//  s: signed char
//  n: unsigned short (16 bit)
//  i: signed short (16 bit)
//  l: unsigned long
//  o: object ("not a pointer")
//  p: pointer
//  a: array
//  x: union (crazy stuff!)
// va: variable arguments
// sz: zero-terminated string
// 
// g_: global
// 
// example: g_auJumpPath - a global variable that is an array of unsigned chars
// --------------------------------------------------------------------------

#include <stdbool.h>
#include <string.h> // memcpy, memset, strcpy
// #include <stdio.h> // sprintf (costing 3000 bytes or so!!!)


#include "include/defines.h"
#include "include/structures.h"
#include "include/defines-pages.h"

// --- Declarations ---------------------------------------------------------
extern void     establishSlotIDsNI_fromC(void);
extern void     memAPI_enaSltPg0_NI_fromC(u8 uSlotID);
extern void     memAPI_enaSltPg2_NI_fromC(u8 uSlotID);

// VDP Stuff
extern void     vdp_setBorderColor(u8 color); 
extern void     vdp_setBorderColorNI(u8 color);
extern void     vdpSetColor0OpaqueNI(void);
extern void     vdpSet16x16SpritesNI(void);
extern void     vdp_regWriteNI(u8 data, u8 vdpreg);
extern void     vdp_disableScreenNI(void);
extern void     vdp_enableScreenNI(void);
extern void     vdpSetSpritesDisabledNI(void);
extern void     vdpSetSpritesEnabledNI(void);
extern void     vdpSetLineIntEnabledNI();
extern void     setVdpWriteAddrNI(u8 uHighBit, u16 VRAMAddr) __preserves_regs(b,c,h,l,iyl,iyh); // pump into port VDPIO/0x98 after this
extern void     vdpWriteDataNI(u8 u) __preserves_regs(b,c,d,e,h,l,iyl,iyh);

extern bool     vdpCmdBusyNI() __preserves_regs(b,c,d,e,h,l,iyl,iyh);
extern void     vdpWaitForCmdReadyNI() __preserves_regs(b,c,d,e,h,l,iyl,iyh);
extern void     executeVDPCmdNI(u8 uCmd, VDPCmd* p) __preserves_regs(h,l,iyl,iyh);
extern void     setSimpleIntHandlerNI() __preserves_regs(b,c,d,e,h,l,iyl,iyh);

extern void     pumpThoseOUTIsNI();


// --- VARS -----------------------------------------------------------------
//
extern u8                           g_uLogicalMode;
extern u8                           g_uLogicalOp;
extern u8                           g_uShape;
extern u8                           g_uScreenMode;
extern u8                           g_uCurrentCmd;
extern bool                         g_bCPUPushOn;
extern bool                         g_bSpritesOn;
extern bool                         g_bLineIntTriggered;
extern u16                          g_nPushVRAMAddrLow;
extern u8                           g_uCurSizeset; // 0 or 1

// --- MISC -----------------------------------------------------------------
//
extern volatile u16                 myJIFFY;
extern u8                           g_myVDPRegs[];

// --------------------------------------------------------------------------
// SLOTS AND MEMORY
//
extern u8                           g_uSlotidPage0BIOS;
extern u8                           g_uSlotidPage0RAM;
extern u8                           g_uSlotidPage2ROM;
extern u8                           g_uSlotidPage2RAM;

// --------------------------------------------------------------------------
// STATIC VARS
//
const u8 auLOGICAL_MODES[] = {VDPCMD_HMMM,VDPCMD_LMMM};
const u8 auLOGICAL_OPS[]   = {LOGICAL_OP_IMP, LOGICAL_OP_AND, LOGICAL_OP_OR, LOGICAL_OP_EOR, LOGICAL_OP_TIMP, LOGICAL_OP_TAND, LOGICAL_OP_TOR, LOGICAL_OP_TEOR};

const u8 auSCREENS[]                    = {5,8}; // only two screens supported now, check "checkInput()" to expand
const u8 auPIXELS_PR_BYTE[]             = {2,1}; // must match screens
const u16 anBYTES_PR_LINE[]             = {128,256}; // must match screens

const u32 auPAGE1_NAME_TABLE_ADDRESS[]  = {0x8000,0x10000}; // must match screens

//                                sxl, syl, syh, dxl, dyl, dyh,  wl,  wh,  hl,  hh
#define SHAPE_LANDSCAPE             0,   0,   1,   0,   0,   0, 128,   0,   8,   0
#define SHAPE_PORTRAIT              0,   0,   1,   0,   0,   0,   8,   0, 128,   0
#define SHAPE_BOX_ORIGIN            0,   0,   1,   0,   0,   0,  32,   0,  32,   0
#define SHAPE_BOX_ODD_DEST          0,   0,   1,   1,   0,   0,  32,   0,  32,   0
#define SHAPE_BOX_DOWN              0,   0,   1, 126, 126,   0,  32,   0,  32,   0

// Double the size OR MORE for fast "mode"
// The BOX size is not exact double of the above square, a few pixels less (24)
// #define SHAPE_LANDSCAPE_X2          0,   0,   1,   0,   0,   0, 254,   0, 48,   0
#define SHAPE_LANDSCAPE_X2          0,   0,   1,   0,   0,   0, 128,   0,  16,   0
#define SHAPE_PORTRAIT_X2           0,   0,   1,   0,   0,   0,  16,   0, 128,   0
#define SHAPE_BOX_ORIGIN_X2         0,   0,   1,   0,   0,   0,  46,   0,  44,   0
#define SHAPE_BOX_ODD_DEST_X2       0,   0,   1,   1,   0,   0,  46,   0,  44,   0
#define SHAPE_BOX_DOWN_X2           0,   0,   1, 126, 126,   0,  46,   0,  44,   0


const VDPCmd sCMD_CPRECTS[2][5] =   {
                                        {  // shape, col, arg, cmd
                                            { SHAPE_LANDSCAPE, 0, 0 },
                                            { SHAPE_PORTRAIT, 0, 0 },
                                            { SHAPE_BOX_ORIGIN, 0, 0 },
                                            { SHAPE_BOX_ODD_DEST, 0, 0 },
                                            { SHAPE_BOX_DOWN, 0, 0 }
                                        },
                                        {
                                            { SHAPE_LANDSCAPE_X2, 0, 0 },
                                            { SHAPE_PORTRAIT_X2, 0, 0 },
                                            { SHAPE_BOX_ORIGIN_X2, 0, 0 },
                                            { SHAPE_BOX_ODD_DEST_X2, 0, 0 },
                                            { SHAPE_BOX_DOWN_X2, 0, 0 }
                                        }
                                    };



// --------------------------------------------------------------------------
// FORWARDS
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
// CODE
// --------------------------------------------------------------------------

// ---------------------------------
// 
// ---------------------------------
void spinForever(void)
{
    while(true){}
}

// ---------------------------------
// 
// ---------------------------------
void setSizeSet()
{
    g_uCurSizeset = (u8)((g_uLogicalMode == 0) && (auPIXELS_PR_BYTE[g_uScreenMode] > 1));
}

// ---------------------------------
// 
// ---------------------------------
void calcPushVRAMaddr()
{
    g_nPushVRAMAddrLow = anBYTES_PR_LINE[g_uScreenMode] * 128;
}

// ---------------------------------
// Returns index num of first bit 0-7. If 8, no bits are set
// ---------------------------------
#pragma disable_warning 85	// because var not used in C context
u8 bitnum(u8 u) __preserves_regs(c,d,e,h,l,iyl,iyh)
{
__asm
	ld 	    b,#0xFF
	or 	    a	; reset carry
00001$:
	inc     b
	rra
	jr 	    nc,00001$
	ld 	    a,b
	ret
__endasm;
}

// ---------------------------------
// Simple keyboard check. Beware some keyboards are different (especially the Russian)
// https://map.grauw.nl/articles/keymatrix.php
// Cost: 70 cycles (+call)
// ---------------------------------
#pragma disable_warning 85	// because var not used in C context
u8 checkKeyboardNI(char linenum) __naked __preserves_regs(b,d,e,h,l,iyl,iyh)
{
__asm
	ld		c,a
	in		a,(#0xaa)
	and		#0xf0
	add		a,c
	out		(#0xaa),a 
	in		a,(#0xa9)
	cpl
	ret
__endasm;
}

// ; ----------------------------------------------------------
// ; IN		   A - if 1: restore palette (not on MSX1, confuses openmsx (and probably real hw too then))
// ;           DE - pointer to zero-terminated string
// ; Modifies: ?
// ; Returns: 	-
// ; DI/EI:	Sets EI (both BIOS_EXTROM/BIOS_RSTPLT and BIOS_CHPUT comes out in EI)
// ; BIOS_RSTPLT does not seem to easily work, so we set values
// ; directly for the default values 4 and 15.
// ; ----------------------------------------------------------
#pragma disable_warning 85	// because vars not used in C context
void printScr0W40ErrorMsgViaBIOS(bool bRestorePalette, u8* pzStr) __naked
{
__asm
BIOS_CHPUT  .equ 0x00a2
BIOS_RSTPLT .equ 0x0145
BIOS_EXTROM .equ 0x015f
BIOS_SUBROM .equ 0x015c
BIOS_CHGMOD .equ 0x005F
BIOS_INITXT .equ 0x006C
BIOS_LINL40 .equ 0xF3AE
BIOS_SETPLT .equ 0x014D
 
    push    de
	push	ix
    push    af

    ld      a, #40              ; 40 columns in screen 0, should work on all msxes
    ld      (BIOS_LINL40), a

    call    BIOS_INITXT 

    pop     af
    or      a
    jr      z, 00004$

	; ld		ix, #BIOS_RSTPLT
	; call	BIOS_EXTROM ; restore org palette

 ;  ld      de, #0x0401   ; // nG
 ;  ld      a, #0x17      ; // RB
 ;  ld      ix, #BIOS_SETPLT
 ;  call    BIOS_SUBROM

 ;  ld      de, #0x0F07   ; // nG
 ;  ld      a, #0x77      ; // RB
 ;  ld      ix, #BIOS_SETPLT
 ;  call    BIOS_SUBROM

        ; doing it directly instead
;        di
;        ld      a, #4
;        out	    (VDPPORT1), a			; starting at color 0
;        ld	    a, #128+#16			    ; set this color in indirect register: #16
;        out	    (VDPPORT1), a       

;        ld      a, #0x17       ; // RB
;        out     (VDPPALETTE), a
;        ld      a, #0x01       ; // G
;        out     (VDPPALETTE), a

;        ld      a, #15
;        out	    (VDPPORT1), a			; starting at color 0
;        ld	    a, #128+#16			    ; set this color in indirect register: #16
;        out	    (VDPPORT1), a       

;        ld      a, #0x77       ; // RB
;        out     (VDPPALETTE), a
;        ld      a, #0x07       ; // G
;        out     (VDPPALETTE), a

00004$:
    pop		ix
    pop     hl
	di
00003$:
	ld	    a, (hl)
	and	    a
	ret	    z
	call	BIOS_CHPUT
	inc	    hl
	jr	    00003$
__endasm;
}

// ---------------------------------
//
// ---------------------------------
void initValues(void)
{
    g_uLogicalMode = 0;
    g_uScreenMode = 0;
    g_bLineIntTriggered = false;
    g_bCPUPushOn = true;
    g_bSpritesOn = true;
    g_uLogicalOp = 0;
    g_uShape = 0;
    g_uCurSizeset = 1;

    calcPushVRAMaddr();

    g_uCurrentCmd = auLOGICAL_OPS[g_uLogicalOp] | auLOGICAL_MODES[g_uLogicalMode];
}

// ---------------------------------
//
// ---------------------------------
void storeVDPRegs(void)
{
__asm

    ld hl, #VDP_0
    ld de, #_g_myVDPRegs + 0
    ld bc, #NUM_STORED_VDP_VALUES_0
    ldir                        ; // 0-7


    ld hl, #VDP_8
    ld de, #_g_myVDPRegs + 8
    ld bc, #NUM_STORED_VDP_VALUES_8
    ldir                        ; // 8- (8-23)
__endasm;
}

// ;------------------------------------------------------------------------------
// ; Uses BIOS
// ; Enter in DI, but system sets DI inside as well
//
#pragma disable_warning 85	// because the var is not used in C context
void setScreenModeDI(u8 mode) __naked
{
__asm 
    push    ix
    ex      af, af'         ; // '
	xor		a
	ld		hl, #0xFAF5		; // DPPAGE
	ld		(hl), a
	inc		hl				; // ACPAGE
	ld		(hl), a

    ex      af, af'         ; // '
	ld 		hl, #0xFCAF		; // Save current mode to FCAF 
	ld 		(hl), a
	ld		ix, #0x005f		; // chgmod on bios
	ld		iy, (#0xfcc0)	; // iyh <= (0xfcc1) : EXPTBL(MAIN-ROM SLOT)
	call	0x001c			; // CALSLT (sets DI)
    pop     ix
	ret
__endasm;
}

// ---------------------------------
// https://www.msx.org/wiki/VDP_Mode_Registers#Control_Register_9
// VDP regs must be stored first
// ---------------------------------
void setPALRefreshRateNI(bool bPAL)
{
    u8 val;
    u8* p = &g_myVDPRegs[ 9 ];

    if(bPAL)
        val = (*p) | 0b00000010;
    else
        val = (*p) & 0b11111101;

    vdp_regWriteNI(val, 0x09);
    *p = val;

    // g_bPAL = bPAL;
}

// ---------------------------------
// https://www.msx.org/wiki/VDP_Mode_Registers#Control_Register_9
// VDP regs must be stored first
// ---------------------------------
void set192Lines(bool b192)
{
    u8 val;
    u8* p = &g_myVDPRegs[ 9 ];

    if(!b192)
        val = (*p) | 0b10000000;
    else
        val = (*p) & 0b01111111;

    vdp_regWriteNI(val, 0x09);
    *p = val;
}

// ---------------------------------
//
// ---------------------------------
void setupVDPNI(void)
{
    vdp_regWriteNI(VDPCMD_ABORT, 44);

    memAPI_enaSltPg0_NI_fromC(g_uSlotidPage0BIOS);
    setScreenModeDI(auSCREENS[g_uScreenMode]);
    memAPI_enaSltPg0_NI_fromC(g_uSlotidPage0RAM);

    storeVDPRegs();
    
    setPALRefreshRateNI(false);                         // always get similar results across machines

    if(g_bSpritesOn)
        vdpSetSpritesEnabledNI();
    else
        vdpSetSpritesDisabledNI();

    set192Lines(false);

    vdp_regWriteNI(0, 18); // setadjust

    vdpSetLineIntEnabledNI();
    vdp_regWriteNI(0, 19); // line interrupt at line 0
}

// ---------------------------------
// Get value from MSXBIOS
// Preserves all registers
// Using interslot-call :)
//
// http://map.grauw.nl/resources/msxsystemvars.php
// MSX version number
// 0 = MSX 1
// 1 = MSX 2
// 2 = MSX 2+
// 3 = MSX turbo R
// ---------------------------------
unsigned char getMSXType() __naked
{
__asm

RDSLT       .equ   0x000C
EXPTBL      .equ   0xFCC1
    push    ix
    push    iy
    ; // push    af
    push    bc
    push    de
    push    hl


    ld      a, (#EXPTBL)      ; // BIOS slot
    ld      hl, #0x002D         ; // Location to read
    di
    call    #RDSLT              ; // interslot call. RDSLT needs slot in A, returns value in A. address in HL

    pop     hl

    pop     de
    pop     bc
    ; // pop     af
    pop     iy
    pop     ix

    ret

__endasm;
}

// ---------------------------------
// 
// ---------------------------------
bool checkMinimumRequireMents(void)
{
    // #define MSX_TYPE_MSX_1                      0
    // #define MSX_TYPE_MSX_2                      1
    // #define MSX_TYPE_MSX_2PLUS                  2
    // #define MSX_TYPE_MSX_TURBOR                 3

 	if(getMSXType() == 0) // see above
    {
		printScr0W40ErrorMsgViaBIOS(false, ":-(MSX2 or higher required"); // Note: toggles EI/DI
		return false;
	}

    return true;
}

// ---------------------------------
// Draw diagonal pattern in page 1 + grid in page 0
// ---------------------------------
void drawStuffNI(void)
{
    u32 lAddr = auPAGE1_NAME_TABLE_ADDRESS[g_uScreenMode];
    setVdpWriteAddrNI((lAddr >> 16)&1, lAddr & 0xFFFF);

    u8 n = 1;
    u8 uRows = 212;
    u16 nCols = anBYTES_PR_LINE[g_uScreenMode];
    u16 nTotal = nCols*uRows;

    if(nCols != 256)
    {
        for(u16 i=0; i < nTotal; i++)
        {
            u8 c = n<<4;

            if(++n == 16)
                n = 1;

            c |= n;

            if(++n == 16)
                n = 1;

            vdpWriteDataNI(c); // blank tile
        }
    }
    else
    {
        for(u16 i=0; i < nTotal; i++)
        {
            if(++n == 16)
                n = 1;

            vdpWriteDataNI(n); // blank tile
        }
    }

    // Now, make grid

    u8 x = 0;

    VDPCmd oCmd = {
        .sxl = 0,
        .syl = 0,
        .syh = 0,
        .dxl = 0,
        .dyl = 0,
        .dyh = 0,
        .wl  = 0,
        .wh  = 1,
        .hl  = 212,
        .hh  = 0,
        .col = 0,
        .arg = 0  // vertical
    };

    // Clear everything
    u8 uCmd = VDPCMD_HMMV;
    vdpWaitForCmdReadyNI();
    executeVDPCmdNI(uCmd, &oCmd);

    uCmd = VDPCMD_LINE;
    oCmd.wl = 212;  // line: longl
    oCmd.wh = 0;    // line: longh
    oCmd.hl = 0;
    oCmd.col = 0x44;

    oCmd.arg = 1;   // isvert

    for(u8 x = 0; x < 64; x ++) // first the vertical lines
    {
        oCmd.dxl += 8;
        vdpWaitForCmdReadyNI();
        executeVDPCmdNI(uCmd, &oCmd);
    }

    oCmd.dxl = 0;
    oCmd.wl = 0;    // line: longl
    oCmd.wh = 1;    // line: longh
    oCmd.hl = 0;
    oCmd.arg = 0;   // isvert

    for(u8 y = 0; y < (212/8); y ++)
    {
        oCmd.dyl += 8;
        vdpWaitForCmdReadyNI();
        executeVDPCmdNI(uCmd, &oCmd);
    }

    vdpWaitForCmdReadyNI();
}

// ---------------------------------
// Enter in EI, leave in EI
// ---------------------------------
void checkInput()
{
    disableInterrupt();

    // https://map.grauw.nl/articles/keymatrix.php
    // Relevant keyboard rows (internaltional shown below):
    // row 0	7 &	6 ^	5 %	4$	3 #	2 @	1 !	0)
    // row 3	J	I	H	G	F	E	D	C
    // row 6	F3	F2	F1	CODE	CAPS	GRAPH	CTRL	SHIFT
    // row 7	RET	SELECT	BS	STOP	TAB	ESC	F5	F4
    // row 8	→	↓	↑	←	DEL	INS	HOME	SPACE

    u8 uKey;
    u8 uIndex;

    uKey = checkKeyboardNI(0);                      // -------------- LOGICAL OP
    
    if(uKey != 0)
    {
        uIndex = bitnum(uKey);
        if(uIndex != g_uLogicalOp)
        {
            g_uLogicalOp = uIndex;
            g_uCurrentCmd =  auLOGICAL_OPS[g_uLogicalOp] | auLOGICAL_MODES[g_uLogicalMode];
        }
    }
    else
    {
        u8 uKey2;
        u8 uKey3 = checkKeyboardNI(7);

        uKey  = checkKeyboardNI(6) & 0b11100000;    // -------------- SHAPE
        uKey2 = uKey3 & 0b00000011;
        uKey3 = uKey3 & 0b10000100;

        if(uKey3 != 0)
        {
            bool bSpritesOn = (uKey3 & 0b10000000);

            if(bSpritesOn != g_bSpritesOn)
            {
                g_bSpritesOn = bSpritesOn;

                if(g_bSpritesOn)
                    vdpSetSpritesEnabledNI();
                else
                    vdpSetSpritesDisabledNI();
            }
        }
        else
        {
            if((uKey != 0) || (uKey2 != 0))
            {
                if(uKey != 0)
                    uIndex = bitnum(uKey) - 5;
                else
                    uIndex = bitnum(uKey2) + 3;

                if(uIndex < ARRAY_LENGTH(sCMD_CPRECTS[0]))
                    if(uIndex != g_uShape)
                        g_uShape = uIndex;
            }
            else
            {
                uKey = checkKeyboardNI(8) & 0b11110000; // -------------- auLOGICAL_MODES[] = {VDPCMD_HMMM,VDPCMD_LMMM};

                if(uKey != 0)
                {
                    if((uKey & 0b00010000)>0) // left arrow
                        g_uLogicalMode = 0;
                    else if((uKey & 0b10000000)>0) // right arrow
                        g_uLogicalMode = 1;
                    else
                        g_bCPUPushOn = uKey & 0b00100000; // up arrow

                    g_uCurrentCmd = auLOGICAL_OPS[g_uLogicalOp] | auLOGICAL_MODES[g_uLogicalMode];
                    
                    setSizeSet();
                }
                else
                {
                    uKey = checkKeyboardNI(3);          // -------------- SCREEN MODE

                    if(uKey != 0)
                    {
                        uIndex = bitnum(uKey);

                        if(uIndex < ARRAY_LENGTH(auSCREENS))
                        {
                            if(uIndex != g_uScreenMode)
                            {
                                g_uScreenMode = uIndex;
                                setSizeSet();
                                calcPushVRAMaddr();

                                setupVDPNI();
                                drawStuffNI();
                            }
                        }
                    }
                }
            }
        }
    }

    enableInterrupt();
}

// ---------------------------------
//
// ---------------------------------
void test()
{
    while(true)
    {
        while(!g_bLineIntTriggered){}

        disableInterrupt();
        g_bLineIntTriggered = false;

        vdp_setBorderColorNI(COLOR_WHITE);

        executeVDPCmdNI(g_uCurrentCmd, &sCMD_CPRECTS[g_uCurSizeset][g_uShape]); // ignores if CMD really is busy or not

        // Set to pump data in at (0,128) in page 0 (visible)
        if(g_bCPUPushOn)
        {
            vdp_setBorderColorNI(COLOR_RED);
            setVdpWriteAddrNI(0, g_nPushVRAMAddrLow); // each line is only 128 bytes in scr5
            pumpThoseOUTIsNI();
        }

        if(vdpCmdBusyNI())
        {
            vdp_setBorderColorNI(COLOR_WHITE);
            vdpWaitForCmdReadyNI();
        }

        vdp_setBorderColorNI(COLOR_BLACK);

        enableInterrupt();

        checkInput();
    }
}

// ---------------------------------
// Enter in DI.
// ---------------------------------
void main(void)
{
    // Fix memory first of all
 	establishSlotIDsNI_fromC();                         // Slot-IDs are stored in upper-ram for later use

    initValues();

    if(!checkMinimumRequireMents())                   // Assumes bios still in page 0
        spinForever();

    setupVDPNI();
    drawStuffNI();

    // ================= CAREFUL WITH BIOS AFTER THIS! ================= 
    memAPI_enaSltPg0_NI_fromC(g_uSlotidPage0RAM);
    // ================= CAREFUL WITH BIOS AFTER THIS! ================= 

    setSimpleIntHandlerNI();

    enableInterrupt();

    test();
}