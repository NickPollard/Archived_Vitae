// ticker.h
#ifndef __TICKER_H__
#define __TICKER_H__

// Tick function signature
typedef void (*tickfunc)( void*, float );
// Render function signature
typedef void (*renderfunc)( void*);

// delegate
// A struct holding a list of objects to be ticked (updated)
// All objects have the same tick handler, ie. they are of the same time
// You should create a separate delegate for each type of object you want to tick
// and then tick them each explicitly
//
// eg. 
//  delegate animations;
//  delegate models;
//  delegate physics;
//
//	delegate_tick(animations, dt);
//  delegate_tick(models, dt);
//  delegate_tick(physics, dt);
//
typedef struct {
	void*		tick;
	void**		data;
	int			count;
	int			max;
} delegate;

// tick all objects in a delegate
void delegate_tick(delegate* t, float dt);

// render all objects in a delegate
void delegate_render( delegate* d );

// create a new delegate
delegate* delegate_create(void* func, int size);

// add an entry to a delegate
int delegate_add(delegate* t, void* entry);

int delegate_isFull( delegate* t );

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

///////////////

#endif // __TICKER_H__
