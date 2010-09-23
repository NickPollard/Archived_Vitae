// Standard C libraries
#ifndef __COMMON_H__
#define __COMMON_H__
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.fwd.h"

// Boolean defines
#define true 1
#define false 0

// types
typedef unsigned int uint;
typedef unsigned char uchar;

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

#endif // __COMMON_H__
