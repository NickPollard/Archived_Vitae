// input.c

#include "common.h"
#include "input.h"
//---------------------

// GLFW Libraries
#include <GL/glfw.h>

// Is the key held down this frame? Regardless of previous state
int input_keyHeld( input* i, int key ) {
	return i->data[i->active].keys[key / 8] & (0x1 << (key % 8));
}

// Was the key held down last frame? Regardless of previous state
int input_keyWasHeld( input* i, int key ) {
	return i->data[1 - i->active].keys[key / 8] & (0x1 << (key % 8));
}

// Was the key first pressed this frame? ie. It is depressed now, but was not last frame
int input_keyPressed( input* i, int key ) {
	return input_keyHeld( i, key) && !input_keyWasHeld( i, key );
}

// Was the key first released this frame? ie. It is not depressed now, but was last frame
int input_keyReleased( input* i, int key ) {
	return !input_keyHeld( i, key) && input_keyWasHeld( i, key );
}

input* input_create() {
	input* i = malloc( sizeof( input ));
	i->active = 0;
	memset (i->data, 0, sizeof( input_data ) * INPUT_DATA_FRAMES );
	return i;
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
