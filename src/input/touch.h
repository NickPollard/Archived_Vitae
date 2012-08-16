// touch.h
#pragma once

/*
	Touch

TODO: Touch documentation
   */

enum touchAction {
	kTouchDown,
	kTouchMove,
	kTouchUp,
	kTouchUnknown
};

#define kMaxMultiTouch 16
#define kMaxTouchPads 16
#define kInvalidTouchUid -1
#define kInvalidIndex -1

// A touch stimulus, there will be one for each current touch in a multi-touch environment
typedef struct touch_s {
	int uid;
	int x;
	int y;
	int drag_x;
	int drag_y;
	bool pressed;
	bool released;
	bool held;
} touch;

// An input area on a touchPanel that we want to track input for
typedef struct touchPad_s {
	int	x;
	int y;
	int width;
	int height;
	bool active;
	int		touch_count;
	touch	touches[kMaxMultiTouch];
} touchPad;

// A touch input device, i.e. a touch screen on a phone or tablet
typedef struct touchPanel_s {
	int		touch_count;
	touch	touches[kMaxMultiTouch];
	/*
	bool	touched;
	int32_t x;
	int32_t y;
	*/
	int touch_pad_count;
	touchPad** touch_pad;
} touchPanel;


// *** Touch
void input_registerTouch( input* in, int uid, int x, int y, enum touchAction action );
void input_getTouchDrag( input* in, int pointer, int* x, int* y );
bool input_touchPressed( input* in, int x_min, int y_min, int x_max, int y_max );
bool input_touchHeld( input* in, int x_min, int y_min, int x_max, int y_max );

// *** Tick function
void input_touchTick( input* in, float dt );

// *** Touch panels
void touchPanel_init( touchPanel* p );
void touchPanel_tick( touchPanel* panel, input* in, float dt );
touchPad* touchPanel_addTouchPad( touchPanel* panel, touchPad* pad );

// *** Touch pads
touchPad* touchPad_create( int x, int y, int w, int h );
bool touchPad_touched( touchPad* p, int* x, int* y );
bool touchPad_dragged( touchPad* p, int* x, int* y );
