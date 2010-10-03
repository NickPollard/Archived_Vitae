#ifndef __LUA_H__
#define __LUA_H__

// Lua Libraries
#include <lauxlib.h>
#include <lualib.h>

#define LUA_CALL(lua, func)					\
	lua_getglobal(lua, func);				\
	lua_pcall(lua,	/* args */			0,	\
					/* returns */		0,	\
					/* error handler */ 0);

#define MAX_CALLBACKS 8

// Lua Callback wrapper for Lua event handlers
struct luaCallback_s {
	int enabled;
	const char* name;
	const char* func;
};

typedef struct luaCallback_s luaCallback;

/*
 *	luaInterface - wrapper class for Lua Callbacks
 *  Every class that wants to use Lua callbacks has one of these
 *  This is what is passed to the lua state for control of callbacks
 */
struct luaInterface_s {
	int	callbackCount;
	luaCallback callbackArray[MAX_CALLBACKS];
};

typedef struct luaInterface_s luaInterface;

// Is the luaCallback currently enabled?
int luaCallback_enabled(luaCallback* l);

// Add a callback to the interface
luaCallback* luaInterface_addCallback(luaInterface* i, const char* name);

// Create a luaInterface
luaInterface* luaInterface_create();

// Find a luaCallback
luaCallback* luaInterface_findCallback(luaInterface* interface, const char* name);

// Register a luaCallback
void luaInterface_registerCallback(luaInterface* i, const char* name, const char* func);

int LUA_registerCallback(lua_State* l);

void lua_registerFunction(lua_State* l, lua_CFunction func, const char* name);

#endif // __LUA_H__
