// input.h
#ifndef __INPUT_H__
#define __INPUT_H__

/*
	Keyboard input

	Keyboard input goes through several stages. We have concepts of:
	Key - e.g. 'Escape', a conceptual key that is one of the keys we expect to find on the
		keyboard.
	KeyCode - e.g. '0x9', the physical signal we get from the keyboard, which happens to be the
		keycode for Escape on my keyboard under Ubuntu.
	KeyBinding - e.g. 'Exit Game', a control that we want to be accessible from the keyboard which
		the user is free to rebind to a different key if they so choose.

	So, for example if we want to use escape to quit the game, what we actually want is to set up
	an 'Exit Game' keybind, so that the user can keybind if they so want. 
	This keybind is then set by default to the 'Escape' key.
	The input system then interrogates the system to find out what keyCode - e.g. 0x9 - we expect
	from the keyboard if the escape key is pressed. Due to different layouts, languages etc. this
	is not guaranteed to be the same key.

	So From a high level downwards:

	KeyBinding ( 'exit' )
	  |
	  |
	  |
	 Key ( 'escape' )
	  |
	  |
	  |
	KeyCode ( 0x9 )
   */

// Mouse
#define BUTTON_LEFT		0x0
#define BUTTON_RIGHT	0x0

enum key {
	KEY_ESC,
	KEY_A,
	KEY_B,
	KEY_C,
	KEY_D,
	KEY_E,
	KEY_F,
	KEY_G,
	KEY_H,
	KEY_I,
	KEY_J,
	KEY_K,
	KEY_L,
	KEY_M,
	KEY_N,
	KEY_O,
	KEY_P,
	KEY_Q,
	KEY_R,
	KEY_S,
	KEY_T,
	KEY_U,
	KEY_V,
	KEY_W,
	KEY_X,
	KEY_Y,
	KEY_Z,
	KEY_UP,
	KEY_DOWN,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_SHIFT,
	KEY_SPACE,
	kMaxKeyCodes
};

int key_codes[kMaxKeyCodes];

// *** General input defines
typedef int keybind;
#define KEY_COUNT 512
#define INPUT_DATA_FRAMES 2
#define INPUT_KEYDATA_SIZE KEY_COUNT / sizeof ( char )

// *** keybind defines
#define INPUT_MAX_KEYBINDS 128

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
	char keys[INPUT_KEYDATA_SIZE];
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

#ifdef LINUX_X
input_data x_key_array;
#endif

// Constructor
input* input_create();

// tick the input, recording this frames input data from devices
void input_tick( input* in, float dt );

// *** Keybinds
int input_registerKeybind( );

// Set a keybind for the given input setup only, overwriting the default
void input_setKeyBind( input* in, keybind bind, int key );

// Set a default keybind. This will be copied into any input that is created after
void input_setDefaultKeyBind( keybind bind, int key );

/*
	Held - the key is currently held
	WasHeld - the key was held the last frame
	Pressed - the key is currently held, but was not held last frame (i.e. key down)
	Released - they key was currently held, but is no longer (i.e. key up)
   */
int input_keyHeld( input* i, enum key k );
int input_keyWasHeld( input* i, enum key k );
int input_keyPressed( input* i, enum key k );
int input_keyReleased( input* i, enum key k );

// Keybind varients of the key functions
int input_keybindPressed( input* in, int keybind );
int input_keybindHeld( input* in, int keybind );
int input_keybindReleased( input* in, int keybind );
int input_keybindWasHeld( input* in, int keybind );

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

#ifdef LINUX_X
void input_xKeyPress( int key );
void input_xKeyRelease( int key );
#endif // LINUX_X

#ifdef LINUX_X
void input_initKeyCodes( xwindow* xwin );
#else
void input_initKeyCodes();
#endif // LINUX_X

// *** Unit Test
void test_input();

#endif // __INPUT_H__
