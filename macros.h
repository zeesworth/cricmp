#pragma once

#define trace(fmt, ...) printf("%s:%d : \n" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define unimplemented() printf("%s:%s : Not implemented!!!\n", __FILE__, __FUNCTION__)

#if DEBUG
#define dprint(...) printf(__VA_ARGS__)
#define dtrace(fmt, ...) printf("%s:%d : " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define dunimplemented() printf("%s:%s : Not implemented!!!\n", __FILE__, __FUNCTION__)
#else
#define dprint(...) 
#define dtrace(fmt, ...) 
#define dunimplemented() 
#endif

#ifndef max
#define max(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b;       \
})
#endif

#ifndef min
#define min(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b;       \
})
#endif

#ifdef _MSC_VER
// hack
#define strdup _strdup
#endif
