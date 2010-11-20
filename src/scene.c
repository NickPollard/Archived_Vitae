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
}

void glTranslate_vector(vector* v) {
	glTranslatef(v->coord.x, v->coord.y, v->coord.z);
}

// Iterate through each model in the scene
// Translate by their transform
// Then draw all the submeshes
void scene_render(scene* s) {
	for (int i = 0; i < s->modelCount; i++) {
		glPushMatrix();
		model_draw(scene_getModel(s, i));
		glPopMatrix();
	}
}

void scene_drawLighting(scene* s) {
	// Ambient Light
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, s->ambient);
	
	// Add a positioned light1
//	GLfloat lightColour0[] = {1.f, 1.0f, 1.0f, 1.f};
//	GLfloat lightPos0[] = {0.f, 0.f, 0.f, 3.f};
//	GLfloat lightAttenuationConstant = 1.f;
//	GLfloat lightAttenuationLinear = 1.f;
//	GLfloat lightAttenuationQuadratic = 0.f;
//	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColour0);
//	glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);
//	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, lightAttenuationLinear);
//	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, lightAttenuationConstant);
//	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, lightAttenuationQuadratic);

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
