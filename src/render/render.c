// render.c

#include "common.h"
#include "render.h"
//-----------------------
#include "light.h"
#include "model.h"
#include "scene.h"

// Iterate through each model in the scene
// Translate by their transform
// Then draw all the submeshes
void render_scene(scene* s) {
	for (int i = 0; i < s->modelCount; i++) {
		model_draw(scene_getModel(s, i));
	}
}

void render_lighting(scene* s) {
	// Ambient Light
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, s->ambient);
	
	light_render(GL_LIGHT0, s->lights[0]);
}
