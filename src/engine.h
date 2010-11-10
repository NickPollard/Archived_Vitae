// engine.h
#ifndef __ENGINE_H__
#define __ENGINE_H__

#include "src/lua.h"

#include "time.h"

// Lua Libraries
#include <lua.h>

extern scene* theScene;

typedef struct {
	frame_timer* timer;
	lua_State* lua;					//!< Engine wide persistant lua state
	luaInterface* callbacks;		//!< Lua Interface for callbacks from the engine
	luaCallback* onTick;			//!< OnTick event handler
} engine;

/*
 *
 *  Static Functions
 *
 */

// tick - process a frame of game update
void tick(int ePtr);

// init - initialises the application
void init(int argc, char** argv);

// run - executes the main loop of the application
void run();

/*
 *
 *  Engine Methods
 *
 */

// Initialise the engine
void engine_init(engine* e, int argc, char** argv);

// Initialise the OpenGL subsystem so it is ready for use
void engine_initOpenGL(engine* e, int argc, char** argv);

// Initialise the Lua subsystem so that it is ready for use
void engine_initLua(engine* e, int argc, char** argv);

// deInit - deInitialises the engine
void engine_deInit(engine* e);

// deInit_lua - deinitialises the Lua interpreter
void engine_deInitLua(engine* e);

// Tick the engine, processing a frame of game update
void engine_tick(engine* e);

// Handle a key press from the user
void engine_handleKeyPress(engine* e, uchar key, int x, int y);


#endif // __ENGINE_H__
