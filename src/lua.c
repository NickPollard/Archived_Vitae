// Lua.c
#include "src/common.h"
#include "src/lua.h"
//---------------------
#include "mem/allocator.h"

// temp
#include "camera/chasecam.h"
#include "camera/flycam.h"
#include "model.h"
#include "scene.h"
#include "engine.h"
#include "transform.h"
#include "input.h"
#include "particle.h"
#include "physic.h"
#include "render/modelinstance.h"
#include "render/texture.h"
#include "system/file.h"

#define MAX_LUA_VECTORS 64
vector lua_vectors[MAX_LUA_VECTORS];
int lua_vector_count;

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

void lua_init() {
	// the vector count starts at 0, is reset to 0 every frame
	// this is for storing temporary vectors
	lua_vector_count = 0;
}

void lua_preTick( float dt ) {
	(void)dt;
	// reset vectors at the start of every tick
	lua_vector_count = 0;
}


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
	(void)l;
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

		lua_pushptr( l, m );
		return 1;
	} else {
		printf( "Error: LUA: No filename specified for vcreateModelInstance().\n" );
		return 0;
	}
}

int LUA_deleteModelInstance( lua_State* l ) {
	modelInstance* m = lua_toptr( l, 1 );
	mem_free( m );
	return 0;
}

// vscene_addModel( scene, model )
int LUA_scene_addModel( lua_State* l ) {
	scene* s = lua_toptr( l, 1 );	
	modelInstance* m = lua_toptr( l, 2 );	
	vAssert( m->trans );
	scene_addModel( s, m );
	return 0;
}
int LUA_scene_removeModel( lua_State* l ) {
	scene* s = lua_toptr( l, 1 );	
	modelInstance* m = lua_toptr( l, 2 );	
	scene_removeModel( s, m );
	return 0;
}

#if LUA_DEBUG
#define LUA_DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define LUA_DEBUG_PRINT(...)
#endif

#define LUA_CREATE_( type, func ) \
int LUA_create##type( lua_State* l ) { \
	LUA_DEBUG_PRINT( "Create %s\n", #type ); \
	type* ptr = func(); \
	lua_pushptr( l, ptr ); \
	return 1; \
}

LUA_CREATE_( transform, transform_create )
LUA_CREATE_( physic, physic_create )


int LUA_model_setTransform( lua_State* l ) {
	LUA_DEBUG_PRINT( "lua model set transform\n" );
	lua_assertnumber( l, 1 );
	lua_assertnumber( l, 2 );
	modelInstance* m = lua_toptr( l, 1 );
	transform* t = lua_toptr( l, 2 );
	m->trans = t;
	return 0;
}

int LUA_physic_setTransform( lua_State* l ) {
	LUA_DEBUG_PRINT( "lua physic set transform\n" );
	physic* p = lua_toptr( l, 1 );
	transform* t = lua_toptr( l, 2 );
	p->trans = t;
	return 0;
}

int LUA_physic_activate( lua_State* l ) {
	LUA_DEBUG_PRINT( "lua physic activate.\n" );
	engine* e = lua_toptr( l, 1 );
	physic* p = lua_toptr( l, 2 );
	startTick( e, p, physic_tick );
	return 0;
}

vector lua_tovector3( lua_State* l, int i ) {
	return Vector( lua_tonumber( l, i ), lua_tonumber( l, i+1 ), lua_tonumber( l, i+2 ), 0.f );
}

int LUA_physic_setVelocity( lua_State* l ) {
//	LUA_DEBUG_PRINT( "lua physic setVelocity.\n" );
	physic* p = lua_toptr( l, 1 );
//	vector v = lua_tovector3( l, 2 );
	vector* v = lua_toptr( l, 2 );
//	v.coord.w = 0.f;
	p->velocity = *v;
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
	if ( lua_isnumber( l, 2 ) ) {
		input* in = lua_toptr( l, 1 );
		int key = (int)lua_tonumber( l, 2 );
		bool pressed = input_keyPressed( in, key );
		lua_pushboolean( l, pressed );
		return 1;
	}
	printf( "Error: Lua: keyPressed called without key specified.\n" );
	return 0;
}
int LUA_keyHeld( lua_State* l ) {
	if ( lua_isnumber( l, 2 ) ) {
		input* in = lua_toptr( l, 1 );
		int key = (int)lua_tonumber( l, 2 );
		bool held = input_keyHeld( in, key );
		lua_pushboolean( l, held );
		return 1;
	}
	printf( "Error: Lua: keyHeld called without key specified.\n" );
	return 0;
}

#ifdef ANDROID
int LUA_touchPressed( lua_State* l ) {
	if ( lua_isnumber( l, 2 ) && 
	 	lua_isnumber( l, 3 ) &&
	 	lua_isnumber( l, 4 ) &&
	 	lua_isnumber( l, 5 ) )
	{
		input* in = lua_toptr( l, 1 );
		int x_min = lua_tonumber( l, 2 );
		int y_min = lua_tonumber( l, 3 );
		int x_max = lua_tonumber( l, 4 );
		int y_max = lua_tonumber( l, 5 );
		bool pressed = input_touchPressed( in, x_min, y_min, x_max, y_max );
		lua_pushboolean( l, pressed );
		return 1;
	}
	printf( "Error: Lua: Invalid touch bounds specified" );
	return 0;
}

int LUA_touchHeld( lua_State* l ) {
	if ( lua_isnumber( l, 2 ) && 
	 	lua_isnumber( l, 3 ) &&
	 	lua_isnumber( l, 4 ) &&
	 	lua_isnumber( l, 5 ) )
	{
		input* in = lua_toptr( l, 1 );
		int x_min = lua_tonumber( l, 2 );
		int y_min = lua_tonumber( l, 3 );
		int x_max = lua_tonumber( l, 4 );
		int y_max = lua_tonumber( l, 5 );
		bool pressed = input_touchHeld( in, x_min, y_min, x_max, y_max );
		lua_pushboolean( l, pressed );
		return 1;
	}
	printf( "Error: Lua: Invalid touch bounds specified" );
	return 0;
}
#else
int LUA_touchHeld( lua_State* l ) {
	lua_pushboolean( l, false );
	return 1;
}
int LUA_touchPressed( lua_State* l ) {
	lua_pushboolean( l, false );
	return 1;
}
#endif // ANDROID

int LUA_transform_yaw( lua_State* l ) {
	transform* t = lua_toptr( l, 1 );
	float yaw = lua_tonumber( l, 2 );
	transform_yaw( t, yaw );
	return 0;
}

int LUA_transform_roll( lua_State* l ) {
	transform* t = lua_toptr( l, 1 );
	float roll = lua_tonumber( l, 2 );
	transform_roll( t, roll );
	return 0;
}

int LUA_transform_pitch( lua_State* l ) {
	transform* t = lua_toptr( l, 1 );
	float pitch = lua_tonumber( l, 2 );
	transform_pitch( t, pitch );
	return 0;
}

vector* lua_createVector( ) {
	if ( !(lua_vector_count < 64) )
		lua_vector_count = 0;
	assert( lua_vector_count < 64 );
	vector* v = &lua_vectors[lua_vector_count++];
	return v;
}

// Make a vector in Lua
// This is using light userdata?
int LUA_vector( lua_State* l ) {
	float x = lua_tonumber( l, 1 );
	float y = lua_tonumber( l, 2 );
	float z = lua_tonumber( l, 3 );
	float w = lua_tonumber( l, 4 );
	vector* v = lua_createVector( x, y, z, w );
	*v = Vector( x, y, z, w );
	lua_pushptr( l, v );
	return 1;
}

int LUA_transformVector( lua_State* l ) {
	transform* t = lua_toptr( l, 1 );
	vector* v = lua_toptr( l, 2 );
	vector* _v = lua_createVector();
	*_v = matrixVecMul( t->world, v );
	lua_pushptr( l, _v );
	return 1;
}

int LUA_transform_setWorldPosition( lua_State* l ) {
	transform* t = lua_toptr( l, 1 );
	vector* v = lua_toptr( l, 2 );
	transform_setWorldSpacePosition( t, v );
	return 0;
}

int LUA_chasecam_follow( lua_State* l ) {
	LUA_DEBUG_PRINT( "LUA chasecam_follow\n" );
	engine* e = lua_toptr( l, 1 );
	transform* t = lua_toptr( l, 2 );
	chasecam* c = chasecam_create();
	startTick( e, (void*)c, chasecam_tick );	
	c->cam.trans = transform_createAndAdd( theScene );
	theScene->cam = &c->cam;
	c->target = t;
	lua_pushptr( l, c );
	return 1;
}

int LUA_flycam( lua_State* l ) {
	LUA_DEBUG_PRINT( "LUA flycam\n" );
	engine* e = lua_toptr( l, 1 );
	flycam* c = flycam_create();
	startTick( e, (void*)c, flycam_tick );	
	startInput( e, (void*)c, flycam_input );	
	c->camera_target.trans = transform_createAndAdd( theScene );
	theScene->cam = &c->camera_target;
	lua_pushptr( l, c );
	return 1;
}

int LUA_setCamera( lua_State* l ) {
	theScene->cam = lua_toptr( l, 1 );
	return 0;
}

int LUA_transform_setWorldSpaceByTransform( lua_State* l ) {
	transform* dst = lua_toptr( l, 1 );
	transform* src = lua_toptr( l, 2 );
	transform_setWorldSpace( dst, src->world );
	return 0;
}

int LUA_particle_create( lua_State* l ) {
	engine* e = lua_toptr( l, 1 );
	transform* t = lua_toptr( l, 2 );
	
	particleEmitter* p = particleEmitter_create();
	p->definition->lifetime = 2.f;
	p->definition->spawn_box = Vector( 0.3f, 0.3f, 0.3f, 0.f );

	// size
	p->definition->size = property_create( 2 );
	property_addf( p->definition->size, 0.f, 1.f );
	property_addf( p->definition->size, 0.3f, 1.f );
	property_addf( p->definition->size, 2.f, 5.f );

	// color
	p->definition->color = property_create( 5 );
	property_addv( p->definition->color, 0.f, Vector( 1.f, 0.f, 0.f, 0.f ));
	property_addv( p->definition->color, 0.3f, Vector( 1.f, 0.5f, 0.f, 1.f ));
	property_addv( p->definition->color, 0.8f, Vector( 1.f, 1.f, 1.f, 0.8f ));
	property_addv( p->definition->color, 1.0f, Vector( 0.5f, 0.5f, 0.5f, 0.8f ));
	property_addv( p->definition->color, 2.f, Vector( 0.5f, 0.5f, 0.5f, 0.f ));

	p->definition->velocity = Vector( 0.f, 0.1f, 0.f, 0.f );
	p->definition->spawn_interval = 0.03f;
	p->trans = t;
	p->definition->flags = p->definition->flags | kParticleWorldSpace
												| kParticleRandomRotation;

	texture_request( &p->definition->texture_diffuse, "assets/img/cloud_rgba128.tga" );

	engine_addRender( e, p, particleEmitter_render );
	startTick( e, p, particleEmitter_tick );
	//
	//
	//

	particleEmitter* p_ = particleEmitter_create();
	p_->definition->lifetime = 2.3f;
	p_->definition->size = property_create( 2 );
	property_addf( p_->definition->size, 0.f, 0.f );
	property_addf( p_->definition->size, 0.5f, 10.f );
	property_addf( p_->definition->size, 2.f, 25.f );
	p_->definition->color = property_create( 5 );
	property_addv( p_->definition->color, 0.f, Vector( 1.f, 1.f, 0.f, 0.8f ));
	property_addv( p_->definition->color, 0.5f, Vector( 1.f, 0.4f, 0.f, 0.8f ));
	property_addv( p_->definition->color, 2.3f, Vector( 1.f, 0.4f, 0.f, 0.f ));
	p_->definition->velocity = Vector( 0.f, 0.0f, 0.f, 0.f );
	p_->definition->spawn_interval = 0.3f;
	p_->definition->spawn_box = Vector( 0.3f, 0.f, 0.3f, 0.f );
	p_->trans = t;
//	p_->definition->texture_diffuse = texture_loadTGA( "assets/img/star_rgba64.tga" );
	texture_request( &p_->definition->texture_diffuse, "assets/img/star_rgba64.tga" );

	engine_addRender( e, p_, particleEmitter_render );
	startTick( e, p_, particleEmitter_tick );




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

// Create a Lua l and load it's initial contents from <filename>
lua_State* vlua_create( engine* e, const char* filename ) {
	lua_State* l = lua_open();
	luaL_openlibs( l );	// Load the Lua libs into our lua l

	// We now use luaL_loadbuffer rather than luaL_loadfile as on Android we need
	// to go through Libzip to get the data
	size_t length;
	const char* buffer = vfile_contents( filename, &length );
	if ( luaL_loadbuffer( l, buffer, length, filename ) || lua_pcall( l, 0, 0, 0)) {
		printf("Error: Failed loading lua from file %s!\n", filename );
		assert( 0 );
	}

	// *** General
	lua_registerFunction( l, LUA_registerCallback, "registerEventHandler" );
	lua_registerFunction( l, LUA_print, "vprint" );
	// *** Vector
	lua_registerFunction( l, LUA_vector, "Vector" );
	// *** Input
	lua_registerFunction( l, LUA_keyPressed, "vkeyPressed" );
	lua_registerFunction( l, LUA_keyHeld, "vkeyHeld" );
	lua_registerFunction( l, LUA_touchPressed, "vtouchPressed" );
	lua_registerFunction( l, LUA_touchHeld, "vtouchHeld" );
	// *** Scene
	lua_registerFunction( l, LUA_createModelInstance, "vcreateModelInstance" );
	lua_registerFunction( l, LUA_deleteModelInstance, "vdeleteModelInstance" );
	lua_registerFunction( l, LUA_createphysic, "vcreatePhysic" );
	lua_registerFunction( l, LUA_createtransform, "vcreateTransform" );
	lua_registerFunction( l, LUA_setWorldSpacePosition, "vsetWorldSpacePosition" );
	lua_registerFunction( l, LUA_model_setTransform, "vmodel_setTransform" );
	lua_registerFunction( l, LUA_physic_setTransform, "vphysic_setTransform" );
	lua_registerFunction( l, LUA_scene_addModel, "vscene_addModel" );
	lua_registerFunction( l, LUA_scene_removeModel, "vscene_removeModel" );
	lua_registerFunction( l, LUA_physic_activate, "vphysic_activate" );
	lua_registerFunction( l, LUA_physic_setVelocity, "vphysic_setVelocity" );
	lua_registerFunction( l, LUA_transform_yaw, "vtransform_yaw" );
	lua_registerFunction( l, LUA_transform_pitch, "vtransform_pitch" );
	lua_registerFunction( l, LUA_transform_roll, "vtransform_roll" );
	lua_registerFunction( l, LUA_transformVector, "vtransformVector" );
	lua_registerFunction( l, LUA_transform_setWorldPosition, "vtransform_setWorldPosition" );
	lua_registerFunction( l, LUA_transform_setWorldSpaceByTransform, "vtransform_setWorldSpaceByTransform" );
	lua_registerFunction( l, LUA_particle_create, "vparticle_create" );
	// *** Camera
	lua_registerFunction( l, LUA_chasecam_follow, "vchasecam_follow" );
	lua_registerFunction( l, LUA_flycam, "vflycam" );
	lua_registerFunction( l, LUA_setCamera, "vscene_setCamera" );

	lua_makeConstantPtr( l, "engine", e );
	lua_makeConstantPtr( l, "input", e->input );

	lua_keycodes( l );

	// *** Always call init
	LUA_CALL( l, "init" );

	return l;
}

void lua_setScene( lua_State* l, scene* s ) {
	lua_makeConstantPtr( l, "scene", s );
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
	char capital_offset = 'A' - 'a';
	for ( char i = 'a'; i <= 'z'; i++ ) {
		char string[2];
		string[0] = i;
		string[1] = '\0';
		lua_setfieldi( l, string, i + capital_offset );
	}
	/*
	lua_setfieldi( l, "w", KEY_W );
	lua_setfieldi( l, "a", KEY_A );
	lua_setfieldi( l, "s", KEY_S );
	lua_setfieldi( l, "d", KEY_D );
	*/
	lua_setfieldi( l, "up", KEY_UP );
	lua_setfieldi( l, "down", KEY_DOWN );
	lua_setfieldi( l, "left", KEY_LEFT );
	lua_setfieldi( l, "right", KEY_RIGHT );
	lua_setfieldi( l, "space", KEY_SPACE );
	lua_setglobal( l, "key" ); // store the table in the 'key' global variable
}
