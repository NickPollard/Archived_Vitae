// ticker.c
#include "src/common.h"
#include "src/ticker.h"
#include <assert.h>

// Tick all tickers in the tick list!
// All tickers in a list share the same tick function
// This is to (hopefully) improve cache usage and debugging
void tick_all(ticklist* t, float dt) {
	for (int i = 0; i < t->count; i++) {
		t->tick(t->data[i], dt);
	}
}

ticklist* ticklist_create(tickfunc tickHandler, int size) {
	ticklist* t = (ticklist*)malloc(sizeof(ticklist));
	t->tick = tickHandler;
	t->count = 0;
	t->data = malloc(size * sizeof(void*));
	t->max = size;

	return t;
}

void ticklist_add(ticklist* t, void* entry) {
	assert(t->count < t->max);
	t->data[t->count++] = entry;
}

//////////////
//
// Tick Test
//
//////////////

void tick_tester_tick(void* t, float dt) {
	tick_tester* tester = (tick_tester*)t;
	tester->tickcount += tester->tickinc;
	printf("Tick Tester: Tick count is %d.\n", tester->tickcount);
}
