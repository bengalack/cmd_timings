#include "include/defines.h"
#include "include/structures.h"

// --------------------------------------------------------------------------
// 
u8                          g_uLogicalMode;
u8                          g_uLogicalOp;
u8                          g_uShape;
u8                          g_uScreenMode;
u8                          g_uCurrentCmd;
u8                          g_uCurSizeset; // 0 (small) or 1 (big)
bool                        g_bCPUPushOn;
bool                        g_bSpritesOn;
bool                        g_bLineIntTriggered;

u16                         g_nPushVRAMAddrLow;

// --------------------------------------------------------------------------
// SLOTS AND MEMORY
//
u8                          g_uSlotidPage0BIOS; // assert uses this value. It may be risky to have this value buried down with all other vars. FIX?
u8                          g_uSlotidPage0RAM;
u8                          g_uSlotidPage2ROM;
u8                          g_uSlotidPage2RAM;
u8                          g_uCurSlotidPage0;

// --------------------------------------------------------------------------
// MISC
//
u8                          g_myVDPRegs[NUM_STORED_VDP_VALUES_TOTAL]; // Could use the org ones in this project, but old code used this array...