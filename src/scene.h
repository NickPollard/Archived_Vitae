// scene.h

/*
   A scene object represents a drawable scene of OpenGL models.

   It contains the scene heirarchy graph, specifying models, lights, transforms, etc.
   */

#ifndef __SCENE_H__
#define __SCENE_H__

#include "common.fwd.h"
#include "maths.h"

#define MAX_TRANSFORMS 128
#define MAX_MODELS 128
#define MAX_LIGHTS 128

// *** Transform ***
struct transform_s {
	matrix		local;
	matrix		world;
	transform*	parent;
};

// *** Scene ***
typedef struct scene_s {
	int			modelCount;
	model*		models[MAX_MODELS];
	int			lightCount;
	light*		lights[MAX_LIGHTS];
	transform	transforms[MAX_TRANSFORMS];
} scene;

// Create a new default transform
transform* transform_createTransform();

// Create a new default transform with the given parent
transform* transform_createTransform_Parent(transform* parent);

#endif // __SCENE_H__
