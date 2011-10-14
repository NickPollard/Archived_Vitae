// scene.c

#include "common.h"
#include "scene.h"
//-----------------------
#include "camera.h"
#include "input.h"
#include "light.h"
#include "model.h"
#include "model_loader.h"
#include "mem/allocator.h"
#include "particle.h"
#include "physic.h"
#include "render/debugdraw.h"
#include "render/modelinstance.h"
//#include "system/file.h"
#include "script/parse.h"
#include "font.h"
#include "debug/debugtext.h"

// *** Private Declarations

void scene_saveFile( scene* s, const char* filename );
scene* scene_loadFile( const char* filename );

// *** Constants

static const size_t modelArraySize = sizeof( modelInstance* ) * MAX_MODELS;
static const size_t lightArraySize = sizeof( light* ) * MAX_LIGHTS;

// *** static data

keybind scene_debug_transforms_toggle;
keybind scene_debug_lights_toggle;

void scene_static_init( ) {
	scene_debug_transforms_toggle = input_registerKeybind( );
	input_setDefaultKeyBind( scene_debug_transforms_toggle, KEY_T );

	scene_debug_lights_toggle = input_registerKeybind( );
	input_setDefaultKeyBind( scene_debug_lights_toggle, KEY_L );
}

// Add an existing modelInstance to the scene
void scene_addModel( scene* s, modelInstance* instance ) {
	vAssert( s->model_count < MAX_MODELS );
	vAssert( instance->trans ); // Disallow adding a model without a transform already set
	s->modelInstances[s->model_count++] = instance;

	for ( int i = 0; i < instance->transform_count; i++ ) {
//		printf( "Adding transform.\n" );
		scene_addTransform( s, instance->transforms[i] );
		// At this point we set up subtransforms to be parented by the modelinstance transform
		// Can't do it earlier as the transform doesn't exist when modelinstance is created
		instance->transforms[i]->parent = instance->trans;
//		vector_printf( "subtransform Translation: ", matrix_getTranslation( instance->transforms[i]->local ));
	}
	for ( int i = 0; i < instance->emitter_count; i++ ) {
//		printf( "Adding emitter.\n" );
		scene_addEmitter( s, instance->emitters[i] );
		if ( !instance->emitters[i]->trans )
			instance->emitters[i]->trans = instance->trans;
		// If the scene is active, activate components immediately
		if ( s->eng ) {
			engine_addRender( s->eng, instance->emitters[i], particleEmitter_render );
			startTick( s->eng, instance->emitters[i], particleEmitter_tick );
		}
	}
}

void scene_addEmitter( scene* s, particleEmitter* e ) {
	s->emitters[s->emitter_count++] = e;
}

// Add an existing light to the scene
void scene_addLight( scene* s, light* l ) {
	assert( s->light_count < MAX_LIGHTS );
	s->lights[s->light_count++] = l;
}

// Add an existing transform to the scene
void scene_addTransform( scene* s, transform* t ) {
	assert( s->transform_count < MAX_TRANSFORMS );
//	memcpy(	&s->transforms[s->transform_count++], t, sizeof( transform ));
	s->transforms[s->transform_count++] = t;
}

int scene_transformIndex( scene* s, transform* t ) {
	for ( int i = 0; i < s->transform_count; i++ ) {
		if ( scene_transform( s, i ) == t )
			return i;
	}
	return -1;
}

modelInstance* scene_model(scene* s, int i) {
	return s->modelInstances[i];
}	

light* scene_light( scene* s, int i ) {
	return s->lights[i];
}

void scene_free( scene* s ) {
	// All these pointers should be valid at this point
	// They should not be cleared prior to calling scene_free
	// This is part of our RAII-based memory strategy
	assert( s->transforms );
	assert( s->lights );
	assert( s->modelInstances );
//	assert( s->cam );

	mem_free( s->transforms );
	mem_free( s->lights );
	mem_free( s->modelInstances );
//	mem_free( s->cam );

	// Finally free our scene
	mem_free( s );
}

// Initialise a scene with some test data
scene* test_scene_init( engine* e ) { 

	sterm* stree = parse_file( "dat/scene/testscene.s" );
	scene* s = eval( stree );
	sterm_free( stree );
	s->eng = e;

	// Activate emitters
	for ( int i = 0; i < s->emitter_count; i++ ) {
		particleEmitter* p = s->emitters[i];
		engine_addRender( s->eng, p, particleEmitter_render );
		startTick( s->eng, p, particleEmitter_tick );
	}

	// Test Misc scene setup
	scene_setAmbient( s, 0.2f, 0.2f, 0.2f, 1.f );

	return s;

//	scene_saveFile( s, "dat/test_scene.s" );
//	scene* s2 = scene_loadFile( "dat/test_scene.s" );

//	return s2;
}

void scene_setAmbient(scene* s, float r, float g, float b, float a) {
	s->ambient[0] = r;
	s->ambient[1] = g;
	s->ambient[2] = b;
	s->ambient[3] = a;
}

// Make a scene
scene* scene_create() {
	scene* s = mem_alloc( sizeof( scene ));
	memset( s, 0, sizeof( scene ));
	s->model_count = s->light_count = s->transform_count = 0;
	s->modelInstances = mem_alloc( sizeof( modelInstance* ) * MAX_MODELS );
	s->lights =			mem_alloc( sizeof( light* ) * MAX_LIGHTS );
	s->transforms =		mem_alloc( sizeof( transform* ) * MAX_TRANSFORMS );
	memset( s->transforms, 0, sizeof( transform* ) * MAX_TRANSFORMS );
	s->emitters =		mem_alloc( sizeof( particleEmitter* ) * MAX_EMITTERS );
	return s;
}

transform* scene_transform( scene* s, int i ) {
	assert( i >= 0 );
	assert( i < s->transform_count );
	return s->transforms[i];
}

// Traverse the transform graph, updating worldspace transforms
void scene_concatenateTransforms(scene* s) {
	for (int i = 0; i < s->transform_count; i++)
		transform_concatenate( scene_transform( s, i ));
}

void scene_debugTransforms( scene* s ) {
	char string[128];
	sprintf( string, "transform_count: %d", s->transform_count );
#ifndef ANDROID
//	PrintDebugText( s->debugtext, string );
#endif
	for (int i = 0; i < s->transform_count; i++) {
		transform_printDebug( s->transforms[i], s->debugtext );
	}
}

void scene_debugLights( scene* s ) {
	char string[128];
	sprintf( string, "light_count: %d", s->light_count );
#ifndef ANDROID
//	PrintDebugText( s->debugtext, string );
#endif
}
// Process input for the scene
void scene_input( scene* s, input* in ) {
	if ( input_keybindPressed( in, scene_debug_transforms_toggle ) )
		s->debug_flags ^= kSceneDebugTransforms;
	if ( input_keybindPressed( in, scene_debug_lights_toggle ) )
		s->debug_flags ^= kSceneLightsTransforms;
}

// Update the scene
void scene_tick(scene* s, float dt) {
	(void)dt;
	scene_concatenateTransforms( s );

	if ( s->debug_flags & kSceneDebugTransforms )
		scene_debugTransforms( s );
	if ( s->debug_flags & kSceneLightsTransforms )
		scene_debugLights( s );

	// TEST
	//test_scene_tick(s, dt);
}

void scene_setCamera(scene* s, float x, float y, float z, float w) {
	vector v = Vector(x, y, z, w);
	matrix trans;
	matrix_cpy( trans, s->cam->trans->world );
	matrix_setTranslation( trans, &v );
	transform_setWorldSpace( s->cam->trans, trans );
}

void test_scene_tick(scene* s, float dt) {
	static float time = 0.f;
	time += dt;

	static const float period = 2.f;
	float animate = 2.f * sinf(time * 2 * PI / period);

	// Animate cubes
	vector translateA = Vector( animate, 0.f, 0.f, 1.f );
	transform_setLocalTranslation( s->modelInstances[0]->trans, &translateA );
	vector translateB = Vector( 0.f, animate, 0.f, 1.f );
	transform_setLocalTranslation( s->modelInstances[1]->trans, &translateB );
	
	vector translateC = Vector( 0.f, 0.f, animate,  1.f );
	transform_setLocalTranslation( s->modelInstances[0]->trans->parent, &translateC);
}

// All sceneData data is owned by the sceneData
// EXCEPT now it will all be one big blob
// so just free the data
void sceneData_free( sceneData* data ) {
	mem_free( data );
}

// save a scene as a binary blob
// the data should be packed in the following format:
//
// SceneData header
// Transform Array (transform_count items)
// Model Array (model_count items)
// Light Array (light_count items)
//
sceneData* scene_save( scene* s ) {
	// Calculate total size of binary blob needed
	// Total size is header + size of all arrays

	size_t models_size =		sizeof( modelInstance ) *	s->model_count;
	size_t lights_size =		sizeof( light ) *			s->light_count;
	size_t transforms_size =	sizeof( transform ) *		s->transform_count;

	size_t blob_size = sizeof( sceneData ) +
						transforms_size +
						models_size +
						lights_size +
						sizeof( camera );

	void* blob = mem_alloc( blob_size );
	void* blob_end = blob + blob_size;	// Used for debug check to ensure we don't write over the end
	
	sceneData* data = (sceneData*)blob;
	data->size = blob_size;

	// transforms or parent transforms are stored as indices rather than pointers
	// so that they can be restored correctly when moved and loaded

	// transforms
	data->transform_count = s->transform_count;
	data->transforms = blob + sizeof( sceneData );
	assert( (void*)data->transforms < blob_end );

	for ( int i = 0; i < s->transform_count; i++ ) {
		data->transforms[i] = *scene_transform( s, i );
		assert( data->transforms[i].parent != scene_transform( s, i ));
		data->transforms[i].parent = (void*)scene_transformIndex( s, data->transforms[i].parent );
	}
	
	// modelInstances
	data->model_count = s->model_count;
	data->modelInstances = (void*)data->transforms + transforms_size;
	assert( (void*)data->modelInstances < blob_end );

	for ( int i = 0; i < s->model_count; i++ ) {
		data->modelInstances[i] = *scene_model( s, i );
		data->modelInstances[i].trans = (void*)scene_transformIndex( s, data->modelInstances[i].trans );
	}

	// lights
	data->light_count = s->light_count;
	data->lights = (void*)data->modelInstances + models_size;
	assert( (void*)data->lights < blob_end );

	for ( int i = 0; i < s->light_count; i++ ) {
		data->lights[i] = *scene_light( s, i );
		data->lights[i].trans = (void*)scene_transformIndex( s, data->lights[i].trans );
	}

	data->cam = (void*)data->lights + lights_size;
	assert( (void*)data->cam < blob_end );

	memcpy( data->cam, s->cam, sizeof( camera ));
	data->cam->trans = (transform*)scene_transformIndex( s, data->cam->trans );

	return data;
}

transform* scene_resolveTransform( scene* s, int i ) {
	if ( i == -1 )
		return NULL;
	else
		return scene_transform( s, i );
}

// TODO
void scene_saveFile( scene* s, const char* filename ) {
	sceneData* data = scene_save( s );
	vfile_writeContents( filename, data, data->size );
	sceneData_free( data );
}

// TODO
scene* scene_loadFile( const char* filename ) {
	size_t buffer_length;
	void* buffer = vfile_contents( filename, &buffer_length );
	sceneData* data = (sceneData*)buffer;
	printf( "Loading scene from file %s. File Size: %d, Data size: %d.\n", filename, buffer_length, data->size );
	assert( data->size == buffer_length );
	return scene_load( buffer );
}

scene* scene_load( sceneData* data ) {
	// create scene
	scene* s = scene_create();
	scene_setAmbient( s, 0.2f, 0.2f, 0.2f, 1.f );

	printf( "Loading %d transforms.\n", data->transform_count );

	size_t models_size =		sizeof( modelInstance ) *	data->model_count;
//	size_t lights_size =		sizeof( light ) *			data->light_count;
	size_t transforms_size =	sizeof( transform ) *		data->transform_count;

	// Need to fix up array pointers
	data->transforms = (void*)data + sizeof( sceneData );
	data->modelInstances = (void*)data->transforms + transforms_size;
	data->lights = (void*)data->modelInstances + models_size;

	// create transforms
	for ( int i = 0; i < data->transform_count; i++ ) {
		transform* t = transform_create( s );
		memcpy( t, &data->transforms[i], sizeof( transform ));
		scene_addTransform( s, t );
		printf(" Loading transform %d. Parent index = %d.\n", i, (int)t->parent );
		t->parent = scene_resolveTransform( s, (int)t->parent );
		assert( t != t->parent );
	}

	s->cam = camera_create( s );
	memcpy( s->cam, data->cam, sizeof( camera ));
	s->cam->trans = scene_resolveTransform( s, (int)s->cam->trans );

	// create modelInstances
	for ( int i = 0; i < data->model_count; i++ ) {
		modelInstance* m = modelInstance_createEmpty( );
		memcpy( m, &data->modelInstances[i], sizeof( modelInstance ));
		m->trans = scene_resolveTransform( s, (int)m->trans );
		scene_addModel( s, m );
	}

	// create lights
	for ( int i = 0; i < data->light_count; i++ ) {
		light* l = light_create( );
		memcpy( l, &data->lights[i], sizeof( light ));
		scene_addLight( s, l );
		l->trans = scene_resolveTransform( s, (int)l->trans );
	}

	return s;
}
