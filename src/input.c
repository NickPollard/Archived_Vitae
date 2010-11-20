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
