// scene.c

#include "common.h"
#include "scene.h"
//-----------------------
#include "camera.h"
#include "input.h"
#include "light.h"
#include "model.h"
#include "mem/allocator.h"
#include "render/debugdraw.h"
#include "render/modelinstance.h"
#include "font.h"
#include "debug/debugtext.h"

static const size_t modelArraySize = sizeof( modelInstance* ) * MAX_MODELS;
static const size_t lightArraySize = sizeof( light* ) * MAX_LIGHTS;

// *** static data

keybind scene_debug_transforms_toggle;

void scene_static_init( ) {
	scene_debug_transforms_toggle = input_registerKeybind( );
	input_setDefaultKeyBind( scene_debug_transforms_toggle, KEY_T );
}

// Add an existing modelInstance to the scene
void scene_addModel( scene* s, modelInstance* m ) {
	assert( s->model_count < MAX_MODELS );
	s->modelInstances[s->model_count++] = m;
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

// Initialise a scene with some test data
scene* test_scene_init( ) {
	scene* s = scene_create();
	s->cam = camera_create( s );
	s->cam->trans = transform_createAndAdd( s );
	scene_setCamera( s, 0.f, 0.f, 0.f, 1.f );
	scene_setAmbient( s, 0.2f, 0.2f, 0.2f, 1.f );

	modelHandle testCube = model_getHandleFromFilename( "invalid.obj" );
	modelInstance* testModelA = modelInstance_create( testCube );
	modelInstance* testModelB = modelInstance_create( testCube );
	testModelA->trans = transform_createAndAdd( s );
	testModelB->trans = transform_createAndAdd( s );
	transform* t = transform_createAndAdd( s );
	testModelA->trans->parent = t;
	testModelB->trans->parent = t;
	vector translate = Vector( -2.f, 0.f, 0.f, 1.f );
	transform_setLocalTranslation(t, &translate);
	transform* t2 = transform_createAndAdd( s );
	t->parent = t2;

	scene_addModel(s, testModelA);
	scene_addModel(s, testModelB);

	scene_setAmbient(s, 0.2f, 0.2f, 0.2f, 1.f);

	light* l = light_createWithTransform( s );
	vector lightPos = {{ 1.f, 1.f, 1.f, 1.f }};
	light_setPosition(l, &lightPos);
	light_setDiffuse(l, 1.f, 0.f, 0.f, 1.f);
	scene_addLight(s, l);

	scene_setCamera(s, 0.f, 0.f, 10.f, 1.f);

	sceneData* data = scene_save( s );
	scene* s2 = scene_load( data );
	sceneData_free( data );
	return s2;

//	return s;
}

void glTranslate_vector(vector* v) {
	glTranslatef(v->coord.x, v->coord.y, v->coord.z);
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
	s->lights = mem_alloc( sizeof( light* ) * MAX_LIGHTS );
	s->transforms = mem_alloc( sizeof( transform* ) * MAX_TRANSFORMS );
	memset( s->transforms, 0, sizeof( transform* ) * MAX_TRANSFORMS );
	return s;
}

transform* scene_transform( scene* s, int i ) {
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
	PrintDebugText( s->debugtext, string );
	for (int i = 0; i < s->transform_count; i++) {
		transform_printDebug( s->transforms[i], s->debugtext );
	}
}

// Process input for the scene
void scene_input( scene* s, input* in ) {
	if ( input_keybindPressed( in, scene_debug_transforms_toggle ) )
		s->debug_flags ^= kSceneDebugTransforms;
}

// Update the scene
void scene_tick(scene* s, float dt) {
	scene_concatenateTransforms(s);

	if ( s->debug_flags & kSceneDebugTransforms )
		scene_debugTransforms( s );

	// TEST
	test_scene_tick(s, dt);
}

void scene_setCamera(scene* s, float x, float y, float z, float w) {
	vector v = Vector(x, y, z, w);
	camera_setTranslation(s->cam, &v);
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
	transform_setLocalTranslation( scene_transform( s, 3 ), &translateC);
}

// All sceneData data is owned by the sceneData
// So free ALL of it
void sceneData_free( sceneData* data ) {
	mem_free( data->transforms );
	mem_free( data->lights );
	mem_free( data->modelInstances );
	mem_free( data->cam );

	// Finally free our data
	mem_free( data );
}

//
//	Load a Scene from a binary blob, as created by scene_save
//

sceneData* scene_save( scene* s ) {
	sceneData* data = mem_alloc( sizeof( sceneData ));

	data->cam = mem_alloc( sizeof( camera ));
	memcpy( data->cam, s->cam, sizeof( camera ));
	data->cam->trans = (transform*)scene_transformIndex( s, data->cam->trans );

	// transforms or parent transforms are stored as indices rather than pointers
	// so that they can be restored correctly when moved and loaded

	// transforms
	data->transform_count = s->transform_count;
	data->transforms = mem_alloc( sizeof( transform ) * data->transform_count );
	for ( int i = 0; i < s->transform_count; i++ ) {
		data->transforms[i] = *scene_transform( s, i );
		assert( data->transforms[i].parent != scene_transform( s, i ));
		data->transforms[i].parent = (void*)scene_transformIndex( s, data->transforms[i].parent );
	}
	
	// modelInstances
	data->model_count = s->model_count;
	data->modelInstances = mem_alloc( sizeof( modelInstance ) * data->model_count );
	for ( int i = 0; i < s->model_count; i++ ) {
		data->modelInstances[i] = *scene_model( s, i );
		data->modelInstances[i].trans = (void*)scene_transformIndex( s, data->modelInstances[i].trans );
	}

	// lights
	data->light_count = s->light_count;
	data->lights = mem_alloc( sizeof( light ) * data->light_count );
	for ( int i = 0; i < s->light_count; i++ ) {
		data->lights[i] = *scene_light( s, i );
		data->lights[i].trans = (void*)scene_transformIndex( s, data->lights[i].trans );
	}

	return data;
}

transform* scene_resolveTransform( scene* s, int i ) {
	if ( i == -1 )
		return NULL;
	else
		return scene_transform( s, i );
}

scene* scene_load( sceneData* data ) {
	// create scene
	scene* s = scene_create();
	scene_setAmbient( s, 0.2f, 0.2f, 0.2f, 1.f );

	// create transforms
	for ( int i = 0; i < data->transform_count; i++ ) {
		transform* t = transform_create( s );
		memcpy( t, &data->transforms[i], sizeof( transform ));
		scene_addTransform( s, t );
		t->parent = scene_resolveTransform( s, (int)t->parent );
	}

	s->cam = camera_create( s );
	memcpy( s->cam, data->cam, sizeof( camera ));
	s->cam->trans = scene_resolveTransform( s, (int)s->cam->trans );

	// create modelInstances
	for ( int i = 0; i < data->model_count; i++ ) {
		modelInstance* m = modelInstance_createEmpty( );
		memcpy( m, &data->modelInstances[i], sizeof( modelInstance ));
		scene_addModel( s, m );
		m->trans = scene_resolveTransform( s, (int)m->trans );
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
