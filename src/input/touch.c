// touch.c
#include "common.h"
#include "touch.h"
//---------------------
#include "input.h"

#ifdef TOUCH
void input_registerTouch( input* in, int x, int y, enum touchAction action ) {
	in->touch.x = x;
	in->touch.y = y;
	if ( action == kTouchDown || action == kTouchMove )
		in->touch.touched = true;
	else if ( action == kTouchUp )
		in->touch.touched = false;
}

void input_getTouchDrag( input* in, int* x, int* y ) {
	*x = 0;
	*y = 0;
	if ( in->data[in->active].touch.touched && in->data[in->active ^ 0x1].touch.touched ) {
		*x = in->data[in->active].touch.x - in->data[in->active ^ 0x1].touch.x;
		*y = in->data[in->active].touch.y - in->data[in->active ^ 0x1].touch.y;
	}
}

bool input_touchPressedInternal( input* in ) {
	return ( in->data[in->active].touch.touched && !(in->data[in->active ^ 0x1].touch.touched) );
}

bool input_touchPressed( input* in, int x_min, int y_min, int x_max, int y_max ) {
	int x = in->data[in->active].touch.x;
	int y = in->data[in->active].touch.y;
	return input_touchPressedInternal( in ) &&
		 	contains( x, x_min, x_max ) &&
		 	contains( y, y_min, y_max );
}

bool input_touchHeldInternal( input* in ) {
	return in->data[in->active].touch.touched;
}

bool input_touchHeld( input* in, int x_min, int y_min, int x_max, int y_max ) {
	if ( x_min < 0 ) x_min += in->w;
	if ( y_min < 0 ) y_min += in->h;
	if ( x_max < 0 ) x_max += in->w;
	if ( y_max < 0 ) y_max += in->h;

	int x = in->data[in->active].touch.x;
	int y = in->data[in->active].touch.y;
	return input_touchHeldInternal( in ) &&
		 	contains( x, x_min, x_max ) &&
		 	contains( y, y_min, y_max );
}

void input_touchTick( input* in, float dt ) {
	// TODO - probably need to sync this properly with Android input thread?
	// Store current state of touch
	in->data[in->active].touch.touched = in->touch.touched;
	in->data[in->active].touch.x = in->touch.x;
	in->data[in->active].touch.y = in->touch.y;
	
	// keep last touched state - we only revert to false if we receive a release event
}
#endif // TOUCH
