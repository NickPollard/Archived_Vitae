#include "common.h"
#include "transform.h"
//-------------------------

void transform_setWorldSpace();

void transform_setLocalSpace();

// Create a new default transform
transform* transform_createTransform() {
	transform* t = malloc(sizeof(transform));
	matrix_setIdentity(&t->local);
	matrix_setIdentity(&t->world);
	t->parent = NULL;
	return t;
}

// Create a new default transform with the given parent
transform* transform_createTransform_Parent(transform* parent) {
	transform* t = transform_createTransform();
	t->parent = parent;
	return t;
}
