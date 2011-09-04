// input.h
#ifndef __INPUT_H__
#define __INPUT_H__

#define GLFW

#ifdef GLFW
// GLFW Libraries
#ifndef ANDROID
#include <GL/glfw.h>
#else

// Ripped from GLFW.h

/* Keyboard key definitions: 8-bit ISO-8859-1 (Latin 1) encoding is used
 * for printable keys (such as A-Z, 0-9 etc), and values above 256
 * represent special (non-printable) keys (e.g. F1, Page Up etc).
 */
#define GLFW_KEY_UNKNOWN      -1
#define GLFW_KEY_SPACE        32
#define GLFW_KEY_SPECIAL      256
#define GLFW_KEY_ESC          (GLFW_KEY_SPECIAL+1)
#define GLFW_KEY_F1           (GLFW_KEY_SPECIAL+2)
#define GLFW_KEY_F2           (GLFW_KEY_SPECIAL+3)
#define GLFW_KEY_F3           (GLFW_KEY_SPECIAL+4)
#define GLFW_KEY_F4           (GLFW_KEY_SPECIAL+5)
#define GLFW_KEY_F5           (GLFW_KEY_SPECIAL+6)
#define GLFW_KEY_F6           (GLFW_KEY_SPECIAL+7)
#define GLFW_KEY_F7           (GLFW_KEY_SPECIAL+8)
#define GLFW_KEY_F8           (GLFW_KEY_SPECIAL+9)
#define GLFW_KEY_F9           (GLFW_KEY_SPECIAL+10)
#define GLFW_KEY_F10          (GLFW_KEY_SPECIAL+11)
#define GLFW_KEY_F11          (GLFW_KEY_SPECIAL+12)
#define GLFW_KEY_F12          (GLFW_KEY_SPECIAL+13)
#define GLFW_KEY_F13          (GLFW_KEY_SPECIAL+14)
#define GLFW_KEY_F14          (GLFW_KEY_SPECIAL+15)
#define GLFW_KEY_F15          (GLFW_KEY_SPECIAL+16)
#define GLFW_KEY_F16          (GLFW_KEY_SPECIAL+17)
#define GLFW_KEY_F17          (GLFW_KEY_SPECIAL+18)
#define GLFW_KEY_F18          (GLFW_KEY_SPECIAL+19)
#define GLFW_KEY_F19          (GLFW_KEY_SPECIAL+20)
#define GLFW_KEY_F20          (GLFW_KEY_SPECIAL+21)
#define GLFW_KEY_F21          (GLFW_KEY_SPECIAL+22)
#define GLFW_KEY_F22          (GLFW_KEY_SPECIAL+23)
#define GLFW_KEY_F23          (GLFW_KEY_SPECIAL+24)
#define GLFW_KEY_F24          (GLFW_KEY_SPECIAL+25)
#define GLFW_KEY_F25          (GLFW_KEY_SPECIAL+26)
#define GLFW_KEY_UP           (GLFW_KEY_SPECIAL+27)
#define GLFW_KEY_DOWN         (GLFW_KEY_SPECIAL+28)
#define GLFW_KEY_LEFT         (GLFW_KEY_SPECIAL+29)
#define GLFW_KEY_RIGHT        (GLFW_KEY_SPECIAL+30)
#define GLFW_KEY_LSHIFT       (GLFW_KEY_SPECIAL+31)
#define GLFW_KEY_RSHIFT       (GLFW_KEY_SPECIAL+32)
#define GLFW_KEY_LCTRL        (GLFW_KEY_SPECIAL+33)
#define GLFW_KEY_RCTRL        (GLFW_KEY_SPECIAL+34)
#define GLFW_KEY_LALT         (GLFW_KEY_SPECIAL+35)
#define GLFW_KEY_RALT         (GLFW_KEY_SPECIAL+36)
#define GLFW_KEY_TAB          (GLFW_KEY_SPECIAL+37)
#define GLFW_KEY_ENTER        (GLFW_KEY_SPECIAL+38)
#define GLFW_KEY_BACKSPACE    (GLFW_KEY_SPECIAL+39)
#define GLFW_KEY_INSERT       (GLFW_KEY_SPECIAL+40)
#define GLFW_KEY_DEL          (GLFW_KEY_SPECIAL+41)
#define GLFW_KEY_PAGEUP       (GLFW_KEY_SPECIAL+42)
#define GLFW_KEY_PAGEDOWN     (GLFW_KEY_SPECIAL+43)
#define GLFW_KEY_HOME         (GLFW_KEY_SPECIAL+44)
#define GLFW_KEY_END          (GLFW_KEY_SPECIAL+45)
#define GLFW_KEY_KP_0         (GLFW_KEY_SPECIAL+46)
#define GLFW_KEY_KP_1         (GLFW_KEY_SPECIAL+47)
#define GLFW_KEY_KP_2         (GLFW_KEY_SPECIAL+48)
#define GLFW_KEY_KP_3         (GLFW_KEY_SPECIAL+49)
#define GLFW_KEY_KP_4         (GLFW_KEY_SPECIAL+50)
#define GLFW_KEY_KP_5         (GLFW_KEY_SPECIAL+51)
#define GLFW_KEY_KP_6         (GLFW_KEY_SPECIAL+52)
#define GLFW_KEY_KP_7         (GLFW_KEY_SPECIAL+53)
#define GLFW_KEY_KP_8         (GLFW_KEY_SPECIAL+54)
#define GLFW_KEY_KP_9         (GLFW_KEY_SPECIAL+55)
#define GLFW_KEY_KP_DIVIDE    (GLFW_KEY_SPECIAL+56)
#define GLFW_KEY_KP_MULTIPLY  (GLFW_KEY_SPECIAL+57)
#define GLFW_KEY_KP_SUBTRACT  (GLFW_KEY_SPECIAL+58)
#define GLFW_KEY_KP_ADD       (GLFW_KEY_SPECIAL+59)
#define GLFW_KEY_KP_DECIMAL   (GLFW_KEY_SPECIAL+60)
#define GLFW_KEY_KP_EQUAL     (GLFW_KEY_SPECIAL+61)
#define GLFW_KEY_KP_ENTER     (GLFW_KEY_SPECIAL+62)
#define GLFW_KEY_KP_NUM_LOCK  (GLFW_KEY_SPECIAL+63)
#define GLFW_KEY_CAPS_LOCK    (GLFW_KEY_SPECIAL+64)
#define GLFW_KEY_SCROLL_LOCK  (GLFW_KEY_SPECIAL+65)
#define GLFW_KEY_PAUSE        (GLFW_KEY_SPECIAL+66)
#define GLFW_KEY_LSUPER       (GLFW_KEY_SPECIAL+67)
#define GLFW_KEY_RSUPER       (GLFW_KEY_SPECIAL+68)
#define GLFW_KEY_MENU         (GLFW_KEY_SPECIAL+69)
#define GLFW_KEY_LAST         GLFW_KEY_MENU
#endif

// Arrow Keys
#define KEY_UP		GLFW_KEY_UP
#define KEY_DOWN	GLFW_KEY_DOWN
#define KEY_LEFT	GLFW_KEY_LEFT
#define KEY_RIGHT	GLFW_KEY_RIGHT

// Other
#define KEY_ESC		GLFW_KEY_ESC
#define KEY_SHIFT	GLFW_KEY_LSHIFT
#define KEY_SPACE	GLFW_KEY_SPACE

#define KEY_T		'T'
#define KEY_L		'L'
#define KEY_W		'W'
#define KEY_A		'A'
#define KEY_S		'S'
#define KEY_D		'D'
#define KEY_Q		'Q'
#define KEY_E		'E'

// Mouse
#define BUTTON_LEFT		GLFW_MOUSE_BUTTON_LEFT
#define BUTTON_RIGHT	GLFW_MOUSE_BUTTON_RIGHT

#endif // GLFW

// *** General input defines
typedef int keybind;
#define KEY_COUNT 512
#define INPUT_DATA_FRAMES 2
#define INPUT_KEYDATA_SIZE KEY_COUNT / sizeof ( char )

// *** keybind defines
#define INPUT_MAX_KEYBINDS 128

// Keys stored as a packed bitmask
// ie. each byte stores 8 flags for 8 keys respectively
typedef struct input_data_s {
	char keys[INPUT_KEYDATA_SIZE];
	char mouse;
	int mouseX;
	int mouseY;
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

// *** Mouse

bool input_mouseHeld( input* in, int button );
bool input_mouseWasHeld( input* in, int button );
bool input_mousePressed( input* in, int button );
bool input_mouseReleased( input* in, int button );
void input_getMouseMove( input* in, int* x, int* y );
void input_getMouseDrag( input* in, int button, int* x, int* y );
#endif // __INPUT_H__
