#pragma once

// https://gist.github.com/panzi/6856583
#pragma region Detect endianness
#if (defined(_WIN16) || defined(_WIN32) || defined(_WIN64)) && !defined(__WINDOWS__)

#	define __WINDOWS__

#endif

#if defined(__linux__) || defined(__CYGWIN__) || defined(__GNU__)

#	include <endian.h>

#elif defined(__APPLE__)

#	include <libkern/OSByteOrder.h>

#	define __BYTE_ORDER    BYTE_ORDER
#	define __BIG_ENDIAN    BIG_ENDIAN
#	define __LITTLE_ENDIAN LITTLE_ENDIAN
#	define __PDP_ENDIAN    PDP_ENDIAN

#elif defined(__OpenBSD__)

#	include <endian.h>

#	define __BYTE_ORDER    BYTE_ORDER
#	define __BIG_ENDIAN    BIG_ENDIAN
#	define __LITTLE_ENDIAN LITTLE_ENDIAN
#	define __PDP_ENDIAN    PDP_ENDIAN

#elif defined(__NetBSD__) || defined(__FreeBSD__) || defined(__DragonFly__)

#	include <sys/endian.h>

#elif defined(__WINDOWS__)

#	include <winsock2.h>
#	ifdef __GNUC__
#		include <sys/param.h>
#	endif

#	define __BYTE_ORDER    BYTE_ORDER
#	define __BIG_ENDIAN    BIG_ENDIAN
#	define __LITTLE_ENDIAN LITTLE_ENDIAN
#	define __PDP_ENDIAN    PDP_ENDIAN

#elif defined(__QNXNTO__)

#	include <gulliver.h>

#	define __LITTLE_ENDIAN 1234
#	define __BIG_ENDIAN    4321

#	if defined(__BIGENDIAN__)

#		define __BYTE_ORDER __BIG_ENDIAN

#	elif defined(__LITTLEENDIAN__)

#		define __BYTE_ORDER __LITTLE_ENDIAN

#	else

#		error byte order not supported

#	endif
/* predefs from newer gcc and clang versions: */
#elif defined(__ORDER_LITTLE_ENDIAN__) && defined(__ORDER_BIG_ENDIAN__) && defined(__BYTE_ORDER__)

#	define __LITTLE_ENDIAN 1234
#	define __BIG_ENDIAN    4321

#  if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)

#     define __BYTE_ORDER __LITTLE_ENDIAN

#  elif (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)

#     define __BYTE_ORDER __BIG_ENDIAN

#  else

#     error byte order not supported

#  endif

#elif defined(__hppa__) || \
      defined(__m68k__) || defined(mc68000) || defined(_M_M68K) || \
      (defined(__MIPS__) && defined(__MIPSEB__)) || \
      defined(__ppc__) || defined(__POWERPC__) || defined(__powerpc__) || defined(__PPC__) || \
      defined(__sparc__)
      
#	define __LITTLE_ENDIAN 1234
#	define __BIG_ENDIAN    4321

#  define __BYTE_ORDER   __BIG_ENDIAN 

#else

#	define __LITTLE_ENDIAN 1234
#	define __BIG_ENDIAN    4321

#  define __BYTE_ORDER   __LITTLE_ENDIAN

#endif
#pragma endregion

#define IS_LITTLE_ENDIAN    __BYTE_ORDER == __LITTLE_ENDIAN
#define IS_BIG_ENDIAN       __BYTE_ORDER == __BIG_ENDIAN

#define SWAP_16(x)                                              \
   ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8))
#define SWAP_32(x)                                              \
   ((((x) & 0xff000000U) >> 24) | (((x) & 0x00ff0000U) >>  8) | \
    (((x) & 0x0000ff00U) <<  8) | (((x) & 0x000000ffU) << 24))
#define SWAP_64(x)                                              \
   ((((x) & 0xff00000000000000ull) >> 56) |                     \
    (((x) & 0x00ff000000000000ull) >> 40) |                     \
    (((x) & 0x0000ff0000000000ull) >> 24) |                     \
    (((x) & 0x000000ff00000000ull) >> 8)  |                     \
    (((x) & 0x00000000ff000000ull) << 8)  |                     \
    (((x) & 0x0000000000ff0000ull) << 24) |                     \
    (((x) & 0x000000000000ff00ull) << 40) |                     \
    (((x) & 0x00000000000000ffull) << 56))

#if IS_LITTLE_ENDIAN
   #define BIGENDIAN_16(x) SWAP_16(x)
   #define BIGENDIAN_32(x) SWAP_32(x)
   #define BIGENDIAN_64(x) SWAP_64(x)
   #define LITTLEENDIAN_16(x) x
   #define LITTLEENDIAN_32(x) x
   #define LITTLEENDIAN_64(x) x
#else
   #define BIGENDIAN_16(x) x
   #define BIGENDIAN_32(x) x
   #define BIGENDIAN_64(x) x
   #define LITTLEENDIAN_16(x) SWAP_16(x)
   #define LITTLEENDIAN_32(x) SWAP_32(x)
   #define LITTLEENDIAN_64(x) SWAP_64(x)
#endif
