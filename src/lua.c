// Lua.c
#include "src/common.h"
#include "src/lua.h"
//---------------------
#include "mem/allocator.h"

// temp
#include "render/modelinstance.h"
#include "model.h"
#include "scene.h"
#include "engine.h"
#include "transform.h"


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


// ***


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

int LUA_createModelInstance( lua_State* l ) {
	if ( lua_isstring( l, 1 ) ) {
		const char* filename = lua_tostring( l, 1 );
		printf( "LUA: Creating instance of model \"%s\"\n", filename );
		modelInstance* m = modelInstance_create( model_getHandleFromFilename( filename ) );
		// TODO - fix static reference
		scene* s = theScene;
		transform* t = transform_create();
		m->trans = t;
		scene_addModel( s, m );
		scene_addTransform( s, t );
		printf( "Transform pointer: %d.\n", (int)t );
		int pointer = (int)t;
		lua_pushnumber( l, (double)pointer );
		return 1;
	} else {
		printf( "Error: LUA: No filename specified for vcreateModelInstance().\n" );
		return 0;
	}
}

int LUA_setWorldSpacePosition( lua_State* l ) {
	if (lua_isnumber( l, 1 ) &&
		   	lua_isnumber( l, 2 ) &&
			lua_isnumber( l, 3 ) &&
			lua_isnumber( l, 4 )) {
		int pointer = (int)lua_tonumber( l, 1 );
		transform* t = (transform*)pointer;
		assert( t );
		float x = lua_tonumber( l, 2 );
		float y = lua_tonumber( l, 3 );
		float z = lua_tonumber( l, 4 );
		vector v = Vector( x, y, z, 1.0 );
		matrix m;
		matrix_cpy( m, t->world );
		matrix_setTranslation( m, &v );
		printf( "Transform pointer: %d.\n", (int)t );
		transform_setWorldSpace( t, m );
	}
	return 0;
}


// ***


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
	lua_registerFunction( state, LUA_createModelInstance, "vcreateModelInstance" );
	lua_registerFunction( state, LUA_setWorldSpacePosition, "vsetWorldSpacePosition" );

	// *** Always call init
	LUA_CALL( state, "init" );

	return state;
}
