// scene.c

#include "common.h"
#include "scene.h"
//-----------------------
#include "light.h"
#include "model.h"
#include "mem/allocator.h"
#include "src/render/debugdraw.h"


model* testModelA = NULL;
model* testModelB = NULL;
transform* t2 = NULL;

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
	transform* t = transform_createTransform(s);
	testModelA->trans->parent = t;
	testModelB->trans->parent = t;
	vector translate = {{ -2.f, 0.f, 0.f, 1.f }};
	transform_setLocalTranslation(t, &translate);
	t2 = transform_createTransform(s);
	t->parent = t2;

	scene_addModel(s, testModelA);
	scene_addModel(s, testModelB);

	scene_setAmbient(s, 0.2f, 0.f, 0.2f, 1.f);

	transform* lightT = transform_createTransform(s);
	light* l = light_create();
	l->trans = lightT;
	vector lightPos = {{ 1.f, 1.f, 1.f, 1.f }};
	light_setPosition(l, &lightPos);
	light_setDiffuse(l, 1.f, 0.f, 0.f, 1.f);
	scene_addLight(s, l);

	scene_setCamera(s, 0.f, 0.f, 10.f, 1.f);
}

void glTranslate_vector(vector* v) {
	glTranslatef(v->coord.x, v->coord.y, v->coord.z);
}

// Iterate through each model in the scene
// Translate by their transform
// Then draw all the submeshes
void scene_render(scene* s) {
	for (int i = 0; i < s->modelCount; i++) {
		model_draw(scene_getModel(s, i));
	}
}

void scene_renderLighting(scene* s) {
	// Ambient Light
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, s->ambient);
	
	light_render(GL_LIGHT0, s->lights[0]);
}

void scene_setAmbient(scene* s, float r, float g, float b, float a) {
	s->ambient[0] = r;
	s->ambient[1] = g;
	s->ambient[2] = b;
	s->ambient[3] = a;
}

// Make a scene
scene* scene_createScene() {
	scene* s = mem_alloc(sizeof(scene));
	s->modelCount = s->lightCount = s->transformCount = 0;
	scene_setCamera(s, 0.f, 0.f, 0.f, 1.f);
	scene_setAmbient(s, 0.2f, 0.2f, 0.2f, 1.f);
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

	// TEST
	test_scene_tick(s, dt);
}

void scene_setCamera(scene* s, float x, float y, float z, float w) {
	s->cameraPos.val[0] = x;
	s->cameraPos.val[1] = y;
	s->cameraPos.val[2] = z;
	s->cameraPos.val[3] = w;
}

void scene_applyCamera(scene* s) {
	glLoadIdentity();
	// Negate as we're doing the inverse of CameraPos
	glTranslatef(-s->cameraPos.coord.x,
				-s->cameraPos.coord.y,
				-s->cameraPos.coord.z);
}

void test_scene_tick(scene* s, float dt) {
	static float time = 0.f;
	time += dt;
	float period = 2.f;
	float animate = 2.f * sinf(time * 2 * PI / period);
	// Animate cubes
	vector translateA = {{ animate, 0.f, 0.f, 1.f}};
	transform_setLocalTranslation(testModelA->trans, &translateA);
	vector translateB = {{ 0.f, animate, 0.f, 1.f}};
	transform_setLocalTranslation(testModelB->trans, &translateB);
	
	vector translateC = {{ 0.f, 0.f, animate,  1.f}};
	transform_setLocalTranslation(t2, &translateC);
}
