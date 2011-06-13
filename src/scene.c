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

// *** static data

modelInstance* testModelA = NULL;
modelInstance* testModelB = NULL;
transform* t2 = NULL;
scene* scene_load( int n_bytes, void* src );

keybind scene_debug_transforms_toggle;

void scene_static_init( ) {
	scene_debug_transforms_toggle = input_registerKeybind( );
	input_setDefaultKeyBind( scene_debug_transforms_toggle, KEY_T );
}

//void scene_addModel( scene* s, model* m ) {
//	s->models[s->model_count++] = m;
//}

void scene_addModel( scene* s, modelInstance* m ) {
	s->models[s->model_count++] = m;
}

void scene_addLight( scene* s, light* l ) {
	s->lights[s->light_count++] = l;
}

void scene_addTransform( scene* s, transform* t ) {
	memcpy(	&s->transforms[s->transform_count++], t, sizeof( transform ));
}

modelInstance* scene_getModel(scene* s, int i) {
	return s->models[i];
}	

// Initialise a scene with some test data
scene* test_scene_init( ) {
	scene* s = scene_create();

	modelHandle testCube = model_getHandleFromFilename( "invalid.obj" );
	testModelA = modelInstance_create( testCube );
	testModelB = modelInstance_create( testCube );
	testModelA->trans = transform_create( s );
	testModelB->trans = transform_create( s );
	transform* t = transform_create( s );
	testModelA->trans->parent = t;
	testModelB->trans->parent = t;
	vector translate = Vector( -2.f, 0.f, 0.f, 1.f );
	transform_setLocalTranslation(t, &translate);
	t2 = transform_create(s);
	t->parent = t2;

	scene_addModel(s, testModelA);
	scene_addModel(s, testModelB);

	scene_setAmbient(s, 0.2f, 0.2f, 0.2f, 1.f);

	light* l = light_createWithTransform(s);
	
	vector lightPos = {{ 1.f, 1.f, 1.f, 1.f }};
	light_setPosition(l, &lightPos);
	light_setDiffuse(l, 1.f, 0.f, 0.f, 1.f);

	scene_addLight(s, l);

	scene_setCamera(s, 0.f, 0.f, 10.f, 1.f);

	int size = ( sizeof( scene ) +
				 sizeof( modelInstance* ) * MAX_MODELS + 
				 sizeof( light* ) * MAX_LIGHTS + 
				 sizeof( transform ) * 6 +
				 sizeof( modelInstance ) * 2 +
				 sizeof( light ));
	void* buffer = malloc( size );

	void* data = buffer;
	// Pack data
	scene* _scene = data;
	memcpy( data, s, sizeof( scene ));
	data += sizeof( scene );

	modelInstance** models = data;
	data += sizeof( modelInstance* ) * MAX_MODELS;

	light** lights = data;
	data += sizeof( light* ) * MAX_LIGHTS;

	modelInstance* a = data;
	memcpy( data, testModelA, sizeof( modelInstance ));
	data += sizeof( modelInstance );

	modelInstance* b = data;
	memcpy( data, testModelB, sizeof( modelInstance ));
	data += sizeof( modelInstance );

	transform* ta = data;
	memcpy( data, testModelA->trans, sizeof( transform ));
	data += sizeof( transform );

	transform* tb = data;
	memcpy( data, testModelB->trans, sizeof( transform ));
	data += sizeof( transform );

	transform* _t = data;
	memcpy( data, t, sizeof( transform ));
	data += sizeof( transform );

	transform* _t2 = data;
	memcpy( data, t2, sizeof( transform ));
	data += sizeof( transform );

	transform* _tl = data;
	memcpy( data, l->trans, sizeof( transform ));
	data += sizeof( transform );
	
	transform* _tc = data;
	memcpy( data, s->cam->trans, sizeof( transform ));
	data += sizeof( transform );

	light* _l = data;
	memcpy( data, l, sizeof( light ));

	_scene->models = models;
	_scene->models[0] = a;
	_scene->models[1] = b;
	_scene->lights = lights;
	_scene->lights[0] = _l;
	_scene->transforms = ta;
	_scene->cam->trans = _tc;
	l->trans = _tl;
	a->trans = ta;
	b->trans = tb;
	ta->parent = _t;
	tb->parent = _t;
	_t->parent = _t2;


	// Fix-up pointers

	scene* s2 = scene_load( size, buffer );
	return s2;
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
	s->models = mem_alloc( sizeof( modelInstance* ) * MAX_MODELS );
	s->lights = mem_alloc( sizeof( light* ) * MAX_LIGHTS );
	s->transforms = mem_alloc( sizeof( transform ) * MAX_TRANSFORMS );
	s->cam = camera_createWithTransform(s);
	scene_setCamera(s, 0.f, 0.f, 0.f, 1.f);
	scene_setAmbient(s, 0.2f, 0.2f, 0.2f, 1.f);
	return s;
}

// Traverse the transform graph, updating worldspace transforms
void scene_concatenateTransforms(scene* s) {
	for (int i = 0; i < s->transform_count; i++)
		transform_concatenate(&s->transforms[i]);
}

void scene_debugTransforms( scene* s ) {
	char string[128];
	sprintf( string, "transform_count: %d", s->transform_count );
	PrintDebugText( s->debugtext, string );
	for (int i = 0; i < s->transform_count; i++) {
		transform_printDebug( &s->transforms[i], s->debugtext );
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
	float period = 2.f;
	float animate = 2.f * sinf(time * 2 * PI / period);
	// Animate cubes
	vector translateA = Vector( animate, 0.f, 0.f, 1.f );
	transform_setLocalTranslation( testModelA->trans, &translateA );
	vector translateB = Vector( 0.f, animate, 0.f, 1.f );
	transform_setLocalTranslation( testModelB->trans, &translateB );
	
	vector translateC = Vector( 0.f, 0.f, animate,  1.f );
	transform_setLocalTranslation(t2, &translateC);
}

//
// Load a pre-computed scene
//

scene* scene_load( int n_bytes, void* src ) {
	void* dst = mem_alloc( n_bytes );
	int offset = dst - src;
	memcpy( dst, src, n_bytes );
	// The scene can just be copied
	scene* s = dst;
	// Pointers will need fixing up
	s->models		+= offset;
	s->transforms	+= offset;
	s->lights		+= offset;
	// Transforms
	for ( int i = 0; i < s->transform_count; i++ ) {
		printf(" transform %d.\n", i );
		transform* t = &s->transforms[i];
		if ( t->parent )	// Don't need to update NULL parents
			t->parent += offset;
	}
	// ModelInstances
	for ( int i = 0; i < s->model_count; i++ ) {
		modelInstance* m = s->models[i];
		if ( m->trans )	// Don't need to update NULL parents
			m->trans += offset;
		// Hookup Model Handle
		// The model parameter should contain an ID derived from the filename
		// (eg. via a hash)
		// We then hookup to the model of that filename, loading it if required
		int id = (int)m->model;
		m->model = model_getHandleFromID( id );
	}
	// Lights
	for ( int i = 0; i < s->light_count; i++ ) {
		light* l = s->lights[i];
		if ( l->trans )	// Don't need to update NULL transforms
			l->trans += offset;
	}
	return dst;
}

