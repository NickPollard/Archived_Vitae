// scene.c

#include "common.h"
#include "scene.h"
//-----------------------
#include "light.h"
#include "model.h"

#include <GL/glut.h>

void scene_concatenateTransforms(scene* s)
{
	// Traverse the transform graph, updating worldspace transforms
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

scene* scene_createTestScene() {
	scene* s = malloc(sizeof(scene));
	s->modelCount = 0;
	s->lightCount = 0;

	model* m = model_createTestCube();
	light* l = light_createTestLight();

	scene_addModel(s, m);
	scene_addLight(s, l);

	return s;
}

void glTranslate_vector(vector* v) {
	glTranslatef(v->coord.x, v->coord.y, v->coord.z);
}

void drawScene(scene* s) {
	// for each model in the scene
	for (int i = 0; i < s->modelCount; i++) {
		glPushMatrix();
		// TODO: Translate by the models transform
		model_draw(scene_getModel(s, i));
		glPopMatrix();
	}
}

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
