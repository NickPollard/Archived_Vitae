// input.c

#include "common.h"
#include "input.h"
//---------------------

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
	input* in = malloc( sizeof( input ));
	memset( in, 0, sizeof( input));
	in->active = 0;
	memset( in->data, 0, sizeof( input_data ) * INPUT_DATA_FRAMES );
	memcpy( in->keybinds, input_keybinds, INPUT_MAX_KEYBINDS ); // init the keybinds to defaults
	return in;
}

// tick the input, recording this frames input data from devices
void input_tick( input* in, float dt ) {
	in->active = in->active ^ 0x1;		// flip the key buffers (we effectively double buffer the key state)

	// Store current state of keys
	for ( int i = 0; i < ( KEY_COUNT / 8 ); i++ ) {
		in->data[in->active].keys[i] = 0x0;
		for ( int j = 0; j < 8 ; j++ ) {
			in->data[in->active].keys[i] |= ( 0x1 & glfwGetKey( i * 8 + j ) ) << j;
		}
	}
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
