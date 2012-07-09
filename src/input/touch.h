// touch.h
#pragma once

enum touchAction {
	kTouchDown,
	kTouchMove,
	kTouchUp
};

typedef struct touch_panel_s {
	bool	touched;
	int32_t x;
	int32_t y;
} touch_panel;

// *** Touch
void input_registerTouch( input* in, int x, int y, enum touchAction action );
void input_getTouchDrag( input* in, int* x, int* y );
bool input_touchPressed( input* in, int x_min, int y_min, int x_max, int y_max );
bool input_touchHeld( input* in, int x_min, int y_min, int x_max, int y_max );

// *** Tick function
void input_touchTick( input* in, float dt );
