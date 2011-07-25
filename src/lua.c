// Lua.c
#include "src/common.h"
#include "src/lua.h"
//---------------------
#include "mem/allocator.h"


// Is the luaCallback currently enabled?
int luaCallback_enabled(luaCallback* l) {
	return l->enabled;
}

// Add a callback to the interface
luaCallback* luaInterface_addCallback(luaInterface* i, const char* name) {
	luaCallback* l = &i->callbackArray[i->callbackCount++];
	l->name = name;
	return l;
}

// Create a luaInterface
luaInterface* luaInterface_create() {
	luaInterface* i = mem_alloc(sizeof(luaInterface));
	i->callbackCount = 0;
	memset(&i->callbackArray[0], 0, sizeof(luaCallback) * MAX_CALLBACKS);
	return i;
}

// Find a luaCallback
luaCallback* luaInterface_findCallback(luaInterface* interface, const char* name) {
	for (int i = 0; i < MAX_CALLBACKS; i++) {
		if (strcmp(interface->callbackArray[i].name, name) == 0)
			return &interface->callbackArray[i];
	}
	return NULL;
}

// Register a luaCallback
void luaInterface_registerCallback(luaInterface* i, const char* name, const char* func) {
	luaCallback* callBack = luaInterface_findCallback(i, name);
	callBack->func = func;
	callBack->enabled = true;
}

int LUA_registerCallback( lua_State* l ) {
	printf("Register callback!\n");
	return 0;
}

int LUA_print( lua_State* l ) {
	if ( lua_isstring( l, 1 ) ) {
		const char* string = lua_tostring( l, 1 );
		printf( "LUA: %s\n", string );
	} else
		printf( "Error: LUA: Tried to print a non-string object from Lua.\n" );
	return 0;
}

void lua_registerFunction( lua_State* l, lua_CFunction func, const char* name ) {
    lua_pushcfunction( l, func );
    lua_setglobal( l, name );
}

// Create a Lua State and load it's initial contents from <filename>
lua_State* vlua_create( const char* filename ) {
	lua_State* state = lua_open();
	luaL_openlibs( state );	// Load the Lua libs into our lua state
	if ( luaL_loadfile( state, filename ) || lua_pcall( state, 0, 0, 0))
		printf("Error: Failed loading lua from file %s!\n", filename );

	lua_registerFunction( state, LUA_registerCallback, "registerEventHandler" );
	lua_registerFunction( state, LUA_print, "vprint" );

	// *** Always call init
	LUA_CALL( state, "init" );

	return state;
}
