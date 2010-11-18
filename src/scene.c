// scene.c

#include "common.h"
#include "scene.h"
//-----------------------
#include "light.h"
#include "model.h"
#include "mem/allocator.h"

#include <GL/glut.h>

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
	scene* s = mem_alloc(sizeof(scene));
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

// Iterate through each model in the scene
// Translate by their transform
// Then draw all the submeshes
void drawScene(scene* s) {
	for (int i = 0; i < s->modelCount; i++) {
		glPushMatrix();
		// TODO: Translate by the models transform
		model_draw(scene_getModel(s, i));
		glPopMatrix();
	}
}

// Make a scene
scene* scene_createScene() {
	scene* s = mem_alloc(sizeof(scene));
	s->modelCount = s->lightCount = s->transformCount = 0;
	return s;
}

// Traverse the transform graph, updating worldspace transforms
void scene_concatenateTransforms(scene* s) {
	for (int i = 0; i < s->transformCount; i++)
		transform_concatenate(&s->transforms[i]);
}

// Update the scene
void scene_tick(scene* s, float dt) {
	scene_concatenateTransforms(s);
}
