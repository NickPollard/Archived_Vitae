// scene.h

/*
   A scene object represents a drawable scene of OpenGL models.

   It contains the scene heirarchy graph, specifying models, lights, transforms, etc.
   */

#ifndef __SCENE_H__
#define __SCENE_H__

#include "common.fwd.h"
#include "maths.h"
#include "transform.h"

#include <GL/glfw.h>

#define MAX_TRANSFORMS 128
#define MAX_MODELS 128
#define MAX_LIGHTS 128

#define kSceneDebugTransforms 0x00000001

// *** Scene ***
struct scene_s {
	int			model_count;
	modelInstance**	models;
	//
	int			light_count;
	light**		lights;
	//
	int			transform_count;
	transform*	transforms;
	//
	GLfloat		ambient[4];
	camera*		cam;

	// Debug
	debugtextframe* debugtext;
	int			debug_flags;
} ;

// *** Static functions

void scene_static_init( );

// *** Scene functions

// Make a Scene
scene* scene_create();

// Traverse the transform graph, updating worldspace transforms
void scene_concatenateTransforms(scene* s);

// Process input for the scene
void scene_input( scene* s, input* in );

// Update the scene
void scene_tick(scene* s, float dt);

void scene_renderLighting(scene* s);

void scene_render(scene* s);

void scene_setAmbient(scene* s, float r, float g, float b, float a);

void scene_setCamera(scene* s, float x, float y, float z, float w);

void scene_addModel(scene* s, modelInstance* m);

void scene_addTransform( scene* s, transform* t );

modelInstance* scene_getModel(scene* s, int i);

// Initialise a scene with some test data
void test_scene_init(scene* s);

void test_scene_tick(scene* s, float dt);


#endif // __SCENE_H__
