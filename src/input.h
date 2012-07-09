// input.h
#pragma once

#include "input/keyboard.h"

// Mouse
#define BUTTON_LEFT		0x0
#define BUTTON_RIGHT	0x0

// *** General input defines
#define INPUT_DATA_FRAMES 2

#ifdef ANDROID
#define TOUCH
#endif

#ifdef TOUCH
enum touchAction {
	kTouchDown,
	kTouchMove,
	kTouchUp
};
#endif

// Keys stored as a packed bitmask
// ie. each byte stores 8 flags for 8 keys respectively
typedef struct input_data_s {
	key_array keys;
	char mouse;
	int mouseX;
	int mouseY;
#ifdef TOUCH
	bool	touched;
	int32_t touchX;
	int32_t touchY;
#endif
} input_data;

struct input_s {
	int active;	// Index into the data array, alternates between 0 and 1
	input_data data[INPUT_DATA_FRAMES]; // This frame, last frame - switch on every frame
	int keybinds[INPUT_MAX_KEYBINDS];
#ifdef TOUCH
	int32_t touchX;
	int32_t touchY;
	bool	touched;

	int w;
	int h;
#endif
};

// Constructor
input* input_create();

// tick the input, recording this frames input data from devices
void input_tick( input* in, float dt );

// *** Mouse

bool input_mouseHeld( input* in, int button );
bool input_mouseWasHeld( input* in, int button );
bool input_mousePressed( input* in, int button );
bool input_mouseReleased( input* in, int button );
void input_getMouseMove( input* in, int* x, int* y );
void input_getMouseDrag( input* in, int button, int* x, int* y );

// *** Touch
#ifdef TOUCH
void input_registerTouch( input* in, int x, int y, enum touchAction action );
void input_getTouchDrag( input* in, int* x, int* y );
bool input_touchPressed( input* in, int x_min, int y_min, int x_max, int y_max );
bool input_touchHeld( input* in, int x_min, int y_min, int x_max, int y_max );

void input_setWindowSize( input* in, int w, int h );
#endif

// *** Unit Test
void test_input();
