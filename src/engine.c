// engine.c
#include "src/common.h"
#include "src/engine.h"
//---------------------
#include "mem/allocator.h"
#include "src/maths.h"
#include "src/model.h"
#include "src/scene.h"
#include "src/ticker.h"
#include "src/transform.h"

// Lua Libraries
#include <lauxlib.h>
#include <lualib.h>

// GLUT Libraries
#include <GL/glut.h>
// GLFW Libraries
#include <GL/glfw.h>

// System libraries
#include <sys/time.h>

model* testModelA = NULL;
model* testModelB = NULL;

scene* theScene = NULL;
transform* t2 = NULL;

engine* static_engine_hack;

/*
 *
 *  Test Functions
 *
 */

void engine_tick_TEST(engine* e, float dt) {
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

void init_TEST() {
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
}




/*
 *
 *  Engine Methods
 *
 */


void tick(int ePtr) {
	engine_tick((engine*)ePtr);
//	glutTimerFunc(25, tick, ePtr);
}


// tick - process a frame of game update
void engine_tick(engine* e) {
	float dt = timer_getDelta(e->timer);

	// TEST
	engine_tick_TEST(e, dt);
	// end TEST

	scene_tick(theScene, dt);

	if (e->onTick && luaCallback_enabled(e->onTick)) {
//		printf("Calling engine::onTick handler: %s\n", e->onTick->func);
		LUA_CALL(e->lua, e->onTick->func);
	}

//	glutPostRedisplay(); // Tell GLUT that the rendering has changed.
}

// Static wrapper
void handleKeyPress(uchar key, int x, int y) {
	engine_handleKeyPress(static_engine_hack, key, x, y);
}

// Handle a key press from the user
void engine_handleKeyPress(engine* e, uchar key, int x, int y) {
	// Lua Test
	lua_getglobal(e->lua, "handleKeyPress");
	lua_pushnumber(e->lua, (int)key);
	lua_pcall(e->lua,	/* args */			1,
						/* returns */		1,
						/* error handler */ 0);
	int ret = lua_tonumber(e->lua, -1);
	printf("Lua says %d!\n", ret);

	switch (key) {
		case 27: // Escape key
			engine_deInit(e); // Exit the program
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

float angle = 340.f;

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

	glBegin(GL_TRIANGLES);
	glVertex3f(0.f, 0.f, 5.f);
	glVertex3f(1.f, 0.f, 5.f);
	glVertex3f(0.f, 1.f, 5.f);
	glEnd();

	glPushMatrix();
	glTranslatef(1.f, -1.f, 0.f); // Move to the center of the pentagon
	model_draw(testModelA);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-1.f, 1.f, 0.f); // Move to the center of the pentagon
	model_draw(testModelB);
	glPopMatrix();

	glfwSwapBuffers(); // Send the 3d scene to the screen (flips display buffers)
}

// Initialise the 3D rendering
void initRendering() {
	// Enable default depth test
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
}

// Initialise the OpenGL subsystem so it is ready for use
void engine_initOpenGL(engine* e, int argc, char** argv) {
	// Initialise GLUT
//	glutInit(&argc, argv);
//	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
//	glutInitWindowSize(900, 600); //Set the window size
	if (!glfwInit())
		printf("ERROR - failed to init glfw.\n");

	glfwOpenWindow(640, 480, 0, 0, 0, 0, 0, 0, GLFW_WINDOW);

	// Create the window
//	glutCreateWindow("Vitae");
	initRendering(); // initialise the rendering

//	glutDisplayFunc(test_drawScene);
//	glutKeyboardFunc(handleKeyPress);
//	glutReshapeFunc(handleResize);
	
//	glutTimerFunc(25, tick, (int)e);
}

// Initialise the Lua subsystem so that it is ready for use
void engine_initLua(engine* e, int argc, char** argv) {
	e->lua = lua_open();
	luaL_openlibs(e->lua);	// Load the Lua libs into our lua state
	if (luaL_loadfile(e->lua, "src/lua/main.lua") || lua_pcall(e->lua, 0, 0, 0))
		printf("Error loading lua!\n");

	lua_registerFunction(e->lua, LUA_registerCallback, "registerEventHandler");

	LUA_CALL(e->lua, "init");
}

// Create a new engine
engine* engine_create() {
	engine* e = mem_alloc(sizeof(engine));
	e->timer = mem_alloc(sizeof(frame_timer));
	e->callbacks = luaInterface_create();
	e->onTick = luaInterface_addCallback(e->callbacks, "onTick");
	return e;
}

// Initialise the engine
void engine_init(engine* e, int argc, char** argv) {
	timer_init(e->timer);
	
	// *** Start up Core Systems

	// *** Initialise OpenGL
	engine_initOpenGL(e, argc, argv);

	// *** Initialise Lua
	engine_initLua(e, argc, argv);
	luaInterface_registerCallback(e->callbacks, "onTick", "tick");

	// TEST
	init_TEST();
}

// Initialises the application
void init(int argc, char** argv) {
	mem_init(argc, argv);

	engine* e = engine_create();
	engine_init(e, argc, argv);
	static_engine_hack = e;
}

// deInit_lua - deinitialises the Lua interpreter
void engine_deInitLua(engine* e) {
	lua_close(e->lua);
}

// deInit - deInitialises the engine
void engine_deInit(engine* e) {
	engine_deInitLua(e);

	glfwTerminate();

	exit(0);
}

void run_OpenGL() {
	int running = true;
	while (running) {
		test_drawScene();

		running = !glfwGetKey(GLFW_KEY_ESC) && glfwGetWindowParam(GLFW_OPENED);
	}
}

// run - executes the main loop of the engine
void run() {
//	TextureLibrary* textures = texture_library_create();

	run_OpenGL();
}

