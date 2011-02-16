// input.h
#ifndef __INPUT_H__
#define __INPUT_H__

#define GLFW

#ifdef GLFW

// Arrow Keys
#define KEY_UP		GLFW_KEY_UP
#define KEY_DOWN	GLFW_KEY_DOWN
#define KEY_LEFT	GLFW_KEY_LEFT
#define KEY_RIGHT	GLFW_KEY_RIGHT

// Other
#define KEY_ESC		GLFW_KEY_ESC

#endif

#define KEY_MAX 256

typedef struct input_data_s {
	int keys[KEY_MAX];
} input_data;

typedef struct input_s {
	input_data currentData;
	input_data lastData;
} input;

// return whether the given key is held down or not
// does not discriminate between whether the key was pressed this frame
int key_held(int key);

#endif // __INPUT_H__
