// time.h
#ifndef __TIME_H__
#define __TIME_H__

#include <sys/time.h>

#define uSecToSec 0.000001
#define SecToUSec 1000000

typedef struct timeval time_v;

typedef struct {
	unsigned long long oldTime;
	float fps;
} frame_timer;

// Seed the RNG, using a time-based seed
void rand_init();

// Return a random float between floor and ceiling
float frand( float floor, float ceiling );

// Initialise the timer
void timer_init(frame_timer* timer);

// Get the time since the last frame
float timer_getDelta(frame_timer* timer);

// Get the time in seconds
float timer_getTimeSeconds();

#endif // __TIME_H__
