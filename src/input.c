// input.c

#include "common.h"
#include "input.h"
//---------------------

// GLFW Libraries
#include <GL/glfw.h>

// return whether the given key is held down or not
// does not discriminate between whether the key was pressed this frame
int key_held(int key) {
	return glfwGetKey(key);
}

int input_keyHeld( input* i, int key ) {
	return i->data[i->active].keys[key / 8] & (1 << (key % 8));
}

int input_keyWasHeld( input* i, int key ) {
	return i->data[1 - i->active].keys[key / 8] & (1 << (key % 8));
}

int input_keyPressed( input* i, int key ) {
	return input_keyHeld( i, key) && !input_keyWasHeld( i, key );
}

int input_keyReleased( input* i, int key ) {
	return !input_keyHeld( i, key) && input_keyWasHeld( i, key );
}
