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
#include "font.h"
#include "debug/debugtext.h"

// *** static data

model* testModelA = NULL;
model* testModelB = NULL;
transform* t2 = NULL;

keybind scene_debug_transforms_toggle;

void scene_static_init( ) {
	scene_debug_transforms_toggle = input_registerKeybind( );
	input_setDefaultKeyBind( scene_debug_transforms_toggle, KEY_T );
}

void scene_addModel(scene* s, model* m) {
	s->models[s->modelCount++] = m;
}

void scene_addLight(scene* s, light* l) {
	s->lights[s->lightCount++] = l;
}

model* scene_getModel(scene* s, int i) {
	return s->models[i];
}	

// Initialise a scene with some test data
void test_scene_init(scene* s) {
	testModelA = model_createTestCube();
	testModelB = model_createTestCube();
	transform* t = transform_create(s);
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
	s->modelCount = s->lightCount = s->transformCount = 0;
	s->cam = camera_createWithTransform(s);
	scene_setCamera(s, 0.f, 0.f, 0.f, 1.f);
	scene_setAmbient(s, 0.2f, 0.2f, 0.2f, 1.f);
	return s;
}

// Traverse the transform graph, updating worldspace transforms
void scene_concatenateTransforms(scene* s) {
	for (int i = 0; i < s->transformCount; i++)
		transform_concatenate(&s->transforms[i]);
}

void scene_debugTransforms( scene* s ) {
	char string[128];
	sprintf( string, "TransformCount: %d", s->transformCount );
	PrintDebugText( s->debugtext, string );
	for (int i = 0; i < s->transformCount; i++) {
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
	transform_setLocalTranslation(testModelA->trans, &translateA);
	vector translateB = Vector( 0.f, animate, 0.f, 1.f );
	transform_setLocalTranslation(testModelB->trans, &translateB);
	
	vector translateC = Vector( 0.f, 0.f, animate,  1.f );
	transform_setLocalTranslation(t2, &translateC);
}

#if 1

// Load a scene from a .sc file (s-expression based)
scene* scene_load( slist* data ) {
	scene* s = scene_create();
	process( data );
}

void file_load( string filename ) {
	int length = 0;
	const char* contents = vfile_contents( filename, &length );
	inputStream* in = inputStream_create( contents );
	slist* s = vfile_sread( in );
	load( s );
}

// Create the objects indicated by a data file
// The file should have only one root
void load( slist* data ) {
	head = data->head;
	tail = data->tail;

	// find the function defined by the head
	// Call it with the data defined by the tail
	eval(head)(eval(tail));
}

// Evaluate a list of sterms
void* eval( slist* data ) {
	if ( sterm_isAtom( data->head )) {
		return lookup( data->head.ptr );
	}
	else if ( sterm_isList( data->head )) {
		return eval(data->head->head)(eval(data->head->tail));
	}
	else {
		printf( "Unrecognised Sterm type: $d.\n", data->head.type_tag );
		assert( 0 );
	}
}


void scene_parse( scene* s, slist* data ) {
	


	if slist->head isToken
	{
		token = slist->head
		if ( token == 'object' ) {
			object obj = object_create();
			object_parse( obj, slist->tail );
		}
	}	
}

// called once for each argument to scene
void scene_process( void* context, slist* data ) {
	
//( scene ( object ( model "cube.obj" )
//				 ( position ( vector 0.0 0.0 0.0 ))))

	/*
		create a scene, scene->parse( subtree)
		create an object, object->parase(subtree);

	   */
}
#endif
