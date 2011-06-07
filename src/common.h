// Standard C libraries
#ifndef __COMMON_H__
#define __COMMON_H__
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "common.fwd.h"

// Boolean defines
#define true 1
#define false 0

#define TERM_RED "[1;31;40m"
#define TERM_GREEN "[1;32;40m"
#define TERM_WHITE "[0;37;40m"

#define then ?
#define otherwise :

// types
typedef unsigned int uint;
typedef unsigned int u32;
typedef unsigned char uchar;
typedef unsigned char ubyte;
typedef unsigned char u8;
typedef const char* String;

inline unsigned long long rdtsc()
{
  #define rdtsc(low, high) \
         __asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high))

  unsigned long low, high;
  rdtsc(low, high);
  return ((unsigned long long)high << 32) | low;
	#undef rdtsc
}

typedef struct vec2_t {
	float x;
	float y;
} vec2;

// Printing
#define printError( format, args... ) 	{ \
											printf( "%sError%s: ", TERM_RED, TERM_WHITE ); \
											printf( format, args ); \
											printf( " [File: %s, Line: %d]\n", __FILE__, __LINE__ ); \
										}

#endif // __COMMON_H__
