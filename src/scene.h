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

#define MAX_TRANSFORMS 128
#define MAX_MODELS 128
#define MAX_LIGHTS 128

// *** Scene ***
struct scene_s {
	int			modelCount;
	model*		models[MAX_MODELS];
	int			lightCount;
	light*		lights[MAX_LIGHTS];
	transform	transforms[MAX_TRANSFORMS];
	int			transformCount;
} ;

// Make a Scene
scene* scene_createScene();

// Traverse the transform graph, updating worldspace transforms
void scene_concatenateTransforms(scene* s);

// Update the scene
void scene_tick(scene* s, float dt);

#endif // __SCENE_H__
