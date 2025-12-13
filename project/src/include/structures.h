#pragma once

#include <stdbool.h>
#include <stdarg.h>     // va_list

//-----------------------------------------------------------------------------
// unsigned integer types
typedef unsigned char       u8;  ///<  8 bits unsigned integer ("u")
typedef unsigned short      u16; ///< 16 bits unsigned integer ("n")
typedef unsigned long       u32; ///< 32 bits unsigned integer ("l")
typedef unsigned long long  u64; ///< 64 bits unsigned integer

//-----------------------------------------------------------------------------
// signed integer types
typedef signed char         s8;	    ///<  8 bits signed integer ("s")
typedef signed short        s16;	///< 16 bits signed integer ("i")
typedef signed long         s32;	///< 32 bits signed integer
typedef signed long long    s64;	///< 64 bits signed integer

#if XL_MAPPER==1
    typedef unsigned short  tSEG;	// 16bit, word
#else
    typedef unsigned char   tSEG;	// 8bit, byte
#endif

// -------------------------------------------------------------------------
// Len: 12
typedef struct VDPCmd{
    u8                      sxl;
    // u8                      sxh; // only for 
    u8                      syl;
    u8                      syh;    // page
    u8                      dxl;
    // u8                      dxh;
    u8                      dyl;
    u8                      dyh;    // page
    u8                      wl;     // line: longl
    u8                      wh;     // line: longh
    u8                      hl;     // line: shortl
    u8                      hh;     // line: shorth (is ALWAYS 0, isn't it? candidate...)
    u8                      col;
    u8                      arg;    // line: isVert
    // u8                      cmd;
} VDPCmd;
