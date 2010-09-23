// engine.c
#include "src/common.h"
#include "src/engine.h"
//---------------------
#include "src/maths.h"
#include "src/model.h"
#include "src/scene.h"
#include "src/ticker.h"
#include "src/transform.h"
#include "src/window.h"

#include <GL/glut.h>

#include <sys/time.h>

// Use GTK library
#define __GTK__

model* testModelA = NULL;
model* testModelB = NULL;

scene* theScene = NULL;
transform* t2 = NULL;

// tick - process a frame of game update
void engine_tick(int ePtr) {
	engine* e = (engine*)ePtr;
	float dt = timer_getDelta(e->timer);
	(void)dt;

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

	scene_tick(theScene, dt);

	glutPostRedisplay(); // Tell GLUT that the rendering has changed.
	glutTimerFunc(25, engine_tick, (int)e);
}

void handleKeyPress(uchar key, int x, int y) {
	switch (key) {
		case 27: // Escape key
			exit(0); // Exit the program
	}
}

void handleResize(int w, int h) {
	// Tell OpenGL how to convert from coordinates to pixel values
	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION); // Enter projection mode, so that we'll modify the projection matrix

	// Set the camera perspective
	glLoadIdentity(); // initialise to the identity matrix
	gluPerspective(45.0,
			(double)w / (double)h,
			1.0,
			200.0);

	// Note to self - does glu normally use doubles rather than floats?
}

float angle =340.f;

void drawLighting() {
	// Ambient Light
	GLfloat ambientColour[] = { 0.2f, 0.2f, 0.2f, 1.f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColour);

	// Add a positioned light
	GLfloat lightColour0[] = {0.5f, 0.f, 0.f, 1.f};
	GLfloat lightPos0[] = {-2.f, 0.f, 2.f, 1.f};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColour0);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);
}

// Draws the 3D scene
void test_drawScene() {
	// Clear information from last draw
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	drawLighting();

	glMatrixMode(GL_MODELVIEW); // Switch to the drawing perspective
	glLoadIdentity(); // Initialise to the identity matrix
	glTranslatef(0.f, 0.f, -15.f); // Move forward 5 units

	glPushMatrix();
	glTranslatef(1.f, -1.f, 0.f); // Move to the center of the pentagon
	model_draw(testModelA);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-1.f, 1.f, 0.f); // Move to the center of the pentagon
	model_draw(testModelB);
	glPopMatrix();

	glutSwapBuffers(); // Send the 3d scene to the screen (flips display buffers)
}
// Initialise the 3D rendering
void initRendering() {
	// Enable default depth test
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
}

void init_OpenGL(engine* e, int argc, char** argv) {
	// Initialise GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(900, 600); //Set the window size

	// Create the window
	glutCreateWindow("Vitae");
	initRendering(); // initialise the rendering

	glutDisplayFunc(test_drawScene);
	glutKeyboardFunc(handleKeyPress);
	glutReshapeFunc(handleResize);
	
	glutTimerFunc(25, engine_tick, (int)e);
}

// init - initialises the engine
void init(int argc, char** argv) {
	engine* e = malloc(sizeof(engine));
	e->timer = malloc(sizeof(frame_timer));
	timer_init(e->timer);

	theScene = scene_createScene();
	testModelA = model_createTestCube();
	testModelB = model_createTestCube();
	transform* t = transform_createTransform(theScene);
	testModelA->trans->parent = t;
	testModelB->trans->parent = t;
	vector translate = {{ -2.f, 0.f, 0.f, 1.f }};
	transform_setLocalTranslation(t, &translate);
	t2 = transform_createTransform(theScene);
	t->parent = t2;
	
	init_OpenGL(e, argc, argv);
}

void run_OpenGL() {
	glutMainLoop();
}

// run - executes the main loop of the engine
void run() {
//	TextureLibrary* textures = texture_library_create();

	run_OpenGL();
}

