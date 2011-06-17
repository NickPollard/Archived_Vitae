#ifndef __TRANSFORM_H__
#define __TRANSFORM_H__

#include "maths.h"
#include "mem/pool.h"

// *** Transform ***
struct transform_s {
	matrix		local;	// Local space matrix; 15th element is 1.f if clean and 2.f if dirty
	matrix		world;
	transform*	parent;
	int			isDirty;
};

DECLARE_POOL( transform )

// *** Static
void transform_initPool();

// Create a new default transform
transform* transform_create(scene* s);

// Create a new default transform with the given parent
transform* transform_create_Parent(scene* s, transform* parent);

// *** Members

// Concatenate the parent world space transforms to produce this world space transform from local
int transform_concatenate(transform* t);

// Mark the transform as dirty (needs concatenation)
void transform_markDirty(transform* t);

// Mark the transform as clean (doesn't need concatenation)
void transform_markClean(transform* t);

// Is the transform dirty? (does it need concatenating?)
int transform_isDirty(transform* t);

// Set the translation of the localspace transformation matrix
void transform_setLocalTranslation(transform* t, vector* v);

void transform_printDebug( transform* t, debugtextframe* f );

#endif // __TRANSFORM_H__
