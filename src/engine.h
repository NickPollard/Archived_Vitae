// engine.h
#ifndef __ENGINE_H__
#define __ENGINE_H__

#include "window.h"

// tick - process a frame of game update
int tick();

// init - initialises the engine
void init(int argc, char** argv);

// run - executes the main loop of the engine
void run(window* rootWindow);

#endif // __ENGINE_H__
