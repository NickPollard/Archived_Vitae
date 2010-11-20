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

// GLFW Libraries
#include <GL/glfw.h>

// System libraries
#include <sys/time.h>

scene* theScene = NULL;
engine* static_engine_hack;

float camX = 0.f;
float camY = 0.f;

/*
 *
 *  Test Functions
 *
 */

void test_engine_tick(engine* e, float dt) {
//	scene_tick(theScene, dt);
}

void test_engine_init() {
	theScene = scene_createScene();
	test_scene_init(theScene);
}

/*
 *
 *  Engine Methods
 *
 */

// tick - process a frame of game update
void engine_tick(engine* e) {
	float dt = timer_getDelta(e->timer);

	// TEST
	test_engine_tick(e, dt);
	// end TEST

	scene_tick(theScene, dt);

	if (e->onTick && luaCallback_enabled(e->onTick)) {
//		printf("Calling engine::onTick handler: %s\n", e->onTick->func);
		LUA_CALL(e->lua, e->onTick->func);
	}

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

// Handle a window resize
// Set the camera perspective and tell OpenGL how to convert 
// from coordinates to pixel values
void handleResize(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (double)w / (double)h, 1.0, 200.0);

//	float aspect_ratio = ((float)w) / (float)h;
//	glFrustum(.5, -.5, -.5 * aspect_ratio, .5 * aspect_ratio, 1, 50);
	// Note to self - does glu normally use doubles rather than floats?
}

float angle = 340.f;

float depth = 8.f;

// Draws the 3D scene
void engine_render() {
	// Clear information from last draw
	glClearColor(0.f, 0.f, 0.0f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Switch to the drawing perspective and initialise to the identity matrix
	glMatrixMode(GL_MODELVIEW); 
//	glLoadIdentity(); 
//	glTranslatef(0.f, 0.f, -10.f);

	scene_applyCamera(theScene);
	scene_renderLighting(theScene);
	scene_render(theScene);

	glPushMatrix();
	glBegin(GL_TRIANGLES);
	glColor4f(1.f, 1.f, 1.f, 1.f);
	glNormal3f(0.f, 0.f, 1.f);
	glVertex3f(0.f, 0.f, depth);
	glColor4f(1.f, 0.f, 1.f, 1.f);
	glVertex3f(1.f, 0.f, depth);
	glColor4f(0.f, 1.f, 1.f, 1.f);
	glVertex3f(0.f, 1.f, depth);

	glColor4f(1.f, 1.f, 1.f, 1.f);
	glVertex3f(1.f, 2.f, depth);
	glColor4f(1.f, 1.f, 1.f, 1.f);
	glVertex3f(1.f, 0.f, depth);
	glColor4f(1.f, 1.f, 1.f, 1.f);
	glVertex3f(0.f, 2.f, depth);
	glEnd();
	glPopMatrix();

	glfwSwapBuffers(); // Send the 3d scene to the screen (flips display buffers)
}

// Initialise the 3D rendering
void render_init() {
	printf("RENDERING: Initialising OpenGL rendering settings.\n");
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_COLOR_MATERIAL);
}

// Initialise the OpenGL subsystem so it is ready for use
void engine_initOpenGL(engine* e, int argc, char** argv) {
	if (!glfwInit())
		printf("ERROR - failed to init glfw.\n");

	glfwOpenWindow(640, 480, 5, 6, 5, 0, 0, 0, GLFW_WINDOW);
	glfwSetWindowTitle("Vitae");
	glfwSetWindowSizeCallback(handleResize);

	render_init(); // initialise the rendering
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
	test_engine_init();
}

// Initialises the application
void init(int argc, char** argv) {

	// *** Initialise Memory
	mem_init(argc, argv);

	// *** Initialise Engine
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

// run - executes the main loop of the engine
void run() {
//	TextureLibrary* textures = texture_library_create();
	int running = true;
	handleResize(640, 480);	// Call once to init
	while (running) {
		engine_tick(static_engine_hack);
		engine_render();

		running = !glfwGetKey(GLFW_KEY_ESC) && glfwGetWindowParam(GLFW_OPENED);

		if (glfwGetKey(GLFW_KEY_UP)) {
			depth += 0.01f;
			camY += 0.01f;
		}
		if (glfwGetKey(GLFW_KEY_DOWN)) {
			depth -= 0.01f;
			camY -= 0.01f;
		}
		if (glfwGetKey(GLFW_KEY_LEFT)) {
			camX -= 0.01f;
		}
		if (glfwGetKey(GLFW_KEY_RIGHT)) {
			camX += 0.01f;
		}

		scene_setCamera(theScene, camX, camY, 10.f, 1.f);
	}
}

