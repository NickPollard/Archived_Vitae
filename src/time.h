// time.h
#ifndef __TIME_H__
#define __TIME_H__

#include <sys/time.h>

#define uSecToSec 0.000001

typedef struct timeval time_v;

typedef struct {
	unsigned long long oldTime;
	float fps;
} frame_timer;

// Initialise the timer
void frame_timer_init(frame_timer* timer);

// Get the time since the last frame
float frame_timer_delta(frame_timer* timer);

#endif // __TIME_H__
