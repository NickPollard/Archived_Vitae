// ticker.h
#ifndef __TICKER_H__
#define __TICKER_H__

// Tick function signature
typedef void (*tickfunc)(void*, float);

// ticklist
// A struct holding a list of objects to be ticked (updated)
// All objects have the same tick handler, ie. they are of the same time
// You should create a separate ticklist for each type of object you want to tick
// and then tick them each explicitly
//
// eg. 
//  ticklist animations;
//  ticklist models;
//  ticklist physics;
//
//	tick_all(animations, dt);
//  tick_all(models, dt);
//  tick_all(physics, dt);
//
typedef struct ticklist_t {
	tickfunc	tick;
	void**		data;
	int			count;
	int			max;
} ticklist;

// tick all objects in a ticklist
void tick_all(ticklist* t, float dt);

// create a new ticklist
ticklist* ticklist_create(tickfunc tickHandler, int size);

// add an entry to a ticklist
void ticklist_add(ticklist* t, void* entry);

//////////////
//
// Tick Test
//
//////////////

typedef struct tick_tester_t {
	int tickcount;
	int tickinc;
} tick_tester;

void tick_tester_tick(void* t, float dt);

#endif // __TICKER_H__
