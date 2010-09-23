// engine.h
#ifndef __ENGINE_H__
#define __ENGINE_H__

#include "window.h"
#include "time.h"

typedef struct {
	frame_timer* timer;
} engine;

// tick - process a frame of game update
int tick();

// init - initialises the engine
void init(int argc, char** argv);

// run - executes the main loop of the engine
void run();

#endif // __ENGINE_H__
