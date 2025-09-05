#pragma once
// Data types

#include <stdbool.h>
#include <stdint.h>

/*+----------------------------------------------------------------------------------------------+*/
typedef uint8_t  u8;				///< 8bit unsigned integer
typedef uint16_t u16;				///< 16bit unsigned integer
typedef uint32_t u32;				///< 32bit unsigned integer
typedef uint64_t u64;				///< 64bit unsigned integer
/*+----------------------------------------------------------------------------------------------+*/
typedef int8_t  s8;					///< 8bit signed integer
typedef int16_t s16;				///< 16bit signed integer
typedef int32_t s32;				///< 32bit signed integer
typedef int64_t s64;				///< 64bit signed integer
/*+----------------------------------------------------------------------------------------------+*/
typedef volatile u8  vu8;			///< 8bit unsigned volatile integer
typedef volatile u16 vu16;			///< 16bit unsigned volatile integer
typedef volatile u32 vu32;			///< 32bit unsigned volatile integer
typedef volatile u64 vu64;			///< 64bit unsigned volatile integer
/*+----------------------------------------------------------------------------------------------+*/
typedef volatile s8  vs8;			///< 8bit signed volatile integer
typedef volatile s16 vs16;			///< 16bit signed volatile integer
typedef volatile s32 vs32;			///< 32bit signed volatile integer
typedef volatile s64 vs64;			///< 64bit signed volatile integer

/*+----------------------------------------------------------------------------------------------+*/
typedef float  f32;
typedef double f64;
/*+----------------------------------------------------------------------------------------------+*/
typedef volatile float  vf32;
typedef volatile double vf64;

/*+----------------------------------------------------------------------------------------------+*/
#ifndef NULL
#define NULL 0
#endif
/*+----------------------------------------------------------------------------------------------+*/
