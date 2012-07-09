// mouse.h
#pragma once

// Mouse
#define BUTTON_LEFT		0x0
#define BUTTON_RIGHT	0x0

typedef struct mouse_s {
	char buttons;
	int x;
	int y;
} mouse;

// *** Mouse Accessors
bool input_mouseHeld( input* in, int button );
bool input_mouseWasHeld( input* in, int button );
bool input_mousePressed( input* in, int button );
bool input_mouseReleased( input* in, int button );
void input_mouseMove( input* in, int* x, int* y );
void input_mouseDrag( input* in, int button, int* x, int* y );

void input_mouseTick( input* in, float dt );
