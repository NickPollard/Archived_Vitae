#ifndef __TRANSFORM_H__
#define __TRANSFORM_H__

#include "maths.h"

// *** Transform ***
struct transform_s {
	matrix		local;
	matrix		world;
	transform*	parent;
};

// Create a new default transform
transform* transform_createTransform();

// Create a new default transform with the given parent
transform* transform_createTransform_Parent(transform* parent);

#endif // __TRANSFORM_H__
