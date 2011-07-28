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
#include "input.h"
#include "physic.h"

void lua_keycodes( lua_State* l );

// *** Helpers ***

void lua_assertnumber( lua_State* l, int index ) {
	assert( lua_isnumber( l, index ));
}
void* lua_toptr( lua_State* l, int index ) {
	lua_assertnumber( l, index );
	int ptr = lua_tonumber( l, index );
	return (void*)ptr;
}


// ***


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

void lua_pushptr( lua_State* l, void* ptr ) {
	int pointer = (int)ptr;
	lua_pushnumber( l, (double)pointer );
}

int LUA_createModelInstance( lua_State* l ) {
	if ( lua_isstring( l, 1 ) ) {
		const char* filename = lua_tostring( l, 1 );
		printf( "LUA: Creating instance of model \"%s\"\n", filename );
		modelInstance* m = modelInstance_create( model_getHandleFromFilename( filename ) );

		printf( "Model pointer: %d.\n", (int)m );
		lua_pushptr( l, m );
		return 1;
	} else {
		printf( "Error: LUA: No filename specified for vcreateModelInstance().\n" );
		return 0;
	}
}

// vscene_addModel( scene, model )
int LUA_scene_addModel( lua_State* l ) {
	printf( "Lua_scene_addModel\n" );
	scene* s = lua_toptr( l, 1 );	
	modelInstance* m = lua_toptr( l, 2 );	
// TODO - fix static reference
//		scene* s = theScene;
//		transform* t = transform_create();
//		m->trans = t;
//		scene_addModel( s, m );
//		scene_addTransform( s, t );
	scene_addModel( s, m );
	return 0;
}

#define LUA_CREATE_( type, func ) \
int LUA_create##type( lua_State* l ) { \
	printf( "Create %s\n", #type ); \
	type* ptr = func(); \
	lua_pushptr( l, ptr ); \
	return 1; \
}

LUA_CREATE_( transform, transform_create )
LUA_CREATE_( physic, physic_create )


int LUA_model_setTransform( lua_State* l ) {
	printf( "lua model set transform\n" );
	lua_assertnumber( l, 1 );
	lua_assertnumber( l, 2 );
	modelInstance* m = lua_toptr( l, 1 );
	transform* t = lua_toptr( l, 2 );
	m->trans = t;
	return 0;
}

int LUA_physic_setTransform( lua_State* l ) {
	printf( "lua physic set transform\n" );
	physic* p = lua_toptr( l, 1 );
	transform* t = lua_toptr( l, 2 );
	p->trans = t;
	return 0;
}

int LUA_physic_activate( lua_State* l ) {
	printf( "lua physic activate.\n" );
	physic* p = lua_toptr( l, 1 );
	engine_addTicker( static_engine_hack, p, physic_tick );
	return 0;
}

vector lua_tovector3( lua_State* l, int i ) {
	return Vector( lua_tonumber( l, i ), lua_tonumber( l, i+1 ), lua_tonumber( l, i+2 ), 0.f );
}

int LUA_physic_setVelocity( lua_State* l ) {
	printf( "lua physic setVelocity.\n" );
	physic* p = lua_toptr( l, 1 );
	vector v = lua_tovector3( l, 2 );
	v.coord.w = 0.f;
	p->velocity = v;
	return 0;
}


int LUA_setWorldSpacePosition( lua_State* l ) {
	if (lua_isnumber( l, 1 ) &&
		   	lua_isnumber( l, 2 ) &&
			lua_isnumber( l, 3 ) &&
			lua_isnumber( l, 4 )) {
		transform* t = lua_toptr( l, 1 );
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

int LUA_keyPressed( lua_State* l ) {
	if ( lua_isnumber( l, 1 ) ) {
		int key = (int)lua_tonumber( l, 1 );
		// TODO - unstatic this!
		input* in = static_engine_hack->input;
		bool pressed = input_keyPressed( in, key );
		lua_pushboolean( l, pressed );
		return 1;
	}
	printf( "Error: Lua: keyPressed called without key specified.\n" );
	return 0;
}


void lua_makeConstantPtr( lua_State* l, const char* name, void* ptr ) {
	lua_pushptr( l, ptr );
	lua_setglobal( l, name ); // Store in the global variable named <name>
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
	if ( luaL_loadfile( state, filename ) || lua_pcall( state, 0, 0, 0)) {
		printf("Error: Failed loading lua from file %s!\n", filename );
		assert( 0 );
	}

	lua_registerFunction( state, LUA_registerCallback, "registerEventHandler" );
	lua_registerFunction( state, LUA_print, "vprint" );
	lua_registerFunction( state, LUA_createModelInstance, "vcreateModelInstance" );
	lua_registerFunction( state, LUA_createphysic, "vcreatePhysic" );
	lua_registerFunction( state, LUA_createtransform, "vcreateTransform" );
	lua_registerFunction( state, LUA_setWorldSpacePosition, "vsetWorldSpacePosition" );
	lua_registerFunction( state, LUA_keyPressed, "vkeyPressed" );
	lua_registerFunction( state, LUA_model_setTransform, "vmodel_setTransform" );
	lua_registerFunction( state, LUA_physic_setTransform, "vphysic_setTransform" );
	lua_registerFunction( state, LUA_scene_addModel, "vscene_addModel" );
	lua_registerFunction( state, LUA_physic_activate, "vphysic_activate" );
	lua_registerFunction( state, LUA_physic_setVelocity, "vphysic_setVelocity" );
	lua_keycodes( state );

	// *** Always call init
	LUA_CALL( state, "init" );

	return state;
}

void lua_setScene( lua_State* l, scene* s ) {
	lua_makeConstantPtr( l, "scene", theScene );
}



// Sets a field for the table that is assumed to be on the top of the Lua stack
void lua_setfieldi( lua_State* l, const char* key, int value ) {
	lua_pushstring( l, key );
	lua_pushnumber( l, (double)value );
	lua_settable( l, -3 ); // The table is at -3, as we have the key and the value on top
}
// Sets a field for the table that is assumed to be on the top of the Lua stack
void lua_setfieldf( lua_State* l, const char* key, float value ) {
	lua_pushstring( l, key );
	lua_pushnumber( l, (double)value );
	lua_settable( l, -3 ); // The table is at -3, as we have the key and the value on top
}

void lua_keycodes( lua_State* l ) {
	lua_newtable( l ); // Create a table
	lua_setfieldi( l, "w", KEY_W );
	lua_setfieldi( l, "a", KEY_A );
	lua_setfieldi( l, "s", KEY_S );
	lua_setfieldi( l, "d", KEY_D );
	lua_setglobal( l, "key" ); // store the table in the 'key' global variable
}
