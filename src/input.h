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

#define KEY_T		'T'
#define KEY_L		'L'
#define KEY_W		'W'
#define KEY_A		'A'
#define KEY_S		'S'
#define KEY_D		'D'
#define KEY_Q		'Q'
#define KEY_E		'E'


#endif // GLFW

// *** General input defines
typedef int keybind;
#define KEY_COUNT 512
#define INPUT_DATA_FRAMES 2
#define INPUT_KEYDATA_SIZE KEY_COUNT / sizeof ( char )

// *** keybind defines
#define INPUT_MAX_KEYBINDS 128

typedef struct input_data_s {
	char keys[INPUT_KEYDATA_SIZE];
} input_data;

struct input_s {
	int active;	// Index into the data array, alternates between 0 and 1
	input_data data[INPUT_DATA_FRAMES]; // This frame, last frame - switch on every frame
	int keybinds[INPUT_MAX_KEYBINDS];
};

// constructor
input* input_create();

// tick the input, recording this frames input data from devices
void input_tick( input* in, float dt );

// Is the key held down this frame? Regardless of previous state
int input_keyHeld( input* i, int key );

// Was the key held down last frame? Regardless of previous state
int input_keyWasHeld( input* i, int key );

// Was the key first pressed this frame? ie. It is depressed now, but was not last frame
int input_keyPressed( input* i, int key );

// Was the key first released this frame? ie. It is not depressed now, but was last frame
int input_keyReleased( input* i, int key );

int input_registerKeybind( );

// Set a keybind for the given input setup only, overwriting the default
void input_setKeyBind( input* in, keybind bind, int key );

// Set a default keybind. This will be copied into any input that is created after
void input_setDefaultKeyBind( keybind bind, int key );

// Keybind varients of the key functions
int input_keybindPressed( input* in, int keybind );
int input_keybindHeld( input* in, int keybind );
int input_keybindReleased( input* in, int keybind );
int input_keybindWasHeld( input* in, int keybind );

#endif // __INPUT_H__
