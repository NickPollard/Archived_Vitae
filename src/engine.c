// engine.c
#include "src/common.h"
#include "src/engine.h"
#include "src/ticker.h"
#include "src/window.h"
#include <sys/time.h>

#include <GL/glut.h>

// Use GTK library
#define __GTK__

ticklist* test_tickers = NULL;

// tick - process a frame of game update
void engine_tick(int ei) {
	engine* e = (engine*)ei;
	float dt = frame_timer_delta(e->timer);
	(void)dt;

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

void drawQuadAsTris(GLfloat* a, GLfloat* b, GLfloat* c, GLfloat* d) {
	glVertex3fv(a);
	glVertex3fv(b);
	glVertex3fv(c);

	glVertex3fv(c);
	glVertex3fv(d);
	glVertex3fv(a);
}

void drawCube() {
	GLfloat ftl[] = { -0.5f, 0.5f, 0.5f };
	GLfloat ftr[] = { 0.5f, 0.5f, 0.5f };
	GLfloat fbl[] = { -0.5f, -0.5f, 0.5f };
	GLfloat fbr[] = { 0.5f, -0.5f, 0.5f };
	GLfloat btl[] = { -0.5f, 0.5f, -0.5f };
	GLfloat btr[] = { 0.5f, 0.5f, -0.5f };
	GLfloat bbl[] = { -0.5f, -0.5f, -0.5f };
	GLfloat bbr[] = { 0.5f, -0.5f, -0.5f };

	glNormal3f(0.f, 0.f, 1.f);
	drawQuadAsTris(ftl, ftr, fbr, fbl);
	glNormal3f(0.f, 0.f, -1.f);
	drawQuadAsTris(btl, btr, bbr, bbl);
	glNormal3f(1.f, 0.f, 0.f);
	drawQuadAsTris(ftr, fbr, bbr, btr);
	glNormal3f(-1.f, 0.f, 0.f);
	drawQuadAsTris(ftl, fbl, bbl, btl);
	glNormal3f(0.f, 1.f, 0.f);
	drawQuadAsTris(ftl, ftr, btr, btl);
	glNormal3f(0.f, -1.f, 0.f);
	drawQuadAsTris(fbl, fbr, bbr, bbl);
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
	glRotatef(angle, 0.f, 0.f, 1.f);
	glTranslatef(0.f, -1.f, 0.f); // Move to the center of the Trapezoid

	glBegin(GL_QUADS); // Begin quad coords
	
	// Trapezoid
	glNormal3f(0.f, 0.f, 1.f);
	glColor3f(0.5f, 0.f, 0.f);
	glVertex3f(-0.7f, -.5f, 0.f);
	glColor3f(0.f, 0.5f, 0.f);
	glVertex3f(0.7f, -.5f, 0.f);
	glColor3f(0.f, 0.f, 0.5f);
	glVertex3f(0.4f, 0.5f, 0.f);
	glColor3f(0.5f, 0.5f, 0.f);
	glVertex3f(-0.4f, 0.5f, 0.f);

	glEnd(); // End Quads

	glPopMatrix();
	glPushMatrix();
	glTranslatef(1.f, 1.f, 0.f); // Move to the center of the pentagon

	glBegin(GL_TRIANGLES); // Begin triangle coords

	//Pentagon
	glColor3f(0.f, 0.5f, 0.f);
	glNormal3f(0.f, 0.f, 1.f);
	glVertex3f(-0.5f, -.5f, 0.f);
	glVertex3f(0.5f, -.5f, 0.f);
	glVertex3f(-0.5f, 0.f, 0.f);

	glVertex3f(-0.5f, 0.f, 0.f);
	glVertex3f(0.5f, -.5f, 0.f);
	glVertex3f(0.5f, 0.f, 0.f);

	glVertex3f(-0.5f, 0.f, 0.f);
	glVertex3f(0.5f, 0.f, 0.f);
	glVertex3f(0.f, 0.5f, 0.f);

	glEnd(); // End Triangles

	glPopMatrix();

	glPushMatrix();
	glTranslatef(1.f, -1.f, 0.f); // Move to the center of the pentagon
	glBegin(GL_TRIANGLES); // Begin triangle coords
	drawCube();
	glEnd(); // End Triangles
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-1.f, 1.f, 0.f); // Move to the center of the pentagon
	glBegin(GL_TRIANGLES); // Begin triangle coords
	drawCube();
	glEnd(); // End Triangles
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
	frame_timer_init(e->timer);

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

