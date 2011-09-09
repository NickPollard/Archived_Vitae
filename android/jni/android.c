// Wrapper for the android activity

/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

//BEGIN_INCLUDE(all)
#include <jni.h>
#include <errno.h>

#include <GLES2/gl2.h>
#include <EGL/egl.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>

// *** Vitae Includes
#include "common.h"
#include "engine.h"
#include "input.h"
#include "maths.h"
#include "particle.h"
#include "mem/allocator.h"
#include "render/render.h"
#include "system/file.h"
#include "system/hash.h"
#include "system/string.h"

#if 0
/**
 * Our saved state data.
 */

struct saved_state {
    float angle;
    int32_t x;
    int32_t y;
};

/**
 * Shared state for our app.
 */
struct android_engine {
    struct android_app* app;

    ASensorManager* sensorManager;
    const ASensor* accelerometerSensor;
    ASensorEventQueue* sensorEventQueue;

    int animating;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;
    struct saved_state state;
};

/**
 * Initialize an EGL context for the current display.
 */
static int engine_init_display(struct android_engine* engine) {
    // initialize OpenGL ES and EGL

    /*
     * Here specify the attributes of the desired configuration.
     * Below, we select an EGLConfig with at least 8 bits per color
     * component compatible with on-screen windows
     */
    const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_NONE
    };
    EGLint w, h, dummy, format;
    EGLint numConfigs;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, 0, 0);

    /* Here, the application chooses the configuration it desires. In this
     * sample, we have a very simplified selection process, where we pick
     * the first EGLConfig that matches our criteria */
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);

    /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
     * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
     * As soon as we picked a EGLConfig, we can safely reconfigure the
     * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

    ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

    surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
    context = eglCreateContext(display, config, NULL, NULL);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        LOGW("Unable to eglMakeCurrent");
        return -1;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    engine->display = display;
    engine->context = context;
    engine->surface = surface;
    engine->width = w;
    engine->height = h;
    engine->state.angle = 0;

    // Initialize GL state.
//    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    glEnable(GL_CULL_FACE);
//    glShadeModel(GL_SMOOTH);
    glDisable(GL_DEPTH_TEST);

    return 0;
}

/**
 * Just the current frame in the display.
 */
static void engine_draw_frame(struct android_engine* engine) {
    if (engine->display == NULL) {
        // No display.
        return;
    }

    // Just fill the screen with a color.
    glClearColor(((float)engine->state.x)/engine->width, engine->state.angle,
            ((float)engine->state.y)/engine->height, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    eglSwapBuffers(engine->display, engine->surface);
}


#endif

#define TEST true

// ###################################

void test() {
	// Memory Tests
	test_allocator();

	test_hash();

	// System Tests
	test_sfile();

	test_matrix();

	test_property();

	test_string();
}

egl_renderer* egl_init( struct android_app* app ) {
    // initialize OpenGL ES and EGL

    /*
     * Here specify the attributes of the desired configuration.
     * Below, we select an EGLConfig with at least 8 bits per color
     * component compatible with on-screen windows
     */
    const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
			EGL_DEPTH_SIZE, 8,
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_NONE
    };
    EGLint w, h, dummy, format;
    EGLint numConfigs;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;

    EGLDisplay display = eglGetDisplay( EGL_DEFAULT_DISPLAY );

    eglInitialize( display, 0, 0 );

    /* Here, the application chooses the configuration it desires. In this
     * sample, we have a very simplified selection process, where we pick
     * the first EGLConfig that matches our criteria */
	printf( "EGL Choosing Config." );
    eglChooseConfig( display, attribs, &config, 1, &numConfigs );

    /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
     * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
     * As soon as we picked a EGLConfig, we can safely reconfigure the
     * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
    eglGetConfigAttrib( display, config, EGL_NATIVE_VISUAL_ID, &format );
    
	printf( "EGL Setting Window Buffers." );
    ANativeWindow_setBuffersGeometry( app->window, 0, 0, format );

	printf( "EGL Creating Surface." );
    surface = eglCreateWindowSurface( display, config, app->window, NULL );
   
	// Ask for a GLES2 context	
	const EGLint context_attribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2, 
		EGL_NONE
	};

	printf( "EGL Creating Context." );
    context = eglCreateContext(display, config, NULL, context_attribs );

    if ( eglMakeCurrent(display, surface, surface, context) == EGL_FALSE ) {
        LOGW( "Unable to eglMakeCurrent" );
        return NULL;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

	egl_renderer* egl = mem_alloc( sizeof( egl_renderer ));

    egl->display = display;
    egl->context = context;
    egl->surface = surface;
    egl->width = w;
    egl->height = h;

    return egl;
}

/**
 * Just the current frame in the display.
 */
static void draw_frame( egl_renderer* egl ) {
	printf( "ANDROID: beginning drawing frame." );
    if (egl->display == NULL) {
        // No display.
        return;
    }

	printf( "ANDROID: drawing frame." );
    // Just fill the screen with a color.
    glClearColor( 0.5f, 1.f, 0.5f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    eglSwapBuffers( egl->display, egl->surface );
}


/**
 * Tear down the EGL context currently associated with the display.
 */
static void egl_term( egl_renderer* egl ) {
    if ( egl->display != EGL_NO_DISPLAY ) {
        eglMakeCurrent( egl->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
        if ( egl->context != EGL_NO_CONTEXT ) {
            eglDestroyContext( egl->display, egl->context );
        }
        if (egl->surface != EGL_NO_SURFACE) {
            eglDestroySurface( egl->display, egl->surface );
        }
        eglTerminate( egl->display );
    }
    egl->display = EGL_NO_DISPLAY;
    egl->context = EGL_NO_CONTEXT;
    egl->surface = EGL_NO_SURFACE;
}

/**
 * Process the next main command.
 */
static void handle_cmd( struct android_app* app, int32_t cmd ) {
//    struct android_engine* droid_engine = (struct android_engine*)app->userData;
    switch (cmd) {
		/*
        case APP_CMD_SAVE_STATE:
            // The system has asked us to save our current state.  Do so.
            engine->app->savedState = malloc(sizeof(struct saved_state));
            *((struct saved_state*)engine->app->savedState) = engine->state;
            engine->app->savedStateSize = sizeof(struct saved_state);
            break;
			*/
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
			printf( "ANDROID: init window." );
            if ( app->window != NULL ) {
				printf( "ANDROID: init EGL." );
				vAssert( app->userData );
				engine* e = app->userData;
                e->egl = egl_init( app );
				e->active = true;
				render_init();
                draw_frame( ((engine*)app->userData)->egl );
            }
            break;
        case APP_CMD_TERM_WINDOW:
			{
				printf( "ANDROID: term window." );
				// The window is being hidden or closed, clean it up.
				engine* e = app->userData;
				egl_term( e->egl );
				e->active = false;
				break;
			}
		case APP_CMD_GAINED_FOCUS:
			{
				printf( "ANDROID: gain focus." );
				/*
				// When our app gains focus, we start monitoring the accelerometer.
				if (engine->accelerometerSensor != NULL) {
				ASensorEventQueue_enableSensor(engine->sensorEventQueue,
				engine->accelerometerSensor);
				// We'd like to get 60 events per second (in us).
				ASensorEventQueue_setEventRate(engine->sensorEventQueue,
				engine->accelerometerSensor, (1000L/60)*1000);
				}
				*/
				break;
			}
		case APP_CMD_LOST_FOCUS:
			{
				printf( "ANDROID: lost focus." );
				/*
				// When our app loses focus, we stop monitoring the accelerometer.
				// This is to avoid consuming battery while not being used.
				if (engine->accelerometerSensor != NULL) {
				ASensorEventQueue_disableSensor(engine->sensorEventQueue,
				engine->accelerometerSensor);
				}
				*/
				break;
			}
	}
}

/**
 * Process the next input event.
 */
static int32_t handle_input(struct android_app* app, AInputEvent* event) {
    engine* e = app->userData;
	vAssert( e );
	vAssert( e->input );
    if ( AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION ) {
		int x = AMotionEvent_getX( event, 0 );
		int y = AMotionEvent_getY( event, 0 );
		//printf( "Touch input. %d %d\n", x, y );
		input_registerTouch( e->input, x, y );
        return 1;
    }
    return 0;
}

void android_init( struct android_app* app ) {
	printf("Loading Vitae.\n");

	// *** Initialise Engine
	// already created
	engine* e = engine_create();
	static_engine_hack = e;
	e->egl = NULL;
	e->app = app;
	e->active = false;
	app->userData = e;

}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main( struct android_app* app ) {
    // Make sure glue isn't stripped.
    app_dummy();

    app->onAppCmd = handle_cmd;
	app->onInputEvent = handle_input;
	app->userData = NULL;

	// Static init - includes Memory init
	init( 0, NULL );

	android_init( app );

	printf( "ANDROID: Waiting to init EGL.\n" );

	// We want to wait until we have an EGL context before we kick off
	// This is so that OpenGL can init correctly
	while ( !((engine*)app->userData)->egl ) {
		//		sleep( 1 );
		int ident;
		int events;
		struct android_poll_source* source;


		while (( ident = ALooper_pollAll(0, NULL, &events, (void**)&source )) >= 0) {
			// Process this event.
			if (source != NULL) {
				source->process( app, source );
			}
		}
	}
	printf( "ANDROID: EGL initialised, away we go!" );

	// Android doesn't have commandline args
	int argc = 0;
	char** argv = NULL;
	// Can't init engine until EGL is initialised
	vAssert( ((engine*)app->userData)->egl );
	engine_init( app->userData, argc, argv );

#if TEST
	test();
#endif

	engine_run( static_engine_hack );
	
}
