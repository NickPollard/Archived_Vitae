// engine.h
#ifndef __ENGINE_H__
#define __ENGINE_H__

#include "time.h"

// Lua Libraries
#include <lua.h>

extern scene* theScene;

typedef struct {
	frame_timer* timer;
	lua_State* lua;
} engine;

// tick - process a frame of game update
int tick();

// init - initialises the engine
void init(int argc, char** argv);

// deInit - deInitialises the engine
void deInit(engine* e);

// run - executes the main loop of the engine
void run();

#endif // __ENGINE_H__
