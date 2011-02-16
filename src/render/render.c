// render.c

#include "common.h"
#include "render.h"
//-----------------------
#include "camera.h"
#include "light.h"
#include "model.h"
#include "scene.h"
#include "render/texture.h"
// temp
#include "engine.h"

// GLFW Libraries
#include <GL/glfw.h>

// Private Function declarations
void render_buildShaders();

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

void render_applyCamera(camera* cam) {
	glLoadIdentity();
	// Negate as we're doing the inverse of camera
	vector* v = camera_getTranslation(cam);
	glTranslatef( -(v->coord.x), -(v->coord.y), -(v->coord.z) );
}

// Clear information from last draw
void render_clear() {
	glClearColor(0.f, 0.f, 0.0f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// Initialise the 3D rendering
void render_init() {
	if (!glfwInit())
		printf("ERROR - failed to init glfw.\n");

	glfwOpenWindow(640, 480, 5, 6, 5, 0, 0, 0, GLFW_WINDOW);
	glfwSetWindowTitle("Vitae");
	glfwSetWindowSizeCallback(handleResize);

	printf("RENDERING: Initialising OpenGL rendering settings.\n");
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_COLOR_MATERIAL);

	texture_init();

	render_buildShaders();
}

// Terminate the 3D rendering
void render_terminate() {
	glfwTerminate();
}

// Render the current scene
// This is where the business happens
void render(scene* s) {
	render_clear();

	// Switch to the drawing perspective and initialise to the identity matrix
	glMatrixMode(GL_MODELVIEW); 

	render_applyCamera(s->cam);
	render_lighting(s);
	render_scene(s);

	glPushMatrix();
	glBegin(GL_TRIANGLES);
	glColor4f(1.f, 1.f, 1.f, 1.f);

	glTexCoord2f(0.f, 0.f);
	glNormal3f(0.f, 0.f, 1.f);
	glVertex3f(0.f, 0.f, depth);

	glTexCoord2f(1.f, 0.f);
	glColor4f(1.f, 0.f, 1.f, 1.f);
	glVertex3f(1.f, 0.f, depth);

	glTexCoord2f(0.f, 1.f);
	glColor4f(0.f, 1.f, 1.f, 1.f);
	glVertex3f(0.f, 1.f, depth);

	glTexCoord2f(1.f, 1.f);
	glColor4f(1.f, 1.f, 1.f, 1.f);
	glVertex3f(1.f, 2.f, depth);

	glTexCoord2f(1.f, 0.f);
	glColor4f(1.f, 1.f, 1.f, 1.f);
	glVertex3f(1.f, 0.f, depth);

	glTexCoord2f(0.f, 1.f);
	glColor4f(1.f, 1.f, 1.f, 1.f);
	glVertex3f(0.f, 2.f, depth);
	glEnd();
	glPopMatrix();

	glfwSwapBuffers(); // Send the 3d scene to the screen (flips display buffers)
}

void render_buildShaders() {

}
