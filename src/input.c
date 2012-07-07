// input.c

#include "common.h"
#include "input.h"
//---------------------
#include "maths.h"
#include "mem/allocator.h"

// GLFW Libraries
#include <GL/glfw.h>

// *** Static data

int input_bindCount = 0;
int input_keybinds[INPUT_MAX_KEYBINDS];


//
// *** keyboard
//

// Is the key held down this frame? Regardless of previous state
int input_keyHeld( input* in, int key ) {
	return in->data[in->active].keys[key / 8] & (0x1 << (key % 8));
}

// Was the key held down last frame? Regardless of previous state
int input_keyWasHeld( input* in, int key ) {
	return in->data[1 - in->active].keys[key / 8] & (0x1 << (key % 8));
}

// Was the key first pressed this frame? ie. It is depressed now, but was not last frame
int input_keyPressed( input* in, int key ) {
	return input_keyHeld( in, key) && !input_keyWasHeld( in, key );
}

// Was the key first released this frame? ie. It is not depressed now, but was last frame
int input_keyReleased( input* in, int key ) {
	return !input_keyHeld( in, key) && input_keyWasHeld( in, key );
}

input* input_create() {
	input* in = mem_alloc( sizeof( input ));
	memset( in, 0, sizeof( input));
	in->active = 0;
	memset( in->data, 0, sizeof( input_data ) * INPUT_DATA_FRAMES );
	memcpy( in->keybinds, input_keybinds, INPUT_MAX_KEYBINDS ); // init the keybinds to defaults
	return in;
}
//
// *** Keybinds
//

int input_registerKeybind( ) {
	return input_bindCount++;
}

// Set a keybind for the given input setup only, overwriting the default
void input_setKeyBind( input* in, keybind bind, int key ) {
	in->keybinds[bind] = key;
}

// Set a default keybind. This will be copied into any input that is created after
void input_setDefaultKeyBind( keybind bind, int key ) {
	input_keybinds[bind] = key;
}

// Keybind varients of the key functions
int input_keybindPressed( input* in, keybind bind ) {
	return input_keyPressed( in, in->keybinds[bind] );
}
int input_keybindHeld( input* in, keybind bind ) {
	return input_keyHeld( in, in->keybinds[bind] );
}
int input_keybindReleased( input* in, keybind bind ) {
	return input_keyReleased( in, in->keybinds[bind] );
}
int input_keybindWasHeld( input* in, keybind bind ) {
	return input_keyWasHeld( in, in->keybinds[bind] );
}

//
// *** Mouse
//

bool input_mouseHeld( input* in, int button ) {
	return in->data[in->active].mouse & (0x1 << button);
}

bool input_mouseWasHeld( input* in, int button ) {
	return in->data[in->active].mouse & (0x1 << button);
}

bool input_mousePressed( input* in, int button ) {
	return input_mouseHeld( in, button ) && !input_mouseWasHeld( in, button );
}

bool input_mouseReleased( input* in, int button ) {
	return !input_mouseHeld( in, button ) && input_mouseWasHeld( in, button );
}

void input_getMousePos( input* in, int* x, int* y ) {
	*x = in->data[in->active].mouseX;
	*y = in->data[in->active].mouseY;
}

void input_getMouseMove( input* in, int* x, int* y ) {
	*x = in->data[in->active].mouseX - in->data[1 - in->active].mouseX;
	*y = in->data[in->active].mouseY - in->data[1 - in->active].mouseY;
}

void input_getMouseDrag( input* in, int button, int* x, int* y ) {
	if ( input_mouseHeld( in, button )) {
		input_getMouseMove( in, x, y );
	}
	else {
		*x = 0;
		*y = 0;
	}
}

#ifdef TOUCH
void input_registerTouch( input* in, int x, int y, enum touchAction action ) {
	in->touchX = x;
	in->touchY = y;
	if ( action == kTouchDown || action == kTouchMove )
		in->touched = true;
	else if ( action == kTouchUp )
		in->touched = false;
}

void input_getTouchDrag( input* in, int* x, int* y ) {
	*x = 0;
	*y = 0;
	if ( in->data[in->active].touched && in->data[in->active ^ 0x1].touched ) {
		*x = in->data[in->active].touchX - in->data[in->active ^ 0x1].touchX;
		*y = in->data[in->active].touchY - in->data[in->active ^ 0x1].touchY;
	}
}

bool input_touchPressedInternal( input* in ) {
	return ( in->data[in->active].touched && !(in->data[in->active ^ 0x1].touched) );
}

bool input_touchPressed( input* in, int x_min, int y_min, int x_max, int y_max ) {
	int x = in->data[in->active].touchX;
	int y = in->data[in->active].touchY;
	return input_touchPressedInternal( in ) &&
		 	contains( x, x_min, x_max ) &&
		 	contains( y, y_min, y_max );
}

bool input_touchHeldInternal( input* in ) {
	return in->data[in->active].touched;
}

bool input_touchHeld( input* in, int x_min, int y_min, int x_max, int y_max ) {
	if ( x_min < 0 ) x_min += in->w;
	if ( y_min < 0 ) y_min += in->h;
	if ( x_max < 0 ) x_max += in->w;
	if ( y_max < 0 ) y_max += in->h;

	int x = in->data[in->active].touchX;
	int y = in->data[in->active].touchY;
	return input_touchHeldInternal( in ) &&
		 	contains( x, x_min, x_max ) &&
		 	contains( y, y_min, y_max );
}

void input_setWindowSize( input* in, int w, int h ) {
	in->w = w;
	in->h = h;
}
#endif

int getKey( int key ) {
//	return glfwGetKey( key ) ) << j;
	(void)key;
	return 0x0;
}

int getMouseButton( int button ) {
	//return glfwGetMouseButton( button );
	(void)button;
	return 0x0;
}

void getMousePos( int* x, int* y ) {
	//glfwGetMousePos( x, y );
	*x = 0;
	*y = 0;
}

// tick the input, recording this frames input data from devices
void input_tick( input* in, float dt ) {
	(void)dt;
	in->active = in->active ^ 0x1;		// flip the key buffers (we effectively double buffer the key state)

#ifndef ANDROID	
	// Store current state of keys
	for ( int i = 0; i < ( KEY_COUNT / 8 ); i++ ) {
		in->data[in->active].keys[i] = 0x0;
		for ( int j = 0; j < 8 ; j++ ) {
			in->data[in->active].keys[i] |= ( 0x1 & getKey( i * 8 + j ) ) << j;
		}
	}

	// Store current state of mouse
	in->data[in->active].mouse = 0x0;
	for ( int i = 0; i < 2; i++ )
		in->data[in->active].mouse |= ( 0x1 & getMouseButton( i )) << i;

	getMousePos( &in->data[in->active].mouseX, &in->data[in->active].mouseY );
#endif

#ifdef TOUCH
	// TODO - probably need to sync this properly with Android input thread?
	// Store current state of touch
	in->data[in->active].touched = in->touched;
	in->data[in->active].touchX = in->touchX;
	in->data[in->active].touchY = in->touchY;
	// keep last touched state - we only revert to false if we receive a release event
	in->touchX = 0;
	in->touchY = 0;
/*
	if ( input_touchPressedInternal( in ) ) {
		printf( "Touched! %d %d\n", in->data[in->active].touchX, in->data[in->active].touchY );
	}
	*/
#endif
}


