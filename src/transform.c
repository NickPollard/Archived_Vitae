#include "common.h"
#include "transform.h"
//-------------------------
#include "scene.h"

void transform_setWorldSpace();

void transform_setLocalSpace();

// Create a new default transform
transform* transform_createTransform(scene* s) {
	transform* t = &s->transforms[s->transformCount];
	s->transformCount++;
	matrix_setIdentity(&t->local);
	matrix_setIdentity(&t->world);
	t->parent = NULL;
	return t;
}

// Create a new default transform with the given parent
transform* transform_createTransform_Parent(scene* s, transform* parent) {
	transform* t = transform_createTransform(s);
	t->parent = parent;
	return t;
}

// Concatenate the parent world space transforms to produce this world space transform from local
void transform_concatenate(transform* t) {
	if (transform_isDirty(t))
	{
		transform_markClean(t);
		if (t->parent)
		{
			transform_concatenate(t->parent);
			t->world = matrix_mul(&t->local, &t->parent->world);
		}
		else
			t->world = t->local;
	}
}

// Mark the transform as dirty (needs concatenation)
void transform_markDirty(transform* t) {
	t->local.val[3][3] = 2.f;
}

// Mark the transform as clean (doesn't need concatenation)
void transform_markClean(transform* t) {
	t->local.val[3][3] = 1.f;
}

// Is the transform dirty? (does it need concatenating?)
int transform_isDirty(transform* t) {
	return (t->local.val[3][3] > 1.f);
}

// Set the translation of the localspace transformation matrix
void transform_setLocalTranslation(transform* t, vector* v) {
	matrix_setTranslation(&t->local, v);
	transform_markDirty(t);
}
