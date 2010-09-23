// time.c
#include "src/common.h"
#include "src/time.h"

void frame_timer_init(frame_timer* timer) {
	time_v t;
	gettimeofday(&t, NULL);
	timer->oldTime = t.tv_sec * 1000000 + t.tv_usec;
	timer->fps = 0.f;
}

float frame_timer_delta(frame_timer* timer) {
	time_v t;
	gettimeofday(&t, NULL);

	unsigned long long newTime = t.tv_sec * 1000000 + t.tv_usec;
	float delta = (float)(newTime - timer->oldTime) * uSecToSec;
	timer->oldTime = newTime;

	float fps = 1.f/delta;
	timer->fps = timer->fps * 0.9f + fps * 0.1f;
//	printf("fps: %.2f\n", timer->fps);

	return delta;
}
