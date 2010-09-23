// time.c
#include "src/common.h"
#include "src/time.h"

void timer_init(frame_timer* timer) {
	time_v t;
	gettimeofday(&t, NULL);
	timer->oldTime = t.tv_sec * SecToUSec + t.tv_usec;
	timer->fps = 0.f;
}

float timer_getDelta(frame_timer* timer) {
	time_v t;
	gettimeofday(&t, NULL);

	unsigned long long newTime = t.tv_sec * SecToUSec + t.tv_usec;
	float delta = (float)(newTime - timer->oldTime) * uSecToSec;
	timer->oldTime = newTime;

	float fps = 1.f/delta;
	timer->fps = timer->fps * 0.9f + fps * 0.1f;
//	printf("fps: %.2f\n", timer->fps);

	return delta;
}

// Get the time in seconds
float timer_getTimeSeconds(frame_timer* t) {
	return ((float)t->oldTime) * uSecToSec;
}
