// input.c

#include "common.h"
#include "input.h"
//---------------------
#include "engine.h"
#include "maths.h"
#include "test.h"
#include "input/keyboard.h"
#include "mem/allocator.h"

#ifdef LINUX_X
#include <X11/Xlib.h>
#endif // LINUX_X

// *** Static data

int input_bindCount = 0;
int input_keybinds[INPUT_MAX_KEYBINDS];

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

int getKeyInternal( int key ) {
	(void)key;
	return 0x0;
}

// tick the input, recording this frames input data from devices
void input_tick( input* in, float dt ) {
	(void)dt;
	in->active = in->active ^ 0x1;		// flip the key buffers (we effectively double buffer the key state)

	input_keyboardTick( in, dt );
	input_mouseTick( in, dt );

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

#if UNIT_TEST
void test_input() {
	printf( "--- Beginning Unit Test: Input System ---\n" );
	test_keyboard();
}
#endif // UNIT_TEST
