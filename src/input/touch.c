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

// Finds the index of the (first) touch with the given uid
// returns -1 if not found
int touch_indexOfUid( touch* touches, int count, int uid ) {
	int index = kInvalidIndex;
	for ( int i = 0; i < count; ++i ) {
		if ( touches[i].uid == uid ) {
			index = i;
			break;
		}
	}
	return index;
}

void input_registerTouch( input* in, int uid, int x, int y, enum touchAction action ) {
	// Find first available touch slot	
	int i = touch_indexOfUid( &in->touch.touches[0], kMaxMultiTouch, kInvalidTouchUid );
	vAssert( i != kInvalidIndex );

	if ( action == kTouchDown ) {
		//printf( "Touch down: %d.\n", uid );
	}

	touch* t = &(in->touch.touches[i]);
	t->x = x;
	t->y = y;
	t->uid = uid;

	t->held =		( action != kTouchUp );
	t->pressed =	( action == kTouchDown );
	t->released =	( action == kTouchUp );
}

void input_getTouchDrag( input* in, int pointer, int* x, int* y ) {
	*x = 0;
	*y = 0;
	if ( in->data[in->active].touch.touches[pointer].held && in->data[in->active ^ 0x1].touch.touches[pointer].held ) {
		*x = in->data[in->active].touch.touches[pointer].x - in->data[in->active ^ 0x1].touch.touches[pointer].x;
		*y = in->data[in->active].touch.touches[pointer].y - in->data[in->active ^ 0x1].touch.touches[pointer].y;
	}
}

bool input_touchPressedInternal( input* in, int i ) {
	return ( in->data[in->active].touch.touches[i].held && !(in->data[in->active ^ 0x1].touch.touches[i].held) );
}

bool input_touchPressed( input* in, int x_min, int y_min, int x_max, int y_max ) {
	if ( x_min < 0 ) x_min += in->w;
	if ( y_min < 0 ) y_min += in->h;
	if ( x_max < 0 ) x_max += in->w;
	if ( y_max < 0 ) y_max += in->h;

	bool touched = false;
	int num_touches = 1; // TODO
	for ( int i = 0; i < num_touches && !touched; ++i ) {
		int x = in->data[in->active].touch.touches[i].x;
		int y = in->data[in->active].touch.touches[i].y;
		touched = input_touchPressedInternal( in, i ) &&
					contains( x, x_min, x_max ) &&
					contains( y, y_min, y_max );
	}
	return touched;
}

bool input_touchHeldInternal( input* in, int i ) {
	return in->data[in->active].touch.touches[i].held;
}

bool input_touchValid( input* in, int i ) {
	return in->data[in->active].touch.touches[i].uid != kInvalidTouchUid;
}

// TODO - this should take into account multi-touch, i.e. if ANY touch has occurred
bool input_touchHeld( input* in, int x_min, int y_min, int x_max, int y_max ) {
	if ( x_min < 0 ) x_min += in->w;
	if ( y_min < 0 ) y_min += in->h;
	if ( x_max < 0 ) x_max += in->w;
	if ( y_max < 0 ) y_max += in->h;

	bool held = false;
	for ( int i = 0; i < kMaxMultiTouch && !held; ++i ) {
		touch* t = &in->data[in->active].touch.touches[i];
		int x = t->x;
		int y = t->y;
		held = input_touchValid( in, i ) && input_touchHeldInternal( in, i ) &&
				contains( x, x_min, x_max ) &&
				contains( y, y_min, y_max );
	}
	return held;
}

bool input_touchInsideBounds( input* in, touch* t, int x_min, int y_min, int x_max, int y_max ) {
	if ( x_min < 0 ) x_min += in->w;
	if ( y_min < 0 ) y_min += in->h;
	if ( x_max < 0 ) x_max += in->w;
	if ( y_max < 0 ) y_max += in->h;

	bool inside = contains( t->x, x_min, x_max ) && contains( t->y, y_min, y_max );
	return inside;
}

void input_touchTick( input* in, float dt ) {
	(void)dt;
	// TODO - probably need to sync this properly with Android input thread?

	/*
		We have the previous frames touches (in->data[in->inactive].touch)
		We also have the pending touches (in->touch)

		We use both of these to construct the new frame
		Pending touches take precedence, everything else is filled in from the old
	   */

	// Copy the old inputs into our new frame
	// Anything with an invalid UID is ignored
	// Anything that was previously released, we remove
	// This gives us the touches that are still alive
	int active_touches = 0;
	touch* new = &in->data[in->active].touch.touches[0];
	for ( int i = 0; i < kMaxMultiTouch; ++i ) {
		touch* old = &in->data[1 - in->active].touch.touches[i];
		if ( old->uid != kInvalidTouchUid && old->released ) {
			printf( "touch %d released\n", old->uid );
		}
		if ( old->uid != kInvalidTouchUid && !old->released ) {
			*new = *old;
			new->drag_x = 0.f;
			new->drag_y = 0.f;
			++new;
			++active_touches;
		}
	}

	// Copy the pending inputs into our new frame 
	// If a touch with the same UID exists, we overwrite and update it
	// If there is no existing touch with that UID, we add a new one
	// (this should be the case for any new presses
	for ( int i = 0; i < kMaxMultiTouch; ++i ) {
		touch* pending = &in->touch.touches[i];
		// Skip invalid touches
		if ( pending->uid == kInvalidTouchUid )
			continue;

		int existing_touch_index = touch_indexOfUid( &in->data[in->active].touch.touches[0], kMaxMultiTouch, pending->uid );
		if ( existing_touch_index == kInvalidIndex ) {
			// It's a new touch, should be a new press
			//vAssert( pending->pressed == true );
			// Add it on the end
			vAssert( active_touches < kMaxMultiTouch );
			touch* new = &in->data[in->active].touch.touches[active_touches];
			*new = *pending;
			++active_touches;
			//printf( "New touch received\n" );
		}
		else {
			touch* old = &in->data[in->active].touch.touches[existing_touch_index];
			int drag_x = pending->x - old->x;
			int drag_y = pending->y - old->y;
			*old = *pending;
			old->drag_x = drag_x;
			old->drag_y = drag_y;
			//printf( "Old touch update received\n" );
		}
	}
	//printf( "Total touches: %d.\n", active_touches );

	// NULL out the old touches no longer used
	for ( int i = active_touches; i < kMaxMultiTouch; ++i )
		in->data[in->active].touch.touches[i].uid = kInvalidTouchUid;

	// Clear out pending touches for next frame
	for ( int i = 0; i < kMaxMultiTouch; ++i )
		in->touch.touches[i].uid = kInvalidTouchUid;

	
	// TODO
	// Calculate drags

	// keep last touched state - we only revert to false if we receive a release event

	touchPanel_tick( &in->touch, in, dt );
}


// ***************
// Touch Panels
void touchPanel_init( touchPanel* p ) {
	memset( p, 0, sizeof( touchPanel ));
	p->touch_pad_count = 0;
	p->touch_pad = mem_alloc( sizeof( touchPad* ) * kMaxTouchPads );
	p->touch_count = 0;
	for ( int i = 0; i < kMaxMultiTouch; ++i ) {
		p->touches[i].uid = kInvalidTouchUid;
	}
}

void touchPanel_tick( touchPanel* panel, input* in, float dt ) {
	for_each( panel->touch_pad, panel->touch_pad_count, touchPad_tick, in, dt );
}

// ***************
// Touch Pads

touchPad* touchPad_create( int x, int y, int w, int h ) {
	touchPad* p = mem_alloc( sizeof( touchPad ));
	p->x = x; p->y = y;
	p->width = w; p->height = h;
	p->active = true;

	for ( int i = 0; i < kMaxMultiTouch; ++i ) {
		p->touches[i].uid = kInvalidTouchUid;
	}

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

	int count = 0;
	touch* local_touch = &p->touches[0];
	for ( int i = 0; i < kMaxMultiTouch; ++i ) {
		touch* global_touch = &in->data[in->active].touch.touches[i];
		if ( input_touchValid( in, i ) && input_touchInsideBounds( in, global_touch, p->x, p->y, p->x + p->width, p->y + p->height )) {
			// copy all touch data, but then update positions to pad space
			*local_touch = *global_touch;
			local_touch->x = touchPad_localX( p, global_touch->x );	
			local_touch->y = touchPad_localY( p, global_touch->y );	
			++local_touch;
			++count;
		}
	}
	//printf( "touchpad has %d touches.\n", count );

	// Blank out unused ones
	while ( local_touch < &p->touches[kMaxMultiTouch] ) {
		local_touch->x = -1;
		local_touch->y = -1;
		local_touch->uid = kInvalidTouchUid;
		++local_touch;
	}
}

bool touchPad_dragged( touchPad* p, int* x, int* y ) {
	*x = p->touches[0].drag_x;
	*y = p->touches[0].drag_y;
	return ( p->touches[0].uid != kInvalidTouchUid ) &&
			!p->touches[0].pressed && 
			!p->touches[0].released;
}

bool touchPad_touched( touchPad* p, int* x, int* y ) {
	*x = p->touches[0].x;
	*y = p->touches[0].y;
	return p->touches[0].uid != kInvalidTouchUid;
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
