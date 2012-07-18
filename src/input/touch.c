// touch.c
#include "common.h"
#include "touch.h"
//---------------------
#include "engine.h"
#include "input.h"
#include "maths/maths.h"
#include "mem/allocator.h"

#ifdef TOUCH
// *** Forward declarations
int touchPad_localX( touchPad* p, int x );
int touchPad_localY( touchPad* p, int y );
void touchPad_tick( touchPad* p, input* in, float dt );

// *** Implementation

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

void input_getTouchPosition( input* in, int* x, int* y ) {
	*x = in->data[in->active].touch.x;
	*y = in->data[in->active].touch.y;
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
	(void)dt;
	// TODO - probably need to sync this properly with Android input thread?
	// Store current state of touch
	in->data[in->active].touch.touched = in->touch.touched;
	in->data[in->active].touch.x = in->touch.x;
	in->data[in->active].touch.y = in->touch.y;
	
	// keep last touched state - we only revert to false if we receive a release event

	touchPanel_tick( &in->touch, in, dt );
}


// ***************
// Touch Panels
void touchPanel_init( touchPanel* p ) {
	memset( p, 0, sizeof( touchPanel ));
	p->touched = false;
	p->touch_pad_count = 0;
	p->touch_pad = mem_alloc( sizeof( touchPad* ) * kMaxTouchPads );
}

void touchPanel_tick( touchPanel* panel, input* in, float dt ) {
	for_each( panel->touch_pad, panel->touch_pad_count, touchPad_tick, in, dt )
}

// ***************
// Touch Pads

touchPad* touchPad_create( int x, int y, int w, int h ) {
	touchPad* p = mem_alloc( sizeof( touchPad ));
	p->x = x; p->y = y;
	p->width = w; p->height = h;
	p->active = true;
	return p;
}

touchPad* touchPanel_addTouchPad( touchPanel* panel, touchPad* pad ) {
	array_add( (void**)panel->touch_pad, &panel->touch_pad_count, pad );
	return pad;
}

void touchPanel_removeTouchPad( touchPanel* panel, touchPad* pad ) {
	array_remove( (void**)panel->touch_pad, &panel->touch_pad_count, pad );
}

void touchPad_tick( touchPad* p, input* in, float dt ) {
	(void)p; (void)in; (void)dt;

	touch* t = &p->touches[0];
	t->x = t->y = -1;
	if (input_touchHeld( in, p->x, p->y, p->x + p->width, p->y + p->height )) {
		int x, y;
		input_getTouchPosition( in, &x, &y );
		t->x = touchPad_localX( p, x );	
		t->y = touchPad_localY( p, y );	
	}
}

bool touchPad_touched( touchPad* p, int* x, int* y ) {
	*x = p->touches[0].x;
	*y = p->touches[0].y;
	return p->touches[0].x != -1;
}

void touchPad_activate( touchPad* p ) {
	p->active = true;
}

void touchPad_deactivate( touchPad* p ) {
	p->active = false;
}

int touchPad_localX( touchPad* p, int x ) {
	return ( x - p->x );
}

int touchPad_localY( touchPad* p, int y ) {
	return ( y - p->y );
}

#endif // TOUCH
