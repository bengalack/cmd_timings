#pragma once

// NOTE: This one cannot be used directly in asm. So asm does this manually TODO: FIX

// --- RAM-DATA -----------------------------------------------------
//
extern tSEG volatile g_tCurSegmentPage1;
extern tSEG volatile g_tCurSegmentPage2;

// tSEG volatile __at(LOC_CUR_SEG_P1) g_tCurSegmentPage1;
// tSEG volatile __at(LOC_CUR_SEG_P2) g_tCurSegmentPage2;

// ------------------------------------------------------------------
//


#if XL_MAPPER==1 && NEO16MAPPER==0

#define ENABLE_SEGMENT_PAGE1_SEI( data ) \
 { __asm di;   __endasm; }\
 ( *( ( u8* volatile )( SEG_P1_SW + data ) ) = ( (u8)data ) );\
 ( g_tCurSegmentPage1  = data );\
 { __asm ei;   __endasm; }

#define ENABLE_SEGMENT_PAGE2_SEI( data ) \
 { __asm di;   __endasm; }\
 ( *( ( u8* volatile )( SEG_P2_SW + data ) ) = ( (u8)data ) );\
 ( g_tCurSegmentPage2  = data );\
 { __asm ei;   __endasm; }

#define ENABLE_SEGMENT_PAGE1( data ) (*((u8* volatile)(SEG_P1_SW + data)) = ((u8)data));(g_tCurSegmentPage1 = data)
#define ENABLE_SEGMENT_PAGE2( data ) (*((u8* volatile)(SEG_P2_SW + data)) = ((u8)data));(g_tCurSegmentPage2 = data)


#else


// Need to use the volatile keyword to make sdcc NOT add some funky non-needed commands!
#define ENABLE_SEGMENT_PAGE1_SEI( data ) \
 { __asm di;   __endasm; }\
 ( *( ( tSEG* volatile )( SEG_P1_SW ) ) = ( ( tSEG )data ) );\
 ( g_tCurSegmentPage1  = data );\
 { __asm ei;   __endasm; }

#define ENABLE_SEGMENT_PAGE2_SEI( data ) \
 { __asm di;   __endasm; }\
 ( *( ( tSEG* volatile )( SEG_P2_SW ) ) = ( ( tSEG )data ) );\
 ( g_tCurSegmentPage2  = data );\
 { __asm ei;   __endasm; }

#define ENABLE_SEGMENT_PAGE1( data ) (*((tSEG* volatile)(SEG_P1_SW)) = ((tSEG)data));(g_tCurSegmentPage1 = data)
#define ENABLE_SEGMENT_PAGE2( data ) (*((tSEG* volatile)(SEG_P2_SW)) = ((tSEG)data));(g_tCurSegmentPage2 = data)

#endif
